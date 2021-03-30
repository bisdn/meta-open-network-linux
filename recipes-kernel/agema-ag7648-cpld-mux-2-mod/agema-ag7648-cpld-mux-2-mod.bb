SUMMARY = "This module supports cpld that read and write"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://x86-64-delta-ag7648-cpld-mux-2.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

#KERNEL_MODULE_AUTOLOAD += " x86-64-delta-ag7648-cpld-mux-2"
