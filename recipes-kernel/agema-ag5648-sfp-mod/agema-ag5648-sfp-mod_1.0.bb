SUMMARY = "An hwmon driver for delta ag5648 qsfp"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
          file://agema_ag5648_sfp.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " agema_ag5648_sfp"

EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

DEPENDS += "agema-ag5648-i2c-cpld-mod"
