# platform-as4610 provides the config, so do not ship one
do_install_append_accton-as4610() {
	rm ${D}${sysconfdir}/fw_env.config
}
