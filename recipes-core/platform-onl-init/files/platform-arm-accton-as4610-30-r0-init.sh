#!/bin/bash

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
	local product_id="$(cat /sys/bus/i2c/drivers/as4610_54_cpld/0-0030/product_id)"
	case "$product_id" in
	"0"|"1")
		echo port25 > /sys/bus/i2c/devices/2-0050/port_name
		echo port26 > /sys/bus/i2c/devices/3-0050/port_name
		echo port27 > /sys/bus/i2c/devices/4-0050/port_name
		echo port28 > /sys/bus/i2c/devices/5-0050/port_name
		echo port29 > /sys/bus/i2c/devices/6-0050/port_name
		echo port30 > /sys/bus/i2c/devices/7-0050/port_name
		;;
	"2"|"3"|"5")
		echo port49 > /sys/bus/i2c/devices/2-0050/port_name
		echo port50 > /sys/bus/i2c/devices/3-0050/port_name
		echo port51 > /sys/bus/i2c/devices/4-0050/port_name
		echo port52 > /sys/bus/i2c/devices/5-0050/port_name
		echo port53 > /sys/bus/i2c/devices/6-0050/port_name
		echo port54 > /sys/bus/i2c/devices/7-0050/port_name
		;;
	*)
		echo "platform-as4610-init: unknown product id $product_id"
		return
		;;
	esac

	# take out PHYs from reset
	i2cset -y -f 0 0x30 0x19 0
	i2cset -y -f 0 0x30 0x2a 0
}
create_i2c_dev
