#!/bin/bash

echo port25 > /sys/bus/i2c/devices/2-0050/port_name
echo port26 > /sys/bus/i2c/devices/3-0050/port_name
echo port27 > /sys/bus/i2c/devices/4-0050/port_name
echo port28 > /sys/bus/i2c/devices/5-0050/port_name
echo port29 > /sys/bus/i2c/devices/6-0050/port_name
echo port30 > /sys/bus/i2c/devices/7-0050/port_name

# take out PHYs from reset
i2cset -y -f 0 0x30 0x19 0
i2cset -y -f 0 0x30 0x2a 0
