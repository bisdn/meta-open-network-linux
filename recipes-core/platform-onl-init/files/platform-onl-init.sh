#!/bin/bash

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
