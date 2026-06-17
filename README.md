# zk_libcod → CoD2rev_Server Port — Status Summary

Porting the **ibuddieat/zk_libcod** GSC feature set onto the **callofduty2x/CoD2rev_Server**
codebase. Target build: **x64**, `nomysql` validated end-to-end (MySQL variant 1 builds once a
client lib is supplied).

**Progress: ~206 of 221 GSC functions** (15 zk names remain — accurate recount; the 221 denominator is approximate) in the case-insensitive delta (functions zk has that rev

## 64-bit pointer correctness (VAR_RAWPOINTER) — x64 handle-truncation fix
libcod stored real C pointers (`MYSQL*`, `MYSQL_RES*`, `sqlite3*`, raw `malloc` addresses, a `msg_t*`) in 32-bit GSC ints via `stackPushInt((intptr_t)ptr)` / `stackGetParams("i")`, truncating 64-bit pointers -> segfault on x64 (x86 was fine because pointers fit). Fixed at the VM level per IzNoGoD's suggestion: a dedicated scalar pointer type, not a handle table.

**VM type (`src/script/`):**
- **script_public.h**: new `VAR_RAWPOINTER` in `var_type_t`, inserted after `VAR_INTEGER` (scalar band — outside `VAR_BEGIN_REF..VAR_END_REF` and the object/dead ranges, so `AddRefToValue`/`RemoveRefToValue`/GC treat it as a no-op scalar, confirmed by reading those switch statements). Matching `"raw pointer"` in the positional `var_typename[]`. Added a dedicated **`void *rawPointerValue`** member to `VariableUnion` (overlays the existing 8-byte `pointerValue`; union size + `static_assert` unchanged) so the push/get are cast-free. NB: do **not** retype the existing `pointerValue` (uintptr_t) to `void*` — it is used as an integer id for array indices/entity ids in ~20 places.
- **scr_vm.cpp**: `Scr_AddPointer(void*)` and `Scr_GetPointer(index)` mirroring `Scr_AddInt`/`Scr_GetInt`. `Scr_AddPointer(NULL)` pushes **VAR_UNDEFINED**, so `isDefined()` is the natural validity test for handle-returning builtins (mysql_init/connect/store_result, sqlite_open, memory_malloc, async results).
- **gsc.hpp / gsc.cpp**: `#define stackPushPointer Scr_AddPointer`; a `'p'` code in `stackGetParams` + `stackGetParamPointer`. The `STACK_*` numeric mirror in gsc.hpp was resynced (added `STACK_RAWPOINTER 7`, bumped `STACK_CODEPOS..STACK_REMOVED_THREAD` +1) to track the shifted enum. `var_type_t` is runtime-only (CoD2 compiles .gsc text at load; no on-disk bytecode stores tags), so shifting later enum values is safe.

**Bindings converted to the pointer type (handles cross GSC as `VAR_RAWPOINTER`, ids/return-codes stay int):**
- `gsc_mysql.cpp` (variant 1) + `gsc_mysql_voron.cpp` (variant 2) — all `MYSQL*`/`MYSQL_RES*` handles; the connection-pool id path and async task ids stay int. (Includes the async `getresult_and_free` result, which used an intermediate `intptr_t ret` and was easy to miss.)
- `gsc_sqlite.cpp` — the `sqlite3*` db handle (statements/store are C-internal).
- `gsc_memory.cpp` — `malloc` addresses + byte-buffer handles; `binarybuffer.address` widened `int`->`intptr_t`.
- `gsc_utils.cpp` — `gsc_utils_remotecommand()` `msg_t*` param.

**Whole-tree `-Wint-to-pointer-cast` audit: clean.** Core engine/VM/game/server produced zero warnings; the only engine site was the benign `scope` sentinel in `scr_compiler.cpp` (fixed with an explicit `(intptr_t)` widen — it stores a 32-bit offset, not a real pointer). All genuine truncations were in the libcod modules above and are fixed. x64 builds warning-free.

**Diagnostics:** implemented the stubbed `Scr_DumpScriptVariables` (was `UNIMPLEMENTED`) in `scr_variable.cpp` as a summary — on an "exceeded maximum number of script variables" overflow it now prints live-variable counts by type, so the GSC variable-pool budget (cap `VARIABLELIST_PARENT_SIZE` = 0x8000) can be diagnosed (e.g. a large result set materialized into a GSC array via `getRows`).

**GSC-side guidance (`_mysql.gsc`):** with NULL->undefined, use `isDefined(handle)` as the validity check; the old `is_valid_pointer()` stringify hack must be removed (a `VAR_RAWPOINTER` cannot be cast to string — `"" + handle` errors). Known latent caveat (not a cast warning, left as-is): `binarybuffer` `'s'` type stores an 8-byte `char*` but advances `pos += 4` — wrong on x64 if used.

lacks), plus the custom-state infrastructure and ~34 native engine hooks. Every round compiles and
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
system-wide.

