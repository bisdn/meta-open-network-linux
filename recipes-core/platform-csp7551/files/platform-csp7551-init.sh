#!/bin/sh

# Platform init for as5835-54x
#
# Based on https://github.com/opencomputeproject/OpenNetworkLinux.git
#   packages/platforms/accton/x86-64/as5835-54x/platform-config/r0/src/
#   python/x86_64_accton_as5835_54x_r0/__init__.py

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

  # For pca9548 multiplexer, set idle_state to -2 (MUX_IDLE_DISCONNECT)
  # Not doing this can result, for instance, in SFP EEPROMs not being
  # read correctly by onlp.
  if [ "$i2c_dev_name" = "pca9548" ]; then
    # Replace leading "0x" with "00"
    i2c_dev_addr_4="00${i2c_dev_addr:2}"
    echo '-2' > /sys/bus/i2c/devices/$i2c_bus_no-$i2c_dev_addr_4/idle_state
  fi
}

# load modules
modprobe fpga_driver
modprobe accton_i2c_cpld
modprobe x86-64-accton-csp7551-sfp
modprobe at24_csp7551
