# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Open Network Linux Components for Yocto integration"
HOMEPAGE = "https://www.opennetlinux.org/"

require onl.inc

LIC_FILES_CHECKSUM += "file://LICENSE;beginline=14;md5=7e6d108802170df125adb4f452c3c9dd"

SRCREV_onl ?= "f97b185a8722b343e9a0b44d786f29bbd936cb6f"
URI_ONL ?= "git://github.com/opencomputeproject/OpenNetworkLinux.git;protocol=https;branch=master"
# commit date (UTC) of ${SRCREV_onl}
PV = "2023-09-13+git${SRCPV}"

# submodules are checked out individually to support license file checking
SRC_URI += " \
           file://onl/0001-platform_manager-do-not-ignore-write-return-value.patch \
           file://onl/0002-file-check-unix-socket-path-is-short-enough.patch \
           file://onl/0003-ym2651y-fix-update-when-MFR_MODEL_OPTION-is-uninmple.patch \
           file://onl/0004-WIP-tools-convert-to-python3.patch \
           file://onl/0005-dynamically-determine-location-of-python3.patch \
           file://onl/0006-onlpm-hardcode-supported-arches.patch \
           file://onl/0007-onlpm-add-an-argument-for-just-printing-the-builddir.patch \
           file://onl/0008-onlpm-allow-overriding-the-dist-codename-from-enviro.patch \
           file://onl/0009-tools-replace-yaml.load-with-yaml.full_load.patch \
           file://onl/0010-optoe-allow-compilation-with-linux-5.5-and-newer.patch \
           file://onl/0011-kmodbuild.sh-don-t-treat-undefined-symbols-as-errors.patch \
           file://onl/0012-optoe-add-of-device-match-table.patch \
           file://onl/0013-ym2561y-add-of-device-match-table.patch \
           file://onl/0014-modules-do-not-call-hwmon_device_register_with_info-.patch \
           file://onl/0015-modules-update-i2c_drivers-with-6.1.patch \
           file://onl/0016-optoe-fix-race-in-sysfs-registraton-on-probe.patch \
           file://onl/0017-modules-update-i2c_drivers-with-6.3-compatibility.patch \
           file://onl/0018-modules-update-class_create-usage-for-6.4.patch \
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
           file://accton-as4610/0003-accton-as4610-do-not-enable-v_out-i_out-p_out-caps.patch \
           file://accton-as4610/0004-accton_as4610_cpld-add-of-device-match-table.patch \
           file://accton-as4610/0005-accton_as4610_psu-add-of-device-match-table.patch \
           file://accton-as4610/0006-accton-as4610-let-CPLD-handle-the-FAN-device.patch \
           file://accton-as4610/0007-accton-as4610-fix-FAN-driver-id-table.patch \
           file://accton-as4610/0008-accton-as4610-let-CPLD-handle-the-LED-device.patch \
           file://accton-as4630-54pe/0001-accton-as4630-54pe-Avoid-undefined-behaviour.patch \
           file://accton-as4630-54te/0001-accton-as4630-54te-do-not-acccess-PSU-via-pmbus.patch \
           file://accton-as5835-54x/0001-Support-psu_fan_dir-sysfs-for-YM-1401A-PSU.patch \
           file://accton-as5835-54x/0002-as5835-rename-psu_serial_numer-psu_serial_number.patch \
           file://cel-questone-2a/0001-add-celestica-questone-2a.patch \
           file://cel-questone-2a/0002-questone-2a-fix-ignoring-unused-result-warnings.patch \
           file://cel-questone-2a/0003-cel-questone-2a-delete-unused-global-variables.patch \
           file://cel-questone-2a/0004-cel-questone-2a-fix-various-issues-in-questone-2a_sw.patch \
           file://cel-questone-2a/0005-cel-questone-2a-do-not-build-optoe.patch \
           file://cel-questone-2a/0006-cel-questone-2a-update-i2c_drivers-with-6.1-compatib.patch \
           file://cel-questone-2a/0007-cel-questone-2a-update-i2c_drivers-with-6.3-compatib.patch \
           file://cel-questone-2a/0008-cel-questone-2a-update-class_create-usage-for-6.4.patch \
           file://delta-ag5648/0001-delta-ag5648-avoid-multiple-definitions-of-mutex-mut.patch \
           file://delta-ag5648/0002-ag5648-update-modules-to-compile-and-work-on-recent-.patch \
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

