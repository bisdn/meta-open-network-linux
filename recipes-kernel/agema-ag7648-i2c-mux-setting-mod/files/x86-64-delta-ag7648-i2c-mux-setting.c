// SPDX-License-Identifier: GPL-2.0-or-later
// Driver to instantiate Delta switch i2c/smbus devices.
//
// Copyright (C) 2018 BISDN GmbH
// Author: Tobias Jungel <tobias.jungel@bisdn.de>

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/dmi.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,7,0)
static inline
struct i2c_client *
i2c_new_scanned_device(struct i2c_adapter *adap,
		       struct i2c_board_info *info,
		       unsigned short const *addr_list,
		       int (*probe)(struct i2c_adapter *adap, unsigned short addr))
{
	return i2c_new_probed_device(adap, info, addr_list, probe);
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
static inline
struct i2c_client *
i2c_new_client_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
	return i2c_new_device(adap, info);
}
#endif

static const char *i2c_adapter_names[] = {
	"undefined",
	"SMBus I801 adapter",
	"SMBus iSMT adapter",
	"i2c-%d-mux",
	"i2c-4-mux", // HACK...
};

/* Keep this enum consistent with i2c_adapter_names */
enum i2c_adapter_type {
	I2C_ADAPTER_UNDEF = 0,
	I2C_ADAPTER_SMBUS_I801,
	I2C_ADAPTER_SMBUS_ISMT,
	I2C_ADAPTER_MUX,
	I2C_ADAPTER_CPLD_1,
};

struct i2c_peripheral {
	struct i2c_board_info board_info;
	unsigned short alt_addr;

	enum i2c_adapter_type type;
	enum i2c_adapter_type parent;
	int nr;

	u32 pci_devid;

	struct platform_device *(*init_cb)(void);
	struct platform_device *pd;
	struct i2c_client *client;
};

struct delta_switch {
	/*
	 * Note that we can't mark this pointer as const because
	 * i2c_new_scanned_device() changes passed in I2C board info, so.
	 */
	struct i2c_peripheral *i2c_peripherals;
	unsigned int num_i2c_peripherals;
};

static const struct delta_switch *dta_switch;

static struct i2c_client *
delta_switch_instantiate_i2c_device(struct i2c_adapter *adapter,
				    struct i2c_board_info *info,
				    unsigned short alt_addr)
{
	const unsigned short addr_list[] = { info->addr, I2C_CLIENT_END };
	struct i2c_client *client;

	/*
	 * Add the i2c device. If we can't detect it at the primary
	 * address we scan secondary addresses. In any case the client
	 * structure gets assigned primary address.
	 */
	client = i2c_new_scanned_device(adapter, info, addr_list, NULL);
	if (IS_ERR_OR_NULL(client) && alt_addr) {
		struct i2c_board_info dummy_info = {
			I2C_BOARD_INFO("dummy", info->addr),
		};
		const unsigned short alt_addr_list[] = { alt_addr,
							 I2C_CLIENT_END };
		struct i2c_client *dummy;

		dummy = i2c_new_scanned_device(adapter, &dummy_info,
					       alt_addr_list, NULL);
		if (!IS_ERR_OR_NULL(dummy)) {
			pr_debug("%d-%02x is probed at %02x\n", adapter->nr,
				 info->addr, dummy->addr);
			i2c_unregister_device(dummy);
			client = i2c_new_client_device(adapter, info);
		}
	}

	if (IS_ERR_OR_NULL(client))
		pr_debug("failed to register device %d-%02x\n", adapter->nr,
			 info->addr);
	else
		pr_debug("added i2c device %d-%02x\n", adapter->nr, info->addr);

	return IS_ERR_OR_NULL(client) ? NULL : client;
}

static bool delta_switch_match_adapter_devid(struct device *dev, u32 devid)
{
	struct pci_dev *pdev;

	if (!dev_is_pci(dev))
		return false;

	pdev = to_pci_dev(dev);

	return devid == PCI_DEVID(pdev->bus->number, pdev->devfn);
}

static const char *
delta_switch_get_adapter_name(const struct i2c_peripheral *i2c_dev,
			      struct i2c_adapter *adapter)
{
	struct i2c_adapter *parent;
	static char buf[20];

	if (i2c_dev->type != I2C_ADAPTER_MUX)
		return i2c_adapter_names[i2c_dev->type];

