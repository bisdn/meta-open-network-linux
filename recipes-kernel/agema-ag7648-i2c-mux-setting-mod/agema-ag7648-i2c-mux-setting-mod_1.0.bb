SUMMARY = "A hwmon driver for the SMSC EMC2305 fan controller"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://x86-64-delta-ag7648-i2c-mux-setting.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " x86-64-delta-ag7648-i2c-mux-setting"
