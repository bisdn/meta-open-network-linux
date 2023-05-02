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

#define PSE_4P_PWR_UP_MODE		0
#define PSE_4P_DETECT_MODE		0
#define PSE_4P_PWR_UP_MODE_CL4		1

struct as4630_poe_pse;

#define MCU_CHAN_INVALID	0xffu

struct as4630_poe_pse {
	struct bcm591xx_pse_mcu mcu;
	struct i2c_client *client;
	struct dentry *debugfs;

	struct mutex mutex;
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
static int as4630_poe_pse_set_power(struct bcm591xx_pse_mcu *mcu, bool on)
{
	int reg;

	reg = as4630_54pe_cpld_read(AS4630_CPLD_ADDRESS,
				    AS4630_CPLD_POE_CONTROL_REG);
	if (reg < 0)
		return reg;

	if (on)
		reg |= POE_CONTROL_EN;
	else
		reg &= ~POE_CONTROL_EN;


	return as4630_54pe_cpld_write(AS4630_CPLD_ADDRESS,
				      AS4630_CPLD_POE_CONTROL_REG, reg);
}

static const struct bcm591xx_ops as4630_poe_pse_ops = {
	.set_power = as4630_poe_pse_set_power,
	.do_txrx = as4630_poe_pse_do_txrx,
};

static int as4630_poe_pse_probe(struct i2c_client *client, const struct i2c_device_id *dev_id)
{
	int ret, reg, psu_rating;
	struct as4630_poe_pse *pse;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		return -EIO;

	pse = devm_kzalloc(&client->dev, sizeof(*pse), GFP_KERNEL);
	if (!pse)
		return -ENOMEM;

	i2c_set_clientdata(client, pse);

	pse->client = client;

	ret = bcm591xx_init(&pse->mcu, &client->dev, &as4630_poe_pse_ops);
	if (ret)
		return ret;

	bcm591xx_debugfs_create(&pse->mcu);

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

	debugfs_remove_recursive(pse->mcu.debugfs);

	bcm591xx_remove(&pse->mcu);

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
