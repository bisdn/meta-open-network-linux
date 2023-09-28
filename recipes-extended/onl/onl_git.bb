# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Open Network Linux Components for Yocto integration"
HOMEPAGE = "https://www.opennetlinux.org/"

require onl.inc

LIC_FILES_CHECKSUM += "file://LICENSE;beginline=14;md5=7e6d108802170df125adb4f452c3c9dd"

SRCREV_onl ?= "dd61cc1792290a459c394c5b7bdf4d4a27b6f651"
URI_ONL ?= "git://github.com/opencomputeproject/OpenNetworkLinux.git;protocol=https;branch=master"
# commit date (UTC) of ${SRCREV_onl}
PV = "2022-04-08+git${SRCPV}"

# submodules are checked out individually to support license file checking
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
           file://onl/0011-optoe-allow-compilation-with-linux-5.5-and-newer.patch \
           file://onl/0012-kmodbuild.sh-don-t-treat-undefined-symbols-as-errors.patch \
           file://onl/0013-optoe-add-of-device-match-table.patch \
           file://onl/0014-ym2561y-add-of-device-match-table.patch \
           file://onl/0015-modules-do-not-call-hwmon_device_register_with_info-.patch \
           file://onl/0016-modules-update-i2c_driver-with-6.1-compatibility.patch \
           file://bigcode/0001-WIP-convert-to-python3.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://bigcode/0002-dynamically-determine-location-of-python3.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://bigcode/0003-avoid-multiple-global-definitions-for-not_empty.patch;patchdir=${SUBMODULE_BIGCODE} \
           file://infra/0001-WIP-convert-to-python3.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0002-dynamically-determine-location-of-python3.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0003-avoid-multiple-global-definitions-for-__not_empty__.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0004-replace-yaml.load-with-yaml.full_load.patch;patchdir=${SUBMODULE_INFRA} \
           file://infra/0005-modtool-skip-hidden-directories-when-loading-modules.patch;patchdir=${SUBMODULE_INFRA} \
           file://accton-as4610/0001-accton-as4610-fix-buffer-overflow-while-accessing-le.patch \
           file://accton-as4610/0002-accton-as4610-fix-value-truncation-when-accessing-le.patch \
           file://accton-as4610/0003-accton-as4610-do-not-try-to-read-out-PSU-values-for-.patch \
           file://accton-as4610/0004-accton_as4610_cpld-add-of-device-match-table.patch \
           file://accton-as4610/0005-accton_as4610_psu-add-of-device-match-table.patch \
           file://accton-as4610/0006-accton-as4610-let-CPLD-handle-the-FAN-device.patch \
           file://accton-as4610/0007-accton-as4610-fix-FAN-driver-id-table.patch \
           file://accton-as4610/0008-accton-as4610-let-CPLD-handle-the-LED-device.patch \
           file://accton-as4610/0009-accton-as4610-do-not-call-hwmon_device_register_with.patch \
           file://accton-as4610/0010-accton-as4610-update-i2c_driver-with-6.1-compatibili.patch \
           file://accton-as4630-54pe/0001-as4630-54pe-Add-SFP-reset-sysfs-and-fix-Led-drv.patch \
           file://accton-as4630-54pe/0002-Add-rest-lpmode-code-to-sfpi_control-set-get-api.patch \
           file://accton-as4630-54pe/0003-Fix-fan-direction-api.patch \
           file://accton-as4630-54pe/0004-accton-as4630-54pe-silence-error.patch \
           file://accton-as4630-54pe/0005-accton-as4630-54pe-Avoid-undefined-behaviour.patch \
           file://accton-as4630-54pe/0006-accton-as4630-54pe-update-i2c_driver-with-6.1-compat.patch \
           file://accton-as4630-54te/0001-accton-as4630-54te-silence-unused-result-warning.patch \
           file://accton-as4630-54te/0002-Edgecore-as4630_54te-Update-thermal-threshold.patch \
           file://accton-as4630-54te/0003-Edgecore-as4630_54te-Support-YM-1151F-power-supply.patch \
           file://accton-as4630-54te/0004-accton-as4630-54te-do-not-acccess-PSU-via-pmbus.patch \
           file://accton-as4630-54te/0005-accton-as4610-54te-do-not-call-hwmon_device_register.patch \
           file://accton-as4630-54te/0006-accton-as4630-54te-update-i2c_driver-with-6.1-compat.patch \
           file://accton-as5835-54x/0001-AS5835-54x-Support-psu_fan_dir-sysfs-for-YM-1401A-PS.patch \
           file://accton-as5835-54x/0002-as5835-rename-psu_serial_numer-psu_serial_number.patch \
           file://accton-as5835-54x/0003-accton-as5835-54x-do-not-call-hwmon_device_register_.patch \
           file://accton-as5835-54x/0004-accton-as5835-54x-update-i2c_driver-with-6.1-compati.patch \
           file://accton-as7726-32x/0001-as7726-32x-silence-ignored-return-value-warnings.patch \
           file://accton-as7726-32x/0002-accton-as7726-32x-do-not-call-hwmon_device_register_.patch \
           file://accton-as7726-32x/0003-accton-as7726-32x-update-i2c_driver-with-6.1-compati.patch \
           file://cel-questone-2a/0001-add-celestica-questone-2a.patch \
           file://cel-questone-2a/0002-questone-2a-fix-ignoring-unused-result-warnings.patch \
           file://cel-questone-2a/0003-cel-questone-2a-delete-unused-global-variables.patch \
           file://cel-questone-2a/0004-cel-questone-2a-fix-various-issues-in-questone-2a_sw.patch \
           file://cel-questone-2a/0005-cel-questone-2a-do-not-build-optoe.patch \
           file://cel-questone-2a/0006-cel-questone-2a-update-i2c_drivers-with-6.1-compatib.patch \
           file://delta-ag5648/0001-delta-ag5648-avoid-multiple-definitions-of-mutex-mut.patch \
           file://delta-ag5648/0002-delta-ag5648-update-modules-to-compile-and-work-on-recent-.patch \
           file://delta-ag5648/0003-delta-ag5648-update-i2c_driver-with-6.1-compatibilit.patch \
           file://delta-ag7648/0001-agema-ag7648-fix-buffer-overflow-while-reading-therm.patch \
           file://delta-ag7648/0002-agema-ag7648-don-t-create-random-rx_los-bitmap-value.patch \
           file://delta-ag7648/0003-bump-kernel-to-4.9.patch \
           file://delta-ag7648/0004-x86-64-delta-ag7648-i2c-mux-setting.ko-added-additio.patch \
           file://delta-ag7648/0005-kernel-update-i2c-settings-module.patch \
           file://delta-ag7648/0006-cpld-mux-2-was-off-by-1-to-select-the-correct-qsfp.patch \
           file://delta-ag7648/0007-i2c-mux-setting-mod-add-match-for-newer-coreboot.patch \
           file://delta-ag7648/0008-ag7648-i2c-mux-setting-mod-fix-race-in-CPLD-device-r.patch \
           file://delta-ag7648/0009-ag7648-i2c-mux-setting-mod-force-mux-bus-numbers.patch \
           file://delta-ag7648/0010-x86-64-delta-ag7648-i2c-mux-setting-mod-update-to-ne.patch \
           file://delta-ag7648/0011-delta-ag7648-do-not-reset-QSFP-modules-when-IRQ-is-a.patch \
           file://delta-ag7648/0012-delta-ag7648-make-sure-i2c-i801-is-loaded-before-cre.patch \
           file://delta-ag7648/0013-delta-ag7648-prevent-potential-deadlock-on-probe.patch \
"

FILES:${PN} = " \
  ${bindir}/onlpd \
  ${bindir}/onlpdump \
  ${libdir}/libonlp*.so.1 \
"

# ONIE platforms that should not be built
ONL_PLATFORMS_IGNORE = ""

# delta_ag5648v1 is an alias for delta_ag5648
ONL_PLATFORMS_IGNORE += "x86_64-delta_ag5648v1-r0"
