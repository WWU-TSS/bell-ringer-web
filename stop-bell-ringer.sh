#!/bin/bash
if [ "a`whoami`" != "aroot" ]; then
    echo "must be root" 1>&2
    exit 1
fi
killall bell-ringer >& /dev/null
echo "stopped bell ringer"
