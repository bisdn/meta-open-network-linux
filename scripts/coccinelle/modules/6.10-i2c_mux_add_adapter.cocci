// SPDX-License-Identifier: GPL-2.0-only
//
// Drop the class argument from i2c_mux_adapter()
//
// Copyright (c) 2025 Jonas Gorski, BISDN GmbH

@r@
expression R,E1,E2,E3,E4;
position p;
@@
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,10,0)
+R = i2c_mux_add_adapter(E1, E2, E3);
+#else
 R = i2c_mux_add_adapter(E1, E2, E3, E4);
+#endif
@i depends on r@
@@
#include <linux/version.h>
@depends on r && !i@
@@
#include <...>
+#include <linux/version.h>
