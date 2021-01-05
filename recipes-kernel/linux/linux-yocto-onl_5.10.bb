KBRANCH ?= "linux-5.10.y"

require linux-yocto-onl.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "5.10.4"
#https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-5.10.y
SRCREV_machine ?= "b1313fe517ca3703119dcc99ef3bbf75ab42bcfb"
SRCREV_meta ?= "30c21625f9989859ade341ebe738664f3efbf122"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.10;destsuffix=kernel-meta \
    file://bisdn-kmeta;type=kmeta;name=bisdn-kmeta;destsuffix=bisdn-kmeta \
"
