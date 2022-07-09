# This recipe uses bin_package to download the dentos Debian package, extract
# both firmware files, and create an ipk package containing them.

DESCRIPTION = "Marvell Prestera Firmware"

# Source Debian package contains LICENCE.Marvell
LICENSE = "CLOSED"

# Version of mrvl-fw-image_*_arm64.deb in
# https://github.com/Marvell-switching/dent-artifacts/
# It may take a long time for the artifacts to turn up in the master branch,
# but the update branches are named predictably and seem to stay long after
# being merged into master.
PV = "3.2.2"

PR = "r0"

SRC_URI = "https://github.com/Marvell-switching/dent-artifacts/raw/update-mrvl-fw-${PV}/REPO/stretch/packages/binary-arm64/mrvl-fw-image_${PV}_arm64.deb;subdir=${BP}"
SRC_URI[sha256sum] = "3b52c8a32fcf399d68e9909d4df8d8ef254ac999fa6cf4e76f76eb54450e11ab"

inherit bin_package

# The firmware is specific to the tn48m
PACKAGE_ARCH = "${MACHINE_ARCH}"
