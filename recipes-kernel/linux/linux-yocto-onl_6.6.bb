KBRANCH ?= "linux-6.6.y"

require linux-yocto-onl.inc
include cve-exclusion_6.6.inc

KCONF_BSP_AUDIT_LEVEL = "1"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "6.6.83"
# https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-6.6.y
SRCREV_machine ?= "594a1dd5138a6bbaa1697e5648cce23d2520eba9"

# Use commit for kver matching (or close to) LINUX_VERSION
# https://git.yoctoproject.org/yocto-kernel-cache/log/kver?h=yocto-6.6
SRCREV_meta ?= "76774218a6ce7d0ac41ab689d68297fd3aa3838e"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-6.6;destsuffix=kernel-meta \
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

DEPENDS += "${@bb.utils.contains('ARCH', 'x86', 'elfutils-native', '', d)}"
DEPENDS += "openssl-native util-linux-native"
DEPENDS += "gmp-native libmpc-native"

# With i2c-i801 and i2c-ismt built as modules there is no guarantee one or the
# other will be probed first and gets bus 0, but platforms use hardcoded paths
# assuming bus 0 is i2c-i801.
# So blacklist i2c-ismt to load it via platform code once i2c-i801 finished
# registering its i2c bus.
KERNEL_MODULE_PROBECONF:append:x86-64 = " i2c-ismt"
module_conf_i2c-ismt = "blacklist i2c-ismt"

# The i2c-i801 reserves resources that may be meant for another driver.
# Blacklist i2c-i801 and only load it as needed.
KERNEL_MODULE_PROBECONF:append:x86-64 = " i2c-i801"
module_conf_i2c-i801 = "blacklist i2c-i801"

# meta-virtualization does not provide a linux-yocto_6.6_virtualization.inc,
# so we need to include it directly.
include ${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'recipes-kernel/linux/linux-yocto_virtualization.inc', '', d)}
