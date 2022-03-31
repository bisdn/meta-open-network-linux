# This recipe uses bin_package to download the dentos Debian package, extract
# both firmware files, and create an ipk package containing them.

DESCRIPTION = "Marvell Prestera Firmware"

# Source Debian package contains LICENCE.Marvell
LICENSE = "CLOSED"

# Version of mrvl-fw-image_*_arm64.deb in
# https://github.com/dentproject/dent-artifacts/tree/master/REPO/stretch/packages/binary-arm64
PV = "3.1.2"

PR = "r0"

SRC_URI = "https://github.com/dentproject/dent-artifacts/raw/master/REPO/stretch/packages/binary-arm64/mrvl-fw-image_${PV}_arm64.deb;subdir=${BP}"
SRC_URI[sha256sum] = "7a24c0f95750fbbe4e4b45cae116a278b626bf199867ecda587e2e93305af450"

inherit bin_package

# The firmware is specific to the tn48m
PACKAGE_ARCH = "${MACHINE_ARCH}"
