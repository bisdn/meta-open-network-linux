LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
    file://accton_as4610_psu.c \
	  file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += " accton_as4610_psu"

EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
