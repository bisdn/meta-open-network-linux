#!/bin/sh

# Platform init for csp7551

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

# Logic taken from CSP-7551_Driver_Package_20220517.7z

modprobe -r i2c-i801 || true
modprobe fpga_driver
modprobe accton_i2c_cpld
modprobe x86-64-accton-csp7551-sfp

create_i2c_dev cpld_csp7551 0x62 33
create_i2c_dev cpld_csp7551 0x64 34

for i in {1..32}; do
  create_i2c_dev csp7551_port$i 0x50 $i
done
