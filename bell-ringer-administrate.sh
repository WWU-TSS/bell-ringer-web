#!/bin/bash
function get_users_in_group
{
    local group_name="$1"
    grep "^$group_name:" < /etc/group | sed '{s/[^:]*:[^:]*:[^:]*:\([^:]*\)/\1/p; s/.*//}' | sed '{s/,/ /g}' | grep "."
}

if [ "a`whoami`" != "aroot" ]; then
    echo "must be run as root" 1>&2
    exit 1
fi
valid_users="`get_users_in_group "adm"`"
found=0
if [ "a$1" != "abell" -a "a$1" != "apasswd" ]; then
    echo "invalid action class" 1>&2
    exit 1
fi
action_class="$1"
for i in $valid_users; do
    if [ "a$i" == "a$2" -a "a$2" != "a" ]; then
        found=1
    fi
done
if [ $found == 0 ]; then
    echo "invalid user name or password" 1>&2
    exit 1
fi
if [ $action_class == bell ]; then
    if [ "a$3" != "astart" -a "a$3" != "astop" ]; then
        echo "invalid action" 1>&2
        exit 1
    fi
    arg=""
    if [ "a$4" != "a" -a "$3" == "start" ]; then
        arg="$(($4))"
    fi
    su "$2" -c "sudo -k; sudo -S -- bash -c \"(${3}-bell-ringer.sh $arg 2>&1); exit 0\"" 2> /dev/null || echo "invalid user name or password" 1>&2
elif [ $action_class == passwd ]; then
    read old_password 
    read new_password
    read new_password2
    invalid_character_set=':'
    if [ "a$new_password" == "a" ]; then
        echo "empty password" 1>&2
        exit 1
    elif [ "a`printf "%s\n" "$new_password" | grep "[$invalid_character_set]"`" != "a" ]; then
        echo "invalid character in password. invalid characters '$invalid_character_set'" 1>&2
        exit 1
    elif [ "a$new_password" != "a$new_password2" ]; then
        echo "new passwords don't match" 1>&2
        exit 1
    else
        printf "%s\n" "$old_password" "$2:$new_password" | su "$2" -c "sudo -k; sudo -S -- bash -c \"(chpasswd 2>&1 && echo password changed successfully); exit 0\"" 2> /dev/null || echo "invalid user name or password" 1>&2
    fi
else
    echo "invalid action class 2" 1>&2
    exit 1
fi
