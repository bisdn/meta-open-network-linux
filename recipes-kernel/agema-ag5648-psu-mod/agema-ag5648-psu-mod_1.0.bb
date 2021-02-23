SUMMARY = "An hwmon driver for delta AG9032v1 PSU dps_800ab_16_d.c - Support for DPS-800AB-16 D Power Supply Module"
LICENSE = "GPLv2" 
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e" 

inherit module

SRC_URI = " \
	  file://Makefile \
          file://agema_ag5648_psu.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " agema_ag5648_psu"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
