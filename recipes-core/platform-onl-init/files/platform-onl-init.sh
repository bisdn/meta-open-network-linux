#!/bin/bash

add_port() {
	local i2c_dev_name=$1
	local portnum=$2
	local i2c_bus_no=$3

	create_i2c_dev ${i2c_dev_name} 0x50 ${i2c_bus_no}
	wait_for_file "/sys/bus/i2c/devices/${i2c_bus_no}-0050/port_name"
	echo "port${portnum}" > "/sys/bus/i2c/devices/${i2c_bus_no}-0050/port_name"
}

create_i2c_dev() {
	local i2c_dev_name=$1
	local i2c_addr=$2
	local i2c_bus_no=$3

	wait_for_file "/sys/bus/i2c/devices/i2c-${i2c_bus_no}/new_device"
	echo ${i2c_dev_name} ${i2c_addr} > /sys/bus/i2c/devices/i2c-${i2c_bus_no}/new_device
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
