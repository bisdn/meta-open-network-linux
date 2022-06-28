KBRANCH ?= "linux-4.19.y"

require linux-yocto-onl.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

LINUX_VERSION ?= "4.19.152"
# https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-5.15.y
SRCREV_machine ?= "ad326970d25cc85128cd22d62398751ad072efff"

# Use commit for kver matching (or close to) LINUX_VERSION
# https://git.yoctoproject.org/yocto-kernel-cache/log/kver?h=yocto-4.19
SRCREV_meta ?= "06035d5c2b695b3ebf1d99a6027b0e985156bc87"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-4.19;destsuffix=kernel-meta \
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

SRC_URI_append_arm = "\
    file://arch/arm/boot/dts;subdir=git \
"

require ${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'recipes-kernel/linux/linux-yocto_virtualization.inc', '', d)}
