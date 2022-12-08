#!/bin/bash

add_port() {
	create_i2c_dev ${1} 0x50 ${3}
	wait_for_file "/sys/bus/i2c/devices/${3}-0050/port_name"
	echo "port${1}" > "/sys/bus/i2c/devices/${3}-0050/port_name"
}

create_i2c_dev() {
	echo $1 $2 > /sys/bus/i2c/devices/i2c-${3}/new_device
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
