# https://git.yoctoproject.org/poky/tree/meta/classes/kernel.bbclass
inherit kernel
# https://git.yoctoproject.org/poky/tree/meta/recipes-kernel/linux/linux-yocto.inc
require recipes-kernel/linux/linux-yocto.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/linux-yocto-onl-${KBRANCH}:"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git;protocol=git;branch=${KBRANCH};nocheckout=1;name=machine"

LINUX_VERSION_EXTENSION:append = "-onl"
PV = "${LINUX_VERSION}+git${SRCPV}"

EXTRA_OEMAKE += " HOSTCXX="${BUILD_CXX} ${BUILD_CXXFLAGS} ${BUILD_LDFLAGS}""

#COMPATIBLE_MACHINE = "(^$)"
