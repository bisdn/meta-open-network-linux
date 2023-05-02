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

#include "bcm591xx.h"

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

#define PSE_4P_PWR_UP_MODE		0
#define PSE_4P_DETECT_MODE		0
#define PSE_4P_PWR_UP_MODE_CL4		1

struct as4610_poe_pse;

#define MCU_CHAN_INVALID	0xffu

struct as4610_poe_pse {
	struct bcm591xx_pse_mcu mcu;
	struct serdev_device *serdev;

	struct completion done;

	struct pse_msg *rx_buf;

	int rx_pos;
	u8 rx_tmp[12];

	int psu_rating;
};

#define to_as4610_poe_pse(d)	container_of(d, struct as4610_poe_pse, mcu)

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

	if (pse->rx_buf)
		memcpy(pse->rx_buf, msg, sizeof(*msg));

	pse->rx_pos = 0;
	complete(&pse->done);

	return need;
}

static int as4610_poe_pse_do_txrx(struct bcm591xx_pse_mcu *mcu,
		                  struct pse_msg *cmd, struct pse_msg *resp)
{
	struct as4610_poe_pse *pse = to_as4610_poe_pse(mcu);
	int ret, timeout;

	reinit_completion(&pse->done);


	pse->rx_buf = resp;
	ret = serdev_device_write(pse->serdev, (void *)cmd, sizeof(*cmd), HZ);
	if (ret < 0) {
		dev_info(&pse->serdev->dev, "failed writing message: %i\n", ret);
		goto out;
	}

	msleep(40);

	timeout = wait_for_completion_timeout(&pse->done, HZ);
	if (!timeout)
		ret = -ETIMEDOUT;
out:
	pse->rx_buf = NULL;
	return ret < 0 ? ret : 0;
}