### MySQL variants

Two complete, **mutually exclusive** MySQL backends are available; you pick one at build time.
The selection swaps the entire function set — there is no coexistence (and so no symbol clash),
because the Makefile compiles only the chosen source file.

| `MYSQL_VARIANT` | Source file | `-D` flag | GSC API exposed |
|---|---|---|---|
| `0` (default) | — | — | none (MySQL off) |
| `1` (classic) | `gsc_mysql.cpp` | `LIBCOD_COMPILE_MYSQL=1` | `mysql_init`, `mysql_real_connect`, `mysql_query`, `mysql_store_result`/`fetch_row`/`free_result`, `mysql_async_create_query`(`_nosave`), `mysql_async_getdone_list`, `mysql_async_getresult_and_free`, `mysql_async_initializer`, `mysql_reuse_connection`, … (21 fns) |
| `2` (VoroN) | `gsc_mysql_voron.cpp` | `LIBCOD_COMPILE_MYSQL_VORON=1` | `mysql_initialize` (one-step connect), `async_mysql_initialize`/`create_query`(`_nosave`)/`checkdone`/`free_task`/`errno`/`error`/`num_rows`/…, plus entity methods `async_mysql_create_entity_query`(`_nosave`) (31 fns) |

Both variants share the synchronous names (`mysql_close`/`query`/`errno`/`error`/`store_result`/…)
but back them with different implementations, which is exactly why they cannot be compiled
together. A GSC script written for one variant will hit "unknown function" on the other, so
standardize a server on a single variant.

**Build steps:**

1. Provide `libmysqlclient` for your build arch (install `libmysqlclient-dev` + `libssl-dev`, or
   drop the lib into `src/libcod/mysql/unix/lib/`). The build links `-lmysqlclient` for variants
   1 and 2 — a missing lib shows up as an *undefined-reference at the final link*, not a compile error.
2. Build the variant you want:
   - `./build.sh mysql1` — classic
   - `./build.sh mysql2` — VoroN
   - `./build.sh nomysql` — off
   - `./build.sh` — interactive prompt (0/1/2)
   - append make args as needed, e.g. `./build.sh mysql2 ARCH=x86`
3. Switching variants: `./build.sh clean` first (stale objects otherwise linger), then rebuild.
4. Verify the right set linked in:
   - variant 1: `strings <binary> | grep -E "mysql_init|mysql_async_create_query"`
   - variant 2: `strings <binary> | grep -E "mysql_initialize|async_mysql_create_query"`

Compile-verified for both variants; **not** link-tested in the sandbox (no client lib there). The
handle-as-int casts are correct for the x86 target (they would truncate on x64, same as rev's own
base MySQL code).

---

## 3. Ported functions by module

New isolated modules in `src/libcod/` (auto-compiled by the wildcard):

### Per-player/team collision (collisionTeam)
`getCollisionTeam()` / `setCollisionTeam("none"|"axis"|"allies"|"axis_allies")` (player methods). Players whose collision teams don't match pass through each other. The `customTeam_t` enum and `collisionTeam` field already existed in custom-state; added the `zk_SkipCollision(a,b)` helper, the custom `axis_allies` script const (`zk_const_axis_allies`, allocated in a `GScr_LoadConsts` hook), and the two get/set methods.

