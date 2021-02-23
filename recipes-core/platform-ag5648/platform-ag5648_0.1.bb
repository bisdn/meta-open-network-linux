SUMMARY = ""
LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-ag5648-init.service \
    file://platform-ag5648-init.sh \
"

FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-ag5648-init.service \
    ${bindir}/platform-ag5648-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-ag5648-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-ag5648-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE_${PN}_append = "platform-ag5648-init.service"
