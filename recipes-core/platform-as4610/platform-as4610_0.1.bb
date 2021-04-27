SUMMARY = ""
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-as4610-init.service \
    file://platform-as4610-init.sh \
    file://90-enp.link \
    file://fw_env.config \
"

FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-as4610-init.service \
    ${bindir}/platform-as4610-init.sh \
    ${sysconfdir}/systemd/network/90-enp.link \
    ${sysconfdir}/fw_env.config \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-as4610-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-as4610-init.sh ${D}${bindir}
        # systemd-networkd
        install -d ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/*.link ${D}${sysconfdir}/systemd/network/
        # uboot env
        install -m 0644 ${WORKDIR}/fw_env.config ${D}/${sysconfdir}/
}

SYSTEMD_SERVICE_${PN}_append = "platform-as4610-init.service"
