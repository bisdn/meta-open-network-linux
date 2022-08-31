# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "dentOS Components for Yocto integration"
HOMEPAGE = "https://dent.dev/"

require onl.inc

LIC_FILES_CHECKSUM += "file://LICENSE;beginline=8;md5=7e6d108802170df125adb4f452c3c9dd"

SRCREV_onl ?= "fa38697b85485ba909b0c9c4f8e5e48bfddbd8fd"
URI_ONL ?= "git://github.com/dentproject/dentOS.git;protocol=https;branch=main"

SRC_URI += " \
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
           file://onl/0015-platforms-remove-CROSS_COMPILE.patch \
           file://bigcode/0001-WIP-convert-to-python3.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://bigcode/0002-dynamically-determine-location-of-python3.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://bigcode/0003-avoid-multiple-global-definitions-for-not_empty.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://infra/0001-WIP-convert-to-python3.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0002-dynamically-determine-location-of-python3.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0003-avoid-multiple-global-definitions-for-__not_empty__.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0004-replace-yaml.load-with-yaml.full_load.patch;patchdir=${SUBMODULE_INFRA} \
           file://delta-tn48m/0001-tn48m-fix-ignoring-unused-results-warnings.patch \
           file://delta-tn48m/0002-tn48m-dn-fix-ignoring-unused-results-warnings.patch \
"