# Build additional platform support:
# - "unsupported" -  buildable, but untested
# - "broken" - does not build (mostly as "known broken" list)
PACKAGECONFIG ??= ""

ONL_PLATFORMS_BUILD = " \
    ${ONL_PLATFORMS_SUPPORTED} \
    ${@bb.utils.contains('PACKAGECONFIG', 'unsupported', '${ONL_PLATFORMS_UNSUPPORTED}', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'broken', '${ONL_PLATFORMS_BROKEN}', '', d)} \
"

ONL_MODULE_VENDORS_BUILD = " \
    ${ONL_MODULE_VENDORS_SUPPORTED} \
    ${@bb.utils.contains('PACKAGECONFIG', 'unsupported', '${ONL_MODULE_VENDORS_UNSUPPORTED}', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'broken', '${ONL_MODULE_VENDORS_BROKEN}', '', d)} \
"

# platforms that build and are tested and verified
ONL_PLATFORMS_SUPPORTED:arm = " \
    arm-accton-as4610-30-r0 \
    arm-accton-as4610-54-r0 \
"

ONL_PLATFORMS_SUPPORTED:x86-64 = " \
    x86-64-accton-as4630-54pe-r0 \
    x86-64-accton-as4630-54te-r0 \
    x86-64-accton-as5835-54x-r0 \
    x86-64-accton-as7726-32x-r0 \
    x86-64-delta-ag5648-r0 \
    x86-64-delta-ag7648-r0 \
    x86-64-cel-questone-2a-r0 \
"

ONL_MODULE_VENDORS_SUPPORTED:arm = " \
"

# delta-ag5648's i2c-cpld is part of delta's common vendor modules
ONL_MODULE_VENDORS_SUPPORTED:x86-64 = " \
    delta \
"

# platforms that do build, but are unsupported
ONL_PLATFORMS_UNSUPPORTED:arm = " \
"

ONL_PLATFORMS_UNSUPPORTED:x86-64 = " \
    x86-64-accton-as4222-28pe-r0 \
    x86-64-accton-as5512-54x-r0 \
    x86-64-accton-as5712-54x-r0 \
    x86-64-accton-as5812-54t-r0 \
    x86-64-accton-as5812-54x-r0 \
    x86-64-accton-as5822-54x-r0 \
    x86-64-accton-as5835-54t-r0 \
    x86-64-accton-as5912-54x-r0 \
    x86-64-accton-as5912-54xk-r0 \
    x86-64-accton-as5915-18x-r0 \
    x86-64-accton-as5916-54x-r1 \
    x86-64-accton-as5916-54xk-r1 \
    x86-64-accton-as5916-54xm-r0 \
    x86-64-accton-as7312-54x-r0 \
    x86-64-accton-as7312-54xs-r0 \
    x86-64-accton-as7315-30x-r0 \
    x86-64-accton-as7512-32x-r0 \
    x86-64-accton-as7716-24sc-r0 \
    x86-64-accton-as7716-24xc-r0 \
    x86-64-accton-as7816-64x-r0 \
    x86-64-accton-as7926-40xfb-r0 \
    x86-64-accton-as7946-74xkb-r0 \
    x86-64-accton-as9926-24d-r0 \
    x86-64-accton-asxvolt16-r0 \
    x86-64-accton-csp9250-r0 \
    x86-64-alphanetworks-snj60d0-320f-r0 \
    x86-64-alphanetworks-snj61d0-320f-r2 \
"

ONL_MODULE_VENDORS_UNSUPPORTED:arm = " \
"

ONL_MODULE_VENDORS_UNSUPPORTED:x86-64 = " \
    ingrasys \
    inventec \
"

# platforms that do not build for various reasons, e.g:
# * they have glaring code issues gcc treats as errors
# * their kernel modules use outdated kernel APIs
# * they do not have kernel modules (currently not handled)
# * they do not have a libonlp library (currently not handled)
# * they require an older "debian" release
ONL_PLATFORMS_BROKEN:arm = " \
    arm-delta-ag6248c-poe-r0 \
    arm-delta-ag6248c-r0 \
    arm-qemu-armv7a-r0 \
"

