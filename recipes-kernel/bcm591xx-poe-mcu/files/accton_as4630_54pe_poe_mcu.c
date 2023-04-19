/*
 * AS4630 PoE MCU driver
 *
 * Copyright (C) 2021 BISDN GmbH
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

#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>

#include <asm/unaligned.h>
#include <linux/version.h>

#include "bcm591xx.h"

/* TODO: replace these with regmap or something more sane */
extern int as4630_54pe_cpld_read(unsigned short cpld_addr, u8 reg);
extern int as4630_54pe_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

#define AS4630_CPLD_ADDRESS		0x60

#define AS4630_CPLD_POE_CONTROL_REG	0x18
#define  POE_CONTROL_EN			BIT(0)
#define  POE_RESET_MCU			BIT(1)
#define  POE_POWER_BANK_MASK		0x1c
#define   POE_POWER_1PSU		0x0
#define   POE_POWER_2PSU		0x2

/* Disable PoE by default for now, 4-Pair ports do not work reliably */
#define PSE_EN_DEFAULT			0

#define PSE_4P_PWR_UP_MODE		0
#define PSE_4P_DETECT_MODE		0
#define PSE_4P_PWR_UP_MODE_CL4		1

struct as4630_poe_pse;

#define MCU_CHAN_INVALID	0xffu

struct as4630_poe_port {
	struct as4630_poe_pse *parent;

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

struct as4630_poe_pse {
	struct bcm591xx_pse_mcu mcu;
	struct i2c_client *client;
	struct dentry *debugfs;

	struct mutex mutex;

	int num_ports;

	struct as4630_poe_port *ports;
};

#define to_as4630_poe_pse(d)    container_of(d, struct as4630_poe_pse, mcu)

static int as4630_poe_pse_do_txrx(struct bcm591xx_pse_mcu *mcu,
				  struct pse_msg *cmd, struct pse_msg *resp)
{
	struct as4630_poe_pse *pse = to_as4630_poe_pse(mcu);
	struct i2c_client *client = pse->client;
	struct pse_msg tmp;
	int ret = 0;

	ret = i2c_smbus_write_i2c_block_data(client, cmd->opcode,
					     sizeof(*cmd) - 1, &cmd->counter);
	if (ret < 0)
		return ret;

	msleep(40);

	ret = i2c_smbus_read_i2c_block_data(client, 0, sizeof(tmp), (u8* )&tmp);
	if (ret < 0)
		return ret;

	if (resp)
		memcpy(resp, &tmp, sizeof(tmp));

	return 0;
}

static int as4630_poe_pse_enable(struct as4630_poe_pse *pse,
				 struct as4630_poe_port *port, bool enable)
{
	struct pse_msg cmd;
	int ret;

	memset(cmd.data, 0xff, sizeof(cmd.data));

	cmd.opcode = MCU_OP_PSE_EN;
	cmd.data[0] = port->port_num;
	cmd.data[1] = enable;

	mutex_lock(&pse->mutex);
	ret = bcm591xx_send(&pse->mcu, &cmd, NULL, COUNTER_AUTO);
	mutex_unlock(&pse->mutex);

