#!/usr/bin/bash
# SPDX-License-Identifier: GPL-2.0-only
#
# script for converting newer Yocto CVE format:
#
# CVE_STATUS[<CVE>] = "<status>"
#
# to older Yocto CVE format:
#
# # <status>
# CVE_CHECK_IGNORE += "<CVE>"
#
# with handling of multi-line statuses.
#
# Usage: $0 <input_file> > <output_file>
#
# Copyright (c) 2025 Jonas Gorski, BISDN GmbH

cont=0
cve=

while read -r line; do
	case "$line" in
	CVE_STATUS*)
		# strip out the CVE from CVE_STATUS[<CVE>]
		cve=$(echo $line | tr '[]' ' ' | cut -d' ' -f2)
		# strip out the CVE status and trip any quotes
		cve_status=$(echo $line | tr '[]' ' ' | cut -d' ' -f5- | tr -d '"')
		# if status ends in a backslash we have a multi-line status
		if [[ " $cve_status" == *\\ ]]; then
			# strip the trailing backslash
			cve_status=${cve_status:0: -1}
			cont=1
		fi

		# print status as comment
		echo "# $cve_status"
		;;
	"#"*|"")
		# leave comments and empty lines as-is
		echo "$line"
		;;
	*)
		# if we are in a multi-line comment, print the next line
		if [ "$cont" = 1 ]; then
			# strip any quotes
			cve_status=$(echo $line | tr -d '"')
			# if status ends in a backslash the multi-line status
			# continues
			if [[ "$cve_status" == *\\ ]]; then
				# strip the trailing backslash
				cve_status=${cve_status:0: -1}
			else
				# last line of the multi-line status
				cont=0
			fi
			# print status as comment
			echo "# $cve_status"
		fi
	esac

	# if we have a CVE and are not in a multi-line status, print the CVE
	if [ -n "$cve" ] && [ "$cont" = 0 ]; then
		echo "CVE_CHECK_IGNORE += \"$cve\""
		cve=
	fi
done < $1