	if (i2c_dev->parent == I2C_ADAPTER_UNDEF)
		return NULL;

	parent = i2c_parent_is_i2c_adapter(adapter);
	if (!parent)
		return NULL;

	if (strncmp(parent->name, i2c_adapter_names[i2c_dev->parent],
		    strlen(i2c_adapter_names[i2c_dev->parent])))
		return NULL;

	snprintf(buf, sizeof(buf), i2c_adapter_names[i2c_dev->type],
		 i2c_adapter_id(parent));

	if (strncmp(parent->name, i2c_adapter_names[i2c_dev->parent],
		    strlen(i2c_adapter_names[i2c_dev->parent])))
		return NULL;

	return buf;
}

static void delta_switch_check_adapter(struct i2c_adapter *adapter)
{
	struct i2c_peripheral *i2c_dev;
	int i;
	const char *adapter_name;
	int all_initialized = 1;

	pr_debug("checking %s id=%u\n", adapter->name, i2c_adapter_id(adapter));

	for (i = 0; i < dta_switch->num_i2c_peripherals; i++) {
		i2c_dev = &dta_switch->i2c_peripherals[i];

		/* Skip devices already created */
		if (i2c_dev->client || i2c_dev->pd)
			continue;

		if (i2c_dev->type == I2C_ADAPTER_MUX &&
		    i2c_dev->nr != i2c_adapter_id(adapter))
			continue;

		adapter_name = delta_switch_get_adapter_name(i2c_dev, adapter);
		if (!adapter_name)
			continue;

		if (strncmp(adapter->name, adapter_name, strlen(adapter_name)))
			continue;

		if (i2c_dev->pci_devid &&
		    !delta_switch_match_adapter_devid(adapter->dev.parent,
						      i2c_dev->pci_devid)) {
			continue;
		}

		if (i2c_dev->init_cb && !i2c_dev->pd) {
			i2c_dev->pd = i2c_dev->init_cb();
		} else {
			i2c_dev->client = delta_switch_instantiate_i2c_device(
				adapter, &i2c_dev->board_info, i2c_dev->alt_addr);
		}
	}

	/* check if all devices are initialized */
	for (i = 0; i < dta_switch->num_i2c_peripherals; i++) {
		i2c_dev = &dta_switch->i2c_peripherals[i];

		if (i2c_dev->client || i2c_dev->pd)
			continue;

		pr_debug("found non initialized device %s\n",
			 i2c_dev->board_info.type);
		all_initialized = 0;
		break;
	}
}

static int delta_switch_i2c_notifier_call(struct notifier_block *nb,
					  unsigned long action, void *data)
{
	struct device *dev = data;

	pr_debug("delta_switch_i2c_notifier_call: action=%lu\n", action);

	switch (action) {
	case BUS_NOTIFY_ADD_DEVICE:
		if (dev->type == &i2c_adapter_type)
			delta_switch_check_adapter(to_i2c_adapter(dev));
		break;
		// TODO add test for client_type
	}

	return 0;
}

static struct notifier_block delta_switch_i2c_notifier = {
	.notifier_call = delta_switch_i2c_notifier_call,
};

