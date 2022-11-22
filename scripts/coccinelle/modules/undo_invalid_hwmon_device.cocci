// SPDX-License-Identifier: GPL-2.0-only
//
// Undo the naive conversion to hwmon_device_register() that gets rejected by
// newer kernels.
//
// Copyright (c) 2022 Jonas Gorski, BISDN GmbH

@@
expression ret;
expression E1,E2;
@@
-ret = hwmon_device_register_with_info(E1, E2, NULL, NULL, NULL);
+ret = hwmon_device_register(E1);

