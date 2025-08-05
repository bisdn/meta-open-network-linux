# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

LICENSE = "EPL-1.0"
LIC_FILES_CHKSUM = "\
  file://${SUBMODULE_INFRA}/LICENSE;md5=457079d296746aac524eb56eb6822ea8 \
  file://${SUBMODULE_BIGCODE}/LICENSE;md5=dc6bd4d967e085fe783aa2abe7655c60 \
"

SRCREV_infra ?= "8621a0bab76affaaad6dee0939fda0737c32c881"
SRCREV_bigcode ?= "4bd372b72472981b5c612b577004fc61c74d98aa"

URI_INFRA ?= "git://github.com/opennetlinux/infra.git;protocol=https;branch=master"
URI_BIGCODE ?= "git://github.com/opennetlinux/bigcode.git;protocol=https;branch=master"

SRCREV_FORMAT = "onl_infra_bigcode"

# submodules are checked out individually to support license file checking
SRC_URI = "${URI_ONL};name=onl \
           ${URI_INFRA};name=infra;destsuffix=git/${SUBMODULE_INFRA} \
           ${URI_BIGCODE};name=bigcode;destsuffix=git/${SUBMODULE_BIGCODE} \
           file://onlpd.service \
"

inherit systemd python3native module-base kernel-module-split

SYSTEMD_SERVICE:${PN} = "onlpd.service"
SYSTEMD_AUTO_ENABLE = "enable"

DEPENDS = "i2c-tools libedit python3-pyyaml-native"

S = "${WORKDIR}/git"
PV = "1.1+git${SRCPV}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PROVIDES += "libonlp libonlp-platform libonlp-platform-defaults"
INSANE_SKIP:${PN} = "file-rdeps"

PACKAGES_DYNAMIC = "^libonlp-.*"

python populate_packages:prepend () {
    # ONL uses dashes in the ARCH name
    arch=d.getVar('TRANSLATED_TARGET_ARCH')
    name_prefix='libonlp-' + arch

    # prevent Yocto from being "helpful" and renaming packages based on the
    # included lib's filename
    def debian_noautoname(file, pkg, pattern, format, basename):
        d.setVar('DEBIAN_NOAUTONAME:%s' % pkg, "1")

    # only split out platform libraries (having arch in their name)
    do_split_packages(
        d, '${libdir}', name_prefix + '-(.*).so.1', name_prefix + '-%s',
        'libonlp (' + arch + '-%s)', prepend=True, hook=debian_noautoname)
    # create -dev packages as well for the .so symlink, else onl-dev will depend
    # on everything
    do_split_packages(
        d, '${libdir}', name_prefix + '-(.*).so$', name_prefix + '-%s-dev',
        'libonlp (' + arch + '-%s) - Development files', prepend=True,
        allow_links=True)
}

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
TOOLCHAIN = "gcc-local"
NO_USE_GCC_VERSION_TOOL="1"

def onl_arch_map(arch, tune):
    tune_features = tune.split()
    if arch == "allarch":
        return "all"
    if arch in ["i586", "i686"]:
        return "i386"
    if arch == "x86_64":
        if "mx32" in tune_features:
            return "x32"
        return "amd64"
    if arch.startswith("mips"):
        endian = ["el", ""]["bigendian" in tune_features]
        if "n64" in tune_features:
            return "mips64" + endian
        if "n32" in tune_features:
            return "mipsn32" + endian
        return "mips" + endian
    if arch == "powerpc":
        return arch + ["", "spe"]["spe" in tune_features]
    if arch == "aarch64":
        return "arm64"
    if arch == "arm":
        return arch + ["el", "hf"]["callconvention-hard" in tune_features]
    return arch

