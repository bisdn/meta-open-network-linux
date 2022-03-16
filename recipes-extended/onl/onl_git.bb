# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Open Network Linux Components for Yocto integration"
HOMEPAGE = "https://www.opennetlinux.org/"

require onl.inc

LIC_FILES_CHECKSUM += "file://LICENSE;beginline=14;md5=7e6d108802170df125adb4f452c3c9dd"

SRCREV_onl ?= "dd61cc1792290a459c394c5b7bdf4d4a27b6f651"
URI_ONL ?= "git://github.com/opencomputeproject/OpenNetworkLinux.git;protocol=https;branch=master"

# submodules are checked out individually to support license file checking
SRC_URI += " \
           file://0001-file_uds-silence-unused-result-warnings.patch \
           file://0002-platform_manager-do-not-ignore-write-return-value.patch \
           file://0003-file-check-unix-socket-path-is-short-enough.patch \
           file://0004-agema-ag7648-fix-buffer-overflow-while-reading-therm.patch \
           file://0005-agema-ag7648-don-t-create-random-rx_los-bitmap-value.patch \
           file://0006-accton-as4610-fix-buffer-overflow-while-accessing-le.patch \
           file://0007-accton-as4610-fix-value-truncation-when-accessing-le.patch \
           file://0009-as7726-32x-silence-ignored-return-value-warnings.patch \
           file://questone-2a;subdir=git/packages/platforms/celestica/x86-64 \
           file://0001-questone-2a-fix-ignoring-unused-result-warnings.patch \
           file://0001-accton-as4630-54pe-silence-error.patch \
           file://0001-as4630-54pe-Add-SFP-reset-sysfs-and-fix-Led-drv.patch \
           file://0002-Add-rest-lpmode-code-to-sfpi_control-set-get-api.patch \
           file://0003-Fix-fan-direction-api.patch \
           file://0001-accton-as4630-54pe-Avoid-undefined-behaviour.patch \
           file://0001-AS5835-54x-Support-psu_fan_dir-sysfs-for-YM-1401A-PS.patch \
           file://0002-as5835-rename-psu_serial_numer-psu_serial_number.patch \
           file://0001-ym2651y-fix-update-when-MFR_MODEL_OPTION-is-uninmple.patch \
           file://0001-accton-as4610-do-not-try-to-read-out-PSU-values-for-.patch \
"
