#!/bin/sh

onl_platform="$(cat /etc/onl/platform)"

[ -e "/usr/bin/platform-${onl_platform}-init.sh" ] && . /usr/bin/platform-${onl_platform}-init.sh
