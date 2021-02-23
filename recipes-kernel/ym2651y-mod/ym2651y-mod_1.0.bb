LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
    file://ym2651y.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " ym2651y"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
