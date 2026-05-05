#!/usr/bin/env bash
set -euo pipefail

ARCH="${ARCH:-x64}"
MODE="${MODE:-dev}"
PROTOCOL="${PROTOCOL:-119}"
LEGACY_PROTOCOLS="${LEGACY_PROTOCOLS:-1}"
PORT="${PORT:-28962}"
FS_CDPATH="${FS_CDPATH:-$HOME/cod2}"
FS_HOMEPATH="${FS_HOMEPATH:-$HOME/.callofduty2}"
PASSWORD="${PASSWORD:-test}"
MAP="${MAP:-}"
GAMETYPE="${GAMETYPE:-dm}"

case "$ARCH" in
	x86)
		BIN="./bin/cod2rev_lnxded"
		;;
	x64)
		BIN="./bin/cod2rev_lnxded_x64"
		;;
	*)
		echo "Unsupported ARCH '$ARCH'. Use ARCH=x86 or ARCH=x64." >&2
		exit 2
		;;
esac

if [[ ! -x "$BIN" ]]; then
	echo "Binary '$BIN' was not found or is not executable." >&2
	echo "Build it first, for example: make ARCH=$ARCH" >&2
	exit 1
fi

COMMON_ARGS=(
	+set fs_cdpath "$FS_CDPATH"
	+set fs_homepath "$FS_HOMEPATH"
	+set protocol "$PROTOCOL"
	+set sv_protocolLegacyMode "$LEGACY_PROTOCOLS"
	+set dedicated 2
	+set net_port "$PORT"
	+set developer 1
	+set g_password "$PASSWORD"
	+set sv_punkbuster 0
	+pb_sv_disable
)

case "$MODE" in
	dev)
		MAP="${MAP:-mp_toujane}"
		ARGS=(
			"${COMMON_ARGS[@]}"
			+set fs_game test
			+set developer_script 1
			+set sv_maxclients 32
			+set scr_testclients 4
			+set scr_forcerespawn 0
			+set g_gametype tdm
			+set sv_cheats 1
			+set sv_cracked 1
			+devmap "$MAP"
		)
		;;
	zpam)
		MAP="${MAP:-mp_toujane_fix}"
		ARGS=(
			"${COMMON_ARGS[@]}"
			+set fs_game ""
			+set sv_pure 1
			+set g_gametype "$GAMETYPE"
			+set sv_wwwDownload 1
			+set sv_wwwBaseURL "http://cod2x.me/zpam"
			+exec server.cfg
			+map "$MAP"
		)
		;;
	*)
		echo "Unsupported MODE '$MODE'. Use MODE=dev or MODE=zpam." >&2
		exit 2
		;;
esac

echo "Running $BIN"
echo "Mode: $MODE, protocol: $PROTOCOL, legacy protocols: $LEGACY_PROTOCOLS"
exec "$BIN" "${ARGS[@]}"
