#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="${COD2_DOCKER_IMAGE:-cod2rev-server:x64}"
HOST_PORT="${COD2_PORT:-28962}"
GAME_PATH="${1:-${COD2_GAME_PATH:-}}"
HOME_PATH="${COD2_HOME_PATH:-$PWD/.docker-home}"
SERVER_PARAMS="${COD2_PARAMS:-+set fs_cdpath /game +set fs_homepath /home/cod2/.callofduty2 +set dedicated 2 +set net_port 28962 +set developer 1 +set sv_maxclients 32 +set g_gametype tdm +set sv_cheats 1 +set sv_cracked 1 +devmap mp_toujane}"

if [[ -z "$GAME_PATH" ]]; then
	echo "Usage: $0 /path/to/call-of-duty-2-data"
	echo
	echo "The game data path should contain the CoD2 assets, usually including a main/ directory with .iwd files."
	echo "You can also set COD2_GAME_PATH instead of passing an argument."
	exit 2
fi

if [[ ! -d "$GAME_PATH" ]]; then
	echo "Game data path does not exist: $GAME_PATH"
	exit 2
fi

mkdir -p "$HOME_PATH"

docker build --platform linux/amd64 -t "$IMAGE_NAME" .
docker run --rm -it \
	--platform linux/amd64 \
	-p "$HOST_PORT:28962/udp" \
	-v "$GAME_PATH:/game:ro" \
	-v "$HOME_PATH:/home/cod2/.callofduty2" \
	-e "COD2_PARAMS=$SERVER_PARAMS" \
	"$IMAGE_NAME"
