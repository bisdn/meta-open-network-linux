KBRANCH ?= "linux-5.15.y"

require linux-yocto-onl.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "5.15.3"
#https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-5.15.y
SRCREV_machine ?= "3b17187f5ca1f5d0c641fdc90a6a7e38afdf8fae"
SRCREV_meta ?= "bee5d6a15909f05935f4a61f83a72cddfca7934a"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.15;destsuffix=kernel-meta \
    file://bisdn-kmeta;type=kmeta;name=bisdn-kmeta;destsuffix=bisdn-kmeta \
"

SRC_URI_append_arm = "\
    file://arch/arm/boot/dts;subdir=git \
"

require ${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'recipes-kernel/linux/linux-yocto_virtualization.inc', '', d)}
