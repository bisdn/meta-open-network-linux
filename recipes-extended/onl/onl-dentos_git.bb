# Copyright (C) 2018 Tobias Jungel <tobias.jungel@bisdn.de>
# Released under the MIT license (see COPYING.MIT for the terms)

require onl.inc

LIC_FILES_CHECKSUM += "file://LICENSE;beginline=8;md5=7e6d108802170df125adb4f452c3c9dd"

SRCREV_onl ?= "fa38697b85485ba909b0c9c4f8e5e48bfddbd8fd"
URI_ONL ?= "git://github.com/dentproject/dentOS.git;protocol=https;branch=main"

SRC_URI += " \
           file://onl/0001-file_uds-silence-unused-result-warnings.patch \
           file://onl/0002-platform_manager-do-not-ignore-write-return-value.patch \
           file://onl/0003-file-check-unix-socket-path-is-short-enough.patch \
"
