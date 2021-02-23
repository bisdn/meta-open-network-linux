/*
 * AS4610 PoE MCU driver
 *
 * Copyright (C) 2019 BISDN GmbH
 * Jonas Gorski <jonas.gorski@bisdn.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/fs.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/serdev.h>
#include <linux/completion.h>
#include <linux/debugfs.h>

#include <asm/unaligned.h>

/* TODO: replace these with regmap or something more sane */
extern int as4610_54_cpld_read(unsigned short cpld_addr, u8 reg);
extern int as4610_54_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

#define AS4610_CPLD_ADDRESS		0x30

#define AS4610_CPLD_POE_STATUS_REG	0x0c
#define  POE_STATUS_PORTS_MASK		0x3
#define  POE_STATUS_48P			BIT(0)
#define  POE_STATUS_24P			BIT(1)
#define  POE_STATUS_54V_OK		BIT(2)

#define AS4610_CPLD_POE_CONTROL_REG	0x0d
#define  POE_CONTROL_EN			BIT(0)
#define  POE_RESET_MCU			BIT(1)
#define  POE_POWER_BANK_MASK		0xc
#define   POE_POWER_1PSU		0x0
#define   POE_POWER_2PSU		0x4

#define COUNTER_AUTO			-1

#define MCU_OP_PSE_EN			0x00
#define MCU_OP_PSE_MAP			0x02
#define MCU_OP_PSE_MAP_RESET		0x03
#define MCU_OP_PSE_RESET		0x09
#define MCU_OP_PSE_SETUP_PORT		0x0e
#define MCU_OP_PSE_PORT_DETECT_TYPE	0x10
#define MCU_OP_PSE_PORT_DISCONNECT	0x13
#define MCU_OP_PSE_PORT_POWERUP_MODE	0x1c
#define MCU_OP_PSE_PORT_POWERUP_MANAGE	0x17
#define MCU_OP_PSE_SET_GUARDBAND	0x18
#define MCU_OP_PSE_STATUS		0x20
#define MCU_OP_PSE_PORT_STATUS		0x21
#define MCU_OP_PSE_PORT_EXT_CONFIG	0x26
#define MCU_OP_PSE_PORT_MEASUREMENT	0x30

/* Disable PoE by default for now, 4-Pair ports do not work reliably */
#define PSE_EN_DEFAULT			0

#define PSE_4P_PWR_UP_MODE		0
#define PSE_4P_DETECT_MODE		0
#define PSE_4P_PWR_UP_MODE_CL4		1

/* UART message format */
struct pse_msg {
	u8 opcode;
	u8 counter;
	u8 data[9];
	u8 csum;
} __packed;

struct as4610_poe_pse;

#define MCU_CHAN_INVALID	0xffu

struct as4610_poe_port {
	struct as4610_poe_pse *parent;

	u8 port_num;
	u8 device;
	u8 pri_chan;
	u8 alt_chan;

	bool pse_en;

	bool fourp_en;
	u8 fourp_powerup;
	u8 fourp_detection;
	u8 fourp_mode;
};

struct as4610_poe_pse {
	struct serdev_device *serdev;
	struct dentry *debugfs;

	struct mutex mutex;
	struct completion done;

	u8 tx_counter;
	struct pse_msg *rx_buf;

	int rx_pos;
	u8 rx_tmp[12];

	int num_ports;
	int psu_rating;

	struct as4610_poe_port *ports;
};

static int as4610_poe_pse_csum(const void *data)
{
	const u8 *tmp = data;
	int i;
	u8 csum = 0;

	for (i = 0; i < sizeof(struct pse_msg) - 1; i++)
		csum += *tmp++;

	return csum;
}

static int as4610_poe_pse_receive(struct serdev_device *serdev,
				  const unsigned char *data, size_t count)
{
	struct as4610_poe_pse *pse = serdev_device_get_drvdata(serdev);
	int need, want;
	const struct pse_msg *msg;

	need = sizeof(struct pse_msg) - pse->rx_pos;
	want = min_t(int, count, need);

	memcpy(&pse->rx_tmp[pse->rx_pos], data, want);
	pse->rx_pos += want;

	if  (count < need)
		return count;

	msg = (struct pse_msg *)&pse->rx_tmp[0];

	dev_dbg(&serdev->dev, "received message: OP=%02x CNT=%02x DATA=%*ph [csum=%02x %s]\n",
		 msg->opcode, msg->counter, sizeof(msg->data), msg->data, msg->csum,
		 msg->csum == as4610_poe_pse_csum(msg) ? "OK" : "NG");

	if (pse->rx_buf)
		memcpy(pse->rx_buf, msg, sizeof(*msg));

	pse->rx_pos = 0;
	complete(&pse->done);

	return need;
}


