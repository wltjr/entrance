#!/bin/bash
# wrapper to run entrance with env vars

sleep_and_kill() {
    SLEEP=45

    echo -e "\e[1;35m${0} Going to sleep for ${SLEEP}\e[0m"
    sleep ${SLEEP}
    echo -e "\e[1;35m${0} Waking up\e[0m"

    killall -v entrance_client
    sleep 5
    killall -v entrance
    sleep 5
}

if [[ ! -f /.dockerenv ]]; then
 	echo "Only meant to be run inside docker container"
 	exit 1
fi

echo -e "\e[1;35m${0} Begin Entrance Tests\e[0m"

/etc/init.d/dbus start

for d in /usr/share/xsessions; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

echo "[Desktop Entry]
Name=XSession
Comment=Xsession
Exec=/etc/entrance/Xsession /usr/bin/xeyes
TryExec=/etc/entrance/Xsession /usr/bin/xeyes
Icon=
Type=Application
" > /usr/share/xsessions/Xsession.desktop

# this is temporary for CI till a better solution for all distros, ideally
sed -i -e "s|system-login|common-auth|" \
	/etc/pam.d/entrance

echo -e "\e[1;35m${0} Test Entrance Start\e[0m"

/usr/sbin/entrance &>/dev/null & disown

sleep_and_kill

echo -e "\e[1;35m${0} Additional client tests\e[0m"
export HOME=/tmp
/usr/lib/x86_64-linux-gnu/entrance/entrance_client
/usr/lib/x86_64-linux-gnu/entrance/entrance_client --help
export HOME=/home/ubuntu

echo -e "\e[1;35m${0} Test autologin\e[0m"
sed -i -e "s|autologin\" uchar: 0|autologin\" uchar: 1|" \
	-e "s|userlogin\" string: \"myusername\"|userlogin\" string: \"ubuntu\"|" \
	-e "s|Enlightenment|Xsession|" \
	/etc/entrance/entrance.conf

/usr/sbin/entrance &>/dev/null & disown

sleep_and_kill

echo -e "\e[1;35m${0} End Entrance Tests\e[0m"
