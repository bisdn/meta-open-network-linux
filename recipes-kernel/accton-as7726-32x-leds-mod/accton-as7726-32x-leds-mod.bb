SUMMARY = "This module supports cpld that read and write"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://x86-64-accton-as7726-32x-leds.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

#KERNEL_MODULE_AUTOLOAD += " x86-64-accton-as7726-32x-leds"
