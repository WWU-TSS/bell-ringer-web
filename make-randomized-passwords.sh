#!/bin/bash
host_name="$(for i in $(hostname -I); do host "$i" | sed '{s/[^ ]\+ domain name pointer \(.*\)\./\1/p; s/.*//}' | grep "..*"; done | head -n 1)"
if [ "a$host_name" != "a" ]; then
    access_msg="You can change your password at https://$host_name/cgi-bin/change-password.cgi"
else
    access_msg=""
fi
for username in "$@"; do
    password="`tr -dc [:alnum:] < /dev/urandom | head -c 16`"
    printf "%s\n" "$username:$password"
    email_file="new-password-email-for-$username.txt"
    
    true > "$email_file"
    chmod 600 "$email_file"
    cat > "$email_file" <<EOF
Your new username and password for the Administration Building's bell are:
Username: $username
Password: $password

$access_msg
EOF
done