#define DECLARE_DELTA_SWITCH(_name)                                            \
	static const struct delta_switch _name __initconst = {                 \
		.i2c_peripherals = _name##_peripherals,                        \
		.num_i2c_peripherals = ARRAY_SIZE(_name##_peripherals),        \
	}

struct i2c_mux_cpld_platform_data {
	u8 cpld_bus; // control bus for mux
	u8 cpld_addr; // control address
	u8 cpld_reg; // control register

	u8 parent_bus; // bus no. of mux

	u8 base_nr; // force bus nr if set otherwise dynamice alloc

	const u8 *values; // channel_ids
	int n_values; // number of channels
	bool idle_in_use; // deselect if enabled
	u8 idle; // value to write in case idle_in_use is set

	void *ctrl_adap;
};

// TODO mark as __initdata and copy like peripherials
static const u8 delta_switch_ag7648_cpld1_values[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
	0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
	0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
};

// TODO mark as __initdata and copy like peripherials
static struct i2c_mux_cpld_platform_data delta_switch_ag7648_cpld1 = {
	.cpld_bus = 0x02,
	.cpld_addr = 0x32,
	.cpld_reg = 0x11,

	.parent_bus = 0x04,

	.values = delta_switch_ag7648_cpld1_values,
	.n_values = 0x30,
	.base_nr = 10,

	.idle_in_use = true,
	.idle = 0x3f,
};

// TODO mark as __initdata and copy like peripherials
static const u8 delta_switch_ag7648_cpld2_values[] = { 0x00, 0x01, 0x02,
						       0x03, 0x04, 0x05 };

// TODO mark as __initdata and copy like peripherials
static struct i2c_mux_cpld_platform_data delta_switch_ag7648_cpld2 = {
	.cpld_bus = 0x02,
	.cpld_addr = 0x32,
	.cpld_reg = 0x0a,

	.parent_bus = 0x05,

	.values = delta_switch_ag7648_cpld2_values,
	.n_values = 0x06,
	.base_nr = 58,
};

static struct platform_device *delta_ag7648_cpld1_init(void)
{
	struct platform_device *pd;

	pd = platform_device_register_data(NULL, "i2c-mux-cpld", 1,
					   &delta_switch_ag7648_cpld1,
					   sizeof(delta_switch_ag7648_cpld1));
	if (IS_ERR(pd))
		return NULL;//PTR_ERR(pd);
	return pd;
}
static struct platform_device * delta_ag7648_cpld2_init(void)
{
	struct platform_device *pd;

	pd = platform_device_register_data(NULL, "i2c-mux-cpld-2", 2,
					   &delta_switch_ag7648_cpld2,
					   sizeof(delta_switch_ag7648_cpld2));
	if (IS_ERR(pd))
		return NULL;//PTR_ERR(pd);
	return pd;
}
static struct i2c_peripheral delta_ag7648_peripherals[] __initdata = {
	/* pca9547. */
	{
		.board_info =
			{
				I2C_BOARD_INFO("pca9547", 0x70),
			},
		.type = I2C_ADAPTER_SMBUS_ISMT,
	},
	/* generic clock source */
	{
		.board_info =
			{
				I2C_BOARD_INFO("clock_gen", 0x69),
			},
		.type = I2C_ADAPTER_SMBUS_I801,
	},
	/* temperature sensor tmp75 */
	{
		.board_info =
			{
				I2C_BOARD_INFO("tmp75", 0x4d),
			},
		.type = I2C_ADAPTER_MUX,
		.parent = I2C_ADAPTER_SMBUS_ISMT,
		.nr = 2,
	},
	/* temperature sensor tmp75 */
	{
		.board_info =
			{
				I2C_BOARD_INFO("tmp75", 0x4c),
			},
		.type = I2C_ADAPTER_MUX,
		.parent = I2C_ADAPTER_SMBUS_ISMT,
		.nr = 3,
	},
	/* temperature sensor tmp75 */
	{
		.board_info =
			{
				I2C_BOARD_INFO("tmp75", 0x4d),
			},
		.type = I2C_ADAPTER_MUX,
		.parent = I2C_ADAPTER_SMBUS_ISMT,
		.nr = 3,
	},
	/* temperature sensor tmp75 */
	{
		.board_info =
			{
				I2C_BOARD_INFO("tmp75", 0x4e),
			},
		.type = I2C_ADAPTER_MUX,
		.parent = I2C_ADAPTER_SMBUS_ISMT,
		.nr = 3,
	},
	/* cpld-1 mux */
	{
		.board_info =
			{
				I2C_BOARD_INFO("cpld1", 0),
			},
		.type = I2C_ADAPTER_MUX,
		.parent = I2C_ADAPTER_SMBUS_ISMT,
		.nr = 9,
		.init_cb = delta_ag7648_cpld1_init,
	},
	/* cpld-2 mux */
	{
		.board_info =
			{
				I2C_BOARD_INFO("cpld2", 0),
			},
		.type = I2C_ADAPTER_CPLD_1, // only after cpld1
		.init_cb = delta_ag7648_cpld2_init,
	},
};
DECLARE_DELTA_SWITCH(delta_ag7648);

static const struct dmi_system_id delta_switch_dmi_table[] __initconst = {
	{
		.ident = "Delta AG7648",
		.matches =
			{
				DMI_MATCH(DMI_SYS_VENDOR, "Delta"),
				DMI_MATCH(DMI_PRODUCT_NAME, "AG7648"),
			},
		.driver_data = (void *)&delta_ag7648,
	},
	{
		.ident = "Delta AG7648 (DNI)",
		.matches =
			{
				DMI_MATCH(DMI_SYS_VENDOR, "DNI"),
				DMI_MATCH(DMI_PRODUCT_NAME, "AG7648"),
			},
		.driver_data = (void *)&delta_ag7648,
	},
	{
		.ident = "Delta AG7648 (OEM)",
		.matches =
			{
				DMI_MATCH(DMI_SYS_VENDOR, "OEM"),
				DMI_MATCH(DMI_PRODUCT_NAME, "AG-7648PL"),
			},
		.driver_data = (void *)&delta_ag7648,
	},
	{}
};
MODULE_DEVICE_TABLE(dmi, delta_switch_dmi_table);

static int __init delta_switch_scan_peripherals(struct device *dev, void *data)
{
	if (dev->type == &i2c_adapter_type) {
		delta_switch_check_adapter(to_i2c_adapter(dev));
	}

	return 0;
}

static int __init delta_switch_prepare_i2c_peripherals(
	struct delta_switch *dta_switch, const struct delta_switch *src)
{
	if (!src->num_i2c_peripherals)
		return 0;

	dta_switch->i2c_peripherals = kmemdup(
		src->i2c_peripherals,
		src->num_i2c_peripherals * sizeof(*src->i2c_peripherals),
		GFP_KERNEL);
	if (!dta_switch->i2c_peripherals)
		return -ENOMEM;

	dta_switch->num_i2c_peripherals = src->num_i2c_peripherals;

	return 0;
}

static void delta_switch_destroy(const struct delta_switch *dta_switch)
{
	struct i2c_peripheral *i2c_dev;
	struct i2c_board_info *info;
	int i;

	for (i = dta_switch->num_i2c_peripherals - 1; i >= 0; i--) {
		i2c_dev = &dta_switch->i2c_peripherals[i];
		info = &i2c_dev->board_info;

		if (i2c_dev->client)
			i2c_unregister_device(i2c_dev->client);
	}

	kfree(dta_switch->i2c_peripherals);
	kfree(dta_switch);
}

static struct delta_switch *__init
delta_switch_prepare(const struct delta_switch *src)
{
	struct delta_switch *dta_switch;
	int error;

	dta_switch = kzalloc(sizeof(*dta_switch), GFP_KERNEL);
	if (!dta_switch)
		return ERR_PTR(-ENOMEM);

	error = delta_switch_prepare_i2c_peripherals(dta_switch, src);

	if (error) {
		delta_switch_destroy(dta_switch);
		return ERR_PTR(error);
	}

	return dta_switch;
}

static int __init delta_switch_init(void)
{
	const struct dmi_system_id *dmi_id;
	int error;

	dmi_id = dmi_first_match(delta_switch_dmi_table);
	if (!dmi_id) {
		pr_debug("unsupported system\n");
		return -ENODEV;
	}

	pr_debug("DMI Matched %s\n", dmi_id->ident);

	dta_switch = delta_switch_prepare((void *)dmi_id->driver_data);
	if (IS_ERR(dta_switch))
		return PTR_ERR(dta_switch);

	if (!dta_switch->num_i2c_peripherals) {
		pr_debug("no relevant devices detected\n");
		error = -ENODEV;
		goto err_destroy_dta_switch;
	}

	error = bus_register_notifier(&i2c_bus_type,
				      &delta_switch_i2c_notifier);
	if (error) {
		pr_err("failed to register i2c bus notifier: %d\n", error);
		goto err_destroy_dta_switch;
	}

	/*
	 * Scan adapters that have been registered and clients that have
	 * been created before we installed the notifier to make sure
	 * we do not miss any devices.
	 */
	i2c_for_each_dev(NULL, delta_switch_scan_peripherals);

	return 0;

err_destroy_dta_switch:
	delta_switch_destroy(dta_switch);
	return error;
}

static void __exit delta_switch_exit(void)
{
	bus_unregister_notifier(&i2c_bus_type, &delta_switch_i2c_notifier);
	delta_switch_destroy(dta_switch);
}

module_init(delta_switch_init);
module_exit(delta_switch_exit);

MODULE_DESCRIPTION("Delta switch driver");
MODULE_AUTHOR("Tobias Jungel <tobias.jungel@bisdn.de>");
MODULE_LICENSE("GPL");
