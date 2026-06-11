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
#   ./build.sh clean           # remove ALL build artifacts (every arch), no prompt
#
# Any argument the script does not recognise as a variant selector or a clean
# request is passed straight through to make (ARCH=x64, -j8, etc.).
#
# `clean` (also `cleanall` / `distclean`) is handled directly here rather than
# delegated to `make clean`, because the Makefile's clean is arch-scoped and
# would otherwise leave the non-default architecture's objects and binary
# behind (e.g. obj/x64 and bin/cod2rev_lnxded_x64).

set -euo pipefail

# Always operate relative to the repo root (where this script lives), so clean
# works regardless of the caller's working directory.
cd "$(dirname "$(readlink -f "$0")")"

variant=""          # empty => ask
do_clean=0
passthrough=()

for arg in "$@"; do
	case "$arg" in
		nomysql|mysql0|MYSQL_VARIANT=0) variant=0 ;;
		mysql1|MYSQL_VARIANT=1)         variant=1 ;;
		mysql2|MYSQL_VARIANT=2)         variant=2 ;;
		clean|cleanall|distclean)       do_clean=1 ;;  # thorough, all arches
		*)            passthrough+=("$arg") ;;
	esac
done

# Thorough clean: remove every build artifact for ALL architectures, not just
# the default x86. The Makefile's own `clean` is arch-scoped (OBJ_DIR=obj/$ARCH
# and the binary carries an arch suffix), so invoking it here would leave e.g.
# obj/x64 and bin/cod2rev_lnxded_x64 behind. This wipes them all.
if [ "$do_clean" -eq 1 ]; then
	echo ">>> Cleaning all build artifacts (all architectures)"
	# All per-arch object trees (obj/x86, obj/x64, ...).
	rm -rf obj
	# All binary + shared-lib variants, both platforms and both arch suffixes.
	rm -f bin/cod2rev_lnxded bin/cod2rev_lnxded_x64 \
	      bin/cod2rev_win32.exe bin/cod2rev_win32_x64.exe \
	      bin/libcod2rev.so bin/libcod2rev.dll \
	      bin/libmysql.dll
	echo ">>> Clean complete"
	exit 0
fi

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
