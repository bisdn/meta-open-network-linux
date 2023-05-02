/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 BISDN GmbH
 *
 * Author: Jonas Gorski <jonas.gorski@bisdn.de>
 */

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/module.h>

#include <asm/unaligned.h>

#include "bcm591xx.h"

static int bcm591xx_csum(const void *data)
{
	const u8 *tmp = data;
	int i;
	u8 csum = 0;

	for (i = 0; i < sizeof(struct pse_msg) - 1; i++)
		csum += *tmp++;

	return csum;
}

int bcm591xx_send(struct bcm591xx_pse_mcu *mcu, struct pse_msg *cmd,
		  struct pse_msg *resp, int counter)
{
	struct pse_msg tmp;
	int ret;

	if (counter >= 0) {
		cmd->counter = counter;
	} else {
		cmd->counter = mcu->tx_counter++;
		if (mcu->tx_counter == 0xfe)
			mcu->tx_counter = 0x01;
	}

	cmd->csum = bcm591xx_csum(cmd);

	dev_dbg(mcu->dev, "sending message: OP=%02x CNT=%02x DATA=%*ph\n",
		cmd->opcode, cmd->counter, sizeof(cmd->data), cmd->data);

	ret = mcu->ops->do_txrx(mcu, cmd, &tmp);
	if (ret) {
		/* MCU_OP_PSE_RESET may not yield a response */
		if (ret == -ETIMEDOUT && cmd->opcode == MCU_OP_PSE_RESET) {
			dev_dbg(mcu->dev, "timeout while waiting for reset\n");
			return 0;
		}

		dev_err(mcu->dev, "failed to send message: %i\n", ret);
		return ret;
	}

	dev_dbg(mcu->dev, "received message: OP=%02x CNT=%02x DATA=%*ph [csum=%02x %s]\n",
		 tmp.opcode, tmp.counter, sizeof(tmp.data), tmp.data, tmp.csum,
		 tmp.csum == bcm591xx_csum(&tmp) ? "OK" : "NG");

	if (tmp.csum != bcm591xx_csum(&tmp))
		return -EIO;

	if (resp)
		memcpy(resp, &tmp, sizeof(tmp));

	return 0;
}
EXPORT_SYMBOL_GPL(bcm591xx_send);

static int bcm591xx_multi_port_cmd(struct bcm591xx_pse_mcu *mcu, u8 opcode,
				   u8 val, u8 p1, u8 p2, u8 p3, u8 p4)
{
	struct pse_msg cmd;
	int ret;

	memset(cmd.data, 0xff, sizeof(cmd.data));

	cmd.opcode = opcode;

	if (p1 != MCU_PORT_UNSET) {
		cmd.data[0] = p1;
		cmd.data[1] = val;
	}

	if (p2 != MCU_PORT_UNSET) {
		cmd.data[2] = p2;
		cmd.data[3] = val;
	}

	if (p3 != MCU_PORT_UNSET) {
		cmd.data[4] = p3;
		cmd.data[5] = val;
	}

	if (p4 != MCU_PORT_UNSET) {
		cmd.data[6] = p4;
		cmd.data[7] = val;
	}

	mutex_lock(&mcu->mutex);
	ret = bcm591xx_send(mcu, &cmd, NULL, COUNTER_AUTO);
	mutex_unlock(&mcu->mutex);

	return ret;
}

static int bcm591xx_port_cmd(struct bcm591xx_pse_mcu *mcu, u8 opcode, u8 val,
			     u8 port_num)
{
	return bcm591xx_multi_port_cmd(mcu, opcode, val, port_num,
			               MCU_PORT_UNSET, MCU_PORT_UNSET,
				       MCU_PORT_UNSET);
}

static int bcm591xx_port_enable(struct bcm591xx_pse_mcu *mcu,
				struct bcm591xx_port *port, bool enable)
{
	int ret;

	ret = bcm591xx_port_cmd(mcu, MCU_OP_PSE_EN, enable, port->port_num);
	if (!ret)
		port->pse_en = enable;
	return ret;
}

static int bcm591xx_init_ports(struct bcm591xx_pse_mcu *mcu)
{
	int i, ret;

	for (i = 0; i < mcu->num_ports; i += 4) {
		ret = bcm591xx_multi_port_cmd(mcu, MCU_OP_PSE_PORT_DETECT_TYPE,
					      2, i, i + 1, i + 2, i + 3);
		if (ret)
			return ret;
		ret = bcm591xx_multi_port_cmd(mcu, MCU_OP_PSE_PORT_DISCONNECT,
					      2, i, i + 1, i + 2, i + 3);
		if (ret)
			return ret;

		ret = bcm591xx_multi_port_cmd(mcu, MCU_OP_PSE_PORT_POWERUP_MODE,
					      2, i, i + 1, i + 2, i + 3);
		if (ret)
			return ret;

		ret = bcm591xx_multi_port_cmd(mcu, MCU_OP_PSE_PORT_POWERUP_MANAGE,
					      4, i, i + 1, i + 2, i + 3);
		if (ret)
			return ret;

		/* Enable class-based low/high power device classification */
		ret = bcm591xx_multi_port_cmd(mcu, MCU_OP_PSE_PORT_PWR_LIMIT_TYPE,
					      1, i, i + 1, i + 2, i + 3);
		if (ret)
			return ret;
	}

	return 0;
}

