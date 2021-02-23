SUMMARY = "This module supports cpld that read and write"
LICENSE = "GPLv2" 
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e" 

inherit module

SRC_URI = " \
	  file://Makefile \
          file://agema_ag5648_i2c_cpld.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " agema_ag5648_i2c_cpld"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

PROVIDES_${PN} += "kernel-module-agema_ag5648_i2c_cpld"
