inherit kernel
require recipes-kernel/linux/linux-yocto.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto-onl-${KBRANCH}:"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git;protocol=git;branch=${KBRANCH};nocheckout=1;name=machine"

LINUX_VERSION_EXTENSION_append = "-onl"
PV = "${LINUX_VERSION}+git${SRCPV}"

#COMPATIBLE_MACHINE = "(^$)"
