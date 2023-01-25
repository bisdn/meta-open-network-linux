KBRANCH ?= "linux-6.1.y"

require linux-yocto-onl.inc

KCONF_BSP_AUDIT_LEVEL = "1"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_VERSION ?= "6.1"
# https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/log/?h=linux-6.1.y
SRCREV_machine ?= "830b3c68c1fb1e9176028d02ef86f3cf76aa2476"

# Use commit for kver matching (or close to) LINUX_VERSION
# https://git.yoctoproject.org/yocto-kernel-cache/log/kver?h=yocto-6.1
SRCREV_meta ?= "185bcfcbe480c742247d9117011794c69682914f"

SRC_URI += "\
    git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=master;destsuffix=kernel-meta \
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

# With i2c-i801 and i2c-ismt built as modules there is no guarantee one or the
# other will be probed first and gets bus 0, but platforms use hardcoded paths
# assuming bus 0 is i2c-i801.
# So blacklist i2c-ismt to load it via platform code once i2c-i801 finished
# registering its i2c bus.
KERNEL_MODULE_PROBECONF:append:x86-64 = " i2c-ismt"
module_conf_i2c-ismt = "blacklist i2c-ismt"

# meta-virtualization does not provide a linux-yocto_6.1_virtualization.inc,
# so we need to include it directly.
include ${@bb.utils.contains('DISTRO_FEATURES', 'virtualization', 'recipes-kernel/linux/linux-yocto_virtualization.inc', '', d)}
