SUMMARY = "CSP 7551 at24 module"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://at24_csp7551.c \
          file://at24.h \
	  file://COPYING \
          "

S = "${WORKDIR}"

#KERNEL_MODULE_AUTOLOAD += " accton-csp7551-at24"
