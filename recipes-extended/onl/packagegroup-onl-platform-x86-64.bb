SUMMARY = "packagroups for ONL x86-64 platform support"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

# x86-64 platform support
PACKAGES = "\
    ${PN}-accton-as4630-54pe \
    ${PN}-accton-as4630-54te \
    ${PN}-accton-as5835-54x \
    ${PN}-accton-as7726-32x \
    ${PN}-cel-questone-2a \
    ${PN}-delta-ag5648 \
    ${PN}-delta-ag7648 \
"

RDEPENDS:${PN}-accton-as4630-54pe = "\
    kernel-module-optoe \
    kernel-module-x86-64-accton-as4630-54pe-cpld \
    kernel-module-x86-64-accton-as4630-54pe-leds \
    kernel-module-x86-64-accton-as4630-54pe-psu \
    kernel-module-ym2651y \
    libonlp-x86-64-accton-as4630-54pe \
"

RDEPENDS:${PN}-accton-as4630-54te = "\
    kernel-module-optoe \
    kernel-module-x86-64-accton-as4630-54te-cpld \
    kernel-module-x86-64-accton-as4630-54te-leds \
    kernel-module-x86-64-accton-as4630-54te-psu \
    kernel-module-ym2651y \
    libonlp-x86-64-accton-as4630-54te \
"

RDEPENDS:${PN}-accton-as7726-32x = " \
    kernel-module-optoe \
    kernel-module-x86-64-accton-as7726-32x-cpld \
    kernel-module-x86-64-accton-as7726-32x-fan \
    kernel-module-x86-64-accton-as7726-32x-leds \
    kernel-module-x86-64-accton-as7726-32x-psu \
    kernel-module-ym2651y \
    libonlp-x86-64-accton-as7726-32x \
"

RDEPENDS:${PN}-accton-as5835-54x = " \
    kernel-module-optoe \
    kernel-module-x86-64-accton-as5835-54x-cpld \
    kernel-module-x86-64-accton-as5835-54x-fan \
    kernel-module-x86-64-accton-as5835-54x-leds \
    kernel-module-x86-64-accton-as5835-54x-psu \
    kernel-module-ym2651y \
    libonlp-x86-64-accton-as5835-54x \
"

RDEPENDS:${PN}-delta-ag5648 = " \
    kernel-module-dni-ag5648-psu \
    kernel-module-dni-ag5648-sfp \
    kernel-module-dni-emc2305 \
    kernel-module-i2c-cpld \
    libonlp-x86-64-delta-ag5648 \
"

RDEPENDS:${PN}-delta-ag7648 = " \
    kernel-module-x86-64-delta-ag7648-cpld-mux-1 \
    kernel-module-x86-64-delta-ag7648-cpld-mux-2 \
    kernel-module-x86-64-delta-ag7648-i2c-mux-setting \
    libonlp-x86-64-delta-ag7648 \
"

RDEPENDS:${PN}-cel-questone-2a = " \
    ipmitool \
    kernel-module-optoe \
    kernel-module-mc24lc64t \
    kernel-module-questone2a-baseboard-cpld \
    kernel-module-questone2a-switchboard \
    libonlp-x86-64-cel-questone-2a \
"