ONL_BUILD_ARCH = "${@onl_arch_map(d.getVar('TARGET_ARCH'), d.getVar('TUNE_FEATURES'))}"
ONL_DEBIAN_SUITE = "buster"

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
  'GCC_FLAGS=${CFLAGS} -DONLPLIB_CONFIG_I2C_USE_CUSTOM_HEADER=0 -Wno-error=unused-result -Wno-error=address -Wno-error=pointer-to-int-cast' \
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

  # there is no package for the generic modules, so call kmodbuild.sh directly.
  # It will create the modules in `pwd`, so let's call it in its own directory
  mkdir -p ${S}/onl-modules
  cd ${S}/onl-modules

  # emerson700 requires extensive changes to build on newer kernels, so skip it
  mods="$(find ${S}/packages/base/any/kernels/modules -iname '*.c' -a ! -iname emerson700.c)"

  # "onl/onl/common" is the target path that ONL uses when building the modules
  # in the context of the kernel build
  ${S}/tools/scripts/kmodbuild.sh "${STAGING_KERNEL_BUILDDIR}" "$mods" "onl/onl/common"
  cd -

  for platform in ${ONL_PLATFORMS_BUILD}; do
      # get the path to the platform
      ONLP_DIR=$(ONL=${ONL} ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE} tools/onlpm.py --packagedirs=${S}/packages --repo=${S}/REPO --show-build-dirs onlp-${platform}:${ONL_BUILD_ARCH})

      V=1 VERBOSE=1 oe_runmake -C ${ONLP_DIR}/builds/lib alltargets

      # get the path to the platform modules
      baseplatform=${platform%-r*}

      MODULES_DIR=$(ONL=${ONL} ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE} tools/onlpm.py --packagedirs=${S}/packages --repo=${S}/REPO --show-build-dirs onl-platform-modules-${baseplatform}:${ONL_BUILD_ARCH})
      V=1 VERBOSE=1 oe_runmake -C ${MODULES_DIR}/builds modules ARCH=${ARCH} KERNELS=${STAGING_KERNEL_BUILDDIR}
  done

  for vendor in ${ONL_MODULE_VENDORS_BUILD}; do
      MODULES_DIR=$(ONL=${ONL} ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE} tools/onlpm.py --packagedirs=${S}/packages --repo=${S}/REPO --show-build-dirs onl-vendor-${vendor}-modules:${ONL_BUILD_ARCH})
      V=1 VERBOSE=1 oe_runmake -C ${MODULES_DIR}/builds modules ARCH=${ARCH} KERNELS=${STAGING_KERNEL_BUILDDIR}
  done
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
    ${D}${libdir} \
    ${D}${nonarch_base_libdir}/modules


  # install onlpdump
  install -m 0755 packages/base/any/onlp/builds/onlpd/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/onlpd ${D}${bindir}
  ln -r -s ${D}${bindir}/onlpd ${D}${bindir}/onlpdump

  # install headers
  install -m 0644 packages/base/any/onlp/src/onlp/module/inc/onlp/*.h ${D}${includedir}/onlp/
  install -m 0644 packages/base/any/onlp/src/onlplib/module/inc/onlplib/*.h ${D}${includedir}/onlplib/
  install -m 0644 sm/bigcode/modules/BigData/BigList/module/inc/BigList/*.h ${D}${includedir}/BigList/
  install -m 0644 sm/bigcode/modules/IOF/module/inc/IOF/*.h ${D}${includedir}/IOF/
  install -m 0644 sm/bigcode/modules/cjson/module/inc/cjson/*.h ${D}${includedir}/cjson/
  install -m 0644 sm/infra/modules/AIM/module/inc/AIM/*.h ${D}${includedir}/AIM/

  # install base kernel modules
  mkdir -p ${D}${nonarch_base_libdir}/modules
  cp -r onl-modules/lib/modules ${D}${nonarch_base_libdir}

  for platform in ${ONL_PLATFORMS_BUILD}; do
      # get the path to the platform
      ONLP_DIR=$(ONL=${ONL} ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE} tools/onlpm.py --packagedirs=${S}/packages --repo=${S}/REPO --show-build-dirs onlp-${platform}:${ONL_BUILD_ARCH})

      # install libonlp-<baseplatform> shared library
      baseplatform=${platform%-r*}
      install -m 0755 ${ONLP_DIR}/builds/lib/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp-${baseplatform}.so ${D}${libdir}
      mv ${D}${libdir}/libonlp-${baseplatform}.so ${D}${libdir}/libonlp-${baseplatform}.so.1
      ln -r -s ${D}${libdir}/libonlp-${baseplatform}.so.1 ${D}${libdir}/libonlp-${baseplatform}.so

      # install platform modules
      MODULES_DIR=$(ONL=${ONL} ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE} tools/onlpm.py --packagedirs=${S}/packages --repo=${S}/REPO --show-build-dirs onl-platform-modules-${baseplatform}:${ONL_BUILD_ARCH})
      cp -r ${MODULES_DIR}/builds/lib/modules ${D}${nonarch_base_libdir}
  done

  # install vendor modules
  for vendor in ${ONL_MODULE_VENDORS_BUILD}; do
      MODULES_DIR=$(ONL=${ONL} ONL_DEBIAN_SUITE=${ONL_DEBIAN_SUITE} tools/onlpm.py --packagedirs=${S}/packages --repo=${S}/REPO --show-build-dirs onl-vendor-${vendor}-modules:${ONL_BUILD_ARCH})
      cp -r ${MODULES_DIR}/builds/lib/modules ${D}${nonarch_base_libdir}
  done

  # install libonlp shared library
  install -m 0755 packages/base/any/onlp/builds/onlp/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp.so ${D}${libdir}
  mv ${D}${libdir}/libonlp.so ${D}${libdir}/libonlp.so.1
  ln -r -s ${D}${libdir}/libonlp.so.1 ${D}${libdir}/libonlp.so

  # install libonlp-platform-defaults shared library
  install -m 0755 packages/base/any/onlp/builds/onlp-platform-defaults/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp-platform-defaults.so ${D}${libdir}
  mv ${D}${libdir}/libonlp-platform-defaults.so ${D}${libdir}/libonlp-platform-defaults.so.1
  ln -r -s ${D}${libdir}/libonlp-platform-defaults.so.1 ${D}${libdir}/libonlp-platform-defaults.so

  # install libonlp-platform shared library
  install -m 0755 packages/base/any/onlp/builds/onlp-platform/BUILD/${ONL_DEBIAN_SUITE}/${TOOLCHAIN}/bin/libonlp-platform.so ${D}${libdir}
  mv ${D}${libdir}/libonlp-platform.so ${D}${libdir}/libonlp-platform.so.1
  ln -r -s ${D}${libdir}/libonlp-platform.so.1 ${D}${libdir}/libonlp-platform.so

  # service file
  install -d ${D}${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/onlpd.service ${D}${systemd_unitdir}/system
  sed -i -e 's,@BINDIR@,${bindir},g' \
         ${D}${systemd_unitdir}/system/*.service
}
