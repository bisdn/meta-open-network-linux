SUMMARY = "An hwmon driver for accton as5835_54x Power Module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://x86-64-accton-as5835-54x-psu.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

# FIXME (rl) do we need KBUILD_MODPOST_WARN=1?
EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

#KERNEL_MODULE_AUTOLOAD += " x86-64-accton-as5835-54x-psu"
