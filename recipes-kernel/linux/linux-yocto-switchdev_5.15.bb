KBRANCH ?= "dent-linux-5.15.y"

require linux-yocto-onl.inc

KCONF_BSP_AUDIT_LEVEL = "1"

# Override FILESEXTRAPATHS:prepend set in linux-yocto-onl.inc
FILESEXTRAPATHS:prepend := "${THISDIR}/linux-yocto-switchdev-${KBRANCH}:"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "5.15.11"

# Linux kernel repo
# Override SRC_URI set in linux-yocto-onl.inc
SRC_URI = "git://github.com/dentproject/linux.git;protocol=https;branch=${KBRANCH};nocheckout=1;name=machine"
SRCREV_machine ?= "9daada80dd204d5ae25cc9538a68dba2e4cc9764"

# Use commit for kver matching (or close to) LINUX_VERSION
# https://git.yoctoproject.org/yocto-kernel-cache/log/kver?h=yocto-5.15
SRCREV_meta ?= "eeb5d0c9dd5e2928835c633644426ee357fbce12"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.15;destsuffix=kernel-meta \
    file://bisdn-kmeta;type=kmeta;name=bisdn-kmeta;destsuffix=bisdn-kmeta \
"

# "Revert" 917043019b46 ("virtio: Add prereqs for tiny") from yocto-kernel-cache
# as it forces several subsystems like DRM on, which we do not need since none
# of our devices have a GPU.
# We cannot use patches for that since patches are applied after parsing meta,
# so the next best thing is copying a pre-commit version of the file.
SRC_URI += "\
    file://kernel-meta;type=kmeta;name=kernel-meta;destsuffix=kernel-meta \
"

SRC_URI:append:arm = "\
    file://arch/arm/boot/dts;subdir=git \
"

DEPENDS += "${@bb.utils.contains('ARCH', 'x86', 'elfutils-native', '', d)}"
DEPENDS += "openssl-native util-linux-native"
DEPENDS += "gmp-native libmpc-native"
