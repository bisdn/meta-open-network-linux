SUMMARY = "This module supports the accton cpld that hold the channel select mechanism for other i2c slave devices, such as SFP."
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
          file://Makefile \
          file://COPYING \
          file://x86-64-accton-as4630-54pe-cpld.c \
          "

S = "${WORKDIR}"

#KERNEL_MODULE_AUTOLOAD += "x86-64-accton-as4630-54pe-cpld"
