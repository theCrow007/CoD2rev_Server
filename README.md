# CoD2rev_Server

## About

This is a reverse-engineered Call of Duty 2 dedicated server codebase.
It fixes several bugs in the original binaries and allows developers to extend server functionality.

The server can be built as either x86 or x64. The x64 build includes CoD2x-compatible server behavior for recent zPAM versions while keeping a runtime option for older vanilla clients.

## Build

Linux x86:

```bash
make ARCH=x86
```

Linux x64:

```bash
make ARCH=x64
```

The x64/CoD2x HTTP and websocket support links OpenSSL. On Debian/Ubuntu/WSL install it with:

```bash
sudo apt update
sudo apt install -y libssl-dev
```

If you changed shared headers such as `src/universal/q_shared.h`, force stale objects to rebuild:

```bash
rm -f obj/x86/common.o obj/x64/common.o bin/cod2rev_lnxded bin/cod2rev_lnxded_x64
make ARCH=x64
```

## Running

Use `run_bin.sh` for local launches. Defaults are x64, latest advertised protocol, and legacy protocol compatibility enabled.

```bash
./run_bin.sh
```

Common options:

```bash
# Run x64 with latest protocol advertised, allowing older clients.
ARCH=x64 PROTOCOL=119 LEGACY_PROTOCOLS=1 ./run_bin.sh

# Run x86.
ARCH=x86 ./run_bin.sh

# Advertise old protocol and only accept that exact protocol.
PROTOCOL=115 LEGACY_PROTOCOLS=0 ./run_bin.sh

# Latest protocol only.
PROTOCOL=119 LEGACY_PROTOCOLS=0 ./run_bin.sh
```

The runtime protocol controls are:

- `protocol`: protocol advertised in server info. Defaults to `119`.
- `sv_protocolLegacyMode`: when `1`, accept clients using protocol `115..119`; when `0`, only accept clients matching `protocol`.

The compile-time `PROTOCOL_VERSION` stays at `115` for compatibility code paths. Do not change it just to advertise a newer protocol.

## zPAM 4.06

zPAM 4.06 expects CoD2x-style server behavior and reads the `shortversion` dvar to detect old servers. This build advertises `PRODUCT_VERSION` as `1.4.6.8` while keeping per-client protocol compatibility.

Server auto-update follows the CoD2x update-server flow. It is enabled by default with `sv_update "1"` and sends a startup request for product `CoD2rev_Server`, update version `1.0`, and the current Linux binary architecture. If an update is available, the update response must include the download URL, new version, exact byte size, and SHA-256 hash. The server downloads the new binary in the background, verifies size and SHA-256, replaces the running binary on disk, and loads it after restart.

Restart behavior is controlled by `sv_updateRestart`: `0` stages the update only, `1` quits immediately after a verified update, and `2` quits when the server is empty. Use a shell loop or systemd `Restart=always` if you want modes `1` or `2` to come back automatically. Disable update checks with `sv_update "0"`. See `docs/cod2rev-update-release.md` for build, upload, hash, and update-response steps.

Install the zPAM files into the game `main` folder:

```text
main/zpam406.iwd
main/zpam_maps_v7.iwd
main/server.cfg
```

Run without a mod folder:

```bash
MODE=zpam ARCH=x64 PROTOCOL=119 LEGACY_PROTOCOLS=1 ./run_bin.sh
```

Fast download defaults are enabled for zPAM:

```cfg
set fs_game ""
set sv_wwwDownload "1"
set sv_wwwBaseURL "http://cod2x.me/zpam"
```

You can still override `sv_wwwBaseURL` in `server.cfg` if you host the files yourself.

For stock 1.3 maps, zPAM warns about old map versions. Use the fixed maps from `zpam_maps_v7.iwd`, for example `mp_toujane_fix`.

## Credits

Original CoD2rev_Server by voron00: https://github.com/voron00/CoD2rev_Server

id Software

Activision / Infinity Ward

CoD4x Server: https://github.com/callofduty4x/CoD4x_Server

OpenBO2: https://github.com/builtbyxeno/OpenBO2

CoD2: https://github.com/callofduty2x/CoD2x

zPAM https://github.com/eyza-cod2/zpam3
