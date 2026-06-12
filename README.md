# zk_libcod → CoD2rev_Server Port — Status Summary

Porting the **ibuddieat/zk_libcod** GSC feature set onto the **callofduty2x/CoD2rev_Server**
codebase. Target build: **x64**, `nomysql` validated end-to-end (MySQL variant 1 builds once a
client lib is supplied).

**Progress: 150 of 221 GSC functions** in the case-insensitive delta (functions zk has that rev
lacks), plus the custom-state infrastructure and 18 native engine hooks. Every round compiles and
the full binary relinks clean.

---

## 1. Architecture decision

The two projects integrate libcod incompatibly:

- **zk_libcod** is an `LD_PRELOAD` shared library that hooks the stock `cod2_lnxded` binary via
  hardcoded memory offsets + byte-patching.
- **CoD2rev_Server** compiles libcod *directly into* its from-scratch reverse-engineered binary,
  calling engine functions by name.

A literal swap is impossible. The port brings zk's **feature set** onto rev's native engine-binding
layer, function by function: zk function bodies are copied into isolated `gsc_zk_*` modules,
reconciled against rev's struct field names, and zk's byte-patch hooks are re-implemented as native
call-site edits in rev's C source.

**Standing decisions:**
- Build **x64** (pointer-width bugs are real, not cosmetic).
- **Keep the zk version**: where rev already implements a feature under a different name, register
  zk's own implementation as authoritative rather than aliasing to rev's. When the two use different
  backing state (e.g. speed/gravity), rev's mechanism is retired so they don't conflict.
- Both names can coexist pointing at one function (the dispatch table allows duplicate pointers;
  GSC lookup is case-insensitive via `strcasecmp`).

---

## 2. Build system (complete)

- **`build.sh`** — interactive wrapper mirroring zk's `doit.sh` MySQL prompt
  (`0`=disabled, `1`=default, `2`=VoroN). `./build.sh clean` (also `cleanall`/`distclean`) does a
  thorough, arch-agnostic clean — it wipes the whole `obj/` tree and every binary/lib variant,
  rather than delegating to the Makefile's arch-scoped `clean` (which would leave the
  non-default architecture's objects and binary behind).
- **Makefile** — `WITH_MYSQL=false` replaced with a `MYSQL_VARIANT ?= 0` selector; per-variant
  `-D` flags; `filter-out` keeps only the selected MySQL source.
- All sources compile with `-D LIBCOD` globally so `#ifdef LIBCOD` works in game/server files.

**MySQL note:** rev ships the MySQL *headers* (`src/libcod/mysql/include/`, v5.7.6 / libmysql
6.1.6) but **not** the client library. Any MySQL variant build needs `libmysqlclient` provided
(matching the build arch) — either dropped into `src/libcod/mysql/unix/lib/` or installed
system-wide. The VoroN variant (`gsc_mysql_voron.cpp`) is not yet ported; use `mysql1` or `nomysql`.

---

## 3. Ported functions by module

New isolated modules in `src/libcod/` (auto-compiled by the wildcard):

