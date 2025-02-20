// SPDX-License-Identifier: GPL-2.0-only
//
// Adds a wrapper for the remove function for 6.11 that returns void to handle
// the changed signature.
//
// Not idempotent!
//
// Copyright (c) 2025 Jonas Gorski, BISDN GmbH

@r@
identifier DRIVER;
identifier remove_fn;
fresh identifier remove_fn_wrap = remove_fn ## "_6_11";
position p;
@@
struct platform_driver DRIVER@p = {
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,11,0)
+	.remove = remove_fn_wrap,
+#else
	.remove = remove_fn,
+#endif
};

@s@
identifier DRIVER;
identifier remove_fn;
fresh identifier remove_fn_wrap = remove_fn ## "_6_11";
position p;
@@
struct platform_driver DRIVER@p = {
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,11,0)
+	.remove = __exit_p(remove_fn_wrap),
+#else
	.remove = __exit_p(remove_fn),
+#endif
};

@i depends on (r || s)@
@@
#include <linux/version.h>
@depends on (r || s) && !i@
@@
#include <...>
+#include <linux/version.h>
@depends on r@
identifier r.remove_fn;
identifier r.remove_fn_wrap;
@@
int remove_fn(...) {...}
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,11,0)
+static void remove_fn_wrap(struct platform_device *pdev)
+{
+	remove_fn(pdev);
+}
+#endif
@depends on s@
identifier s.remove_fn;
identifier s.remove_fn_wrap;
@@
int remove_fn(...) {...}
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,11,0)
+static void remove_fn_wrap(struct platform_device *pdev)
+{
+	remove_fn(pdev);
+}
+#endif
