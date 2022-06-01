KBRANCH ?= "linux-5.10.y"

require linux-yocto-onl.inc

# Override FILESEXTRAPATHS_prepend set in linux-yocto-onl.inc
FILESEXTRAPATHS_prepend := "${THISDIR}/linux-yocto-switchdev-${KBRANCH}:"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "5.10.80"

# Linux kernel repo
# Override SRC_URI set in linux-yocto-onl.inc
SRC_URI = "git://github.com/dentproject/linux.git;protocol=git;branch=${KBRANCH};nocheckout=1;name=machine"
SRCREV_machine ?= "f884bb85b8d877d4e0c670403754813a7901705b"

# Use commit for kver matching (or close to) LINUX_VERSION
# https://git.yoctoproject.org/yocto-kernel-cache/log/kver?h=yocto-5.10
SRCREV_meta ?= "1a4cd99824c919ba17dc62935532f3748ef18469"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.10;destsuffix=kernel-meta \
    file://bisdn-kmeta;type=kmeta;name=bisdn-kmeta;destsuffix=bisdn-kmeta \
"

# "Revert" 917043019b46 ("virtio: Add prereqs for tiny") from yocto-kernel-cache
# as it forces several subsystems like DRM on, which we do not need since none
# of our devices have a GPU.
# We cannot use patches for that since patches are applied after parsing meta,
# so the next best thing is copying a pre-commit version of the file.
#SRC_URI += "\
#    file://kernel-meta;type=kmeta;name=kernel-meta;destsuffix=kernel-meta \
#"

SRC_URI_append_arm = "\
    file://arch/arm/boot/dts;subdir=git \
"

require ${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'recipes-kernel/linux/linux-yocto_virtualization.inc', '', d)}
