SUMMARY = ""
LICENSE = "CLOSED"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-as4610-init.service \
    file://platform-as4610-init.sh \
    file://90-enp.link \
"

FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-as4610-init.service \
    ${bindir}/platform-as4610-init.sh \
    ${sysconfdir}/systemd/network/90-enp.link \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-as4610-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-as4610-init.sh ${D}${bindir}
        # systemd-networkd
        install -d ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/*.link ${D}${sysconfdir}/systemd/network/
}

SYSTEMD_SERVICE_${PN}_append = "platform-as4610-init.service"
