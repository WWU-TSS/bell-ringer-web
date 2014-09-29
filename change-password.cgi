#!/bin/bash

function unescape
{
    local s="$1"
    while [ "a$s" != "a" ]; do
        if [ "a${s:0:1}" == "a%" -a "a`echo "${s:1:2}" | sed '{s/\([0-9a-fA-F]\)/\1/gp; s/.*//}'`" == "a${s:1:2}" ]; then
            printf "\x${s:1:2}"
            s="${s:3}"
        else
            printf "%s" "${s:0:1}"
            s="${s:1}"
        fi
    done
}

function escape
{
    local s="$1"
    while [ "a$s" != "a" ]; do
        if [ "a`echo "${s:0:1}" | grep "[a-zA-Z0-9 ]"`" != "a" ]; then
            printf "%s" "${s:0:1}"
        else            
            printf "%s" "${s:0:1}" | hexdump -e '/1 "&#%02X;"'
        fi
        s="${s:1}"
    done
}

function output_content
{
    cat <<EOF
Content-Type: text/html; charset=UTF-8

<!DOCTYPE html>
<html>
<head>
<title>Administration Building Bell Ringer - Change Password</title>
</head>
<body>
<div style="margin-top: 10%; margin-left: 30%; margin-right: 30%; background-color: #E5E2E0; text-align: center">
<span><a href="bell-ringer.cgi">Main Page</a> | Set Password</span>
<h2>Bell Ringer - Change Password</h2><br/>
${status_message}
<form action="change-password.cgi" method="POST">
Username: <input type="text" name="username" value="$username"/><br/>
Old Password: <input type="password" name="password"></input><br/>
New Password: <input type="password" name="password1"></input><br/>
Reenter New Password: <input type="password" name="password2"></input><br/>
<input type="submit" value="Change Password"/>
</form>
</div>
</body>

EOF
}

if [ "${HTTPS:=off}" == "off" ]; then
    printf "Location: https://%s%s\n\n" "${HTTP_HOST}" "${REQUEST_URI}"
    exit 0
fi
if [ "${REQUEST_METHOD:=GET}" == "GET" ]; then
    output_content
    exit 0
fi
sed '{s/\([-_a-z0-9A-Z]*\)=\([^&=]*\)&*/\1=\2\n/gp; s/.*//}' | grep "..*" | (
    while read input_line; do
        var_name="`echo "$input_line" | sed '{s/\([^=]*\)=.*/\1/p; s/.*//}' | grep "..*"`"
        if [ "a$var_name" != "a" ]; then
            var_value="$(unescape "`echo "$input_line" | sed '{s/[^=]*=\(.*\)/\1/}'`")"
            if [ "a$var_name" == "ausername" ]; then
                username="$var_value"
            elif [ "a$var_name" == "apassword" ]; then
                password="$var_value"
            elif [ "a$var_name" == "apassword1" ]; then
                password1="$var_value"
            elif [ "a$var_name" == "apassword2" ]; then
                password2="$var_value"
            fi
        fi
    done
    status_message="`printf "%s\n" "$password" "$password1" "$password2" | (bell-ringer-administrate passwd "$username" 2>&1) | (while read s; do printf "%s<br/>" "$s"; done)`"
    if [ "a$status_message" != "a" ]; then
        status_message="<p>$status_message</p><br/>"
    fi
    sleep 0.5
    output_content
)
