# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Open Network Linux Components for Yocto integration"
HOMEPAGE = "https://www.opennetlinux.org/"
LICENSE = "EPLv1.0"
LIC_FILES_CHKSUM = "\
  file://LICENSE;beginline=14;md5=7e6d108802170df125adb4f452c3c9dd \
  file://${SUBMODULE_INFRA}/LICENSE;md5=457079d296746aac524eb56eb6822ea8 \
  file://${SUBMODULE_BIGCODE}/LICENSE;md5=dc6bd4d967e085fe783aa2abe7655c60 \
"

SRCREV_onl ?= "dc450d30dafde183f8192e3ffc88c0c68856fcb0"
SRCREV_infra ?= "8621a0bab76affaaad6dee0939fda0737c32c881"
SRCREV_bigcode ?= "981aee67ebf433d42f444d4faf1a46a596b39555"

URI_ONL ?= "git://github.com/opencomputeproject/OpenNetworkLinux.git"
URI_INFRA ?= "git://github.com/floodlight/infra.git"
URI_BIGCODE ?= "git://github.com/floodlight/bigcode.git"

SRCREV_FORMAT = "onl_infra_bigcode"

# submodules are checked out individually to support license file checking
SRC_URI = "${URI_ONL};name=onl \
           ${URI_INFRA};name=infra;destsuffix=git/${SUBMODULE_INFRA} \
           ${URI_BIGCODE};name=bigcode;destsuffix=git/${SUBMODULE_BIGCODE} \
           file://onlpdump.service \
           file://0001-file_uds-silence-unused-result-warnings.patch \
           file://0002-platform_manager-do-not-ignore-write-return-value.patch \
           file://0003-file-check-unix-socket-path-is-short-enough.patch \
           file://0004-agema-ag7648-fix-buffer-overflow-while-reading-therm.patch \
           file://0005-agema-ag7648-don-t-create-random-rx_los-bitmap-value.patch \
           file://0006-accton-as4610-fix-buffer-overflow-while-accessing-le.patch \
           file://0007-accton-as4610-check-led-mode-before-trying-to-snprin.patch \
"

inherit systemd

SYSTEMD_SERVICE_${PN} = "onlpdump.service"
SYSTEMD_AUTO_ENABLE = "enable"

DEPENDS = "i2c-tools libedit"

S = "${WORKDIR}/git"
PV = "1.1+git${SRCPV}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PROVIDES += "libonlp libonlp-platform libonlp-platform-defaults"
INSANE_SKIP_${PN} = "file-rdeps"

#### TODO onl.bbclass?
ONL = "${S}"

SUBMODULE_INFRA = "sm/infra"
SUBMODULE_BIGCODE = "sm/bigcode"
BUILDER = "${S}/${SUBMODULE_INFRA}/builder/unix"

BUILDER_MODULE_DATABASE = "${WORKDIR}/modules/modules.json"
BUILDER_MODULE_DATABASE_ROOT = "${WORKDIR}"
BUILDER_MODULE_MANIFEST = "${WORKDIR}/modules/modules.mk"
MODULEMANIFEST = "${BUILDER_MODULE_MANIFEST}"

#export SUBMODULE_INFRA BUILDER BUILDER_MODULE_DATABASE BUILDER_MODULE_DATABASE_ROOT BUILDER_MODULE_MANIFEST MODULEMANIFEST ONL
ARCH = "${TARGET_ARCH}"
TOOLCHAIN = "gcc-local"
NO_USE_GCC_VERSION_TOOL="1"

###
# TODO CFLAGS?
EXTRA_OEMAKE = "\
  'AR=${AR}' \
  'ARCH=${ONL_BUILD_ARCH}' \
  'BUILDER=${BUILDER}' \
  'BUILDER_MODULE_DATABASE=${BUILDER_MODULE_DATABASE}' \
  'BUILDER_MODULE_DATABASE_ROOT=${BUILDER_MODULE_DATABASE_ROOT}' \
  'BUILDER_MODULE_MANIFEST=${BUILDER_MODULE_MANIFEST}' \
  'MODULEMANIFEST=${MODULEMANIFEST}' \
  'GCC=${CC}' \
  'GCC_FLAGS=${CFLAGS} -DONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER=0' \
  'MODULEMANIFEST=${MODULEMANIFEST}' \
  'NO_USE_GCC_VERSION_TOOL=${NO_USE_GCC_VERSION_TOOL}' \
  'ONL=${ONL}' \
  'ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE}' \
  'SUBMODULE_BIGCODE=${SUBMODULE_BIGCODE}' \
  'SUBMODULE_INFRA=${SUBMODULE_INFRA}' \
  'SUBMODULE_INFRA=${SUBMODULE_INFRA}' \
  'TOOLCHAIN=${TOOLCHAIN}' \
"