	if (!ret)
		port->pse_en = enable;
	return ret;
}

static int as4630_poe_pse_init_ports(struct as4630_poe_pse *pse,
				     int p1, int p2, int p3, int p4)
{
	struct pse_msg cmd;

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

	return bcm591xx_send(&pse->mcu, &cmd, NULL, COUNTER_AUTO);
}

static int as4630_poe_pse_init(struct as4630_poe_pse *pse)
{
	int i, ret = 0, reg;

	reg = as4630_54pe_cpld_read(AS4630_CPLD_ADDRESS,
				    AS4630_CPLD_POE_CONTROL_REG);
	if (reg < 0)
		return reg;

	mutex_lock(&pse->mutex);

	for (i = 0; i < pse->num_ports; i += 4) {
		struct as4630_poe_port *p1, *p2, *p3, *p4;

		p1 = &pse->ports[i];
		p2 = &pse->ports[i + 1];
		p3 = &pse->ports[i + 2];
		p4 = &pse->ports[i + 3];

		p1->parent = pse;
		p2->parent = pse;
		p3->parent = pse;
		p4->parent = pse;

		p1->port_num = i;
		p2->port_num = i + 1;
		p3->port_num = i + 2;
		p4->port_num = i + 3;

		p1->pse_en = PSE_EN_DEFAULT;
		p2->pse_en = PSE_EN_DEFAULT;
		p3->pse_en = PSE_EN_DEFAULT;
		p4->pse_en = PSE_EN_DEFAULT;

		ret = as4630_poe_pse_init_ports(pse, p1->port_num, p2->port_num,
						p3->port_num, p4->port_num);
		if (ret)
			goto out;
	}
out:
	mutex_unlock(&pse->mutex);

	/* enable power to ports */
	if (!ret)
		as4630_54pe_cpld_write(AS4630_CPLD_ADDRESS,
				       AS4630_CPLD_POE_CONTROL_REG,
				       reg | POE_CONTROL_EN);
	return ret;
}

static ssize_t read_file_port_enable(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct as4630_poe_port *port = file->private_data;
	unsigned int len;
	char buf[32];

	len = sprintf(buf, "%d\n", port->pse_en);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_port_enable(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct as4630_poe_port *port = file->private_data;
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
		as4630_poe_pse_enable(port->parent, port, enable);

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
	struct as4630_poe_port *port = file->private_data;
	struct as4630_poe_pse *pse = port->parent;
	struct pse_msg cmd, resp;
	unsigned int len;
	char buf[32];
	int ret;

	mutex_lock(&pse->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_PORT_STATUS;
	cmd.data[0] = port->port_num;

	ret = bcm591xx_send(&pse->mcu, &cmd, &resp, COUNTER_AUTO);
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
	struct as4630_poe_port *port = file->private_data;
	struct as4630_poe_pse *pse = port->parent;
	struct pse_msg cmd, resp;
	char buf[64];
	unsigned int len;
	int ret, voltage, curr, temp, power;

	mutex_lock(&pse->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_PORT_MEASUREMENT;
	cmd.data[0] = port->port_num;

	ret = bcm591xx_send(&pse->mcu, &cmd, &resp, COUNTER_AUTO);
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
	struct as4630_poe_pse *pse = file->private_data;
	struct pse_msg cmd, resp;
	unsigned int len;
	char buf[32];
	int ret;

	mutex_lock(&pse->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;

	ret = bcm591xx_send(&pse->mcu, &cmd, &resp, COUNTER_AUTO);
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

static void as4630_poe_pse_debugfs_create(struct as4630_poe_pse *pse)
{
	int i;

	pse->debugfs = debugfs_create_dir(dev_name(&pse->client->dev), NULL);
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

static const struct bcm591xx_ops as4630_poe_pse_ops = {
	.do_txrx = as4630_poe_pse_do_txrx,
};

static int as4630_poe_pse_probe(struct i2c_client *client, const struct i2c_device_id *dev_id)
{
	int num_ports = 0, ret, reg, psu_rating;
	struct as4630_poe_pse *pse;
	struct pse_msg cmd, resp;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		return -EIO;

	pse = devm_kzalloc(&client->dev, sizeof(*pse), GFP_KERNEL);
	if (!pse)
		return -ENOMEM;

	i2c_set_clientdata(client, pse);
	mutex_init(&pse->mutex);

	pse->client = client;
	pse->mcu.dev = &client->dev;
	pse->mcu.tx_counter = 1;
	pse->mcu.ops = &as4630_poe_pse_ops,

	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;
	ret = bcm591xx_send(&pse->mcu, &cmd, &resp, 0);
	if (ret)
		return ret;

	pse->num_ports = resp.data[1];
	dev_info(&client->dev, "Found %i port PoE PSE\n", pse->num_ports);

	pse->ports = devm_kcalloc(&client->dev, sizeof(pse->ports[0]), pse->num_ports, GFP_KERNEL);
	if (!pse->ports)
		return -ENOMEM;

	as4630_poe_pse_init(pse);

	as4630_poe_pse_debugfs_create(pse);

	return 0;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
static int as4630_poe_pse_probe_6_3(struct i2c_client *client)
{
	return as4630_poe_pse_probe(client, i2c_client_get_device_id(client));
}
#endif

static int as4630_poe_pse_remove(struct i2c_client *client)
{
	struct as4630_poe_pse *pse = i2c_get_clientdata(client);
	int reg;

	debugfs_remove_recursive(pse->debugfs);

	/* disable power to ports */
	reg = as4630_54pe_cpld_read(AS4630_CPLD_ADDRESS,
				    AS4630_CPLD_POE_CONTROL_REG);
	if (reg >= 0)
		as4630_54pe_cpld_write(AS4630_CPLD_ADDRESS,
				       AS4630_CPLD_POE_CONTROL_REG,
				       reg & ~POE_CONTROL_EN);
	else
		dev_warn(&client->dev, "failed to access CPLD: %i\n", reg);

	return 0;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
static void as4630_poe_pse_remove_6_1(struct i2c_client *client)
{
	as4630_poe_pse_remove(client);
}
#endif

static const struct i2c_device_id as4630_54pe_poe_pse_id[] = {
    { "as4630_54pe_poe_mcu" },
    { },
};
MODULE_DEVICE_TABLE(i2c, as4630_54pe_poe_pse_id);

static struct i2c_driver as4630_poe_pse_driver = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
	.probe = as4630_poe_pse_probe_6_3,
#else
	.probe = as4630_poe_pse_probe,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
	.remove = as4630_poe_pse_remove_6_1,
#else
	.remove = as4630_poe_pse_remove,
#endif
	.id_table = as4630_54pe_poe_pse_id,
	.driver = {
		.name = "as4630_54pe_poe_mcu",
		.owner = THIS_MODULE,
	},
};
module_i2c_driver(as4630_poe_pse_driver);

MODULE_AUTHOR("Jonas Gorski <jonas.gorski@bisdn.de>");
MODULE_DESCRIPTION("AS4630 PoE PSE MCU driver");
MODULE_LICENSE("GPL");
