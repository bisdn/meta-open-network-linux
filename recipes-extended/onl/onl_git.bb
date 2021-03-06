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
           file://questone-2a;subdir=git/packages/platforms/celestica/x86-64 \
           file://onl/0001-file_uds-silence-unused-result-warnings.patch \
           file://onl/0002-platform_manager-do-not-ignore-write-return-value.patch \
           file://onl/0003-file-check-unix-socket-path-is-short-enough.patch \
           file://onl/0004-ym2651y-fix-update-when-MFR_MODEL_OPTION-is-uninmple.patch \
           file://onl/0005-WIP-tools-convert-to-python3.patch \
           file://onl/0006-dynamically-determine-location-of-python3.patch \
           file://onl/0007-onlpm-hardcode-supported-arches.patch \
           file://onl/0008-onlpm-add-an-argument-for-just-printing-the-builddir.patch \
           file://onl/0009-onlpm-allow-overriding-the-dist-codename-from-enviro.patch \
           file://onl/0010-tools-replace-yaml.load-with-yaml.full_load.patch \
           file://bigcode/0001-WIP-convert-to-python3.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://bigcode/0002-dynamically-determine-location-of-python3.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://bigcode/0003-avoid-multiple-global-definitions-for-not_empty.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://infra/0001-WIP-convert-to-python3.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0002-dynamically-determine-location-of-python3.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0003-avoid-multiple-global-definitions-for-__not_empty__.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0004-replace-yaml.load-with-yaml.full_load.patch;patchdir=${SUBMODULE_INFRA} \
           file://accton-as4610/0001-accton-as4610-fix-buffer-overflow-while-accessing-le.patch \
           file://accton-as4610/0002-accton-as4610-fix-value-truncation-when-accessing-le.patch \
           file://accton-as4610/0003-accton-as4610-do-not-try-to-read-out-PSU-values-for-.patch \
           file://accton-as4630-54pe/0001-as4630-54pe-Add-SFP-reset-sysfs-and-fix-Led-drv.patch \
           file://accton-as4630-54pe/0002-Add-rest-lpmode-code-to-sfpi_control-set-get-api.patch \
           file://accton-as4630-54pe/0003-Fix-fan-direction-api.patch \
           file://accton-as4630-54pe/0004-accton-as4630-54pe-silence-error.patch \
           file://accton-as4630-54pe/0005-accton-as4630-54pe-Avoid-undefined-behaviour.patch \
           file://accton-as5835-54x/0001-AS5835-54x-Support-psu_fan_dir-sysfs-for-YM-1401A-PS.patch \
           file://accton-as5835-54x/0002-as5835-rename-psu_serial_numer-psu_serial_number.patch \
           file://accton-as7726-32x/0001-as7726-32x-silence-ignored-return-value-warnings.patch \
           file://cel-questone-2a/0001-questone-2a-fix-ignoring-unused-result-warnings.patch \
           file://cel-questone-2a/0002-cel-questone-2a-delete-unused-global-variables.patch \
           file://delta-ag5648/0001-delta-ag5648-avoid-multiple-definitions-of-mutex-mut.patch \
           file://delta-ag7648/0001-agema-ag7648-fix-buffer-overflow-while-reading-therm.patch \
           file://delta-ag7648/0002-agema-ag7648-don-t-create-random-rx_los-bitmap-value.patch \
"

ONL_PLATFORM_SUPPORT:arm = " \
    arm-accton-as4610-30-r0 \
    arm-accton-as4610-54-r0 \
"

ONL_PLATFORM_SUPPORT:x86-64 = " \
    x86-64-accton-as4630-54pe-r0 \
    x86-64-accton-as5835-54x-r0 \
    x86-64-accton-as7726-32x-r0 \
    x86-64-cel-questone-2a-r0 \
    x86-64-delta-ag5648-r0 \
    x86-64-delta-ag7648-r0 \
"
