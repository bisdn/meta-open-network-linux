LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://questone2a_switchboard.c;beginline=6;endline=11;md5=5c4ec744723e9bc290837205861cbe99"

inherit module

SRC_URI = " \
          file://Makefile \
          file://questone2a_switchboard.c \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "questone2a_switchboard"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
