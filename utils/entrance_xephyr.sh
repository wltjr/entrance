#!/bin/bash

# Ensure script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root (use sudo)" 
   exit 1
fi

# Use a separate socket for Xephyr test instance to avoid interfering with running entrance
SOCKET="/tmp/.ecore_service|entrance_xephyr|1"

[[ -e "${SOCKET}" ]] && rm -v "${SOCKET}"

# Use separate XDG_RUNTIME_DIR for test instance
export XDG_RUNTIME_DIR="/tmp/ecore_xephyr"

[[ ! -d "${XDG_RUNTIME_DIR}" ]] && mkdir -p "${XDG_RUNTIME_DIR}/.ecore"

# for both run and coverage
find build -type d -exec chmod 0777 {} \; 2>/dev/null

DPI=${DPI:-142}
SCREEN=${SCREEN:-1024x768}

# Determine entrance binary location
if [[ -f build/src/daemon/entrance ]]; then
	ENTRANCE=${ENTRANCE:-build/src/daemon/entrance}
	# Ensure binary is executable
	chmod +x "${ENTRANCE}"
elif [[ -f /usr/sbin/entrance ]]; then
	ENTRANCE=${ENTRANCE:-/usr/sbin/entrance}
else
	echo "ERROR: entrance binary not found. Run meson compile -C build first"
	exit 1
fi

echo "Using entrance binary: ${ENTRANCE}"

# Setup test config directory for entrance
TEST_CONFIG_DIR="build/test/entrance"
mkdir -p "${TEST_CONFIG_DIR}"
cp -f build/data/entrance.conf "${TEST_CONFIG_DIR}/" 2>/dev/null || \
	cp -f /etc/entrance/entrance.conf "${TEST_CONFIG_DIR}/" 2>/dev/null || \
	echo "WARNING: Could not find entrance.conf"

# Ensure proper permissions
chmod -R 0755 build/test 2>/dev/null

while :
do
	case "${1}" in
		-d | --dpi)
			DPI="${1}"
			shift
			;;
		-e | --entrance)
			ENTRANCE="${1}"
			shift
			;;
		-g | --gdb)
			GDB=1
			shift
			;;
		-s | --screen)
			SCREEN="${1}"
			shift
			;;
		-v | --valgrind)
			VALGRIND=1
			shift
			;;
		*)
			break
			;;

    	esac
done

# Assume dbus is already running on the host system
# (Xephyr entrance will use the existing session dbus)

# Ensure we have DISPLAY set for showing Xephyr window
if [[ -z "${DISPLAY}" ]]; then
    export DISPLAY=:0
    echo "WARNING: DISPLAY not set, defaulting to :0"
fi
HOST_DISPLAY="${DISPLAY}"

# Find available display number (lightdm often uses :1, so try :2, :3, etc.)
DISPLAY_NUM=2
while [[ -e /tmp/.X${DISPLAY_NUM}-lock ]]; do
    ((DISPLAY_NUM++))
    if [[ ${DISPLAY_NUM} -gt 10 ]]; then
        echo "ERROR: Could not find available display number"
        exit 1
    fi
done

# Kill any existing Xephyr on this display
pkill -f "Xephyr :${DISPLAY_NUM}" 2>/dev/null
sleep 0.5

#rm -f ~/.Xauthority
# Start Xephyr with HOST_DISPLAY so window appears on current desktop
DISPLAY="${HOST_DISPLAY}" /usr/bin/Xephyr :${DISPLAY_NUM} -nolisten tcp -noreset -ac -br -dpi "${DPI}" -screen "${SCREEN}" &
XEPHYR_PID=$!
sleep 1

# Check if Xephyr started successfully
if ! kill -0 ${XEPHYR_PID} 2>/dev/null; then
    echo "ERROR: Xephyr failed to start on display :${DISPLAY_NUM}"
    exit 1
fi

# Set DISPLAY to connect to Xephyr
export DISPLAY=:${DISPLAY_NUM}

# Disable Xinerama for Xephyr testing (it crashes in Xephyr)
export ENTRANCE_NO_XINERAMA=1

# Set client path for testing from build directory
if [[ -f build/src/bin/entrance_client ]]; then
    export ENTRANCE_CLIENT_PATH="${PWD}/build/src/bin/entrance_client"
    echo "Using entrance_client: ${ENTRANCE_CLIENT_PATH}"
fi

echo "Starting entrance on display ${DISPLAY}..."

if [[ ${GDB} ]]; then
	gdb --args "${ENTRANCE}" -x
elif [[ ${VALGRIND} ]]; then
	valgrind \
		--leak-check=full \
		--read-var-info=yes \
		--show-reachable=yes \
		--track-origins=yes \
		"${ENTRANCE}" -x
else
	"${ENTRANCE}" -x
fi

# Cleanup
kill ${XEPHYR_PID} 2>/dev/null
rm -rf "${XDG_RUNTIME_DIR}"
