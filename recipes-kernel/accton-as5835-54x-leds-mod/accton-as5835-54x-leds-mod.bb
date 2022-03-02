SUMMARY = "This module supports the accton cpld that hold the channel select mechanism for other i2c slave devices, such as SFP."
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://x86-64-accton-as5835-54x-leds.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

# FIXME (rl) is KBUILD_MODPOST_WARN needed?
EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

#KERNEL_MODULE_AUTOLOAD += " x86-64-accton-as5835-54x-leds"