### gsc_zk_utils.cpp — 22 functions
`abs`, `atan2`, `ceil`, `floor`, `chr`, `ord`, `tohex`, `fromhex`, `roundTo`, `error`,
`executeCommand`, `getMilliseconds`, `getMicroseconds`, `getCvarFlags`, `getSystemTime`,
`getLocalTime`, `fremove`, `loadDir`, `logPrintConsole`, `getSurfaceName`,
`findConfigStringIndex`, `findConfigStringIndexOriginal`. (`pow` = alias to rev's exponent fn.)

### gsc_zk_weapons.cpp — 8 functions
`isSemiAutoWeapon`, `getWeaponRaiseTime`/`setWeaponRaiseTime`, `getWeaponFuseTime`/`setWeaponFuseTime`,
`getWeaponMoveSpeedScale`/`setWeaponMoveSpeedScale`, `getWeaponDisplayName`.
(Reconciled Hungarian field names: `bSemiAuto`→`semiAuto`, `iRaiseTime`→`raiseTime`, etc.)

### gsc_zk_entity.cpp — 32 methods
`getClipmask`/`setClipmask`, `getVmax`, `getVmin`, `isTurret`, `isLinkedTo`, `getTurretOwner`,
`setLight`, `hasTag`, `getTagOrigin`, plus item/grenade accessors: `getGrenadeFuseTime`,
`addGrenadeFuseTime`, `getWeaponItemAmmo`/`setWeaponItemAmmo`,
`getWeaponItemClipAmmo`/`setWeaponItemClipAmmo`, `getItemQuantity`/`setItemQuantity`, plus the
entity-gravity interface: `enableGravity`/`disableGravity`/`isGravityEnabled`,
`enableBounce`/`disableBounce`, `getEntityVelocity`/`setEntityVelocity`/`addEntityVelocity`,
`getMaxEntityVelocity`/`setMaxEntityVelocity`, plus per-client solidity:
`notSolidForPlayer`/`solidForPlayer`, plus `getContents` (override-aware) and `hideFromPlayer`
(per-player invisibility via the engine's `r.clientMask`, which rev's snapshot builder already
honors — no new hook required).

### gsc_zk_physics.cpp — entity gravity integrator (subsystem)
Custom per-entity gravity/bounce physics for `script_model` entities, ported from zk's
`G_RunGravityModelNoBounce` / `G_RunGravityModelWithBounce` / `G_BounceGravityModel` (themselves
based on rev's `G_RunItem`/`G_RunMissile`/`G_BounceMissile`). Driven by
`customEntityState[].gravityType` (`GRAVITY_NONE`/`NO_BOUNCE`/`BOUNCE`) and dispatched from
`G_RunFrameForEntity` via `zk_EntityHasGravity()`/`zk_RunEntityGravity()`. Missing rev helpers
(`VectorClampLength`, `IsNullVector`) implemented inline; `VectorCross`→`Vec3Cross`,
`VecToAngles`→`vectoangles`; zk's `SV_Trace` signature is identical to rev's so the `collideModels`
arg passes through verbatim. **This is the feature most in need of in-game physics validation.**
Two documented deviations: the optional `land`/`bounce` script notifies are omitted (they need
custom script-constant registration; physics is unaffected), and the `EF_TAGCONNECT`
`Missile_TraceNoContents` edge case is skipped (rev lacks that primitive).

### gsc_zk_level.cpp — 5 functions
`getMovers`, `getEntityCount`, `setNorthYaw`, `getSavePersist`/`setSavePersist`
(via rev's `G_GetSavePersist`/`G_SetSavePersist`).

### gsc_zk_player.cpp — 83 functions
- **Custom-state setters:** `enableSilent`/`disableSilent`, `overrideContents`,
  `setWeaponSpreadScale`, `setTurretSpreadScale`, `setMeleeRangeScale`/`setMeleeWidthScale`/`setMeleeHeightScale`,
  `setSpeed`/`setGravity`, `setHiddenFromScoreboard`/`isHiddenFromScoreboard`,
  `setHiddenFromServerStatus`/`isHiddenFromServerStatus`, `setPing`.
- **playerState/gclient readers:** `getSpeed`, `getGravity`, `isReloading`, `isFiring`,
  `isMeleeing`, `isThrowingGrenade`, `getCurrentWeaponAmmo`, `getCurrentWeaponClipAmmo`,
  `getGroundEntity`, `getPlayerStateFlags`, `isShellshocked`, `isChatting`, `getEnterTime`,
  `getInactivityTime`, `getWeaponAnimation`, `getSpectatorClient`, `getClientHudElemCount`,
  `isUsingBinoculars`, `canMantle`, `getCurrentOffhandSlotAmmo`, `getJumpSlowdownTimer`.
- **playerState/gclient setters:** `noclip`, `setCurrentWeaponAmmo`, `setCurrentWeaponClipAmmo`,
  `playScriptAnimation`, `processSuicide`, `stopUseTurret`, `forceShot`.
- **server client_t accessors:** `aimButtonPressed`, `backButtonPressed`, `forwardButtonPressed`,
  `fragButtonPressed`, `holdBreathButtonPressed`, `jumpButtonPressed`, `leanLeftButtonPressed`,
  `leanRightButtonPressed`, `leftButtonPressed`, `reloadButtonPressed`, `rightButtonPressed`,
  `smokeButtonPressed`, `isBot`, `getClientConnectState`, `getLastMsg`, `getLastConnectTime`,
  `getAddressType`, `getServerCommandQueueSize`, `getUserinfo`, `setGuid`, `muteClient`,
  `unmuteClient`, `renameClient`, `setUserinfo`, `setConfigStringForPlayer`,
  `setNorthYawForPlayer`, `resetNextReliableTime`, `connectionlessPacketToClient`,
  `connectionlessPacketToServer`, `setHoldingWeaponDown`, plus bullet-mask: `setFireThroughWalls`, `getBulletMask`, `setBulletMask`, plus spectator/turret/state readers: `isAllowingSpectators`, `setAllowSpectators`, `isHoldingWeaponDown`, `canUseTurret`.

### gsc_zk_custom_state.cpp — infrastructure (not GSC functions)
`customPlayerState[MAX_CLIENTS]` + `customEntityState[MAX_GENTITIES]` arrays, lifecycle
(reset on connect / init-gentity / startup), and **13 accessor functions** consumed by the native
engine hooks (below).

---

## 4. Native engine hooks (custom-state consumption)

Each custom-state feature pairs a GSC setter (writes `customPlayerState`) with one or more
consumption hooks in rev's engine code. Hooks call small accessor functions via a local
forward-declaration, keeping the struct internal to libcod and each game/server edit to ~2 lines.

| Feature | Accessor | Consumption site(s) |
|---|---|---|
| silent | `zk_IsPlayerSilent` | `BG_AddPredictableEventToPlayerstate` (footstep/foliage/noammo events) |
| overrideContents | `zk_GetPlayerContentsOverride` | `G_SetClientContents` |
| weapon spread scale | `zk_GetWeaponSpreadScale` | `FireWeaponAntiLag` |
| turret spread scale | `zk_GetTurretSpreadScale` | turret `Bullet_Fire` in `g_misc_mp` |
| melee scales | `zk_ApplyMeleeScales` | `FireWeaponMelee` |
| speed / gravity | `zk_ApplyPlayerSpeedGravity` | per-frame player-state update in `g_active_mp` (replaces rev's `player_g_speed[]`) |
| hiddenFromScoreboard | `zk_IsHiddenFromScoreboard` | `DeathmatchScoreboardMessage` (+ `visiblePlayers` count fix) |
| hiddenFromServerStatus | `zk_IsHiddenFromServerStatus` | `SVC_Info` (both count loops) + `SVC_Status` (player lines) |
| overridePing | `zk_GetPingOverride` | `SV_GetClientPing` |
| overrideStatusPing | `zk_GetStatusPingOverride` | `SVC_Status` player line |
| holdingDownWeapon | `zk_GetHoldingDownWeapon` | `PM_Weapon` (force lowered weapon, early-return) + `Player_UpdateCursorHints` (suppress item hint) |
| entity gravity | `zk_EntityHasGravity` / `zk_RunEntityGravity` | `G_RunFrameForEntity` (intercept before `physicsObject` path) |
| bullet mask / fireThroughWalls | `zk_GetBulletMask` | `Bullet_Fire_Extended` (override trace contentmask per attacker) |
| allow-spectators | `zk_IsNotAllowingSpectators` | `Cmd_FollowCycle_f` (skip clients who disabled being spectated) |
| per-client solidity | `zk_IsNonSolidForClient` / `zk_ClearNonSolidForClient` / `zk_playerMovementTrace` | `SV_ClipMoveToEntity` (server skip, gated by `Pmove` wrapper in `g_active_mp.cpp`) + `SV_EmitPacketEntities` (OR `EF_NONSOLID_BMODEL` into the client's snapshot copy) + `ClientDisconnect` (clear flags) |

---

## 5. Modified rev source files (each has a delivered patch)

| File | Purpose |
|---|---|
| `src/libcod/gsc.cpp` | all table registrations + module includes |
| `src/game/g_client_mp.cpp` | reset custom player state on `ClientConnect`; clear solidity flags on `ClientDisconnect` |
| `src/game/g_utils_mp.cpp` | reset custom entity state on `G_InitGentity` |
| `src/bgame/bg_misc.cpp` | silent event suppression |
| `src/game/g_active_mp.cpp` | overrideContents + speed/gravity consumption; `playerMovementTrace` wrap around `Pmove` |
| `src/game/g_weapon_mp.cpp` | weapon spread + melee scales + bullet mask (`Bullet_Fire_Extended`) |
| `src/game/g_misc_mp.cpp` | turret spread scale |
| `src/game/g_cmds_mp.cpp` | hiddenFromScoreboard (scoreboard builder); notAllowingSpectators skip in `Cmd_FollowCycle_f` |
| `src/server/sv_main_mp.cpp` | hiddenFromServerStatus + overrideStatusPing |
| `src/server/sv_game_mp.cpp` | overridePing |
| `src/bgame/bg_weapons.cpp` | holdingDownWeapon enforcement in `PM_Weapon` |
| `src/game/player_use_mp.cpp` | holdingDownWeapon suppresses item cursor hint |
| `src/game/g_main_mp.cpp` | entity-gravity dispatch in `G_RunFrameForEntity` |
| `src/server/sv_world_mp.cpp` | per-client solidity skip in `SV_ClipMoveToEntity` |
| `src/server/sv_snapshot_mp.cpp` | per-client `EF_NONSOLID_BMODEL` in `SV_EmitPacketEntities` |

---

## 6. Validation

- Full clean build from empty `obj/`: all ~150 rev TUs + 6 zk modules + 10 edited rev files compile.
- Links into `cod2rev_lnxded_x64` (~3.77 MB) with no duplicate/unresolved symbols; ~59 zk symbols
  in the binary.
- Executable starts, prints the banner, runs `FS_Startup`, and proceeds until it needs game data
  (graceful — confirms a valid, runnable binary).
- In-game functional testing remains the operator's responsibility (needs real game data + client).
- Build needs `libssl-dev` for rev's pre-existing mongoose/websocket component.

---

## 7. Known issues / deferred

### gsc_memory.cpp x64 pointer-width warnings (rev's own file)
6 sites store addresses in `int` → real truncation on x64 (GSC ints are 32-bit, can't hold 64-bit
pointers). `gsc_utils.cpp:825` is a *different*, benign float-bits pun (not affected). Two fix
options for the end-of-port cleanup pass:
1. Gate the module off on x64 (`LIBCOD_COMPILE_MEMORY=0`).
2. **Handle-table redesign** (recommended): `malloc` returns a small int handle indexing a table of
   real 64-bit pointers; `free`/`int_get`/`int_set`/`memset` look up the pointer.

Casting through `intptr_t` only silences the warning — it does not fix the truncation. New zk ports
use x64-safe patterns to avoid adding instances.

### Deferred features (need deeper work or subsystems)
- **gsc_bots** — all bot delta functions need zk's bot-usercmd hook / testclient state.
- **Entity stragglers** — `getTagAngles` (`FL_LINKTO_ENABLED` missing in rev). The entity-gravity
  physics cluster and per-client solidity (`notSolidForPlayer`/`solidForPlayer`) are now **ported**;
  the `customEntityState` cluster is complete. Solidity avoided zk's invasive `MSG_WriteDeltaStruct`
  signature change (3 extra params threaded through netcode) by ORing `EF_NONSOLID_BMODEL` into the
  per-client snapshot copy in `SV_EmitPacketEntities` instead.
- **Bullet-path stragglers** — the core `fireThroughWalls`/`getBulletMask`/`setBulletMask` are now
  ported (trace-contentmask override in `Bullet_Fire_Extended`). Deferred: the dropping-bullets
  ballistics subsystem (`enableBulletDrop`/`setBulletVelocity`/`setBulletDrag`/`setBulletModel`/
  `enableBulletImpacts` — needs the trimmed `droppingBullet*` state fields + zk's
  `droppingBullet_t` projectile system and `custom_Bullet_Fire_Drop`); `setFireRangeScale`
  (consumed in `Bullet_GetDamage`, which lacks the attacker in scope); and the through-walls
  bullet-impact visual recreation (`G_TempEntity` effect when the body-only mask skips the wall hit).
- **Weapons stragglers** — `setDefaultWeapon` (zk byte-patches hardcoded stock-binary addresses —
  fundamentally incompatible with rev's native model), `ignoreWeapon`/`resetIgnoredWeapons` (need
  zk's ignore-list globals + a pickup-time consumption hook).
- **fireRangeScale** — rev's `Bullet_GetDamage(wp, dist)` has no attacker in scope; needs threading
  `clientNum` through the signature + callers.
- **Bullet-path features** — `fireThroughWalls`, `bulletMask`, bullet drag/velocity/model: deep in
  the `Bullet_Fire_Extended` trace/penetration loop.
- **Subsystems (struct fields trimmed, need un-trimming + hooks):** jump (full pmove physics
  replacement — high risk), objectives (`objective_player_*`), custom sound (`playSoundFile` family),
  animation (`setAnimation`), talker icons, the `previousButtons` button-edge logic.
- **Snapshot deep features** — `getNumberOfEntsInSnapshot`, `notAllowingSpectators`.
- **VoroN MySQL variant 2** — `gsc_mysql_voron.cpp` not ported.
- **gclient setters/readers blocked on missing rev symbols** — `setOriginAndAngles`
  (`SetClientViewAngles`), `isRechambering`/`setRechambering`/`getCurrentWeaponSlot` (rev lacks `GetCurrentWeaponSlot`),
  `isUseTouching` (`PMF_SPECTATING` differs).
- **client_t blocked items** — `playSoundFile` (custom sound subsystem). `setHoldingWeaponDown`
  is ported (field un-trimmed, setter, `PM_Weapon` + cursor-hint hooks); deferred only is its
  turret-exit re-apply (re-running the drop state when a holding player leaves a turret — an edge
  case in `custom_G_ClientStopUsingTurret`). Reconciles applied: `WeaponDef_t`→`WeaponDef`,
  `iDropTime`→`dropTime`, `iRaiseTime`→`raiseTime`, `overlayReticle`→`adsOverlayReticle`,
  `bADSFire`→`adsFire`; zk's always-true `pm_flags | PMF_FRAG` typo corrected to `& PMF_FRAG`.

---

## 8. Remaining work (rough map)

| Bucket | Approx. remaining | Nature |
|---|---|---|
| gclient/playerState | ~4 (blocked) | Bucket essentially done; only deferred items remain (need missing rev symbols) |
| server client_t | ~1 (blocked) | Bucket essentially done; `playSoundFile` (sound subsystem) remains. `setHoldingWeaponDown` now ported (core + PM_Weapon hook); only its turret-exit re-apply edge case is deferred |
| custom-state deep/subsystems | ~40 | Snapshot/bullet/pmove hooks + trimmed subsystems |
| entity / weapons / misc stragglers | remainder | Struct reconciliation, union mapping |

The `client_t` server-command senders use rev's variadic `SV_SendServerCommand(client, type, fmt, ...)`
called as `(client, SV_CMD_RELIABLE, "%s", cmd)` rather than passing the script-built string as the
format directly — avoids a format-string bug when a configstring value contains `%`.

Deferred gclient items, all blocked on missing rev symbols: `isRechambering`/`setRechambering`
(need `GetCurrentWeaponSlot`), `isUseTouching` (zk's `PMF_SPECTATING` is split into
`PMF_SPECTATOR_FREE`/`PMF_SPECTATOR_FOLLOW` in rev), `setOriginAndAngles` (needs
`SetClientViewAngles` plus full teleport/unlink logic). ~11 functions in this bucket were already
present in rev under matching names and skipped.

**Highest yield-per-effort so far:** checking whether rev already implements a feature before
porting — several zk "missing" functions were already present under different names
(speed/gravity, item pickup, configstring finders, save-persist), turning ports into free aliases or
trivial rebinds.

---

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


