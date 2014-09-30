#!/bin/bash
host_name="$(for i in $(hostname -I); do host "$i" | sed '{s/[^ ]\+ domain name pointer \(.*\)\./\1/p; s/.*//}' | grep "..*"; done | head -n 1)"
if [ "a$host_name" != "a" ]; then
    url="https://$host_name/cgi-bin/change-password.cgi"
    access_msg="You can change your password at $url"
    echo "Change password URL: $url"
else
    url=""
    access_msg=""
fi
if [ -z "$1" ]; then
    echo "usage: sudo $0 <new user name> [<new user name> [...]]" >&2
    exit 1
elif [ "`whoami`" != "root" ]; then
    echo "must be root." >&2
    exit 1
fi
for username in "$@"; do
    echo "$username"
    adduser --disabled-password "$username" || exit 1
    for i in adm dialout sudo audio video plugdev games users netdev input spi gpio; do
        adduser "$username" "$i"
    done
    password="`tr -dc [:alnum:] < /dev/urandom | head -c 16`"
    printf "%s\n" "$username:$password" | chpasswd
    email_file="new-password-email-for-$username.txt"
    
    touch "$email_file"
    chmod 600 "$email_file"
    if [ "a$SUDO_USER" != "a" ]; then
    chown "$SUDO_USER:$SUDO_USER" "$email_file"
    cat > "$email_file" <<EOF
Your new username and password for the Administration Building's bell are:
Username: $username
Password: $password

$access_msg
EOF
    echo "email contents of $email_file to $username" >&2
done

