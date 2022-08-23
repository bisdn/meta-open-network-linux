#!/bin/sh

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

# PCA9547 modulize
wait_for_file /sys/bus/i2c/devices/i2c-1
create_i2c_dev 24lc64t 0x56 1

# load baseboard cpld module
modprobe questone2a_baseboard_cpld
