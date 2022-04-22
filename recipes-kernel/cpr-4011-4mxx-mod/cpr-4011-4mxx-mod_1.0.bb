SUMMARY = "A hwmon driver for the CPR-4011-4Mxx Redundant Power Module"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://cpr_4011_4mxx.c \
           file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "cpr_4011_4mxx"