static int as4610_poe_pse_send(struct as4610_poe_pse *pse, struct pse_msg *cmd,
			       struct pse_msg *resp, int counter)
{
	int csum, timeout, ret = 0;

	reinit_completion(&pse->done);

	if (counter >= 0) {
		cmd->counter = counter;
	} else {
		cmd->counter = pse->tx_counter++;
		if (pse->tx_counter == 0xfe)
			pse->tx_counter = 0x01;
	}

	cmd->csum = as4610_poe_pse_csum(cmd);

	dev_dbg(&pse->serdev->dev, "sending message: OP=%02x CNT=%02x DATA=%*ph\n",
		 cmd->opcode, cmd->counter, sizeof(cmd->data), cmd->data);

	pse->rx_buf = resp;
	ret = serdev_device_write(pse->serdev, (void *)cmd, sizeof(*cmd), HZ);
	if (ret < 0) {
		dev_info(&pse->serdev->dev, "failed writing message: %i\n", ret);
		goto out;
	}

	msleep(40);

	timeout = wait_for_completion_timeout(&pse->done, HZ);
	if (!timeout) {
		/* MCU_OP_PSE_RESET may not yield a response */
		if (cmd->opcode != MCU_OP_PSE_RESET) {
			dev_err(&pse->serdev->dev, "time out while waiting for response\n");
			ret = -ETIMEDOUT;
		}
		goto out;
	}

	pse->rx_buf = NULL;
	if (resp) {
		csum = resp->csum;
		resp->csum = 0;
		resp->csum = as4610_poe_pse_csum(resp);
		if (resp->csum != csum)
			ret = -EIO;
	}
out:
	return ret < 0 ? ret : 0;
}

static int as4610_poe_pse_enable(struct as4610_poe_pse *pse,
				 struct as4610_poe_port *port, bool enable)
{
	struct pse_msg cmd;
	int ret;

	memset(cmd.data, 0xff, sizeof(cmd.data));

	cmd.opcode = MCU_OP_PSE_EN;
	cmd.data[0] = port->port_num;
	cmd.data[1] = enable;

	mutex_lock(&pse->mutex);
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	mutex_unlock(&pse->mutex);

	if (!ret)
		port->pse_en = enable;
	return ret;
}

static int as4610_poe_pse_init_ports(struct as4610_poe_pse *pse,
				     int p1, int p2, int p3, int p4)
{
	struct pse_msg cmd;
	int ret;

	cmd.opcode = MCU_OP_PSE_EN;
	cmd.data[0] = p1;
	cmd.data[1] = PSE_EN_DEFAULT;
	cmd.data[2] = p2;
	cmd.data[3] = PSE_EN_DEFAULT;
	cmd.data[4] = p3;
	cmd.data[5] = PSE_EN_DEFAULT;
	cmd.data[6] = p4;
	cmd.data[7] = PSE_EN_DEFAULT;
	cmd.data[8] = 0xff;
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	cmd.opcode = MCU_OP_PSE_PORT_DETECT_TYPE;
	cmd.data[0] = p1;
	cmd.data[1] = 2;
	cmd.data[2] = p2;
	cmd.data[3] = 2;
	cmd.data[4] = p3;
	cmd.data[5] = 2;
	cmd.data[6] = p4;
	cmd.data[7] = 2;
	cmd.data[8] = 0xff;
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	cmd.opcode = MCU_OP_PSE_PORT_DISCONNECT;
	cmd.data[0] = p1;
	cmd.data[1] = 2;
	cmd.data[2] = p2;
	cmd.data[3] = 2;
	cmd.data[4] = p3;
	cmd.data[5] = 2;
	cmd.data[6] = p4;
	cmd.data[7] = 2;
	cmd.data[8] = 0xff;
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	cmd.opcode = MCU_OP_PSE_PORT_POWERUP_MODE;
	cmd.data[0] = p1;
	cmd.data[1] = 2;
	cmd.data[2] = p2;
	cmd.data[3] = 2;
	cmd.data[4] = p3;
	cmd.data[5] = 2;
	cmd.data[6] = p4;
	cmd.data[7] = 2;
	cmd.data[8] = 0xff;
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	cmd.opcode = MCU_OP_PSE_PORT_POWERUP_MANAGE;
	cmd.data[0] = p1;
	cmd.data[1] = 4;
	cmd.data[2] = p2;
	cmd.data[3] = 4;
	cmd.data[4] = p3;
	cmd.data[5] = 4;
	cmd.data[6] = p4;
	cmd.data[7] = 4;
	cmd.data[8] = 0xff;
	return as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
}

