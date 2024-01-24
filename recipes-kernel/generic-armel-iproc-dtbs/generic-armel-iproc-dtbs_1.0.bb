LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-or-later;md5=fed54355545ffd980b814dab4a3b312c"

inherit devicetree

PROVIDES = "virtual/dtb"

COMPATIBLE_MACHINE = "generic-armel-iproc"

SRC_URI = " \
          file://arm-accton-as4610-30p.dts \
          file://arm-accton-as4610-30t.dts \
          file://arm-accton-as4610-54.dts \
          file://arm-accton-as4610-54p.dts \
          file://arm-accton-as4610-54t.dts \
          file://arm-accton-as4610.dts \
          file://arm-accton-as4610.dtsi \
          "
