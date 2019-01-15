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

SRCREV_onl ?= "ff19ef83e0cc241c32a78dea259a4ebd9f9da139"
SRCREV_infra ?= "f78cc91121bf520b48b4888f08e055ed92b9fbf2"
SRCREV_bigcode ?= "59b58a4c8b7fb59060e4d76a2872458bb806ed5f"

URI_ONL ?= "git://github.com/opencomputeproject/OpenNetworkLinux.git"
URI_INFRA ?= "git://github.com/floodlight/infra.git"
URI_BIGCODE ?= "git://github.com/floodlight/bigcode.git"

SRCREV_FORMAT = "onl_infra_bigcode"

# submodules are checked out individually to support license file checking
SRC_URI = "${URI_ONL};name=onl \
           ${URI_INFRA};name=infra;destsuffix=git/${SUBMODULE_INFRA} \
           ${URI_BIGCODE};name=bigcode;destsuffix=git/${SUBMODULE_BIGCODE} \
           file://ar.patch;patchdir=${SUBMODULE_INFRA} \
           file://56.patch;patchdir=${SUBMODULE_INFRA} \
           file://onlpdump.service \
           file://0001-i2c-patch.patch \
"

inherit systemd

SYSTEMD_SERVICE_${PN} = "onlpdump.service"
SYSTEMD_AUTO_ENABLE = "enable"

DEPENDS = "i2c-tools"

S = "${WORKDIR}/git"
PV = "1.0+git${SRCPV}"

PACKAGE_ARCH = "${MACHINE_ARCH}"

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

do_compile() {
  #oe_runmake -C packages/base/any/onlp/builds/ show_targets show_libs show_bins show_shared show_scripts
  #oe_runmake -C packages/base/any/onlp/builds/ alltargets

  V=1 oe_runmake -C packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM}/onlp/builds/ alltargets
}

do_install() {
  # so file
  #install -d ${D}${libdir}
  #packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM}/onlp/builds/lib/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/

  # onlpdump
  install -d ${D}${bindir}
  install -m 0755 packages/platforms/${ONIE_VENDOR}/${ONL_ARCH}/${ONL_PLATFORM}/onlp/builds/onlpdump/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/onlpdump ${D}${bindir}

  # platform file
  install -d ${D}${sysconfdir}/onl
  echo "${ONL_PLATFORM}-r${ONIE_MACHINE_REV}" > ${D}${sysconfdir}/onl/platform

  # service file
  install -d ${D}${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/onlpdump.service ${D}${systemd_unitdir}/system
  sed -i -e 's,@BINDIR@,${bindir},g' \
         ${D}${systemd_unitdir}/system/*.service
}
