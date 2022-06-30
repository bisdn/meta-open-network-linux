#
# Yocto recipe to build the out-of-tree Intel bf_kdrv driver for Tofino.
#

DESCRIPTION = "bf_tun driver for Tofino"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://bf_tun.c;endline=14;md5=ea35c61bcd0d0306462c284b88195c72"

PR = "r0"

inherit module

SRC_URI = " \
          file://Makefile \
          file://bf_tun.c \
          file://bf_tun_3.c \
          file://bf_tun_v4.19.67.c \
"

S = "${WORKDIR}"
