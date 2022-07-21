#
# Yocto recipe to build the out-of-tree Intel ice driver for the E810.
#

DESCRIPTION = "Intel ice driver for E810 devices"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${S}/../COPYING;md5=a216b4192dc6b777b6f0db560e9a8417"

PR = "r0"

inherit module

SRC_URI =   "https://downloads.sourceforge.net/project/e1000/ice%20stable/${PV}/ice-${PV}.tar.gz"
SRC_URI[md5sum] = "f13f8894329c4be389181f7ff72de58c"

S = "${WORKDIR}/ice-${PV}/src"

# Allow the Intel Makefile to find the kernel headers
EXTRA_OEMAKE='KSRC="${STAGING_KERNEL_BUILDDIR}" KVER="${KERNEL_VERSION}" INSTALL_MOD_PATH="${D}"'

# Add firmware files that are not included by default
FILES:${PN} += "/lib/firmware/updates"

do_install:append() {
    # Delete files we don't want in the package
    rm -rf ${D}/tmp
    rm -rf ${D}/lib/modules/*/extern-symvers/
}
