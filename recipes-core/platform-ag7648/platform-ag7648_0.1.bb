SUMMARY = ""
LICENSE = "MPL-2.0"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-ag7648-init.service \
    file://platform-ag7648-init.sh \
"

FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-ag7648-init.service \
    ${bindir}/platform-ag7648-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-ag7648-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-ag7648-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE_${PN}_append = "platform-ag7648-init.service"
