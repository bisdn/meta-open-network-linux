/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 BISDN GmbH
 *
 * Author: Jonas Gorski <jonas.gorski@bisdn.de>
 */

#ifndef __BCM591XX_H
#define __BCM591XX_H

#include <linux/types.h>

#define COUNTER_AUTO			-1

#define MCU_OP_PSE_EN			0x00
#define MCU_OP_PSE_MAP			0x02
#define MCU_OP_PSE_MAP_RESET		0x03
#define MCU_OP_PSE_HIGH_POWER_LIMIT	0x07
#define MCU_OP_PSE_RESET		0x09
#define MCU_OP_PSE_SETUP_PORT		0x0e
#define MCU_OP_PSE_PORT_DETECT_TYPE	0x10
#define MCU_OP_PSE_PORT_DISCONNECT	0x13
#define MCU_OP_PSE_PORT_PWR_LIMIT_TYPE	0x15
#define MCU_OP_PSE_PORT_POWERUP_MANAGE	0x17
#define MCU_OP_PSE_SET_GUARDBAND	0x18
#define MCU_OP_PSE_PORT_POWERUP_MODE	0x1c
#define MCU_OP_PSE_STATUS		0x20
#define MCU_OP_PSE_PORT_STATUS		0x21
#define MCU_OP_PSE_PORT_EXT_CONFIG	0x26
#define MCU_OP_PSE_PORT_MEASUREMENT	0x30

/* message format */
struct pse_msg {
	u8 opcode;
	u8 counter;
	u8 data[9];
	u8 csum;
} __packed;

struct bcm591xx_pse_mcu;
struct device;

struct bcm591xx_ops {
	int (*do_txrx)(struct bcm591xx_pse_mcu *mcu, struct pse_msg *cmd,
		       struct pse_msg *resp);
};

struct bcm591xx_pse_mcu {
	struct device *dev;
	struct mutex mutex;
	u8 tx_counter;

	const struct bcm591xx_ops *ops;
};

extern int bcm591xx_send(struct bcm591xx_pse_mcu *mcu, struct pse_msg *cmd,
			 struct pse_msg *resp, int counter);

#endif /* __BCM5911X_H */