do_configure() {
  mkdir -p $(dirname ${BUILDER_MODULE_DATABASE})
  MODULEMANIFEST_=$(${BUILDER}/tools/modtool.py --db ${BUILDER_MODULE_DATABASE} --dbroot ${BUILDER_MODULE_DATABASE_ROOT} --make-manifest ${BUILDER_MODULE_MANIFEST})
  # XXX compare MODULEMANIFEST_ vs MODULEMANIFEST
  echo ${MODULEMANIFEST_}
}

do_compile() {
  # examples
  #oe_runmake -C packages/base/any/onlp/builds/ show_targets show_libs show_bins show_shared show_scripts
  #oe_runmake -C packages/base/any/onlp/builds/ alltargets

  V=1 VERBOSE=1 oe_runmake -C packages/base/any/onlp/builds alltargets
  V=1 VERBOSE=1 oe_runmake -C packages/base/any/onlp/builds/onlpd alltargets

  V=1 VERBOSE=1 oe_runmake -C packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM_SUBDIR}/${ONIE_MACHINE_TYPE}/onlp/builds/lib alltargets
  V=1 VERBOSE=1 oe_runmake -C packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM_SUBDIR}/${ONIE_MACHINE_TYPE}/onlp/builds/onlpdump alltargets
}

do_install() {
  # folders in dest
  install -d \
    ${D}${bindir} \
    ${D}${includedir}/AIM \
    ${D}${includedir}/BigList \
    ${D}${includedir}/IOF \
    ${D}${includedir}/cjson \
    ${D}${includedir}/onlp \
    ${D}${includedir}/onlplib \
    ${D}${libdir}

  # install onlpdump
  install -m 0755 packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM_SUBDIR}/${ONIE_MACHINE_TYPE}/onlp/builds/onlpdump/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/onlps ${D}${bindir}/onlpdump

  # install headers
  install -m 0644 packages/base/any/onlp/src/onlp/module/inc/onlp/*.h ${D}${includedir}/onlp/
  install -m 0644 packages/base/any/onlp/src/onlplib/module/inc/onlplib/*.h ${D}${includedir}/onlplib/
  install -m 0644 sm/bigcode/modules/BigData/BigList/module/inc/BigList/*.h ${D}${includedir}/BigList/
  install -m 0644 sm/bigcode/modules/IOF/module/inc/IOF/*.h ${D}${includedir}/IOF/
  install -m 0644 sm/bigcode/modules/cjson/module/inc/cjson/*.h ${D}${includedir}/cjson/
  install -m 0644 sm/infra/modules/AIM/module/inc/AIM/*.h ${D}${includedir}/AIM/

  # install libonlp-platform shared library (includes AIM.a  AIM_posix.a  BigList.a  cjson.a  cjson_util.a  IOF.a  onlplib.a  x86_64_delta_ag7648.a)
  install -m 0755 packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM_SUBDIR}/${ONIE_MACHINE_TYPE}/onlp/builds/lib/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp-${ONL_PLATFORM}.so ${D}${libdir}
  mv ${D}${libdir}/libonlp-${ONL_PLATFORM}.so ${D}${libdir}/libonlp-${ONL_PLATFORM}.so.1
  ln -r -s ${D}${libdir}/libonlp-${ONL_PLATFORM}.so.1 ${D}${libdir}/libonlp-${ONL_PLATFORM}.so
  ln -r -s ${D}${libdir}/libonlp-${ONL_PLATFORM}.so.1 ${D}${libdir}/libonlp-platform.so.1

  # install libonlp shared library (includes TODO)
  install -m 0755 packages/base/any/onlp/builds/onlp/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp.so ${D}${libdir}
  mv ${D}${libdir}/libonlp.so ${D}${libdir}/libonlp.so.1
  ln -r -s ${D}${libdir}/libonlp.so.1 ${D}${libdir}/libonlp.so

  # install libonlp shared library (includes TODO)
  install -m 0755 packages/base/any/onlp/builds/onlp-platform-defaults/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp-platform-defaults.so ${D}${libdir}
  mv ${D}${libdir}/libonlp-platform-defaults.so ${D}${libdir}/libonlp-platform-defaults.so.1
  ln -r -s ${D}${libdir}/libonlp-platform-defaults.so.1 ${D}${libdir}/libonlp-platform-defaults.so

  # platform file
  install -d ${D}${sysconfdir}/onl
  echo "${ONL_PLATFORM}-r${ONIE_MACHINE_REV}" > ${D}${sysconfdir}/onl/platform

  # service file
  install -d ${D}${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/onlpdump.service ${D}${systemd_unitdir}/system
  sed -i -e 's,@BINDIR@,${bindir},g' \
         ${D}${systemd_unitdir}/system/*.service
}