static int as4610_poe_init_map(struct as4610_poe_pse *pse)
{
	struct pse_msg cmd, resp;
	bool persist = false;
	int i, ret = 0;

	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_MAP;
	cmd.data[0] = 0x01;
	ret = as4610_poe_pse_send(pse, &cmd, NULL, 0);
	if (ret)
		return ret;

	/* check if the port pair config is up to date */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;

	ret = as4610_poe_pse_send(pse, &cmd, &resp, 0);
	if (ret)
		return ret;

	/* port channel configuration needs to be persisted to be effective */
	if (resp.data[1] != pse->num_ports) {
		dev_dbg(&pse->serdev->dev, "Unexpected number of ports (expected %i, have %i).\n",
			pse->num_ports, resp.data[1]);
		persist = true;
	} else {
		memset(cmd.data, 0xff, sizeof(cmd.data));

		for (i = 0; i < pse->num_ports; i++) {
			struct as4610_poe_port *port = &pse->ports[i];
			u8 byte5, byte6;

			cmd.opcode = MCU_OP_PSE_PORT_EXT_CONFIG;
			cmd.data[0] = i;
			ret = as4610_poe_pse_send(pse, &cmd, &resp,
						  COUNTER_AUTO);
			if (ret)
				return ret;

			/* setup expected values */
			byte5 = (port->device << 3) | (port->pri_chan);
			if (port->alt_chan != MCU_CHAN_INVALID) {
				byte5 |= BIT(7);
				byte6 = port->alt_chan;
				byte6 |= (PSE_4P_PWR_UP_MODE << 3);
				byte6 |= (PSE_4P_DETECT_MODE << 5);
				byte6 |= (PSE_4P_PWR_UP_MODE_CL4 << 7);
			} else {
				byte6 = port->pri_chan;
			}

			if (resp.data[5] != byte5 ||
			    resp.data[6] != byte6) {
				dev_dbg(&pse->serdev->dev, "Port %i configuration wrong (expected [%02x %02x], have [%02x %02x]).\n",
					i, byte5, byte6, resp.data[5], resp.data[6]);
				persist = true;
				break;
			}
		}
	}

	if (persist) {
		dev_err(&pse->serdev->dev, "Persistent port configuration needs to be updated.\n");

		memset(cmd.data, 0xff, sizeof(cmd.data));
		cmd.opcode = MCU_OP_PSE_MAP;
		cmd.data[0] = 0x02;
		cmd.data[1] = pse->num_ports;

		ret = as4610_poe_pse_send(pse, &cmd, NULL, 0);
		if (ret)
			return ret;

		for (i = 0; i < pse->num_ports; i++) {
			struct as4610_poe_port *port = &pse->ports[i];

			cmd.opcode = MCU_OP_PSE_SETUP_PORT;
			cmd.data[0] = port->port_num;
			cmd.data[2] = port->device;
			cmd.data[3] = port->pri_chan;

			if (port->alt_chan != MCU_CHAN_INVALID) {
				cmd.data[1] = 1;
				cmd.data[4] = port->alt_chan;
				cmd.data[5] = PSE_4P_PWR_UP_MODE;
				cmd.data[6] = PSE_4P_DETECT_MODE;
				cmd.data[7] = PSE_4P_PWR_UP_MODE_CL4;
			} else {
				cmd.data[1] = 0;
				cmd.data[4] = port->pri_chan;
				cmd.data[5] = 0;
				cmd.data[6] = 0;
				cmd.data[7] = 0;
			}

			cmd.data[8] = 0xff;
			ret = as4610_poe_pse_send(pse, &cmd, NULL, 0);
			if (ret)
				return ret;
		}

		memset(cmd.data, 0xff, sizeof(cmd.data));
		cmd.opcode = MCU_OP_PSE_MAP;
		cmd.data[0] = 0x03;
		cmd.data[1] = pse->num_ports;

		ret = as4610_poe_pse_send(pse, &cmd, NULL, 0);
		if (ret)
			return ret;
		msleep(1000);
		dev_err(&pse->serdev->dev, "Finished updating persistent port configuration.\n");
	} else {
		dev_info(&pse->serdev->dev, "Persistent port configuration does not need updating.\n");
	}

	return 0;
}

