LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://mc24lc64t.c;beginline=6;endline=9;md5=7a0d0f37f38de4d6e61a03d518a9e16d"

inherit module

SRC_URI = " \
          file://Makefile \
          file://mc24lc64t.c \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "mc24lc64t"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
