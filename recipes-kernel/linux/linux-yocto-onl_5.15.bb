KBRANCH ?= "linux-5.15.y"

require linux-yocto-onl.inc
include cve-exclusion_5.15.inc

KCONF_BSP_AUDIT_LEVEL = "1"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "5.15.174"
# https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-5.15.y
SRCREV_machine ?= "963e654022cc32d72c184b4ab86a76226b3e3b8d"

# Use commit for kver matching (or close to) LINUX_VERSION
# https://git.yoctoproject.org/yocto-kernel-cache/log/kver?h=yocto-5.15
SRCREV_meta ?= "65cd92b42128e83bbe4920be8d58d2f44fa0245a"

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
