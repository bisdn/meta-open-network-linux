SUMMARY = ""
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MPL-2.0;md5=815ca599c9df247a0c7f619bab123dad"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RPROVIDES:${PN} += " u-boot-default-env"

inherit systemd

SRC_URI += " \
    file://platform-as4610-init.service \
    file://platform-as4610-init.sh \
    file://fw_env.config \
"

FILES:${PN} = " \
    ${systemd_system_unitdir}/platform-as4610-init.service \
    ${bindir}/platform-as4610-init.sh \
    ${sysconfdir}/fw_env.config \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-as4610-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-as4610-init.sh ${D}${bindir}
        # uboot env
        install -d ${D}${sysconfdir}
        install -m 0644 ${WORKDIR}/fw_env.config ${D}/${sysconfdir}/
}

SYSTEMD_SERVICE:${PN}:append = "platform-as4610-init.service"
