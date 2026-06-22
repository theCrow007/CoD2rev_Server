# zk_libcod → CoD2rev_Server — Port Report

Generated from the staged source of record (`gsc.cpp` registration tables, the `gsc_zk_*` feature modules, and `libcod.cpp`'s `RegisterLibcodDvars()`). This lists the **zk feature set ported onto rev's native engine-binding layer** — the GSC builtins now callable from script and the dvars now registered. rev's own pre-existing libcod modules (MySQL, SQLite, HTTP, memory, exec, match, websocket) and rev/cod2x-native player/entity builtins are **not** counted here; several of those received x64 pointer-correctness fixes but were not part of the zk feature port.

## Summary

- **GSC builtins ported (zk feature modules): 237** — 50 functions + 187 methods.
- **Dvars ported: 82** registered in `RegisterLibcodDvars()` (plus a few at native engine sites, e.g. `sv_minimizeSysteminfo`).
- **Builtin coverage vs zk:** 353 / 356 (the 3 unported are sound-alias *metadata* builtins, blocked on rev's deliberately-absent sound-alias subsystem).
- **Build switch:** `NOSPEEX=1` (or `./build.sh nomysql nospeex`) drops libspeex + the 8 sound-file builtins.

| Module | Functions | Methods |
|---|---:|---:|
| `gsc_zk_player.cpp` — Player methods | 0 | 128 |
| `gsc_zk_entity.cpp` — Entity methods | 0 | 33 |
| `gsc_zk_weapons.cpp` — Weapon functions/methods | 11 | 1 |
| `gsc_zk_level.cpp` — Level functions | 5 | 0 |
| `gsc_zk_utils.cpp` — Utility functions | 28 | 0 |
| `gsc_zk_sound.cpp` — Custom voice / speex (this phase) | 4 | 4 |
| `gsc_bots.cpp` — Bot AI methods/functions | 2 | 21 |
| **Total** | **50** | **187** |

## Part A — Functions & Methods Ported

> In libcod terms a **function** is a free builtin (registered in `scriptFunctions[]`); a **method** takes an entity reference (`scriptMethods[]`, called as `<ent> method(...)`).

### Player methods  — `gsc_zk_player.cpp`
*Player state, input/buttons, weapons, movement, fx, snapshots, objectives, client control.*  (0 functions, 128 methods)

**Methods:**

`addEntToSnapshots`, `aimButtonPressed`, `backButtonPressed`, `canMantle`, `canUseTurret`, `clientHasClientMuted`, `connectionlessPacketToClient`, `connectionlessPacketToServer`, `disableBulletDrop`, `disableBulletImpacts`, `disableEarthquakes`, `disableSilent`, `disableTalkerIcon`, `earthquakeForPlayer`, `enableBulletDrop`, `enableBulletImpacts`, `enableEarthquakes`, `enableSilent`, `enableTalkerIcon`, `executeClientCommand`, `forceShot`, `forwardButtonPressed`, `fragButtonPressed`, `getAddressType`, `getBulletMask`, `getClientConnectState`, `getClientHudElemCount`, `getCollisionTeam`, `getCurrentOffhandSlotAmmo`, `getCurrentWeaponAmmo`, `getCurrentWeaponClipAmmo`, `getCurrentWeaponSlot`, `getEnterTime`, `getGravity`, `getGroundEntity`, `getInactivityTime`, `getJumpSlowdownTimer`, `getLastConnectTime`, `getLastMsg`, `getNumberOfEntsInSnapshot`, `getPlayerStateFlags`, `getProtocolString`, `getServerCommandQueueSize`, `getSpectatorClient`, `getSpeed`, `getUserinfo`, `getWeaponAnimation`, `holdBreathButtonPressed`, `isAllowingSpectators`, `isBot`, `isChatting`, `isFiring`, `isHiddenFromScoreboard`, `isHiddenFromServerStatus`, `isHoldingWeaponDown`, `isMeleeing`, `isRechambering`, `isReloading`, `isShellshocked`, `isThrowingGrenade`, `isUseTouching`, `isUsingBinoculars`, `jumpButtonPressed`, `leanLeftButtonPressed`, `leanRightButtonPressed`, `leftButtonPressed`, `lookAtKiller`, `muteClient`, `noclip`, `objective_player_add`, `objective_player_delete`, `objective_player_icon`, `objective_player_position`, `objective_player_state`, `overrideContents`, `playFxForPlayer`, `playFxOnTagForPlayer`, `playScriptAnimation`, `processClientCommand`, `processClientUserinfoChange`, `processSuicide`, `reloadButtonPressed`, `removeEntFromSnapshots`, `renameClient`, `resetNextReliableTime`, `rightButtonPressed`, `setActivateOnUseButtonRelease`, `setAllowSpectators`, `setAnimation`, `setBulletDrag`, `setBulletMask`, `setBulletModel`, `setBulletVelocity`, `setCollisionTeam`, `setConfigStringForPlayer`, `setCullFogForPlayer`, `setCurrentWeaponAmmo`, `setCurrentWeaponClipAmmo`, `setExpFogForPlayer`, `setFireRangeScale`, `setFireThroughWalls`, `setGravity`, `setGuid`, `setHiddenFromScoreboard`, `setHiddenFromServerStatus`, `setHoldingWeaponDown`, `setJumpHeight`, `setJumpSlowdownEnable`, `setMeleeHeightScale`, `setMeleeRangeScale`, `setMeleeWidthScale`, `setNorthYawForPlayer`, `setOriginAndAngles`, `setPing`, `setProneStepSize`, `setRechambering`, `setSpeed`, `setStepSize`, `setTurretSpreadScale`, `setUserinfo`, `setWeaponSpreadScale`, `setg_gravity`, `setg_speed`, `smokeButtonPressed`, `stopUseTurret`, `unmuteClient`, `useEntity`, `useTurret`

### Entity methods  — `gsc_zk_entity.cpp`
*Entity velocity/gravity/bounce, contents/clipmask, tags, turrets, items, per-player visibility/solidity.*  (0 functions, 33 methods)

**Methods:**

`addEntityVelocity`, `addGrenadeFuseTime`, `disableBounce`, `disableGravity`, `enableBounce`, `enableGravity`, `getClipmask`, `getContents`, `getEntityVelocity`, `getGrenadeFuseTime`, `getItemQuantity`, `getMaxEntityVelocity`, `getTagAngles`, `getTagOrigin`, `getTurretOwner`, `getVmax`, `getVmin`, `getWeaponItemAmmo`, `getWeaponItemClipAmmo`, `hasTag`, `hideFromPlayer`, `isGravityEnabled`, `isLinkedTo`, `isTurret`, `notSolidForPlayer`, `setClipmask`, `setEntityVelocity`, `setItemQuantity`, `setLight`, `setMaxEntityVelocity`, `setWeaponItemAmmo`, `setWeaponItemClipAmmo`, `solidForPlayer`

### Weapon functions/methods  — `gsc_zk_weapons.cpp`
*Weapon metadata (raise/fuse/movespeed), semi-auto/display-name queries, fuse/move overrides, ignore-list, grenade spawning.*  (11 functions, 1 methods)

**Functions:**

`getWeaponDisplayName`, `getWeaponFuseTime`, `getWeaponMoveSpeedScale`, `getWeaponRaiseTime`, `ignoreWeapon`, `isSemiAutoWeapon`, `resetIgnoredWeapons`, `setDefaultWeapon`, `setWeaponFuseTime`, `setWeaponMoveSpeedScale`, `setWeaponRaiseTime`

**Methods:**

`spawnGrenade`

### Level functions  — `gsc_zk_level.cpp`
*Entity counts, mover enumeration, north-yaw, save-persist toggles.*  (5 functions, 0 methods)

**Functions:**

`getEntityCount`, `getMovers`, `getSavePersist`, `setNorthYaw`, `setSavePersist`

### Utility functions  — `gsc_zk_utils.cpp`
*Math/string helpers, config-string lookup, time, hex, file ops, remote commands, raw packets.*  (28 functions, 0 methods)

**Functions:**

`abs`, `atan2`, `ceil`, `chr`, `error`, `executecommand`, `findConfigStringIndex`, `findConfigStringIndexOriginal`, `floor`, `fremove`, `fromhex`, `getcvarflags`, `getlocaltime`, `getmicroseconds`, `getmilliseconds`, `getsurfacename`, `getsystemtime`, `loaddir`, `logprintconsole`, `makeClientLocalizedString`, `makeString`, `ord`, `processRemoteCommand`, `roundto`, `sendCommandToClient`, `sendPacket`, `setConsolePrefix`, `tohex`

### Custom voice / speex (this phase)  — `gsc_zk_sound.cpp`
*Server-side sound-file playback over the speex voice channel.*  (4 functions, 4 methods)

**Functions:**

`getSoundFileDuration`, `loadSoundFile`, `loadSpeexFile`, `saveSpeexFile`

**Methods:**

`getRemainingSoundFileDuration`, `isPlayingSoundFile`, `playSoundFile`, `stopSoundFile`

### Bot AI methods/functions  — `gsc_bots.cpp`
*Native fake-client control: aim, fire, melee, reload, lean, stance, walk, weapon switch, grenades, naming.*  (2 functions, 21 methods)

**Functions:**

`resetTestClientNaming`, `setNextTestClientName`

**Methods:**

`adsAim`, `adsaim`, `fireWeapon`, `fireweapon`, `meleeWeapon`, `meleeweapon`, `reloadWeapon`, `reloadweapon`, `setAim`, `setBotStance`, `setLean`, `setWalkDir`, `setWalkValues`, `setbotstance`, `setlean`, `setwalkdir`, `switchToWeaponId`, `switchtoweaponid`, `throwGrenade`, `throwSmokeGrenade`, `thrownade`

### Additional self-contained builtin (this phase) — `gsc_utils.cpp`
*Built on rev's existing source-buffer/codepos machinery.*  (1 function)

**Functions:** `getCallStack` — returns the current GSC call stack as `[file, line, …]` (meaningful with `developer 1`).

> Note: the bot method list includes intentional case-variant aliases (e.g. `adsAim`/`adsaim`, `fireWeapon`/`fireweapon`) — the same C function registered under both spellings.

## Part B — Dvars Ported

All 82 dvars below are registered in libcod's `RegisterLibcodDvars()`. Defaults preserve rev's prior observable behaviour unless parity with zk was the explicit goal.

### Server messages

| Dvar | Type | Default |
|---|---|---|
| `sv_kickMessages` | Bool | `true` |
| `sv_botKickMessages` | Bool | `true` |
| `sv_disconnectMessages` | Bool | `true` |
| `sv_timeoutMessages` | Bool | `true` |
| `sv_wwwDlDisconnectedMessages` | Bool | `true` |
| `sv_downloadMessage` | String | `""` |
| `sv_downloadMessageAtMap` | Bool | `true` |
| `sv_downloadNotifications` | Bool | `false` |
| `sv_downloadMessageForLegacyClients` | String | `""` |
| `sv_genericServerErrorMessage` | Bool | `false` |

### Logging / RCON

| Dvar | Type | Default |
|---|---|---|
| `sv_logRcon` | Bool | `true` |
| `sv_logHeartbeats` | Bool | `true` |
| `sv_limitLocalRcon` | Bool | `false` |
| `sv_allowRcon` | Bool | `true` |
| `logErrors` | Bool | `false` |
| `logTimestamps` | Bool | `false` |
| `logfileName` | String | `""` |
| `logfileRotate` | Int | `0` |
| `con_coloredPrints` | Bool | `false` |
| `g_logPickup` | Bool | `true` |
| `g_debugCallbacks` | Bool | `false` |
| `g_debugEvents` | Bool | `false` |
| `g_debugStaticModels` | Bool | `false` |

### Downloads

| Dvar | Type | Default |
|---|---|---|
| `sv_downloadRetransmitTimeout` | Int | `1000` |
| `sv_fastDownload` | Bool | `false` |
| `sv_fastDownloadSpeed` | Int | `MAX_DOWNLOAD_WINDOW` |

### Bots

| Dvar | Type | Default |
|---|---|---|
| `sv_botReconnectMode` | Int | `0` |
| `sv_kickbots` | Bool | `false` |
| `g_spectateBots` | Bool | `true` |
| `g_droppedWeaponsNeglectBots` | Bool | `false` |

### Auth / master servers

| Dvar | Type | Default |
|---|---|---|
| `sv_authorizeServer` | String | `AUTHORIZE_SERVER_NAME` |
| `sv_authorizePort` | Int | `PORT_AUTHORIZE` |
| `sv_authorizeTimeout` | Int | `AUTHORIZE_TIMEOUT` |
| `sv_noauthorize` | Bool | `false` |
| `sv_masterServer` | String | `MASTER_SERVER_NAME` |
| `sv_masterPort` | Int | `PORT_MASTER` |
| `sv_master1` | String | `MASTER_SERVER_NAME` |
| `sv_master2` | String | `""` |
| `sv_master3` | String | `""` |
| `sv_master4` | String | `""` |
| `sv_master5` | String | `""` |

### File system / manymaps

| Dvar | Type | Default |
|---|---|---|
| `fs_gametypes` | String | `""` |
| `fs_callbacks` | String | `""` |
| `fs_mapScriptDirectories` | Int | `0` |
| `fs_replaceStockMaps` | Bool | `false` |
| `fs_library` | String | `""` |

### Snapshots / networking

| Dvar | Type | Default |
|---|---|---|
| `sv_maxSnapshotEntities` | Int | `MAX_SNAPSHOT_ENTITIES` |
| `sv_autoAddSnapshotEntities` | Bool | `true` |
| `g_forceRate` | Int | `0` |
| `g_forceSnaps` | Int | `0` |

### Gamestate splitting

| Dvar | Type | Default |
|---|---|---|
| `sv_kickGamestateLimitedClients` | Bool | `false` |
| `sv_reservedConfigstringBufferSize` | Int | `0` |

### Movement / physics

| Dvar | Type | Default |
|---|---|---|
| `g_resetSlide` | Bool | `false` |
| `g_pointTraceMovement` | Bool | `false` |
| `jump_carryMoverVelocity` | Bool | `false` |
| `jump_bounceEnable` | Bool | `false` |
| `g_bulletDrop` | Bool | `false` |
| `g_bulletDropMaxTime` | Int | `10000` |

### Player collision

| Dvar | Type | Default |
|---|---|---|
| `g_playerCollision` | Bool | `true` |
| `g_playerEject` | Bool | `true` |
| `g_playerCollisionEjectDuration` | Int | `300` |
| `g_playerCollisionEjectDamageAllowed` | Bool | `false` |
| `g_corpseHit` | Bool | `true` |

### Weapons / combat

| Dvar | Type | Default |
|---|---|---|
| `g_fixedWeaponSpreads` | Bool | `false` |
| `g_dropGrenadeOnDeath` | Bool | `true` |
| `g_mantleBlockEnable` | Bool | `true` |
| `scr_turretDamageName` | Bool | `true` |
| `g_turretMissingTagTerminalError` | Bool | `true` |

### World / spawns / movers

| Dvar | Type | Default |
|---|---|---|
| `g_spawnMapWeapons` | Bool | `true` |
| `g_spawnMapTurrets` | Bool | `true` |
| `g_noMoverBlockage` | Bool | `false` |
| `g_brushModelCollisionTweaks` | Bool | `true` |
| `g_triggerMode` | Int | `1` |
| `g_sendEmtpyOffhandEvents` | Bool | `true` |

### Precache / models

| Dvar | Type | Default |
|---|---|---|
| `g_reservedModels` | Int | `0` |
| `g_safePrecache` | Bool | `false` |

### Misc

| Dvar | Type | Default |
|---|---|---|
| `sv_updateCursorHints` | Bool | `true` |
| `sv_isLookingAtOnDemand` | Bool | `false` |
| `sv_verifyIwds` | Bool | `true` |
| `sv_cracked` | Bool | `false` |
| `sv_botUseTriggerUse` | Bool | `false` |
| `libcod` | Bool | `true` |

## Part C — Not Ported

**Builtins (3) — blocked on rev's absent sound-alias subsystem.** rev stubs the entire system (`Com_FindSoundAlias()` returns `NULL; // Not supported`; no loader, no `snd_alias_build` list). These need build-time alias metadata that a dedicated server never produces, so implementing the sound-alias loader is a prerequisite (its own effort) rather than porting the builtins:

- `getSoundDuration`, `getSoundInfo`, `getSoundAliasesFromFile`

**Dvars — documented as non-portable / deferred:**

- `net_noFragmentationDelay` (inert — rev already flushes all fragments per frame)
- `sv_version` (architectural — rev uses compile-time `PROTOCOL_VERSION`)
- `loc_loadLocalizedMods` (niche, no clean call site)
- `cl_allowDownload`, `cl_wwwDownload` (client-only — covered server-side by `sv_minimizeSysteminfo` mode 3)


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


