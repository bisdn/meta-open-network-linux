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

SRCREV_onl = "95917ac48a1f85dd41a35792f5b1aadf171a94d7"
SRCREV_infra = "16ce9cd77f6639aac4813d698f9dd11f3ee47e7a"
SRCREV_bigcode = "081f26bb5be40d51a8551d35395f06be137349cb"

# SRCREV = "${AUTOREV}"
# SRCREV = "${SRCREV_ONL}"
SRCREV_FORMAT = "onl_infra_bigcode"

# submodules are checked out individually to support license file checking
SRC_URI = "git://github.com/opencomputeproject/OpenNetworkLinux.git;name=onl \
           git://github.com/floodlight/infra.git;name=infra;destsuffix=git/${SUBMODULE_INFRA} \
           git://github.com/floodlight/bigcode.git;name=bigcode;destsuffix=git/${SUBMODULE_BIGCODE} \
           file://ar.patch;patchdir=${SUBMODULE_INFRA} \
           file://56.patch;patchdir=${SUBMODULE_INFRA} \
"

DEPENDS = "i2c-tools "

S = "${WORKDIR}/git"
PV = "1.0+git${SRCPV}"

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
ONL_DEBIAN_SUITE = "yocto"
NO_USE_GCC_VERSION_TOOL="1"

###
# TODO CFLAGS?
EXTRA_OEMAKE = "\
  'AR=${AR}' \
  'ARCH=${ARCH}' \
  'BUILDER=${BUILDER}' \
  'GCC=${CC}' \
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

# FIXME
do_compile() {
  oe_runmake -C packages/base/any/onlp/builds/ show_targets show_libs show_bins show_shared show_scripts
  oe_runmake -C packages/base/any/onlp/builds/ alltargets
  oe_runmake -C packages/platforms/delta/x86-64/x86-64-delta-ag7648/onlp/builds/ alltargets
  false
}

do_install() {
  install -d ${D}${libdir}
  install -d ${D}${includedir}/AIM

  install -m 0644 ${S}/targets/utests/AIM/build/gcc-local/lib/AIM.a ${D}${libdir}
  install -m 0644 ${S}/targets/utests/AIM/build/gcc-local/lib/AIM_posix.a ${D}${libdir}

  install -m 0644 ${S}/modules/AIM/module/inc/AIM/*.h ${D}${includedir}/AIM/
}