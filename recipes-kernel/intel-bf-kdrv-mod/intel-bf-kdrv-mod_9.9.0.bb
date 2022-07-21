#
# Yocto recipe to build the out-of-tree Intel bf_kdrv driver for Tofino.
#

DESCRIPTION = "Intel bf_kdrv driver for Tofino"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"

PR = "r0"

inherit module

SRC_URI = " \
          file://COPYING \
          file://Makefile \
          file://bf_ioctl.h \
          file://bf_kdrv.h \
          file://bf_kdrv.c \
"

S = "${WORKDIR}"

# Allow the Intel Makefile to find the kernel headers
#EXTRA_OEMAKE='KSRC="${STAGING_KERNEL_BUILDDIR}" KVER="${KERNEL_VERSION}" INSTALL_MOD_PATH="${D}"'

# Add firmware files that are not included by default
#FILES_${PN} += "/lib/firmware/updates"

#do_install_append() {
#    # Delete files we don't want in the package
#    rm -rf ${D}/tmp
#    rm -rf ${D}/lib/modules/*/extern-symvers/
#}
