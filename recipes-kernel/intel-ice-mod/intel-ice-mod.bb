#
# Yocto recipe to build the out-of-tree Intel ice driver for the E810.
#

DESCRIPTION = "Intel ice driver for E810 devices"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/${BP}/COPYING;md5=a216b4192dc6b777b6f0db560e9a8417"
PR = "r0"

inherit module

SRC_URI =   "file://ice-1.6.7.tar.gz"
SRC_URI[md5sum] = "37d46b3cc4c9bef7bd80f2fc36879905"

S = "${WORKDIR}/${BP}/src"
