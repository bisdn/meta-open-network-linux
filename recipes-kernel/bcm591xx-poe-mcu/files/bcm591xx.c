/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 BISDN GmbH
 *
 * Author: Jonas Gorski <jonas.gorski@bisdn.de>
 */

#include <linux/device.h>
#include <linux/export.h>
#include <linux/module.h>

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

MODULE_AUTHOR("Jonas Gorski <jonas.gorski@bisdn.de>");
MODULE_DESCRIPTION("BCM591XX MCU library functions");
MODULE_LICENSE("GPL");
