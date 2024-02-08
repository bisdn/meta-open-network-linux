// SPDX-License-Identifier: GPL-2.0-only
//
// Adds a wrapper for the probe function for 6.3's changed signature.
//
// Not idempotent!
//
// Copyright (c) 2024 Jonas Gorski, BISDN GmbH

@r@
identifier DRIVER;
identifier probe_fn;
fresh identifier probe_fn_wrap = probe_fn ## "_6_3";
position p;
@@
struct i2c_driver DRIVER@p = {
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
+	.probe = probe_fn_wrap,
+#else
	.probe = probe_fn,
+#endif
};
@i depends on r@
@@
#include <linux/version.h>
@depends on r && !i@
@@
#include <...>
+#include <linux/version.h>
@depends on r@
identifier r.probe_fn;
identifier r.probe_fn_wrap;
@@
int probe_fn(...) {...}
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
+static int probe_fn_wrap(struct i2c_client *client)
+{
+	return probe_fn(client, i2c_client_get_device_id(client));
+}
+#endif
