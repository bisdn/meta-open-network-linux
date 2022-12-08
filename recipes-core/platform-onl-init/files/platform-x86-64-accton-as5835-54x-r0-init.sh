#!/bin/bash

# Platform init for as5835-54x
#
# Based on https://github.com/opencomputeproject/OpenNetworkLinux.git
#   packages/platforms/accton/x86-64/as5835-54x/platform-config/r0/src/
#   python/x86_64_accton_as5835_54x_r0/__init__.py

set -o errexit -o nounset

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
# make sure i2c-i801 is present
wait_for_file /sys/bus/i2c/devices/i2c-0

# load modules
modprobe i2c-ismt
modprobe x86-64-accton-as5835-54x-cpld
modprobe x86-64-accton-as5835-54x-psu
modprobe x86-64-accton-as5835-54x-leds
modprobe x86-64-accton-as5835-54x-fan

# initialize multiplexer (PCA9548)
create_i2c_dev pca9548 0x77 1
create_i2c_dev pca9548 0x70 2
create_i2c_dev pca9548 0x71 2
create_i2c_dev pca9548 0x72 2

# initialize CPLD
create_i2c_dev as5835_54x_cpld1 0x60 3
create_i2c_dev as5835_54x_cpld2 0x61 3
create_i2c_dev as5835_54x_cpld3 0x62 3
create_i2c_dev as5835_54x_fan 0x63 3

# initiate PSU-1 AC Power
create_i2c_dev as5835_54x_psu1 0x50 11
create_i2c_dev ym2401 0x58 11

# initiate PSU-2 AC Power
create_i2c_dev as5835_54x_psu2 0x53 12
create_i2c_dev ym2401 0x5b 12

# inititate LM75
create_i2c_dev lm75 0x4b 18
create_i2c_dev lm75 0x4c 19
create_i2c_dev lm75 0x49 20
create_i2c_dev lm75 0x4a 21

# initialize multiplexer (PCA9548)
create_i2c_dev pca9548 0x70 3
create_i2c_dev pca9548 0x71 34
create_i2c_dev pca9548 0x72 35
create_i2c_dev pca9548 0x73 36
create_i2c_dev pca9548 0x74 37
create_i2c_dev pca9548 0x75 38
create_i2c_dev pca9548 0x76 39

# initiate IDPROM
create_i2c_dev 24c02 0x57 1

# initialize SFP devices
for port in {1..48}; do
	create_i2c_dev optoe2 0x50 $((port + 41))
done

for port in {1..48}; do
	echo "port$port" > "/sys/bus/i2c/devices/$((port + 41))-0050/port_name"
done

# initialize QSFP devices
for port in {49..54}; do
	create_i2c_dev optoe1 0x50 $((port - 23))
	echo 0 > "/sys/bus/i2c/devices/3-0062/module_reset_$port"
done

port=49
for sfp_no in 28 29 26 30 31 27; do
	echo "port$port" > "/sys/bus/i2c/devices/$sfp_no-0050/port_name"
	port=$((port + 1))
done

# Set disable tx_disable to sfp port
for port in {1..38}; do
	echo 0 > "/sys/bus/i2c/devices/3-0061/module_tx_disable_$port"
done

for port in {39..48}; do
	echo 0 > "/sys/bus/i2c/devices/3-0062/module_tx_disable_$port"
done