static int as4610_config_check(struct bcm591xx_pse_mcu *mcu)
{
	struct device_node *np = mcu->dev->of_node, *child = NULL;
	struct pse_msg cmd, resp;
	int ret = 0, num_ports;
	bool persist = false;

	num_ports = of_get_child_count(np);

	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_MAP;
	cmd.data[0] = 0x01;
	ret = bcm591xx_send(mcu, &cmd, NULL, 0);
	if (ret)
		return ret;

	/* check if the port pair config is up to date */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_STATUS;

	ret = bcm591xx_send(mcu, &cmd, &resp, 0);
	if (ret)
		return ret;

	/* port channel configuration needs to be persisted to be effective */
	if (resp.data[1] != num_ports) {
		dev_dbg(mcu->dev, "Unexpected number of ports (expected %i, have %i).\n",
			num_ports, resp.data[1]);
		persist = true;
	} else {
		memset(cmd.data, 0xff, sizeof(cmd.data));

		while ((child = of_get_next_child(np, child)) != NULL) {
			u32 reg, device, pri_chan = 0, alt_chan = 0;
			u8 byte5, byte6;

			of_property_read_u32(child, "reg", &reg);

			of_property_read_u32(child, "brcm,device", &device);
			of_property_read_u32(child, "brcm,primary-channel", &pri_chan);

			if (of_property_read_u32(child, "brcm,alternative-channel", &alt_chan))
				alt_chan = MCU_CHAN_INVALID;

			cmd.opcode = MCU_OP_PSE_PORT_EXT_CONFIG;
			cmd.data[0] = reg;
			ret = bcm591xx_send(mcu, &cmd, &resp, COUNTER_AUTO);
			if (ret)
				return ret;

			/* setup expected values */
			byte5 = (device << 3) | (pri_chan);
			if (alt_chan != MCU_CHAN_INVALID) {
				byte5 |= BIT(7);
				byte6 = alt_chan;
				byte6 |= (PSE_4P_PWR_UP_MODE << 3);
				byte6 |= (PSE_4P_DETECT_MODE << 5);
				byte6 |= (PSE_4P_PWR_UP_MODE_CL4 << 7);
			} else {
				byte6 = pri_chan;
			}

			if (resp.data[5] != byte5 ||
			    resp.data[6] != byte6) {
				dev_dbg(mcu->dev, "Port %i configuration wrong (expected [%02x %02x], have [%02x %02x]).\n",
					reg, byte5, byte6, resp.data[5], resp.data[6]);
				persist = true;
				break;
			}
		}
	}

	if (persist) {
		dev_err(mcu->dev, "Persistent port configuration needs to be updated.\n");

		memset(cmd.data, 0xff, sizeof(cmd.data));
		cmd.opcode = MCU_OP_PSE_MAP;
		cmd.data[0] = 0x02;
		cmd.data[1] = num_ports;

		ret = bcm591xx_send(mcu, &cmd, NULL, 0);
		if (ret)
			return ret;

		while ((child = of_get_next_child(np, child)) != NULL) {
			u32 reg, device, pri_chan = 0, alt_chan = 0;

			of_property_read_u32(child, "reg", &reg);

			of_property_read_u32(child, "brcm,device", &device);
			of_property_read_u32(child, "brcm,primary-channel", &pri_chan);

			if (of_property_read_u32(child, "brcm,alternative-channel", &alt_chan))
				alt_chan = MCU_CHAN_INVALID;

			cmd.opcode = MCU_OP_PSE_SETUP_PORT;
			cmd.data[0] = reg;
			cmd.data[2] = device;
			cmd.data[3] = pri_chan;

			if (alt_chan != MCU_CHAN_INVALID) {
				cmd.data[1] = 1;
				cmd.data[4] = alt_chan;
				cmd.data[5] = PSE_4P_PWR_UP_MODE;
				cmd.data[6] = PSE_4P_DETECT_MODE;
				cmd.data[7] = PSE_4P_PWR_UP_MODE_CL4;
			} else {
				cmd.data[1] = 0;
				cmd.data[4] = pri_chan;
				cmd.data[5] = 0;
				cmd.data[6] = 0;
				cmd.data[7] = 0;
			}

			cmd.data[8] = 0xff;
			ret = bcm591xx_send(mcu, &cmd, NULL, 0);
			if (ret)
				return ret;
		}

		memset(cmd.data, 0xff, sizeof(cmd.data));
		cmd.opcode = MCU_OP_PSE_MAP;
		cmd.data[0] = 0x03;
		cmd.data[1] = num_ports;

		ret = bcm591xx_send(mcu, &cmd, NULL, 0);
		if (ret)
			return ret;
		msleep(1000);
		dev_err(mcu->dev, "Finished updating persistent port configuration.\n");
	} else {
		dev_info(mcu->dev, "Persistent port configuration does not need updating.\n");
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
	ret = bcm591xx_send(&pse->mcu, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	/* ??? */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_SET_GUARDBAND;
	cmd.data[0] = 1;
	put_unaligned_be16(psu_rating * 10, &cmd.data[1]);
	put_unaligned_be16(guard * 10, &cmd.data[3]);
	ret = bcm591xx_send(&pse->mcu, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	/* 2 working PSUs */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_SET_GUARDBAND;
	cmd.data[0] = 2;
	put_unaligned_be16(2 * psu_rating * 10, &cmd.data[1]);
	put_unaligned_be16(guard * 10, &cmd.data[3]);
	ret = bcm591xx_send(&pse->mcu, &cmd, NULL, COUNTER_AUTO);
	if (ret)
		return ret;

	/* Set High Power Device limit */
	memset(cmd.data, 0xff, sizeof(cmd.data));
	cmd.opcode = MCU_OP_PSE_HIGH_POWER_LIMIT;
	cmd.data[0] = 2; /* 31.2W */
	return bcm591xx_send(&pse->mcu, &cmd, NULL, COUNTER_AUTO);
}

static int as4610_poe_pse_init(struct as4610_poe_pse *pse)
{
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


	mutex_lock(&pse->mcu.mutex);

	ret = as4610_poe_set_power_limits(pse, pse->psu_rating, 20);
	if (ret)
		goto out;
out:
	mutex_unlock(&pse->mcu.mutex);

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

static const struct bcm591xx_ops as4610_poe_pse_ops = {
	.config_check = as4610_config_check,
	.do_txrx = as4610_poe_pse_do_txrx,
};

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

	pse->psu_rating = psu_rating;
	pse->serdev = serdev;

	init_completion(&pse->done);

	serdev_device_set_drvdata(serdev, pse);
	serdev_device_set_client_ops(serdev, &a4610_poe_pse_serdev_ops);

	ret = serdev_device_open(serdev);
	if (ret)
		return ret;

	serdev_device_set_rts(serdev, false);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_baudrate(serdev, 19200);

	ret = bcm591xx_init(&pse->mcu, dev, &as4610_poe_pse_ops);
	if (ret) {
		serdev_device_close(pse->serdev);
		return ret;
	}

	as4610_poe_pse_init(pse);

	bcm591xx_debugfs_create(&pse->mcu);

	return 0;
}

static void as4610_poe_pse_remove(struct serdev_device *serdev)
{
	struct as4610_poe_pse *pse = serdev_device_get_drvdata(serdev);
	int reg;

	debugfs_remove_recursive(pse->mcu.debugfs);

	/* disable power to ports */
	reg = as4610_54_cpld_read(AS4610_CPLD_ADDRESS,
				  AS4610_CPLD_POE_CONTROL_REG);
	if (reg >= 0)
		as4610_54_cpld_write(AS4610_CPLD_ADDRESS,
				     AS4610_CPLD_POE_CONTROL_REG,
				     reg & ~POE_CONTROL_EN);
	else
		dev_warn(&pse->serdev->dev, "failed to access CPLD: %i\n", reg);

	bcm591xx_remove(&pse->mcu);

	serdev_device_close(pse->serdev);
}

static const struct of_device_id as4610_poe_pse_of_match[] = {
	{ .compatible = "accton,as4610-poe-pse", },
	{ },
};
MODULE_DEVICE_TABLE(of, as4610_poe_pse_of_match);

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
