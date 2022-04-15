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
PV = "3.2.1"

PR = "r0"

SRC_URI = "https://github.com/Marvell-switching/dent-artifacts/raw/update-mrvl-fw-${PV}/REPO/stretch/packages/binary-arm64/mrvl-fw-image_${PV}_arm64.deb;subdir=${BP}"
SRC_URI[sha256sum] = "2959ad8088ee4ef0b8a20820c14ad3e5481bf698a7c348eeb5934307128a1e66"

inherit bin_package

# The firmware is specific to the tn48m
PACKAGE_ARCH = "${MACHINE_ARCH}"
