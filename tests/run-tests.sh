#!/bin/bash
# wrapper to run entrance with env vars

cat_and_rm() {
    LOG="/var/log/entrance.log"

    if [[ -f ${LOG} ]]; then
        cat ${LOG}
        rm -v ${LOG}
        echo
        echo
    fi

    return 0
}

sleep_and_kill() {
    SLEEP=30

    echo -e "\e[1;35m${0} Going to sleep for ${SLEEP}\e[0m"
    sleep ${SLEEP}
    echo -e "\e[1;35m${0} Waking up\e[0m"

    killall -v entrance_client
    sleep 5
    killall -v entrance
    sleep 5

    return 0
}

if [[ ! -f /.dockerenv ]]; then
 	echo "Only meant to be run inside docker container"
 	exit 1
fi

echo -e "\e[1;35m${0} Begin Entrance Tests\e[0m"

/etc/init.d/dbus start

start-stop-daemon --start --verbose \
                --pidfile /run/elogind.pid \
                --exec /usr/lib/elogind/elogind -- --daemon

ps xa

# copy test dummy xorg.conf
cp -v "$(dirname $0)/xorg.conf" /etc/X11/

# create xsession, directory and desktop file
"$(dirname $0)/../utils/create_xsession.sh"

#Fix permissions for CI code coverage to allow entrance user/nobody access
umask 000
if [[ -d "./build" ]]; then
    find ./build/src -type d -exec chmod 7777 {} \;
    find ./build/src -type f -exec chmod 666 {} \;
fi

echo -e "\e[1;35m${0} Test Entrance Start\e[0m"

/usr/sbin/entrance &>/dev/null & disown

sleep_and_kill
cat_and_rm

echo -e "\e[1;35m${0} Additional client tests\e[0m"
export HOME=/tmp
# this is known to seg fault, likely need better solution for incorrect usage
/usr/lib/x86_64-linux-gnu/entrance/entrance_client

# change owner of files created by user "nobody"
chown root:root -R /entrance/build/src/bin

/usr/lib/x86_64-linux-gnu/entrance/entrance_client --help

cat_and_rm

echo -e "\e[1;35m${0} Test autologin\e[0m"
export HOME=/home/ubuntu
sed -i -e "s|autologin\" uchar: 0|autologin\" uchar: 1|" \
	-e "s|userlogin\" string: \"myusername\"|userlogin\" string: \"ubuntu\"|" \
	-e "s|Enlightenment|Xsession|" \
	/etc/entrance/entrance.conf

/usr/sbin/entrance &>/dev/null & disown

sleep_and_kill
cat_and_rm

echo -e "\e[1;35m${0} End Entrance Tests\e[0m"
