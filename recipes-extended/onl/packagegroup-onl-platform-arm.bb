SUMMARY = "packagroups for ONL arm platform support"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit packagegroup

# arm platform support
PACKAGES = "\
    ${PN}-accton-as4610 \
"

RDEPENDS:${PN}-accton-as4610 = "\
    kernel-module-accton-as4610-cpld \
    kernel-module-accton-as4610-fan \
    kernel-module-accton-as4610-leds \
    kernel-module-accton-as4610-psu \
    kernel-module-optoe \
    kernel-module-ym2651y \
    libonlp-arm-accton-as4610-30 \
    libonlp-arm-accton-as4610-54 \
"
