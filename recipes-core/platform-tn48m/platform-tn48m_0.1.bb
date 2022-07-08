SUMMARY = ""
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RPROVIDES:${PN} += " u-boot-default-env"

inherit systemd

SRC_URI += " \
    file://platform-tn48m-init.service \
    file://platform-tn48m-init.sh \
    file://90-enp.link \
    file://fw_env.config \
    file://10-switchdev-net.rules \
"

FILES:${PN} = " \
    ${systemd_system_unitdir}/platform-tn48m-init.service \
    ${bindir}/platform-tn48m-init.sh \
    ${sysconfdir}/systemd/network/90-enp.link \
    ${sysconfdir}/fw_env.config \
    /etc/udev/rules.d/10-switchdev-net.rules \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-tn48m-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-tn48m-init.sh ${D}${bindir}
        # systemd-networkd
        install -d ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/*.link ${D}${sysconfdir}/systemd/network/
        # uboot env
        install -m 0644 ${WORKDIR}/fw_env.config ${D}/${sysconfdir}/

        # udev interface renaming
        install -d ${D}/etc/udev/rules.d
        install -m 0644 ${WORKDIR}/10-switchdev-net.rules ${D}/etc/udev/rules.d/
}

SYSTEMD_SERVICE:${PN}:append = "platform-tn48m-init.service"
