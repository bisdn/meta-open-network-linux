#!/bin/sh

# Platform init for tn48m
#
# XXX We might be able to learn something from
# https://github.com/dentproject/dentOS/blob/fa38697b85485ba909b0c9c4f8e5e48bfddbd8fd/packages/platforms/delta/arm64/tn48m/tn48m/platform-config/r0/src/python/arm64_delta_tn48m_r0/__init__.py

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
if grep -q tn48m-dn /sys/firmware/devicetree/base/model; then
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
