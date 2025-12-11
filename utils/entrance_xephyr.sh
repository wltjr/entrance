#!/bin/bash

SOCKET="/tmp/.ecore_service|entrance|43"

[[ -e "${SOCKET}" ]] && rm -v "${SOCKET}"

export XDG_RUNTIME_DIR="/tmp/ecore"

[[ ! -d "${XDG_RUNTIME_DIR}" ]] && mkdir -p "${XDG_RUNTIME_DIR}/.ecore"

# for both run and coverage
find build -type d -exec chmod 0777 {} \;

DPI=${DPI:-142}
SCREEN=${SCREEN:-1024x768}
ENTRANCE=${ENTRANCE:-/usr/sbin/entrance}

[[ -d build ]] && cd build

if [[ ! -f /usr/sbin/entrance ]]; then
	echo "/usr/sbin/entrance does not exist, run ./utils/local_build_docker.sh"
	exit 1
fi

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

#rm -f ~/.Xauthority
/usr/bin/Xephyr :1 -nolisten tcp -noreset -ac -br -dpi "${DPI}" -screen "${SCREEN}" &
sleep 1

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

rm -r "${XDG_RUNTIME_DIR}"