**Hooks (3):** `GScr_LoadConsts` (scr_const_mp.cpp) allocates the `axis_allies` const; `SV_ClipMoveToEntity` (sv_world_mp.cpp) skips the clip when `zk_SkipCollision` is true (movement/trace pass-through); `StuckInClient` (g_active_mp.cpp) skips the player-ejection pass. **Intentionally NOT ported:** zk's 3rd site in `MSG_WriteDeltaStruct` (snapshot-delta encoder) — a client-side prediction refinement; server-side collision is fully handled by the two physics hooks above.

### Weapon ignore / default weapon (in gsc_zk_weapons.cpp)
`ignoreWeapon(name)`, `resetIgnoredWeapons()`, `setDefaultWeapon(name)`. A `BG_GetWeaponIndexForName` hook remaps any ignored weapon name to the default before lookup. **Limitation:** zk's `setDefaultWeapon` byte-patches the engine's global default-weapon fallback at fixed addresses (`G_SetupWeaponDef`/`BG_FillInAmmoItems`) — not portable to rev's recompiled binary — so rev's `setDefaultWeapon` only sets the weapon that `ignoreWeapon` substitutions resolve to (defaults to `defaultweapon_mp`), it does not change the engine-wide fallback.

### gsc_bots.cpp — 11 bot-action methods (rev's native bot module, now registered)
rev already shipped a button-based bot input system (`bot_buttons`/`bot_forwardmove`/`bot_rightmove`/`bot_weapon`, consumed by `SV_BotUserMove_libcod`) but left it **unregistered**. Registered the full zk-compatible method set: `setAim` (=ADS toggle), `setWalkDir`, `setWalkValues` (analog forward/right), `setLean`, `setBotStance`, `fireWeapon`, `meleeWeapon`, `reloadWeapon`, `switchToWeaponId`, `throwGrenade`, `throwSmokeGrenade`. `setWalkValues` and `throwSmokeGrenade` (BUTTON_SMOKE) are new; the rest expose rev's existing implementations. No engine hook needed — the consumption (`SV_BotUserMove_libcod`) was already wired.

### gsc_zk_utils.cpp — 26 functions
`abs`, `atan2`, `ceil`, `floor`, `chr`, `ord`, `tohex`, `fromhex`, `roundTo`, `error`,
`executeCommand`, `getMilliseconds`, `getMicroseconds`, `getCvarFlags`, `getSystemTime`,
`getLocalTime`, `fremove`, `loadDir`, `logPrintConsole`, `getSurfaceName`,
`findConfigStringIndex`, `findConfigStringIndexOriginal`, plus networking: `sendCommandToClient`, `sendPacket`, plus string helpers: `makeString` (localized→plain) and `makeClientLocalizedString` (wrap in \\x14..\\x15 markers). (`pow` = alias to rev's exponent fn.) Async-exec functions `execute`/`execute_async_create`/`execute_async_create_nosave`/`execute_async_checkdone` are exposed (zk names aliased onto rev's pre-existing `exec*` implementation: synchronous + threaded `popen` with GSC callbacks).

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

