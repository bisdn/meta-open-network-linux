# This recipe uses bin_package to download the dentos Debian package, extract
# both firmware files, and create an ipk package containing the the firmware
# file needed by the Marvel AC3x.

DESCRIPTION = "Marvell Prestera Firmware"

# Source Debian package contains LICENCE.Marvell
LICENSE = "CLOSED"

# Version of mrvl-fw-image_*_arm64.deb in appropriate (non-master) branch of
# https://github.com/Marvell-switching/dent-artifacts/
# It may take a long time for the artifacts to turn up in the master branch,
# but the update branches are named predictably and seem to stay long after
# being merged into master.
PV = "3.2.2"

PR = "r0"

SRC_URI = "https://github.com/Marvell-switching/dent-artifacts/raw/update-mrvl-fw-${PV}/REPO/stretch/packages/binary-arm64/mrvl-fw-image_${PV}_arm64.deb;subdir=${BP}"
SRC_URI[sha256sum] = "3b52c8a32fcf399d68e9909d4df8d8ef254ac999fa6cf4e76f76eb54450e11ab"

# https://git.yoctoproject.org/poky/tree/meta/classes/bin_package.bbclass
inherit bin_package

# do_install has been copied from bin_package.bbclass. The only change is the
# deletion of mvsw_prestera_fw_arm64.img (which is part of the upstream Debian
# package but not needed for the AC3x).

# Install the files to ${D}
do_install () {
    # Do it carefully
    [ -d "${S}" ] || exit 1
    if [ -z "$(ls -A ${S})" ]; then
        bbfatal bin_package has nothing to install. Be sure the SRC_URI unpacks into S.
    fi
    rm ${S}${nonarch_base_libdir}/firmware/marvell/mvsw_prestera_fw_arm64.img
    cd ${S}
    tar --no-same-owner --exclude='./patches' --exclude='./.pc' -cpf - . \
        | tar --no-same-owner -xpf - -C ${D}
}

# The firmware is specific to the tn48m
PACKAGE_ARCH = "${MACHINE_ARCH}"
