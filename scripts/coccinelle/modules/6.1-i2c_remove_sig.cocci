// SPDX-License-Identifier: GPL-2.0-only
//
// Adds a wrapper for the remove function for 6.1 that returns void to handle
// the changed signature.
//
// Not idempotent!
//
// Copyright (c) 2022 Jonas Gorski, BISDN GmbH

@r@
identifier DRIVER;
identifier remove_fn;
fresh identifier remove_fn_wrap = remove_fn ## "_6_1";
position p;
@@
struct i2c_driver DRIVER@p = {
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
+	.remove = remove_fn_wrap,
+#else
	.remove = remove_fn,
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
identifier r.remove_fn;
identifier r.remove_fn_wrap;
@@
int remove_fn(...) {...}
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
+static void remove_fn_wrap(struct i2c_client *client)
+{
+	remove_fn(client);
+}
+#endif
