#!/bin/bash

set_clocksouce() {
	echo tsc > /sys/devices/system/clocksource/clocksource0/current_clocksource
}

enable_tx() {
	# Set Module TX-Disable Registers
	i2cset -y 2 0x33 0x08 0x00
	i2cset -y 2 0x33 0x09 0x00
	i2cset -y 2 0x33 0x0a 0x00
	i2cset -y 2 0x33 0x0b 0x00
	i2cset -y 2 0x33 0x0c 0x00
	i2cset -y 2 0x33 0x0d 0x00
}

enable_qsfp() {
	# disable LP Mode
	i2cset -y 2 0x32 0x0b 0xc0
	# clear ResetL
	i2cset -y 2 0x32 0x0d 0x3f
}

BOOT_STATE="success"
wait_for_file /sys/class/net/enp0s20f0 || BOOT_STATE="failure"
if [ "$BOOT_STATE" != "success" ]; then
	echo "Management interface enp0s20f0 is missing."
else
	PCS_LSTS=$(ethtool -d enp0s20f0 | grep PCS_LSTS | awk '{ print $NF }')
	if [ "$(($PCS_LSTS & 0x10))" = "0" ]; then
		# PHY did not succesfully sync with MAC
		echo "PHY of enp0s20f0 failed to sync (PCS_LSTS=$PCS_LSTS)"
		BOOT_STATE="failure"
	else
		echo "PHY of enp0s20f0 synced successfully."
	fi
fi

[ -f /var/phy_state_last_boot ] && LAST_BOOT_STATE=$(cat /var/phy_state_last_boot)
echo "$BOOT_STATE" > /var/phy_state_last_boot

if [ "$BOOT_STATE" != "success" ]; then
	if [ "$LAST_BOOT_STATE" != "failure" ]; then
		echo "rebooting now ..."
		reboot
	else
		echo "refusing to reboot due to persistent failure."
	fi
fi

# make sure i2c-i801 is present
modprobe i2c-i801
wait_for_file /sys/bus/i2c/devices/i2c-0

# load modules
modprobe i2c-ismt

wait_for_file /sys/bus/i2c/devices/0-0069 && set_clocksouce
wait_for_file /sys/bus/i2c/devices/i2c-2 && enable_tx && enable_qsfp
