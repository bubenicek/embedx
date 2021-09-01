#!/bin/bash

print_usage()
{
cat << EOF
Run cyclic test using provided script/command with delay between the cycles

Usage $0 <command.sh> <sleep_in_seconds>

EOF
}

if [[ "$#" -lt 2 ]]; then
    print_usage;
    exit 1
fi

COMMAND=$1
SLEEP=$2

while [ 1 ]; do
	echo 
	$COMMAND 
	sleep $SLEEP
done