### gsc_zk_player.cpp — 108 functions
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
  `connectionlessPacketToServer`, `setHoldingWeaponDown`, plus bullet-mask: `setFireThroughWalls`, `getBulletMask`, `setBulletMask`, plus spectator/turret/state readers: `isAllowingSpectators`, `setAllowSpectators`, `isHoldingWeaponDown`, `canUseTurret`, plus movement tuning: `setStepSize`/`setProneStepSize`, `setJumpHeight`/`setJumpSlowdownEnable`, plus per-player fog: `setCullFogForPlayer`/`setExpFogForPlayer` (send the stock `"d 12"` fog server-command to one client — no engine hook), plus per-player earthquakes: `enableEarthquakes`/`disableEarthquakes` (mask the global `earthquake()` temp entity so opted-out players don't feel it), plus talker icons: `enableTalkerIcon`/`disableTalkerIcon` (inject fake voice packets so a target shows a talking HUD icon), plus `setAnimation` (force a named animation on a live player via a `BG_PlayAnim` override, or write a corpse's legs anim directly), plus client-command: `executeClientCommand` (force a client to run a command) and `processClientUserinfoChange` (re-apply a client's userinfo), plus use/look stragglers: `lookAtKiller`, `useEntity`, `useTurret`, `isUseTouching` (all call existing rev engine funcs; `GetEntity` reconciled to a by-number lookup helper).

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
| per-player step size | `zk_GetStepSizeOverride` | `PM_StepSlideMove` (override STEPSIZE / STEPSIZE_PRONE) |
| per-player forced snapshot ents | `zk_GetForcedSnapshotCount`, `zk_GetForcedSnapshotEnt` | `SV_BuildClientSnapshot` (non-cached path: append forced ents via `SV_AddEntToSnapshot` after the PVS gather) |
| setAnimation override | `zk_GetPlayerAnimationOverride` (+ `zk_GetAnimationId` helper) | `BG_PlayAnim` (substitute the custom animation for non-death anims; corpse path writes `s.legsAnim` directly) |
| talker icons | `zk_RunTalkerIcons` (+ `zk_ClearTalkerIconsForClient`) | `SV_SendClientMessages` (inject fake voice packets each frame so `enableTalkerIcon` targets show a talking HUD icon); disconnect cleanup in client-disconnect path |
| per-player earthquakes | `zk_ApplyEarthquakeClientMask` | `GScr_Earthquake` (mask the EV_EARTHQUAKE temp ent's `r.clientMask` for players who called `disableEarthquakes`) |
| per-player objectives | `zk_GetPlayerObjective` | `G_UpdateObjectiveToClients` (prefer per-player objective over `level.objectives[]` when set) |
| per-player jump height/slowdown | `zk_GetJumpHeightOverride`, `zk_GetJumpSlowdownOverride` | `bg_jump.cpp` (swap `jump_height`/`jump_slowdownEnable` dvar reads for per-player accessors across `Jump_IsPlayerAboveMax`/`Jump_GetStepHeight`/`Jump_ClampVelocity`/`Jump_*Slowdown*`/`Jump_Check`) |
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
| `src/game/g_scr_main_mp.cpp` | per-player earthquake suppression in `GScr_Earthquake` |
| `src/game/g_main_mp.cpp` | entity-gravity dispatch in `G_RunFrameForEntity` + per-player objectives in `G_UpdateObjectiveToClients` |
| `src/server/sv_world_mp.cpp` | per-client solidity skip in `SV_ClipMoveToEntity` |
| `src/bgame/bg_slidemove.cpp` | per-player step size override in `PM_StepSlideMove` |
| `src/bgame/bg_animation_mp.cpp` | `setAnimation` override + `zk_GetAnimationId` helper in `BG_PlayAnim` |
| `src/bgame/bg_jump.cpp` | per-player jump height + jump slowdown overrides (dvar reads → accessors) |
| `src/server/sv_snapshot_mp.cpp` (also) | talker-icon fake-voice injection in `SV_SendClientMessages` |
| `src/server/sv_snapshot_mp.cpp` | forced snapshot ents in `SV_BuildClientSnapshot`; plus original: per-client `EF_NONSOLID_BMODEL` in `SV_EmitPacketEntities` |
| `src/server/sv_client_mp.cpp` | command-gate (`hook_ClientCommand` already wired) + `setNextTestClientName` in `SV_AddTestClient` |
| `src/server/sv_main_pc_mp.cpp` | rcon command-gate (`CodeCallback_RemoteCommand`) in `SVC_RemoteCommand` |
| `src/server/sv_ccmds_mp.cpp` | `setConsolePrefix` overrides the console say/tell sender in `SV_ConSay_f`/`SV_ConTell_f` |
| `src/game/player_use_mp.cpp` (also) | `setActivateOnUseButtonRelease` release-branch in `Player_UpdateActivate` |
| `src/script/script_public.h` | `VAR_RAWPOINTER` type + `var_typename` + `void *rawPointerValue` union member + decls |
| `src/script/scr_vm.cpp` | `Scr_AddPointer`/`Scr_GetPointer` (NULL->undefined) |
| `src/script/scr_variable.cpp` | `Scr_DumpScriptVariables` summary implementation |
| `src/script/scr_compiler.cpp` | benign `scope` int->ptr cast widened to `(intptr_t)` |
| `src/libcod/gsc_mysql.cpp`, `gsc_mysql_voron.cpp`, `gsc_sqlite.cpp`, `gsc_memory.cpp`, `gsc_utils.cpp` | x64 pointer-handle conversion (VAR_RAWPOINTER / widened field) |

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

### x64 pointer-truncation — RESOLVED (see "64-bit pointer correctness" section above)
All libcod modules that passed real pointers through GSC as ints (`gsc_memory`, `gsc_mysql`,
`gsc_mysql_voron`, `gsc_sqlite`, `gsc_utils` remotecommand) now use the `VAR_RAWPOINTER` type
(or a widened `intptr_t` field for `binarybuffer.address`). A whole-tree `-Wint-to-pointer-cast`
sweep is clean; the core engine had no real truncations (only the benign `scope` sentinel, fixed).
Remaining latent item: `binarybuffer` `'s'` type advances `pos += 4` for an 8-byte pointer — a
buffer-format/logic issue, not a cast; left as-is unless a mod uses `binarybuffer_write(bb,"s",..)`.

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
- **Subsystems (struct fields trimmed, need un-trimming + hooks):** jump height/slowdown DONE
  (`setJumpHeight`/`setJumpSlowdownEnable` via per-player accessors threaded into rev's native
  `bg_jump.cpp` — no `Jump_Check` replacement needed; `setStepSize`/`setProneStepSize` also DONE),
  custom sound (`playSoundFile` family),
  the `previousButtons` button-edge logic.
- **Snapshot forcing** — DONE: `addEntToSnapshots`/`removeEntFromSnapshots`/`getNumberOfEntsInSnapshot` via `SV_BuildClientSnapshot` hook (non-cached path only; the cached/anti-lag archive path and zk's `sv_autoAddSnapshotEntities` "forced-only" mode are intentionally not replicated).
- **VoroN MySQL variant 2** — PORTED: `gsc_mysql_voron.cpp`/`.hpp` added (31 functions; sync + async + 2 entity-query methods), registered under `LIBCOD_COMPILE_MYSQL_VORON`, builds with `MYSQL_VARIANT=2` (needs the user's `libmysqlclient`). Base MySQL (variant 1) was already present in upstream. Compile-verified; not link-tested in sandbox (no client lib).
- **gclient setters/readers blocked on missing rev symbols** — `setOriginAndAngles`
  (`SetClientViewAngles`), `isRechambering`/`setRechambering`/`getCurrentWeaponSlot` — **DONE**: rev has `BG_GetWeaponSlotForName`/`BG_GetWeaponSlotNameForIndex`/`BG_GetStackSlotForWeapon`/`PM_Weapon_FinishRechamber` natively + `playerState.weaponslots`/`weaponrechamber`; `GetCurrentWeaponSlot` derived via `BG_GetStackSlotForWeapon`. No stock-binary offsets needed,
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


All deliverables are in the outputs folder: the 6 `gsc_zk_*` module pairs, `gsc_zk_custom_state`,
and per-file patches for `gsc.cpp` and each modified rev source file.

## Command gate (processClientCommand / processRemoteCommand)
A CodeCallback gate: the engine routes each client/rcon command to a GSC callback (`CodeCallback_PlayerCommand`/`_RemoteCommand`), which decides whether to run it by calling `processClientCommand`/`processRemoteCommand`.
- **processClientCommand** (player method): rev already had `hook_ClientCommand` + `codecallback_playercommand` + resolution wired into `SV_ExecuteClientCommand`; it only lacked the `playerCommand` guard flag (now set around the `Scr_ExecEntThread` call) and the GSC function (calls `ClientCommand(id)` when the flag is set). Without this, a defined `CodeCallback_PlayerCommand` would silently block *all* client commands.
- **processRemoteCommand** (function): the remote side was absent. Added `codecallback_remotecommand` + resolution, a `zk_remoteCommand` store, and three libcod helpers (`zk_RemoteCommandGate` runs the callback; `zk_RemoteCommandActive`; `zk_RemoteCommandExecute` mirrors the rcon execute-with-redirect path). Injected the gate into `SVC_RemoteCommand` (sv_main_pc_mp.cpp) after the password check — no signature change, no rate-limit re-entry: the callback runs synchronously and `processRemoteCommand` executes the still-current args directly.

## Misc one-offs
- `playFxForPlayer(fxIndex, origin, [forward], [up])` (player method) — spawns an `EV_PLAY_FX` temp-entity targeted via `otherEntityNum`; self-contained, no hook. Reconciled `VectorCross`→`Vec3Cross`, `VecToAngles`→`vectoangles`, `Scr_FxParamError`→`stackError`, and built an explicit axis matrix for `AxisToAngles` (zk relied on adjacent stack locals).
- `setActivateOnUseButtonRelease(bool)` (player method) — fires the activate/use action on button *release* instead of press. Fields already existed in custom-state; added accessors and wrapped rev's `Player_UpdateActivate` (player_use_mp.cpp) with the release branch (reconciled `KEY_MASK_USE/USERELOAD`→`BUTTON_USE/USERELOAD`, the use-flag set→`PMF_RELOAD`). The original activate block is preserved byte-identical inside an `#ifdef LIBCOD` `else`.
- `setConsolePrefix(str)` — overrides the server-console `say`/`tell` sender name (default `console: `). The consumption is NOT a Com_Printf hook (earlier note was wrong); zk replaces the literal `strcpy(text,"console: ")` in `SV_ConSay_f`/`SV_ConTell_f`. Added a `zk_GetConsolePrefix()` accessor + the global, and swapped both strcpy calls in sv_ccmds_mp.cpp under `#ifdef LIBCOD`.
- `spawnGrenade(weapon, origin, [dir], [velocity], [fuseSeconds])` (player/weapons method) — launches a live grenade via rev's `fire_grenade`; no hook. Reconciled `WeaponDef_t`→`WeaponDef`, `iFuseTime`→`fuseTime`, `IsValidWeaponId`→index check.

**Still pending (need hooks/infra, not quick):** `getTagAngles` (needs the `enableLinkTo`/tag system), `processClientCommand`/`processRemoteCommand` (need a CodeCallback gate).

## Test client naming
`setNextTestClientName(name)` (sticky 1-31 char name for subsequently spawned bots) and `resetTestClientNaming()` (revert to default `bot<n>`). zk byte-patched the engine's static connect-string template; rev instead stores the name in a `gsc_bots.cpp` global and a `SV_AddTestClient` hook (sv_client_mp.cpp) substitutes `name\\%s` for the default `name\\bot%d` when one is set. Functions, not methods.

## Ballistics — status

`enableBulletImpacts`/`disableBulletImpacts`/`setFireRangeScale` were already done in earlier rounds. The
remaining 5 (`enableBulletDrop`, `disableBulletDrop`, `setBulletDrag`, `setBulletVelocity`, `setBulletModel`) are
the **bullet-drop projectile simulation** — a ~400-line core-combat subsystem. Unlike Speex it is fully portable
(rev exposes `Bullet_Fire`, `Bullet_Fire_Extended`, `Bullet_GetDamage`, `G_LocationalTrace`, `G_TempEntity`,
`Bullet_Endpos`, etc.) and sandbox-verifiable, but it touches the most gameplay-critical path, so it is being done
in phases.

**Phase 1 — DONE & verified (compiles + links):** `droppingBullet_t` struct + `MAX_DROPPING_BULLETS` + 8
`customPlayerState` fields (custom_state.hpp); the 5 GSC setters (gsc_zk_player.cpp), registered as methods. These
store state only — they are INERT until Phase 2/3.

**Phase 2 — DONE & verified (compiles clean + links):** new module `gsc_zk_ballistics.cpp` with the 9 sim functions (`zk_Bullet_Fire_Drop`, `_Think` + `_AntiLag`, `zk_Bullet_Drop_Firstpos`/`_Nextpos`/`_Free`, visual `_Create`/`_Update`/`_Finalize`). Reconciliations applied: `Bullet_CalcDamageRange`→`Bullet_GetDamage`; `Bullet_Endpos` 5-arg (added `-1`); `bRifleBullet`→`rifleBullet`; `VecToAngles`→`vectoangles`; `antilagClientStore`→`AntilagClientStore`; priority maps used by name (rev defines them as `unsigned char[]`); added `bullet` classname const via the `GScr_LoadConsts` hook; registered `g_corpseHit` + `g_bulletDrop` dvars. Functions are present-but-uncalled until Phase 3 wires them.

**Phase 3 — DONE & verified (compiles + links):** registered `g_bulletDropMaxTime` (int, default 10000ms); added helpers `zk_RunDroppingBullets()` + `zk_TryFireDroppingBullet()` to the ballistics module; hooked `G_RunFrame` (g_main_mp.cpp) to advance active bullets each frame; hooked `Bullet_Fire` (g_weapon_mp.cpp) to queue a simulated bullet instead of hitscan when `g_bulletDrop` is on and the shooter has `droppingBulletsEnabled`. Feature is LIVE (gated behind `g_bulletDrop`, default off). Dvars: `g_bulletDrop` (on/off), `g_bulletDropMaxTime` (max flight ms), reuses `g_antilag`, `g_corpseHit`.

## Sound / Speex subsystem — feasibility (investigated, NOT a quick win)

Unlike the other groups, the 11 sound/speex names are one integrated custom-sound system, not independent
functions. Three distinct situations:

**A. Hard-blocked — rev's reverse-engineered binary does not expose the needed symbol/field (cannot port):**
- `getSoundDuration`, `getSoundInfo` — both need `snd_alias_list_t->head->soundFile->soundName` to resolve an
  alias to its WAV file. rev's `snd_alias_t` (the `head`) is opaque: `soundFile`/`soundName` appear nowhere in
  the source. (getSoundDuration additionally shells out to `ffprobe` at runtime.)
- `getSoundAliasesFromFile` — needs `customSoundAliasInfo`, which zk builds by walking the engine's internal
  alias-loader globals `saLoadObjGlob`/`saLoadedObjs` via the `snd_alias_build_s` struct. All three are MISSING
  in rev (internal symbols not linkable by name).

**B. Portable, but the whole custom-sound subsystem + libspeex-dev, and NOT sandbox-verifiable:**
- `loadSpeexFile`, `saveSpeexFile` — `#include <speex/speex.h>` (encode/decode). The sandbox has the speex
  *runtime* (`libspeex.so.1`) but not the *dev header*, and apt isn't available here, so these cannot be
  compile-verified in the sandbox. They compile + link on a build machine with `libspeex-dev` installed.
- `loadSoundFile`, `playSoundFile`, `stopSoundFile`, `isPlayingSoundFile`, `getRemainingSoundFileDuration` —
  require the custom-sound packet store `voiceDataStore[MAX_CUSTOMSOUNDS][MAX_STOREDVOICEPACKETS]`
  (= [64][30720], ~8 MB/10-min sound), the constants (MAX_VOICEFRAMESIZE 160, MAX_VOICEPACKETSPERFRAME 2.56,
  MAX_CUSTOMSOUNDDURATION 10), per-player playback state, and a per-frame streaming hook feeding packets through
  the voice path (`SV_QueueVoicePacket`, already used by talker icons). Large but mechanically portable.

**C. Cleanly portable + sandbox-verifiable, but marginal alone:**
- `getSoundFileDuration` — all rev deps present; needs only the 5 constants above. It measures a file already in
  the custom-sound packet format, so it is only meaningful once the (B) playback system exists.

**Net:** 3 of 11 are hard-blocked. The other 8 form the single largest feature in the port and depend on
`libspeex-dev` + partial (structure-only) sandbox verification. This is a dedicated effort, not a drop-in round.



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