static int as4610_poe_set_power_limits(struct as4610_poe_pse *pse,
				       int psu_rating, int guard)
{
	struct pse_msg cmd;
	int ret;

	/* 1 working PSU */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_SET_GUARDBAND;
	cmd.data[0] = 0;
	put_unaligned_be16(psu_rating * 10, &cmd.data[1]);
	put_unaligned_be16(guard * 10, &cmd.data[3]);
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	/* ??? */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_SET_GUARDBAND;
	cmd.data[0] = 1;
	put_unaligned_be16(psu_rating * 10, &cmd.data[1]);
	put_unaligned_be16(guard * 10, &cmd.data[3]);
	ret = as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	/* 2 working PSUs */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_SET_GUARDBAND;
	cmd.data[0] = 2;
	put_unaligned_be16(2 * psu_rating * 10, &cmd.data[1]);
	put_unaligned_be16(guard * 10, &cmd.data[3]);
	return as4610_poe_pse_send(pse, &cmd, NULL, COUNTER_AUTO);
}

static int as4610_poe_pse_init(struct as4610_poe_pse *pse)
{
	struct pse_msg cmd;
	int i, ret = 0, reg;

	reg = as4610_54_cpld_read(AS4610_CPLD_ADDRESS,
				  AS4610_CPLD_POE_CONTROL_REG);
	if (reg < 0)
		return reg;

#if 0
	/* reset MCU to get a clear state */
	/* FIXME: currently hangs the MCU - wrong delay? */
	as4610_54_cpld_write(AS4610_CPLD_ADDRESS,
			     AS4610_CPLD_POE_CONTROL_REG,
			     reg | POE_RESET_MCU);

	msleep(100);

	as4610_54_cpld_write(AS4610_CPLD_ADDRESS,
			     AS4610_CPLD_POE_CONTROL_REG,
			     reg);
	msleep(100);
#endif


	mutex_lock(&pse->mutex);

	ret = as4610_poe_init_map(pse);
	if (ret)
		goto out;

	for (i = 0; i < pse->num_ports; i += 4) {
		struct as4610_poe_port *p1, *p2, *p3, *p4;

		p1 = &pse->ports[i];
		p2 = &pse->ports[i + 1];
		p3 = &pse->ports[i + 2];
		p4 = &pse->ports[i + 3];

		p1->pse_en = PSE_EN_DEFAULT;
		p2->pse_en = PSE_EN_DEFAULT;
		p3->pse_en = PSE_EN_DEFAULT;
		p4->pse_en = PSE_EN_DEFAULT;

		ret = as4610_poe_pse_init_ports(pse, p1->port_num, p2->port_num,
						p3->port_num, p4->port_num);
		if (ret)
			goto out;
	}

	ret = as4610_poe_set_power_limits(pse, pse->psu_rating, 20);
	if (ret)
		goto out;
out:
	mutex_unlock(&pse->mutex);

	/* enable power to ports */
	if (!ret)
		as4610_54_cpld_write(AS4610_CPLD_ADDRESS,
				     AS4610_CPLD_POE_CONTROL_REG,
				     reg | POE_CONTROL_EN);
	return ret;
}

static void as4610_poe_pse_wakeup(struct serdev_device *serdev)
{
	struct as4610_poe_pse *pse = serdev_device_get_drvdata(serdev);

	dev_dbg(&pse->serdev->dev, "as4610_poe_pse_wakeup()\n");
}

static struct serdev_device_ops a4610_poe_pse_serdev_ops = {
	.receive_buf = as4610_poe_pse_receive,
	.write_wakeup = as4610_poe_pse_wakeup,
};

static ssize_t read_file_port_enable(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct as4610_poe_port *port = file->private_data;
	unsigned int len;
	char buf[32];

	len = sprintf(buf, "%d\n", port->pse_en);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_port_enable(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct as4610_poe_port *port = file->private_data;
	unsigned long enable;
	unsigned int len;
	char buf[32];

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &enable))
		return -EINVAL;

	if (enable != 0 && enable != 1)
		return -EINVAL;

	if (port->pse_en != enable)
		as4610_poe_pse_enable(port->parent, port, enable);

	return count;
}

