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

echo cpld_csp7551 0x62 > /sys/bus/i2c/devices/i2c-33/new_device
echo cpld_csp7551 0x64 > /sys/bus/i2c/devices/i2c-34/new_device
echo csp7551_port1 0x50 > /sys/bus/i2c/devices/i2c-1/new_device
echo csp7551_port2 0x50 > /sys/bus/i2c/devices/i2c-2/new_device
echo csp7551_port3 0x50 > /sys/bus/i2c/devices/i2c-3/new_device
echo csp7551_port4 0x50 > /sys/bus/i2c/devices/i2c-4/new_device
echo csp7551_port5 0x50 > /sys/bus/i2c/devices/i2c-5/new_device
echo csp7551_port6 0x50 > /sys/bus/i2c/devices/i2c-6/new_device
echo csp7551_port7 0x50 > /sys/bus/i2c/devices/i2c-7/new_device
echo csp7551_port8 0x50 > /sys/bus/i2c/devices/i2c-8/new_device
echo csp7551_port9 0x50 > /sys/bus/i2c/devices/i2c-9/new_device
echo csp7551_port10 0x50 > /sys/bus/i2c/devices/i2c-10/new_device
echo csp7551_port11 0x50 > /sys/bus/i2c/devices/i2c-11/new_device
echo csp7551_port12 0x50 > /sys/bus/i2c/devices/i2c-12/new_device
echo csp7551_port13 0x50 > /sys/bus/i2c/devices/i2c-13/new_device
echo csp7551_port14 0x50 > /sys/bus/i2c/devices/i2c-14/new_device
echo csp7551_port15 0x50 > /sys/bus/i2c/devices/i2c-15/new_device
echo csp7551_port16 0x50 > /sys/bus/i2c/devices/i2c-16/new_device
echo csp7551_port17 0x50 > /sys/bus/i2c/devices/i2c-17/new_device
echo csp7551_port18 0x50 > /sys/bus/i2c/devices/i2c-18/new_device
echo csp7551_port19 0x50 > /sys/bus/i2c/devices/i2c-19/new_device
echo csp7551_port20 0x50 > /sys/bus/i2c/devices/i2c-20/new_device
echo csp7551_port21 0x50 > /sys/bus/i2c/devices/i2c-21/new_device
echo csp7551_port22 0x50 > /sys/bus/i2c/devices/i2c-22/new_device
echo csp7551_port23 0x50 > /sys/bus/i2c/devices/i2c-23/new_device
echo csp7551_port24 0x50 > /sys/bus/i2c/devices/i2c-24/new_device
echo csp7551_port25 0x50 > /sys/bus/i2c/devices/i2c-25/new_device
echo csp7551_port26 0x50 > /sys/bus/i2c/devices/i2c-26/new_device
echo csp7551_port27 0x50 > /sys/bus/i2c/devices/i2c-27/new_device
echo csp7551_port28 0x50 > /sys/bus/i2c/devices/i2c-28/new_device
echo csp7551_port29 0x50 > /sys/bus/i2c/devices/i2c-29/new_device
echo csp7551_port30 0x50 > /sys/bus/i2c/devices/i2c-30/new_device
echo csp7551_port31 0x50 > /sys/bus/i2c/devices/i2c-31/new_device
echo csp7551_port32 0x50 > /sys/bus/i2c/devices/i2c-32/new_device
