SUMMARY = ""
LICENSE = "EPLv1.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/EPL-1.0;md5=57f8d5e2b3e98ac6e088986c12bf94e6"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

inherit systemd

SRC_URI += " \
    file://platform-as4630-54pe-init.service \
    file://platform-as4630-54pe-init.sh \
"

# TODO ADD FILES
FILES_${PN} = " \
    ${systemd_system_unitdir}/platform-as4630-54pe-init.service \
    ${bindir}/platform-as4630-54pe-init.sh \
"


do_install() {
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/platform-as4630-54pe-init.service ${D}${systemd_system_unitdir}
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/platform-as4630-54pe-init.sh ${D}${bindir}
}

SYSTEMD_SERVICE_${PN}_append = "platform-as4630-54pe-init.service"
