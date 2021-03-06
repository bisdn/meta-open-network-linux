SUMMARY = ""
LICENSE = "EPL-1.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/EPL-1.0;md5=57f8d5e2b3e98ac6e088986c12bf94e6"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-questone-2a-init.service \
    file://platform-questone-2a-init.sh \
"

FILES:${PN} = " \
    ${systemd_system_unitdir}/platform-questone-2a-init.service \
    ${bindir}/platform-questone-2a-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-questone-2a-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-questone-2a-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE:${PN}:append = "platform-questone-2a-init.service"
