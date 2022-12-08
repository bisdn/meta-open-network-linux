#!/bin/bash

echo port49 > /sys/bus/i2c/devices/2-0050/port_name
echo port50 > /sys/bus/i2c/devices/3-0050/port_name
echo port51 > /sys/bus/i2c/devices/4-0050/port_name
echo port52 > /sys/bus/i2c/devices/5-0050/port_name
echo port53 > /sys/bus/i2c/devices/6-0050/port_name
echo port54 > /sys/bus/i2c/devices/7-0050/port_name

# take out PHYs from reset
i2cset -y -f 0 0x30 0x19 0
i2cset -y -f 0 0x30 0x2a 0
