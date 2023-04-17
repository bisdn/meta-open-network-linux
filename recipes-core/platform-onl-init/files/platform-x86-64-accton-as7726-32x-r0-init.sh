#!/bin/bash

# IR3570A chip casue problem when read eeprom by i2c-block mode.
# It happen when read 16th-byte offset that value is 0x8. So disable chip
ir3570_check() {
	i2cget -y 0 0x42 0x1 || return 1
	version="$(i2cdump -y 0 0x42 s 0x9a | tail -n 1 | awk '{ print $2 }')"
	if [ "$version" = "24" ]; then
		# disable IR3570A
		i2cget -y 0 0x42 0x1 || return 1
		i2cset -y 0 0x4 0xe5 0x01
		i2cset -y 0 0x4 0x12 0x02
	fi
}

setup_10g() {
	# Jitter Adjustment
	i2cset -y 16 0x18 0xff 0x4
	ic2set -y 16 0x18 0x15 0x54
	i2cset -y 16 0x18 0xff 0x5
	ic2set -y 16 0x18 0x15 0x54
	# Amplitude Adjustment
	i2cset -y 16 0x18 0xff 0x4
	ic2set -y 16 0x18 0x2d 0x81
	i2cset -y 16 0x18 0xff 0x5
	ic2set -y 16 0x18 0x2d 0x81

	# Check devices
	i2cget -y 14 0x58 0x00 b > /dev/null || return 1

	# VOD DEM EQ Adjustment
	i2cset -y 14 0x58 0x06 0x18
	i2cset -y 14 0x58 0x0f 0x00
	i2cset -y 14 0x58 0x16 0x00
	i2cset -y 14 0x58 0x17 0xaa
	i2cset -y 14 0x58 0x18 0x00
	i2cset -y 14 0x58 0x1d 0x00
	i2cset -y 14 0x58 0x24 0x00
	i2cset -y 14 0x58 0x25 0xaa
	i2cset -y 14 0x58 0x26 0x00
	i2cset -y 14 0x58 0x2c 0x00
	i2cset -y 14 0x58 0x2d 0xaa
	i2cset -y 14 0x58 0x2e 0x00
	i2cset -y 14 0x58 0x34 0xaa
	i2cset -y 14 0x58 0x35 0x00
	i2cset -y 14 0x58 0x3a 0x00
	i2cset -y 14 0x58 0x3b 0xaa
	i2cset -y 14 0x58 0x3c 0x00
	i2cset -y 14 0x58 0x41 0x00
	i2cset -y 14 0x58 0x42 0xaa
	i2cset -y 14 0x58 0x43 0x00

	if [ "$1" = "cpu" ]; then
		i2cset -y 14 0x58 0x5e 0x06
		i2cset -y 14 0x58 0x5f 0xf0
	elif [ "$1" = "sfp" ]; then
		i2cset -y 14 0x58 0x5e 0x06
		i2cset -y 14 0x58 0x5f 0x00
	else
		return 1
	fi
}

# make sure i2c-i801 is present
modprobe i2c-i801
wait_for_file /sys/bus/i2c/devices/i2c-0

# load modules
modprobe i2c-ismt
modprobe x86-64-accton-as7726-32x-cpld
modprobe x86-64-accton-as7726-32x-fan
modprobe x86-64-accton-as7726-32x-psu
modprobe x86-64-accton-as7726-32x-leds

# add PCA9548 muxes and enable disconnect on idle
create_i2c_dev pca9548 0x77 0 "idle_state=-2"
create_i2c_dev pca9548 0x76 1 "idle_state=-2"
create_i2c_dev pca9548 0x72 1 "idle_state=-2"
create_i2c_dev pca9548 0x73 1 "idle_state=-2"
create_i2c_dev pca9548 0x74 1 "idle_state=-2"
create_i2c_dev pca9548 0x75 1 "idle_state=-2"
create_i2c_dev pca9548 0x71 2 "idle_state=-2"

# add CPLD
create_i2c_dev as7726_32x_cpld1 0x60 11
create_i2c_dev as7726_32x_cpld2 0x62 12
create_i2c_dev as7726_32x_cpld3 0x63 13

# add FAN
create_i2c_dev as7726_32x_fan 0x66 54
create_i2c_dev lm75 0x4c 54
# add sensors
create_i2c_dev lm75 0x48 55
create_i2c_dev lm75 0x49 55
create_i2c_dev lm75 0x4a 55
create_i2c_dev lm75 0x4b 55

# add PSUs
create_i2c_dev as7726_32x_psu1 0x53 50
create_i2c_dev ym2651 0x5B 50

create_i2c_dev as7726_32x_psu2 0x50 49
create_i2c_dev ym2651 0x58 49

# Set multiplexer to SFP+
setup_10g "sfp"

# add QSFP28 ports
for port in {1..4}; do
	add_port 'optoe1' $port $((port + 20))
done

add_port 'optoe1' 5 26
add_port 'optoe1' 6 25
add_port 'optoe1' 7 28
add_port 'optoe1' 8 27

for port in {9..12}; do
	add_port 'optoe1' $port $((port + 8))
done
for port in {13..16}; do
	add_port 'optoe1' $port $((port + 16))
done
for port in {17..20}; do
	add_port 'optoe1' $port $((port + 16))
done
for port in {21..24}; do
	add_port 'optoe1' $port $((port + 24))
done
for port in {25..32}; do
	add_port 'optoe1' $port $((port + 12))
done

# add SFP+ ports
for port in {33..34}; do
	add_port 'optoe2' $port $((port - 18))
done

# add EEPROM
create_i2c_dev 24c02 0x56 0

ir3570_check
/usr/bin/8v89307_init.sh
