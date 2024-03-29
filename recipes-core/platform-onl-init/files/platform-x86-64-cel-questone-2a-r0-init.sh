#!/bin/sh

# make sure i2c-i801 is present
modprobe i2c-i801
wait_for_file /sys/bus/i2c/devices/i2c-0

# load modules
modprobe i2c-ismt

# PCA9547 modulize
create_i2c_dev 24lc64t 0x56 1

# load baseboard cpld module
modprobe questone2a_baseboard_cpld
