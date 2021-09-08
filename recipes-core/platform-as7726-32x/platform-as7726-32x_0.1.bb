SUMMARY = ""
LICENSE = "EPLv1.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/EPL-1.0;md5=57f8d5e2b3e98ac6e088986c12bf94e6"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://8v89307_init.sh \
    file://platform-as7726-32x-init.service \
    file://platform-as7726-32x-init.sh \
"

FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-as7726-32x-init.service \
    ${bindir}/8v89307_init.sh \
    ${bindir}/platform-as7726-32x-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-as7726-32x-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/8v89307_init.sh ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-as7726-32x-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE_${PN}_append = "platform-as7726-32x-init.service"
