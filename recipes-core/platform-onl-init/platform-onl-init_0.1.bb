SUMMARY = "Platform Init service for ONL platforms"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

inherit systemd

# This will actually be provided by the installer, but we need to pretend to
# provide it so Yocto will be happy.
RPROVIDES:${PN} += " u-boot-default-env"

SRC_URI += " \
    file://8v89307_init.sh \
    file://platform-onl-init.service \
    file://platform-onl-init.sh \
    file://platform-arm-accton-as4610-30-r0-init.sh \
    file://platform-arm-accton-as4610-54-r0-init.sh \
    file://platform-x86-64-accton-as4630-54pe-r0-init.sh \
    file://platform-x86-64-accton-as4630-54te-r0-init.sh \
    file://platform-x86-64-accton-as5835-54x-r0-init.sh \
    file://platform-x86-64-accton-as7726-32x-r0-init.sh \
    file://platform-x86-64-cel-questone-2a-r0-init.sh \
    file://platform-x86-64-delta-ag5648-r0-init.sh \
    file://platform-x86-64-delta-ag7648-r0-init.sh \
"

do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-onl-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/*.sh ${D}${bindir}
}

SYSTEMD_SERVICE:${PN}:append = "platform-onl-init.service"
