LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
	  file://Makefile \
	  file://accton_as4610_poe_mcu.c \
	  file://COPYING \
	  file://poectl \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "accton_as4610_poe_mcu"

EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

FILES_${PN} += "${sbindir}/poectl"

do_install_append() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/poectl ${D}${sbindir}
}

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.