# yocto-cfg-fragments defaults to 6.6 with a fixed revision, so switch to 6.12
# to match our kernel

LINUX_VERSION = "6.12"
SRCREV = "19b5087399932add6bc893493fe1146fca7d4799"

FILESEXTRAPATHS:prepend := "${THISDIR}/linux-yocto-onl-linux-${LINUX_VERSION}.y:"
# "Revert" 917043019b46 ("virtio: Add prereqs for tiny") from yocto-kernel-cache
# as it forces several subsystems like DRM on, which we do not need since none
# of our devices have a GPU.
SRC_URI += "\
    file://0001-Revert-virtio-Add-prereqs-for-tiny.patch \
"
