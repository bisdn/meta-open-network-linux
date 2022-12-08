#!/bin/bash

add_port() {
	local i2c_dev_name=$1
	local portnum=$2
	local i2c_bus_no=$3

	create_i2c_dev ${i2c_dev_name} 0x50 ${i2c_bus_no} "port_name=port${portnum}"
}

create_i2c_dev() {
	local i2c_dev_name=$1
	local i2c_addr=$2
	local i2c_bus_no=$3
	local i2c_addr_4="00${i2c_addr:2}"

	wait_for_file "/sys/bus/i2c/devices/i2c-${i2c_bus_no}/new_device"
	echo ${i2c_dev_name} ${i2c_addr} > /sys/bus/i2c/devices/i2c-${i2c_bus_no}/new_device

	# apply additional parameters passed as <parameter>=<value> strings
	while [ "$#" -gt 3 ]; do
		local parameter="${4%%=*}"
		local value="${4#*=}"
		wait_for_file "/sys/bus/i2c/devices/${i2c_bus_no}-${i2c_addr_4}/${parameter}"
		echo "$value" > "/sys/bus/i2c/devices/${i2c_bus_no}-${i2c_addr_4}/${parameter}"
		shift
	done
}

wait_for_file() {
	local FILE=$1
	local i=0

	while [ $i -lt 10 ]; do
		test -e $FILE && return 0
		i=$((i + 1))
		sleep 1
	done
	return 1
}

onl_platform="$(cat /etc/onl/platform)"

[ -e "/usr/bin/platform-${onl_platform}-init.sh" ] && . /usr/bin/platform-${onl_platform}-init.sh
