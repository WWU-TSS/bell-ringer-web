#!/bin/bash
if [ "a`whoami`" != "aroot" ]; then
    echo "must be root" 1>&2
    exit 1
fi
killall bell-ringer >& /dev/null
if [ "a$1" == "a" ]; then
    su bell-ringer -c "(bell-ringer -v &>> /home/bell-ringer/bell-ringer.log&)" 2> /dev/null
    echo "started bell ringer"
else
    su bell-ringer -c "(bell-ringer -vr $(($1)) &>> /home/bell-ringer/bell-ringer.log&)" 2> /dev/null
    echo "started bell ringer ringing $(($1)) times"
fi

