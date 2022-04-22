SUMMARY = "An hwmon driver for accton as4630_54pe Power Module"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
          file://Makefile \
          file://COPYING \
          file://x86-64-accton-as4630-54pe-psu.c \
          "
S = "${WORKDIR}"

EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

#KERNEL_MODULE_AUTOLOAD += " x86-64-accton-as4630-54pe-psu"
