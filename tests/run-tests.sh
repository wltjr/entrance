#!/bin/bash
# wrapper to run entrance with env vars

if [[ ! -f /.dockerenv ]]; then
 	echo "Only meant to be run inside docker container"
 	exit 1
fi

echo -e "\e[1;35m${0} Begin Entrance Tests\e[0m"

/etc/init.d/dbus start

export XDG_RUNTIME_DIR="/tmp/ecore"

for d in "${XDG_RUNTIME_DIR}"{,/.ecore} /usr/share/xsessions; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

echo "[Desktop Entry]
Name=XSession
Comment=Xsession
Exec=/etc/entrance/Xsession
TryExec=/etc/entrance/Xsession
Icon=
Type=Application
" > /usr/share/xsessions/Xsession.desktop

sed -i -e "s|nobody|ubuntu|" /etc/entrance/entrance.conf

echo -e "\e[1;35m${0} Test Entrance Start\e[0m"

/usr/sbin/entrance &>/dev/null & disown

SLEEP=90

echo -e "\e[1;35m${0} Going to sleep for ${SLEEP}\e[0m"
sleep ${SLEEP}
echo -e "\e[1;35m${0} Waking up\e[0m"

killall -v entrance_client

sleep 5

killall -v entrance

sleep 5

echo -e "\e[1;35m${0} Additional client tests\e[0m"
export HOME=/tmp
/usr/lib/x86_64-linux-gnu/entrance/entrance_client
/usr/lib/x86_64-linux-gnu/entrance/entrance_client --help
export HOME=/home/ubuntu

echo -e "\e[1;35m${0} Test autologin\e[0m"
useradd -g users -m -p 1234 -s /bin/bash myusername
sed -i -e "s|autologin\" uchar: 0|autologin\" uchar: 1|" \
	/etc/entrance/entrance.conf

/usr/sbin/entrance &>/dev/null & disown

SLEEP=60

echo -e "\e[1;35m${0} Going to sleep for ${SLEEP}\e[0m"
sleep ${SLEEP}
echo -e "\e[1;35m${0} Waking up\e[0m"

killall -v entrance_client
killall -v entrance

echo -e "\e[1;35m${0} End Entrance Tests\e[0m"
