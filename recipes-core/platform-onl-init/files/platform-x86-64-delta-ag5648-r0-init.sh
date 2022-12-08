#!/bin/bash

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

# Occasionally the management interface goes missing, a reboot seems to fix it.
# Until we can find the root cause, check for its presence and reboot if not found.

MGMT_STATE="FAILURE"
wait_for_file /sys/class/net/enp0s20 &&	MGMT_STATE="SUCCESS"

[ -f "/var/last_boot.log" ] && LAST_STATE="$(tail -n 1 /var/last_boot.log | awk '{print $1;}')"
echo "$MGMT_STATE $(date)" >> /var/last_boot.log

if [ "$MGMT_STATE" = "FAILURE" ]; then
	if [ "$LAST_STATE" != "FAILURE" ]; then
		echo "Management interface missing, rebooting now..."
		reboot
	else
		echo "Persistent management interface failure, refusing to reboot."
	fi
else
	echo "Management interface present."
fi

# make sure i2c-i801 is present
wait_for_file /sys/bus/i2c/devices/i2c-0

# load modules
modprobe i2c-ismt

# PCA9547 modulize
wait_for_file /sys/bus/i2c/devices/i2c-1
create_i2c_dev pca9548 0x70 1

# Insert swpld module and create devs
modprobe i2c_cpld
wait_for_file /sys/bus/i2c/devices/i2c-2
create_i2c_dev cpld 0x31 2
create_i2c_dev cpld 0x35 2
create_i2c_dev cpld 0x39 2

# IDEEPROM modulize
create_i2c_dev 24c02 0x53 2

# Insert psu module
modprobe dni_ag5648_psu
create_i2c_dev dni_ag5648_psu 0x58 6
create_i2c_dev dni_ag5648_psu 0x59 6

# insert fan module
modprobe dni_emc2305
create_i2c_dev emc2305 0x4d 3
create_i2c_dev emc2305 0x4d 5

# Insert temperature modules
create_i2c_dev tmp75 0x4d 2
create_i2c_dev tmp75 0x49 3
create_i2c_dev tmp75 0x4b 3
create_i2c_dev tmp75 0x4c 3
create_i2c_dev tmp75 0x4e 3
create_i2c_dev tmp75 0x4f 3

# Insert sfp module
modprobe dni_ag5648_sfp
echo 0x18 > /sys/bus/i2c/devices/2-0035/data
create_i2c_dev dni_ag5648_sfp 0x50 4

# set front panel sys light
echo 0x04 > /sys/bus/i2c/devices/2-0039/addr
echo 0x10 > /sys/bus/i2c/devices/2-0039/data

# set thermal Thigh & Tlow
echo 80000 > /sys/class/hwmon/hwmon5/temp1_max
echo 70000 > /sys/class/hwmon/hwmon6/temp1_max
echo 60000 > /sys/class/hwmon/hwmon7/temp1_max
echo 85000 > /sys/class/hwmon/hwmon8/temp1_max
echo 65000 > /sys/class/hwmon/hwmon9/temp1_max
echo 60000 > /sys/class/hwmon/hwmon10/temp1_max

echo 75000 > /sys/class/hwmon/hwmon5/temp1_max_hyst
echo 65000 > /sys/class/hwmon/hwmon6/temp1_max_hyst
echo 55000 > /sys/class/hwmon/hwmon7/temp1_max_hyst
echo 80000 > /sys/class/hwmon/hwmon8/temp1_max_hyst
echo 60000 > /sys/class/hwmon/hwmon9/temp1_max_hyst
echo 55000 > /sys/class/hwmon/hwmon10/temp1_max_hyst

# tx enable for optical sfp
i2cset -f -y 2 0x39 0x0d 0x00
i2cset -f -y 2 0x39 0x0e 0x00
i2cset -f -y 2 0x39 0x0f 0x00
i2cset -f -y 2 0x39 0x10 0x00
i2cset -f -y 2 0x39 0x11 0xf0
i2cset -f -y 2 0x35 0x0a 0x00
i2cset -f -y 2 0x35 0x0b 0x0f

# disable qsfp low power mode
i2cset -f -y 2 0x35 0x11 0xc0
# take qsfp out of reset
i2cset -f -y 2 0x35 0x13 0xff
