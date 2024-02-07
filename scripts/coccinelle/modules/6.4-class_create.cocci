// SPDX-License-Identifier: GPL-2.0-only
//
// Update class_create() callers to the reduced arguments changed in 6.4.
//
// Not idempotent!
//
// Copyright (c) 2024 Jonas Gorski, BISDN GmbH

@r@
expression ret;
expression E1,E2;
@@
ret = class_create(E1, E2);
@depends on r@
expression ret;
expression E1,E2;
@@
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
+ret = class_create(E2);
+#else
 ret = class_create(E1, E2);
+#endif
@i depends on r@
@@
#include <linux/version.h>
@depends on r && !i@
@@
 #include <...>
+#include <linux/version.h>
