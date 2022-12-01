SUMMARY = "Platform Init service for ONL platforms"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

inherit systemd

SRC_URI += " \
    file://platform-onl-init.service \
    file://platform-onl-init.sh \
    file://platform-x86-64-delta-ag5648-r0-init.sh \
"

do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-onl-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/*.sh ${D}${bindir}
}

SYSTEMD_SERVICE:${PN}:append = "platform-onl-init.service"
