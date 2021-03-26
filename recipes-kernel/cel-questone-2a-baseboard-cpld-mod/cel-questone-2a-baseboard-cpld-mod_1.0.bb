LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://questone2a_baseboard_cpld.c;beginline=6;endline=9;md5=7a0d0f37f38de4d6e61a03d518a9e16d"

inherit module

SRC_URI = " \
          file://Makefile \
          file://questone2a_baseboard_cpld.c \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "questone2a_baseboard_cpld"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
