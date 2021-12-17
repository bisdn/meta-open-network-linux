KBRANCH ?= "linux-5.15.y"

require linux-yocto-onl.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "5.15.14"
#https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-5.15.y
SRCREV_machine ?= "d114b082bef784345bfac1e1d5c17257005284f2"
SRCREV_meta ?= "72e4eafb6b3c999aefc56e1c1b9dfa0c94ae2fbb"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.15;destsuffix=kernel-meta \
    file://bisdn-kmeta;type=kmeta;name=bisdn-kmeta;destsuffix=bisdn-kmeta \
"

SRC_URI_append_arm = "\
    file://arch/arm/boot/dts;subdir=git \
"

require ${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'recipes-kernel/linux/linux-yocto_virtualization.inc', '', d)}
