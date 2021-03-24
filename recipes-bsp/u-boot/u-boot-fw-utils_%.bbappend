FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append_accton-as4610 += " \
	file://fw_env.config \
"

UBOOT_CONFIG = "sandbox"
UBOOT_CONFIG[sandbox] = "sandbox_defconfig"

do_install_append_accton-as4610() {
	install -m 0644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/
}