static ssize_t read_file_port_enable(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct bcm591xx_port *port = file->private_data;
	unsigned int len;
	char buf[32];

	len = sprintf(buf, "%d\n", port->pse_en);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t write_file_port_enable(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct bcm591xx_port *port = file->private_data;
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
		bcm591xx_port_enable(port->parent, port, enable);

	return count;
}

static const struct file_operations bcm591xx_port_en_ops = {
	.read = read_file_port_enable,
	.write = write_file_port_enable,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_port_status(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct bcm591xx_port *port = file->private_data;
	struct bcm591xx_pse_mcu *mcu = port->parent;
	struct pse_msg cmd, resp;
	unsigned int len;
	char buf[32];
	int ret;

	mutex_lock(&mcu->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_PORT_STATUS;
	cmd.data[0] = port->port_num;

	ret = bcm591xx_send(mcu, &cmd, &resp, COUNTER_AUTO);
	mutex_unlock(&mcu->mutex);
	if (ret)
		return ret;

	len = sprintf(buf, "%*ph\n", sizeof(resp.data), resp.data);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations bcm591xx_port_status_ops = {
	.read = read_file_port_status,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_port_measurement(struct file *file, char __user *user_buf,
				          size_t count, loff_t *ppos)
{
	struct bcm591xx_port *port = file->private_data;
	struct bcm591xx_pse_mcu *mcu = port->parent;
	struct pse_msg cmd, resp;
	char buf[64];
	unsigned int len;
	int ret, voltage, curr, temp, power;

	mutex_lock(&mcu->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_PORT_MEASUREMENT;
	cmd.data[0] = port->port_num;

	ret = bcm591xx_send(mcu, &cmd, &resp, COUNTER_AUTO);
	mutex_unlock(&mcu->mutex);
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

static const struct file_operations bcm591xx_port_measurement_ops = {
	.read = read_file_port_measurement,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static ssize_t read_file_status(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct bcm591xx_pse_mcu *mcu = file->private_data;
	struct pse_msg cmd, resp;
	unsigned int len;
	char buf[32];
	int ret;

	mutex_lock(&mcu->mutex);
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;

	ret = bcm591xx_send(mcu, &cmd, &resp, COUNTER_AUTO);
	mutex_unlock(&mcu->mutex);
	if (ret)
		return ret;

	len = sprintf(buf, "%*ph\n", sizeof(resp.data), resp.data);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations bcm591xx_status_ops = {
	.read = read_file_status,
	.open = simple_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

void bcm591xx_debugfs_create(struct bcm591xx_pse_mcu *mcu)
{
	int i;

	mcu->debugfs = debugfs_create_dir(dev_name(mcu->dev), NULL);
	debugfs_create_file("status", 0200, mcu->debugfs, mcu, &bcm591xx_status_ops);

	for (i = 0; i < mcu->num_ports; i++) {
		struct dentry *portdir;
		char dirname[16];

		sprintf(dirname, "port%d", i);

		portdir = debugfs_create_dir(dirname, mcu->debugfs);

		if (!portdir)
			return;

		debugfs_create_file("enable", 0600, portdir, &mcu->ports[i], &bcm591xx_port_en_ops);
		debugfs_create_file("status", 0200, portdir, &mcu->ports[i], &bcm591xx_port_status_ops);
		debugfs_create_file("measurement", 0200, portdir, &mcu->ports[i], &bcm591xx_port_measurement_ops);
	}
}
EXPORT_SYMBOL_GPL(bcm591xx_debugfs_create);

int bcm591xx_init(struct bcm591xx_pse_mcu *mcu, struct device *dev,
			 const struct bcm591xx_ops *ops)
{
	struct pse_msg cmd, resp;
	int ret, i;

	mcu->dev = dev;
	mcu->tx_counter = 1;
	mcu->ops = ops;
	mutex_init(&mcu->mutex);

	if (mcu->ops->config_check) {
		ret = mcu->ops->config_check(mcu);
		if (ret)
			return ret;
	}

	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;
	ret = bcm591xx_send(mcu, &cmd, &resp, 0);
	if (ret)
		return ret;

	mcu->num_ports = resp.data[1];
	dev_info(mcu->dev, "Found %i port PoE PSE\n", mcu->num_ports);

	mcu->ports = devm_kcalloc(mcu->dev, sizeof(mcu->ports[0]), mcu->num_ports, GFP_KERNEL);
	if (mcu->ports)
		return -ENOMEM;

	for (i = 0; i < mcu->num_ports; i++) {
		mcu->ports[i].port_num = i;
		mcu->ports[i].parent = mcu;
	}

	return bcm591xx_init_ports(mcu);
}
EXPORT_SYMBOL_GPL(bcm591xx_init);

MODULE_AUTHOR("Jonas Gorski <jonas.gorski@bisdn.de>");
MODULE_DESCRIPTION("BCM591XX MCU library functions");
MODULE_LICENSE("GPL");
