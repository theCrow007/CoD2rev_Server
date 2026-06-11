#!/usr/bin/env bash
#
# build.sh - interactive front-end for CoD2rev_Server.
#
# Plays the same role doit.sh plays for zk_libcod: it prompts for the MySQL
# variant, then drives the (non-interactive) Makefile with MYSQL_VARIANT=N.
# Keeping the prompt out here means `make` stays clean for CI/docker, which
# can just pass MYSQL_VARIANT=N directly.
#
# Usage:
#   ./build.sh                 # prompt for MySQL variant, then build
#   ./build.sh mysql2          # variant 2 (VoroN), no prompt
#   ./build.sh nomysql         # variant 0 (disabled), no prompt
#   ./build.sh mysql1 ARCH=x64 # variant + passthrough make args
#   ./build.sh clean           # clean, no prompt
#
# Any argument the script does not recognise as a variant selector is passed
# straight through to make (ARCH=x64, -j8, clean, etc.).

set -euo pipefail

variant=""          # empty => ask
passthrough=()

for arg in "$@"; do
	case "$arg" in
		nomysql|mysql0|MYSQL_VARIANT=0) variant=0 ;;
		mysql1|MYSQL_VARIANT=1)         variant=1 ;;
		mysql2|MYSQL_VARIANT=2)         variant=2 ;;
		clean)        variant=0; passthrough+=("clean") ;;  # no prompt for clean
		*)            passthrough+=("$arg") ;;
	esac
done

if [ -z "$variant" ]; then
	read -rsp $'\nChoose Your MySQL variant:\n
	0. MySQL disabled. (default)\n
	1. Default MySQL variant: A classic MySQL implementation
	made by kungfooman and IzNoGoD. Multiple connections, multiple threads,
	good for servers that use remote MySQL sessions, IRC stuff, and etc.\n
	2. VoroN'\''s MySQL variant (his own MySQL implementation). Native
	callbacks, native arguments, single connection, single thread, good
	for local MySQL session, less cpu usage, less memory usage.\n
	Press a key to continue...\n' -n1 key
	echo
	case "$key" in
		1) variant=1 ;;
		2) variant=2 ;;
		*) variant=0 ;;
	esac
fi

case "$variant" in
	0) echo ">>> MySQL: disabled" ;;
	1) echo ">>> MySQL: variant 1 (default / kungfooman + IzNoGoD)" ;;
	2) echo ">>> MySQL: variant 2 (VoroN)" ;;
esac

exec make MYSQL_VARIANT="$variant" "${passthrough[@]}"
