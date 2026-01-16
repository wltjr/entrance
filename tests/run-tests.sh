#!/bin/bash
# wrapper to run entrance with env vars

cat_and_rm() {
    LOG="/var/log/entrance.log"

    if [[ -f ${LOG} ]]; then
        cat ${LOG}
        rm ${LOG}
        echo
        echo
    fi

    return 0
}

sleep_and_kill() {
    SLEEP=45

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

start-stop-daemon --start --quiet \
                --pidfile /run/elogind.pid \
                --exec /usr/lib/elogind/elogind -- --daemon

ps xa

# copy test dummy xorg.conf
cp -v "$(dirname $0)/xorg.conf" /etc/X11/

# create xsession, directory and desktop file
"$(dirname $0)/../utils/create_xsession.sh"

# Fix permissions for CI build directory to allow entrance user access
if [[ -d "./build" ]]; then
    chmod -R 777 ./build
fi

echo -e "\e[1;35m${0} Test Entrance Start\e[0m"

/usr/sbin/entrance &>/dev/null & disown

sleep_and_kill
cat_and_rm

echo -e "\e[1;35m${0} Additional client tests\e[0m"
export HOME=/tmp
/usr/lib/x86_64-linux-gnu/entrance/entrance_client
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
