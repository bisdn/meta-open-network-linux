FILESEXTRAPATHS:prepend := "${THISDIR}/linux-yocto-onl-linux-5.15.y:"

BBCLASSEXTEND = "onl:target"
PN:class-onl = "linux-yocto-onl"

# https://git.yoctoproject.org/linux-yocto/log/?h=v5.15/base
KBRANCH:class-onl = "v5.15/base"
SRCREV_machine:class-onl ?= "${SRCREV_machine:class-devupstream}"

SRC_URI:append:class-onl = " \
    file://bisdn-kmeta;type=kmeta;name=bisdn-kmeta;destsuffix=bisdn-kmeta \
    file://arch/arm/boot/dts;subdir=git \
"

# "Revert" 917043019b46 ("virtio: Add prereqs for tiny") from yocto-kernel-cache
# as it forces several subsystems like DRM on, which we do not need since none
# of our devices have a GPU.
# We cannot use patches for that since patches are applied after parsing meta,
# so the next best thing is copying a pre-commit version of the file.
SRC_URI:append:class-onl = "\
    file://kernel-meta;type=kmeta;name=kernel-meta;destsuffix=kernel-meta \
"

# we need to set the compatible machines here
COMPATIBLE_MACHINE:class-onl = "accton-as4610|accton-as4630-54pe|accton-as5835-54x|accton-as7726-32x|agema-ag5648|agema-ag7648|cel-questone-2a"
