#!/bin/sh

# Platform init for tn48m

set -o errexit -o nounset

function wait_for_file() {
  FILE=$1
  i=0
  while [ $i -lt 10 ]; do
    test -e $FILE && return 0
    i=$(( i + 1 ))
    sleep 1
  done
  return 1
}

create_i2c_dev() {
  local i2c_dev_name=$1
  local i2c_dev_addr=$2
  local i2c_bus_no=$3

  wait_for_file "/sys/bus/i2c/devices/i2c-$i2c_bus_no/new_device"
  echo "$i2c_dev_name" "$i2c_dev_addr" > /sys/bus/i2c/devices/i2c-${i2c_bus_no}/new_device
}

# load modules
if grep -q tn48m-dn /etc/onl/platform; then
  modprobe arm64-delta-tn48m-dn-cpld
  modprobe arm64-delta-tn48m-dn-led
else
  modprobe arm64-delta-tn48m-cpld
  modprobe arm64-delta-tn48m-led
fi
modprobe prestera_pci

# fan controller
create_i2c_dev adt7473 0x2e 1

# temperature devices
create_i2c_dev tmp1075 0x4a 1
create_i2c_dev tmp1075 0x4b 1
