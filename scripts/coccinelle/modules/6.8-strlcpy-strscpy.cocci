// SPDX-License-Identifier: GPL-2.0-only
//
// Replace strlcpy() which was removed in 6.8 with strscpy()
//
// Copyright (c) 2025 Jonas Gorski, BISDN GmbH

@@
expression E1,E2,E3;
@@
-strlcpy(E1, E2, E3);
+strscpy(E1, E2, E3);
