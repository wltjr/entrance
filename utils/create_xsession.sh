#!/bin/bash
# Create directory and xsession desktop file

DESKTOP_FILE=/usr/share/xsessions/Xsession.desktop

# create directory
for d in /usr/share/xsessions; do
	[[ ! -d "${d}" ]] && mkdir -pv "${d}"
done

# create desktop file
if [[ ! -f "${DESKTOP_FILE}"  ]]; then
    echo "[Desktop Entry]
Name=XSession
Comment=Xsession
Exec=/etc/entrance/Xsession /usr/bin/xeyes
TryExec=/etc/entrance/Xsession /usr/bin/xeyes
Icon=
Type=Application
" > "${DESKTOP_FILE}"
    echo "created ${DESKTOP_FILE}"
fi
