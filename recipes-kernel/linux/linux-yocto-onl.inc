# https://git.yoctoproject.org/poky/tree/meta/classes/kernel.bbclass
inherit kernel
# https://git.yoctoproject.org/poky/tree/meta/recipes-kernel/linux/linux-yocto.inc
require recipes-kernel/linux/linux-yocto.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/linux-yocto-onl-${KBRANCH}:"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git;protocol=git;branch=${KBRANCH};nocheckout=1;name=machine"

LINUX_VERSION_EXTENSION:append = "-onl"
PV = "${LINUX_VERSION}+git${SRCPV}"

EXTRA_OEMAKE += " HOSTCXX="${BUILD_CXX} ${BUILD_CXXFLAGS} ${BUILD_LDFLAGS}""

#COMPATIBLE_MACHINE = "(^$)"

do_compile_kernelmodules() {
        unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
        if (grep -q -i -e '^CONFIG_MODULES=y$' ${B}/.config); then
                cc_extra=$(get_cc_option)
                oe_runmake -C ${B} ${PARALLEL_MAKE} modules CC="${KERNEL_CC} $cc_extra " LD="${KERNEL_LD}" ${KERNEL_EXTRA_ARGS}

                # Module.symvers gets updated during the
                # building of the kernel modules. We need to
                # update this in the shared workdir since some
                # external kernel modules has a dependency on
                # other kernel modules and will look at this
                # file to do symbol lookups
                cp ${B}/Module.symvers ${STAGING_KERNEL_BUILDDIR}/
		if [ -e ${B}/scripts/module.lds ]; then
		    mkdir -p ${STAGING_KERNEL_BUILDDIR}/scripts
		    cp ${B}/scripts/module.lds ${STAGING_KERNEL_BUILDDIR}/scripts/
		fi
        else
                bbnote "no modules to compile"
        fi
}

do_shared_workdir () {
        cd ${B}

        kerneldir=${STAGING_KERNEL_BUILDDIR}
        install -d $kerneldir

        #
        # Store the kernel version in sysroots for module-base.bbclass
        #

        echo "${KERNEL_VERSION}" > $kerneldir/${KERNEL_PACKAGE_NAME}-abiversion

        # Copy files required for module builds
        cp System.map $kerneldir/System.map-${KERNEL_VERSION}
        [ -e Module.symvers ] && cp Module.symvers $kerneldir/
	if [ -e ./scripts/module.lds ]; then
	    mkdir -p $kerneldir/scripts
	    cp ./scripts/module.lds $kerneldir/scripts/
	fi
        cp .config $kerneldir/
        mkdir -p $kerneldir/include/config
        cp include/config/kernel.release $kerneldir/include/config/kernel.release
        if [ -e certs/signing_key.x509 ]; then
                # The signing_key.* files are stored in the certs/ dir in
                # newer Linux kernels
                mkdir -p $kerneldir/certs
                cp certs/signing_key.* $kerneldir/certs/
        elif [ -e signing_key.priv ]; then
                cp signing_key.* $kerneldir/
        fi

        # We can also copy over all the generated files and avoid special cases
        # like version.h, but we've opted to keep this small until file creep starts
        # to happen
        if [ -e include/linux/version.h ]; then
                mkdir -p $kerneldir/include/linux
                cp include/linux/version.h $kerneldir/include/linux/version.h
        fi

        # As of Linux kernel version 3.0.1, the clean target removes
        # arch/powerpc/lib/crtsavres.o which is present in
        # KBUILD_LDFLAGS_MODULE, making it required to build external modules.
        if [ ${ARCH} = "powerpc" ]; then
                if [ -e arch/powerpc/lib/crtsavres.o ]; then
                        mkdir -p $kerneldir/arch/powerpc/lib/
                        cp arch/powerpc/lib/crtsavres.o $kerneldir/arch/powerpc/lib/crtsavres.o
                fi
        fi

        if [ -d include/generated ]; then
                mkdir -p $kerneldir/include/generated/
                cp -fR include/generated/* $kerneldir/include/generated/
        fi

        if [ -d arch/${ARCH}/include/generated ]; then
                mkdir -p $kerneldir/arch/${ARCH}/include/generated/
                cp -fR arch/${ARCH}/include/generated/* $kerneldir/arch/${ARCH}/include/generated/
        fi

        if (grep -q -i -e '^CONFIG_UNWINDER_ORC=y$' $kerneldir/.config); then
                # With CONFIG_UNWINDER_ORC (the default in 4.14), objtool is required for
                # out-of-tree modules to be able to generate object files.
                if [ -x tools/objtool/objtool ]; then
                        mkdir -p ${kerneldir}/tools/objtool
                        cp tools/objtool/objtool ${kerneldir}/tools/objtool/
                fi
        fi
}
