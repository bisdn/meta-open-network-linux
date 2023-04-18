LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
          file://Makefile \
          file://accton_as4610_poe_mcu.c \
          file://accton_as4630_54pe_poe_mcu.c \
          file://bcm591xx.h \
          file://COPYING \
          file://poectl \
          "

S = "${WORKDIR}"

EXTRA_OEMAKE += " KBUILD_MODPOST_WARN=1"

FILES:${PN} += "${sbindir}/poectl"

do_install:append() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/poectl ${D}${sbindir}
}

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
