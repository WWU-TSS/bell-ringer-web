#!/bin/bash
function is_package_installed
{
    if [ "a`dpkg-query -s "$1" | grep "Status: .*installed"`" != "a" ]; then
        return 0
    else
        return 1
    fi
}

function handle_error
{
    printf "error : "
    echo "$@" 1>&2
    echo "installation aborted" 1>&2
    exit 1
}

parallel_port="/dev/parport0"

for i in "$@"; do 
    if [ "a$i" == "a--help" -o "a$i" == "a-h" ]; then
        cat <<EOF
install.sh -- install bell-ringer web server
EOF
        exit 0
    fi
done
if [ "a`whoami`" != "aroot" ]; then
    echo "running sudo to install"
    exec sudo -- "$0" "$@"
fi
if [ "a$1" == "aparallel" ]; then
    port_type=parallel
elif [ "a$1" == "agpio" ]; then
    port_type=gpio
elif [ "a$1" != "a" -a "a$1" != "aauto" ]; then
    handle_error "invalid port type : $1"
elif [ -d "/sys/class/gpio" ]; then
    port_type=gpio
elif [ -c "$parallel_port" ]; then
    port_type=parallel
else
    handle_error "port not detected"
fi

if [ "$port_type" == "parallel" ]; then
    port_define=USE_PARALLEL_PORT
    port_group=lp
elif [ "$port_type" == "gpio" ]; then
    port_define=USE_GPIO
    port_group=gpio
fi

echo "using $port_type port"
apt-get install apache2 ssl-cert gcc realpath g++
is_package_installed apache2 || handle_error "apache2 not installed"
is_package_installed ssl-cert || handle_error "ssl-cert not installed"
is_package_installed gcc || handle_error "gcc not installed"
is_package_installed g++ || handle_error "g++ not installed"
is_package_installed realpath || handle_error "realpath not installed"
echo "setting up apache2 https server"
found=no
for i in /etc/apache2/sites-enabled/*; do
    if [ "`realpath "$i"`" == "`realpath /etc/apache2/sites-available/default-ssl`" ]; then
        found=yes
    fi
done
if [ $found == no ]; then
    ln -s /etc/apache2/sites-available/default-ssl /etc/apache2/sites-enabled/000-default-ssl
fi
found_ssl_conf=no
found_ssl_load=no
for i in /etc/apache2/mods-enabled/*; do
    if [ "`realpath "$i"`" == "`realpath /etc/apache2/mods-available/ssl.conf`" ]; then
        found_ssl_conf=yes
    elif [ "`realpath "$i"`" == "`realpath /etc/apache2/mods-available/ssl.load`" ]; then
        found_ssl_load=yes
    fi
done
if [ $found_ssl_conf == no ]; then
    ln -s /etc/apache2/mods-available/ssl.conf /etc/apache2/mods-enabled/ssl.conf
fi
if [ $found_ssl_load == no ]; then
    ln -s /etc/apache2/mods-available/ssl.load /etc/apache2/mods-enabled/ssl.load
fi
echo "adding bell-ringer user"
adduser --quiet --system --shell /bin/bash --home /home/bell-ringer bell-ringer || handle_error "user bell-ringer couldn't be added"
adduser --quiet bell-ringer $port_group || handle_error "user bell-ringer couldn't be added to group $port_group"
echo "compiling bell-user-administrate"
gcc bell-ringer-administrate.c -o bell-ringer-administrate || handle_error "can't compile bell-ringer-administrate.c"
echo "compiling bell-ringer"
g++ -std=c++0x "-D$port_define" bell-ringer.cpp parallel_port.cpp -o bell-ringer || handle_error "can't compile bell-ringer"
echo "installing bell-ringer"
install -T -m 6755 bell-ringer-administrate /usr/local/bin/bell-ringer-administrate
install -T bell-ringer /usr/local/bin/bell-ringer
install -T bell-ringer-administrate.sh /usr/local/bin/bell-ringer-administrate.sh
install -T start-bell-ringer.sh /usr/local/bin/start-bell-ringer.sh
install -T stop-bell-ringer.sh /usr/local/bin/stop-bell-ringer.sh
install -T bell-ringer.cgi /usr/lib/cgi-bin/bell-ringer.cgi
install -T change-password.cgi /usr/lib/cgi-bin/change-password.cgi
install -T -m 644 --backup=numbered index.html /var/www/index.html
touch /home/bell-ringer/bell-ringer.log
chown bell-ringer:nogroup /home/bell-ringer/bell-ringer.log
cat > /etc/cron.d/bell-ringer <<'EOF'
SHELL=/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin

@reboot root start-bell-ringer.sh
EOF
echo "restarting web server"
service apache2 reload
echo "starting bell ringer"
start-bell-ringer.sh
echo "finished installing"

