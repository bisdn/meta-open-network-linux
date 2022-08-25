#!/bin/sh

set -e

create_i2c_dev() {
  echo $1 $2 > /sys/bus/i2c/devices/i2c-${3}/new_device
}

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

add_port() {
	create_i2c_dev ${1} 0x50 ${3}
	wait_for_file "/sys/bus/i2c/devices/${3}-0050/port_name"
	echo "port${2}" > "/sys/bus/i2c/devices/${3}-0050/port_name"
}

# load modules
modprobe x86-64-accton-as4630-54pe-cpld
modprobe x86-64-accton-as4630-54pe-psu
modprobe x86-64-accton-as4630-54pe-leds

# init mux (PCA9548)
create_i2c_dev pca9548 0x77 1
create_i2c_dev pca9548 0x71 2
create_i2c_dev pca9548 0x70 3

# allow SFP LEDs to be controlled by MAC
OTHER=$(i2cget -y -f 0x3 0x60 0x86)
if [ "$(($OTHER & 0x80))" == "0" ]; then
	OTHER=$(($OTHER | 0x80))
	i2cset -y -f 0x3 0x60 0x86 $OTHER
fi

# add CPLD
create_i2c_dev as4630_54pe_cpld 0x60 3

# init LM77
create_i2c_dev lm77 0x48 14
create_i2c_dev lm75 0x4a 25
create_i2c_dev lm75 0x4b 24

# init PSU-1
create_i2c_dev as4630_54pe_psu1 0x50 10
create_i2c_dev ype1200am 0x58 10

# init PSU-2
create_i2c_dev as4630_54pe_psu2 0x51 11
create_i2c_dev ype1200am 0x59 11

for port in {49..52}; do
	add_port 'optoe2' $port $((port-31))
done

for port in {53..54}; do
	add_port 'optoe1' $port $((port-31))
done

# add EEPROM
create_i2c_dev 24c02 0x57 1

# add PoE MCU
create_i2c_dev as4630_54pe_poe_mcu 0x20 16
