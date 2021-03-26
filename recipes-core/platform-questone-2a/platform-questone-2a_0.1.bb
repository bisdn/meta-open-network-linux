SUMMARY = ""
LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-questone-2a-init.service \
    file://platform-questone-2a-init.sh \
"

FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-questone-2a-init.service \
    ${bindir}/platform-questone-2a-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-questone-2a-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-questone-2a-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE_${PN}_append = "platform-questone-2a-init.service"