static const struct file_operations pse_en_ops = {
	.read = read_file_port_enable,
	.write = write_file_port_enable,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_port_status(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct as4610_poe_port *port = file->private_data;
	struct as4610_poe_pse *pse = port->parent;
	struct pse_msg cmd, resp;
	unsigned int len;
	char buf[32];
	int ret;

	mutex_lock(&pse->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_PORT_STATUS;
	cmd.data[0] = port->port_num;

	ret = as4610_poe_pse_send(pse, &cmd, &resp, COUNTER_AUTO);
	mutex_unlock(&pse->mutex);
	if (ret)
		return ret;

	len = sprintf(buf, "%*ph\n", sizeof(resp.data), resp.data);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations pse_port_status_ops = {
	.read = read_file_port_status,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_port_measurement(struct file *file, char __user *user_buf,
				          size_t count, loff_t *ppos)
{
	struct as4610_poe_port *port = file->private_data;
	struct as4610_poe_pse *pse = port->parent;
	struct pse_msg cmd, resp;
	char buf[64];
	unsigned int len;
	int ret, voltage, curr, temp, power;

	mutex_lock(&pse->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_PORT_MEASUREMENT;
	cmd.data[0] = port->port_num;

	ret = as4610_poe_pse_send(pse, &cmd, &resp, COUNTER_AUTO);
	mutex_unlock(&pse->mutex);
	if (ret)
		return ret;

	voltage = DIV_ROUND_CLOSEST(get_unaligned_be16(&resp.data[1]) * 6445, 1000);
	curr = get_unaligned_be16(&resp.data[3]);
	temp = ((get_unaligned_be16(&resp.data[5]) - 120) * -125) + 12500;
	power = get_unaligned_be16(&resp.data[7]);

	len = sprintf(buf, "%i.%02iV\n%i.%03iA\n%i.%02iC\n%i.%01iW\n",
		      voltage / 100, voltage % 100,
		      curr / 1000, curr % 1000,
		      temp / 100, temp % 100,
		      power / 10, power % 10);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations pse_measurement_ops = {
	.read = read_file_port_measurement,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_status(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct as4610_poe_pse *pse = file->private_data;
	struct pse_msg cmd, resp;
	unsigned int len;
	char buf[32];
	int ret;

	mutex_lock(&pse->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;

	ret = as4610_poe_pse_send(pse, &cmd, &resp, COUNTER_AUTO);
	mutex_unlock(&pse->mutex);
	if (ret)
		return ret;

	len = sprintf(buf, "%*ph\n", sizeof(resp.data), resp.data);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations pse_status_ops = {
	.read = read_file_status,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static void as4610_poe_pse_debugfs_create(struct as4610_poe_pse *pse)
{
	int i;

	pse->debugfs = debugfs_create_dir(dev_name(&pse->serdev->dev), NULL);
	debugfs_create_file("status", 0200, pse->debugfs, pse, &pse_status_ops);

	for (i = 0; i < pse->num_ports; i++) {
		struct dentry *portdir;
		char dirname[16];

		sprintf(dirname, "port%d", i);

		portdir = debugfs_create_dir(dirname, pse->debugfs);

		if (!portdir)
			return;

		debugfs_create_file("enable", 0600, portdir, &pse->ports[i], &pse_en_ops);
		debugfs_create_file("status", 0200, portdir, &pse->ports[i], &pse_port_status_ops);
		debugfs_create_file("measurement", 0200, portdir, &pse->ports[i], &pse_measurement_ops);
	}
}

static int as4610_poe_pse_probe(struct serdev_device *serdev)
{
	struct device *dev = &serdev->dev;
	struct device_node *np = dev->of_node, *child = NULL;
	int num_ports = 0, ret, reg, psu_rating;
	struct as4610_poe_pse *pse;

	while ((child = of_get_next_child(np, child)) != NULL) {
		u32 reg;

		if (of_property_read_u32(child, "reg", &reg)) {
			dev_err(dev, "port %i has no reg\n", num_ports);
			return -EINVAL;
		}

		if (reg != num_ports) {
			dev_err(dev, "invalid config: port %i has reg %i\n",
				num_ports, reg);
			return -EINVAL;
		}

		num_ports++;
	}
	/* do some sanity checks */
	reg = as4610_54_cpld_read(AS4610_CPLD_ADDRESS,
			          AS4610_CPLD_POE_STATUS_REG);
	if (reg < 0) {
		dev_err(dev, "CPLD not accessible or missing: %i\n", reg);
		return reg;
	}

	if (!(reg & POE_STATUS_54V_OK)) {
		dev_err(dev, "PoE 54V not available!\n");
		return -EINVAL;
	}

	switch (reg & POE_STATUS_PORTS_MASK) {
	case POE_STATUS_48P:
		/* TODO: don't hardcode */
		psu_rating = 750;
		if (num_ports == 48)
			break;
		dev_err(dev, "48P version detected, but %i ports found\n",
			num_ports);
		return -EINVAL;
	case POE_STATUS_24P:
		/* TODO: don't hardcode */
		psu_rating = 400;
		if (num_ports == 24)
			break;
		dev_err(dev, "24P version detected, but %i ports found\n",
			num_ports);
		return -EINVAL;
	default:
		dev_err(dev, "unknown configuration found: %i\n",
			reg & POE_STATUS_PORTS_MASK);
		return -EINVAL;
	}

	pse = devm_kzalloc(dev, sizeof(*pse), GFP_KERNEL);
	if (!pse)
		return -ENOMEM;
	pse->ports = devm_kcalloc(dev, sizeof(pse->ports[0]), num_ports, GFP_KERNEL);
	if (!pse->ports)
		return -ENOMEM;

	pse->num_ports = num_ports;
	pse->psu_rating = psu_rating;

	child = NULL;
	while ((child = of_get_next_child(np, child)) != NULL) {
		struct as4610_poe_port *port;
		u32 reg, device, pri_chan = 0, alt_chan = 0;

		of_property_read_u32(child, "reg", &reg);
		port = &pse->ports[reg];

		of_property_read_u32(child, "brcm,device", &device);
		of_property_read_u32(child, "brcm,primary-channel", &pri_chan);

		port->port_num = reg;
		port->device = device;
		port->pri_chan = pri_chan;

		if (!of_property_read_u32(child, "brcm,alternative-channel", &alt_chan))
			port->alt_chan = alt_chan;
		else
			port->alt_chan = MCU_CHAN_INVALID;

		port->parent = pse;
	}

	pse->serdev = serdev;
	pse->tx_counter = 1;

	mutex_init(&pse->mutex);
	init_completion(&pse->done);

	serdev_device_set_drvdata(serdev, pse);
	serdev_device_set_client_ops(serdev, &a4610_poe_pse_serdev_ops);

	ret = serdev_device_open(serdev);
	if (ret)
		return ret;

	serdev_device_set_rts(serdev, false);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_baudrate(serdev, 19200);

	as4610_poe_pse_init(pse);

	as4610_poe_pse_debugfs_create(pse);

	dev_info(dev, "Found %i port PoE PSE\n", pse->num_ports);

	return 0;
}

static void as4610_poe_pse_remove(struct serdev_device *serdev)
{
	struct as4610_poe_pse *pse = serdev_device_get_drvdata(serdev);
	int reg;

	debugfs_remove_recursive(pse->debugfs);

	/* disable power to ports */
	reg = as4610_54_cpld_read(AS4610_CPLD_ADDRESS,
				  AS4610_CPLD_POE_CONTROL_REG);
	if (reg >= 0)
		as4610_54_cpld_write(AS4610_CPLD_ADDRESS,
				     AS4610_CPLD_POE_CONTROL_REG,
				     reg & ~POE_CONTROL_EN);
	else
		dev_warn(&pse->serdev->dev, "failed to access CPLD: %i\n", reg);

	serdev_device_close(pse->serdev);
}

static const struct of_device_id as4610_poe_pse_of_match[] = {
	{ .compatible = "accton,as4610-poe-pse", }
};

static struct serdev_device_driver as4610_poe_pse_driver = {
	.probe = as4610_poe_pse_probe,
	.remove = as4610_poe_pse_remove,
	.driver = {
		.name = "as4610-poe-pse",
		.of_match_table = of_match_ptr(as4610_poe_pse_of_match),
	}
};
module_serdev_device_driver(as4610_poe_pse_driver);

MODULE_AUTHOR("Jonas Gorski <jonas.gorski@bisdn.de>");
MODULE_DESCRIPTION("AS4610 PoE PSE MCU driver");
MODULE_LICENSE("GPL");
