SUMMARY = ""
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-ag5648-init.service \
    file://platform-ag5648-init.sh \
"

FILES:${PN} = " \
    ${systemd_system_unitdir}/platform-ag5648-init.service \
    ${bindir}/platform-ag5648-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-ag5648-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-ag5648-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE:${PN}:append = "platform-ag5648-init.service"