ONL_PLATFORMS_BROKEN:x86-64 = " \
    x86-64-accton-as5916-26xb-r0 \
    x86-64-accton-as5916-54xks-r0 \
    x86-64-accton-as5916-54xl-r0 \
    x86-64-accton-as6712-32x-r0 \
    x86-64-accton-as6812-32x-r0 \
    x86-64-accton-as7112-54x-r0 \
    x86-64-accton-as7315-27xb-r0 \
    x86-64-accton-as7316-26xb-r0 \
    x86-64-accton-as7326-56x-r0 \
    x86-64-accton-as7535-28xb-r0 \
    x86-64-accton-as7712-32x-r0 \
    x86-64-accton-as7716-32x-r0 \
    x86-64-accton-as7926-40xke-r0 \
    x86-64-accton-as7926-80xk-r0 \
    x86-64-accton-as7936-22xke-r0 \
    x86-64-accton-as9516-32d-r0 \
    x86-64-accton-as9716-32d-r0 \
    x86-64-accton-as9926-24db-r0 \
    x86-64-accton-asgvolt64-r0 \
    x86-64-accton-es7632bt3-r0 \
    x86-64-accton-minipack-r0 \
    x86-64-accton-wedge-16x-r0 \
    x86-64-accton-wedge100-r0 \
    x86-64-accton-wedge100-32x-r0 \
    x86-64-accton-wedge100bf-32x-r0 \
    x86-64-accton-wedge100bf-65x-r0 \
    x86-64-accton-wedge100s-32x-r0 \
    x86-64-alphanetworks-scg60d0-484t-r1 \
    x86-64-alphanetworks-snx60a0-486f-r0 \
    x86-64-alphanetworks-stx60d0-062f-r0 \
    x86-64-alphanetworks-stx60d0-126f-r0 \
    x86-64-cel-belgite-r0 \
    x86-64-cel-redstone-xp-r0 \
    x86-64-cel-seastone-r0 \
    x86-64-cel-silverstone-r0 \
    x86-64-delta-ag7648c-r0 \
    x86-64-delta-ag8032-r0 \
    x86-64-delta-ag9032v1-r0 \
    x86-64-delta-ag9032v2a-r0 \
    x86-64-delta-ag9032v2-r0 \
    x86-64-delta-ag9064-r0 \
    x86-64-delta-agc032a-r0 \
    x86-64-delta-agc032-r0 \
    x86-64-delta-agc5648s-r0 \
    x86-64-delta-agc7008s-r0 \
    x86-64-delta-agc7646slv1b-r0 \
    x86-64-delta-agc7646v1-r0 \
    x86-64-delta-agc7648a-r0 \
    x86-64-delta-agc7648sv1-r0 \
    x86-64-delta-agv424-r0 \
    x86-64-delta-agv848v1-r0 \
    x86-64-delta-ak7448-r0 \
    x86-64-delta-wb2448-r0 \
    x86-64-facebook-wedge100-r0 \
    x86-64-ingrasys-s9100-r0 \
    x86-64-ingrasys-s9180-32x-r0 \
    x86-64-ingrasys-s9230-64x-r0 \
    x86-64-ingrasys-s9280-64x-r0 \
    x86-64-inventec-d10056-r0 \
    x86-64-inventec-d10064-r0 \
    x86-64-inventec-d3352-r0 \
    x86-64-inventec-d5052-r0 \
    x86-64-inventec-d5254-r0 \
    x86-64-inventec-d5264q28b-r0 \
    x86-64-inventec-d6254qs-r0 \
    x86-64-inventec-d6332-r0 \
    x86-64-inventec-d6356-r0 \
    x86-64-inventec-d6432-r0 \
    x86-64-inventec-d6556-r0 \
    x86-64-inventec-d7054q28b-r0 \
    x86-64-inventec-d7264q28b-r0 \
    x86-64-inventec-d7332-r0 \
    x86-64-kvm-x86-64-r0 \
    x86-64-lenovo-ne10032-r0 \
    x86-64-lenovo-ne2572-r0 \
    x86-64-mitac-ly1200-b32h0-c3-r0 \
    x86-64-mlnx-mqm8700-r0 \
    x86-64-mlnx-msb7700-r0 \
    x86-64-mlnx-msb7800-r0 \
    x86-64-mlnx-msn2010-r0 \
    x86-64-mlnx-msn2100b-r0 \
    x86-64-mlnx-msn2100-r0 \
    x86-64-mlnx-msn24102-r0 \
    x86-64-mlnx-msn2410b-r0 \
    x86-64-mlnx-msn2410-r0 \
    x86-64-mlnx-msn27002-r0 \
    x86-64-mlnx-msn2700b-r0 \
    x86-64-mlnx-msn2700-r0 \
    x86-64-mlnx-msn2740b-r0 \
    x86-64-mlnx-msn2740-r0 \
    x86-64-mlnx-msn3420-r0 \
    x86-64-mlnx-msn3510-r0 \
    x86-64-mlnx-msn3700c-r0 \
    x86-64-mlnx-msn3700-r0 \
    x86-64-mlnx-msn3800-r0 \
    x86-64-mlnx-msn4600c-r0 \
    x86-64-mlnx-msn4600-r0 \
    x86-64-mlnx-msn4700-r0 \
    x86-64-mlnx-mtq8100-r0 \
    x86-64-mlnx-mtq8200-r0 \
    x86-64-netberg-aurora-420-rangeley-r0 \
    x86-64-netberg-aurora-610-r0 \
    x86-64-netberg-aurora-620-rangeley-r0 \
    x86-64-netberg-aurora-710-r0 \
    x86-64-netberg-aurora-720-rangeley-r0 \
    x86-64-netberg-aurora-750-r0 \
    x86-64-netberg-aurora-820-r0 \
    x86-64-quanta-ix1b-rglbmc-r0 \
    x86-64-quanta-ix1-rangeley-r0 \
    x86-64-quanta-ix2-rangeley-r0 \
    x86-64-quanta-ix7-rglbmc-r0 \
    x86-64-quanta-ix8-rglbmc-r0 \
    x86-64-quanta-ly4r-r0 \
    x86-64-quanta-ly6-rangeley-r0 \
    x86-64-quanta-ly7-rglbmc-r0 \
    x86-64-quanta-ly8-rangeley-r0 \
    x86-64-quanta-ly9-rangeley-r0 \
    x86-64-ufispace-s9700-23d-r0 \
    x86-64-ufispace-s9700-23d-r1 \
    x86-64-ufispace-s9700-23d-r2 \
    x86-64-ufispace-s9700-23d-r3 \
    x86-64-ufispace-s9700-23d-r4 \
    x86-64-ufispace-s9700-23d-r5 \
    x86-64-ufispace-s9700-23d-r6 \
    x86-64-ufispace-s9700-23d-r7 \
    x86-64-ufispace-s9700-23d-r8 \
    x86-64-ufispace-s9700-53dx-r0 \
    x86-64-ufispace-s9700-53dx-r1 \
    x86-64-ufispace-s9700-53dx-r2 \
    x86-64-ufispace-s9700-53dx-r3 \
    x86-64-ufispace-s9700-53dx-r4 \
    x86-64-ufispace-s9700-53dx-r5 \
    x86-64-ufispace-s9700-53dx-r6 \
    x86-64-ufispace-s9700-53dx-r7 \
    x86-64-ufispace-s9700-53dx-r8 \
    x86-64-ufispace-s9700-53dx-r9 \
    x86-64-ufispace-s9700-53dx-r10 \
    x86-64-ufispace-s9705-48d-r0 \
    x86-64-ufispace-s9705-48d-r1 \
    x86-64-ufispace-s9705-48d-r2 \
    x86-64-ufispace-s9705-48d-r3 \
    x86-64-ufispace-s9705-48d-r4 \
    x86-64-ufispace-s9705-48d-r5 \
    x86-64-ufispace-s9705-48d-r6 \
    x86-64-ufispace-s9705-48d-r7 \
    x86-64-ufispace-s9705-48d-r8 \
    x86-64-ufispace-s9705-48d-r9 \
    x86-64-wnc-rseb-w1-32-r0 \
"

ONL_MODULE_VENDORS_BROKEN:arm = " \
"

# technically accton builds, but conflicts with csp7550's modules
ONL_MODULE_VENDORS_BROKEN:x86-64 = " \
    accton \
    netberg \
    quanta \
    ufispace \
"
