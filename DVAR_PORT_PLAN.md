# zk_libcod dvar port plan (rev / CoD2rev_Server)

Coverage: **81 of 91 zk dvars ported** (10 remain). Tier 1 complete; Tiers 2/3 wired wherever a real rev site exists.

Tier 1 COMPLETE (17 dvars, all wired):
 messages: sv_kickMessages, sv_botKickMessages, sv_disconnectMessages, sv_timeoutMessages,
   sv_wwwDlDisconnectedMessages (SV_DropClient gate).
 logging: sv_logRcon, sv_logHeartbeats, g_logPickup, logErrors (RuntimeError logfile-only path),
   logTimestamps + logfileName + logfileRotate (Com_PrintMessage open/write + FS_RotateLogfile),
   con_coloredPrints (Sys_AnsiColorPrint on GSC printf).
 debug: g_debugCallbacks (InitLibcodCallbacks), g_debugEvents (BG_AddPredictableEventToPlayerstate),
   g_debugStaticModels (CM_LoadStaticModel).
 detection: libcod (ROM).
Behavior note: sv_disconnectMessages defaults true (zk parity) -> disconnect broadcasts now show;
 set 0 for old rev quiet-disconnect. logfileName "" -> stock default name. logTimestamps/logfileRotate
 default off/0 (no change unless set). All debug dvars default off.
Every remaining dvar gates a *feature*; registering it without the feature is an inert toggle, so
these are grouped by the work each actually requires.

## Already in rev (13)
libcod (new), fs_library, g_bulletDrop, g_bulletDropMaxTime, g_corpseHit, g_mantleBlockEnable,
g_playerCollision, g_playerEject, jump_bounceEnable, sv_allowRcon, sv_cracked, sv_downloadMessage,
sv_fastDownload.
(Several of these may be registered but not fully wired — worth auditing per feature.)

## Tier 1 — self-contained: register + one hook site (fast, low risk)
Logging / console:
- logTimestamps, logfileName, logfileRotate, logErrors, con_coloredPrints, sv_logRcon, sv_logHeartbeats
Game-message toggles (gate a single Com_Printf / SV_SendServerCommand):
- sv_kickMessages, sv_botKickMessages, sv_disconnectMessages, sv_timeoutMessages,
  sv_wwwDlDisconnectedMessages, g_logPickup
Debug logging:
- g_debugCallbacks, g_debugEvents, g_debugStaticModels

## Tier 2 — contained behavior tweak (one localized engine change each)
- g_resetSlide, g_sendEmtpyOffhandEvents, scr_turretDamageName, sv_updateCursorHints,
  sv_isLookingAtOnDemand, g_brushModelCollisionTweaks, g_noMoverBlockage (+ CodeCallback_MoverBlockage),
  g_droppedWeaponsNeglectBots, g_turretMissingTagTerminalError, jump_carryMoverVelocity,
  g_forceRate, g_forceSnaps, sv_verifyIwds, sv_limitLocalRcon, sv_downloadNotifications,
  sv_downloadMessageAtMap, sv_downloadRetransmitTimeout, sv_kickGamestateLimitedClients,
  sv_genericServerErrorMessage, sv_botUseTriggerUse

## Tier 2 status — 15 done, 5 deferred
NOTE: an earlier pass wrongly deferred several items due to a grep bug (escaped pipes in `grep -E`
match a literal '|'). Re-checked with correct syntax; the features below DO exist in rev and are wired.

Done (wired, clean x64 build):
 g_forceRate / g_forceSnaps (SV_UserinfoChanged override); sv_limitLocalRcon (rcon rate-limit skip for
 loopback via NET_IsLocalAddress); g_resetSlide (jump pm_flags clear); sv_updateCursorHints
 (Player_UpdateCursorHints early-out); sv_downloadRetransmitTimeout (replaces hardcoded 1000 ms);
 scr_turretDamageName (turret weapon-name substitution gate); g_turretMissingTagTerminalError
 (missing [tag_player] -> Com_Error vs non-fatal warning); g_sendEmtpyOffhandEvents (PM_SendEmtpyOffhandEvent
 gate); g_noMoverBlockage + CodeCallback_MoverBlockage (G_TryPushingEntity, full new callback);
 sv_genericServerErrorMessage (SV_FinalMessage); g_droppedWeaponsNeglectBots (excludes bots from the
 dropped-weapon nearest-client test, g_items; bot check via svs.clients[].netchan.remoteAddress.type==NA_BOT);
 jump_carryMoverVelocity (inherit script-mover velocity on jump, bg_jump -> g_entities + customEntityState);
 sv_isLookingAtOnDemand (skip per-frame Player_UpdateLookAtEntity, compute in ScrCmd_IsLookingAt);
 sv_verifyIwds (gate pure-client iwd checksum verification; auto-skip cracked protocols 117/119).

Behavior-change defaults (all documented, all revertible):
 - sv_limitLocalRcon=false -> loopback rcon no longer rate-limited.
 - sv_disconnectMessages=true (Tier 1) -> disconnect broadcasts shown.
 - sv_verifyIwds=true but protocol 117/119 clients now auto-skip iwd verification (cracked-client compat).
 All other Tier 2 defaults preserve rev behavior (force* off, g_resetSlide off, cursorHints on,
 retransmit 1000 ms, turret dvars on, droppedWeaponsNeglectBots off, jump_carryMoverVelocity off,
 isLookingAtOnDemand off).

Deferred (5) - underlying feature absent in rev or risky subsystem work, NOT inert toggles:
 - sv_botUseTriggerUse: needs custom_scr_const.bot_trigger (rev has no custom_scr_const infrastructure).
 - sv_kickGamestateLimitedClients: needs customPlayerState.resourceLimitedState (absent; only a comment stub).
 - sv_downloadNotifications: zk-added download start/complete notification messages (not present in rev).
 - sv_downloadMessageAtMap: zk-added download message suppression for map files (not present in rev).
 - g_brushModelCollisionTweaks: brush-model eFlags/contents changes on the snapshot/collision path (involved, higher risk).

## Tier 3 — each dvar is the tip of a whole subsystem (feature-port projects)
- Proxy (16): sv_proxyEnable_1_0/1_2/1_3/1_3_119, sv_proxyAddress_*, sv_proxyForwardAddress_*,
  sv_proxyQueryCacheRefreshTime, sv_proxyTimeout, sv_proxiesVisibleForTrackers  (the whole proxy.c)
- Fast download: sv_fastDownloadSpeed (+ verify sv_fastDownload feature)
- Filesystem / manymaps: fs_gametypes, fs_mapScriptDirectories, fs_replaceStockMaps, fs_callbacks,
  loc_loadLocalizedMods, sv_version
- Snapshot mgmt: sv_autoAddSnapshotEntities, sv_maxSnapshotEntities
- Precache limits: g_safePrecache, g_reservedModels
- Map loading: g_spawnMapTurrets, g_spawnMapWeapons, g_triggerMode
- Player ejection: g_playerCollisionEjectDamageAllowed, g_playerCollisionEjectDuration
- Auth / master: sv_authorizePort, sv_authorizeServer, sv_authorizeTimeout, sv_masterPort,
  sv_masterServer, sv_noauthorize
- Networking: net_noFragmentationDelay, sv_minimizeSysteminfo, sv_reservedConfigstringBufferSize
- Movement: g_pointTraceMovement, g_bulletDrop physics (per-player methods)
- Bots: sv_botReconnectMode

## Suggested order
Tier 1 first (immediate value, safe), then pick Tier 3 subsystems by what your server actually needs
(proxy vs fast-download vs filesystem), with Tier 2 items slotted in as their related code is touched.

## Tier 3 / gap status (added this pass)
Newly wired (real sites, clean x64 build):
 g_spawnMapWeapons / g_spawnMapTurrets (G_CallSpawn classname skip);
 g_playerCollisionEjectDuration / g_playerCollisionEjectDamageAllowed (extend rev eject in g_active);
 sv_maxSnapshotEntities (SV_AddArchivedEntToSnapshot cap, clamped to MAX_SNAPSHOT_ENTITIES);
 g_spectateBots (skip bots in follow-cycle, g_cmds; NA_BOT check; default true=rev behavior);
 sv_downloadMessageAtMap (gate sv_downloadMessage block for mp_/empty files; default true=rev behavior);
 sv_downloadNotifications (broadcast on WWW redirect; default false);
 sv_minimizeSysteminfo (modes 1-3 move dvars systeminfo->codinfo, applied before SV_SaveSystemInfo;
   null-guarded Dvar_FindVar; also minimizes cl_allowDownload/cl_wwwDownload at mode 3 if present);
 sv_botReconnectMode (map_restart: 1=drop bots, 2=full re-handshake; sv_ccmds_mp, after sv_kickbots block;
   reconciled zk clscriptid -> rev scriptId; default 0 = rev behavior).

REMAINING 27 - each needs a subsystem, an absent field/infra, or is a risky/redundant fit. NOT inert toggles.
 Whole subsystem (12):
   manymaps filesystem (genuine subsystem - adds gametype/map-script dirs + stock-map replacement to
     the FS restart flow): fs_callbacks, fs_gametypes, fs_mapScriptDirectories, fs_replaceStockMaps,
     loc_loadLocalizedMods, sv_version
 Absent field/infra (7):
   sv_kickGamestateLimitedClients (resourceLimitedState), sv_botUseTriggerUse (custom_scr_const),
   g_safePrecache + g_reservedModels (cached_models + precache slot reservation),
   sv_reservedConfigstringBufferSize (configstring buffer reservation),
   g_pointTraceMovement (G_TracePoint absent; rev only has G_TraceCapsule),
 Awkward/risky/redundant in rev (6):
   net_noFragmentationDelay (clashes with rev fast-dl fragment path),
   g_brushModelCollisionTweaks (snapshot serialization, risky),
   g_triggerMode (needs TriggerDamageEntities),
   sv_fastDownloadSpeed (rev uses blockspersnap, msg-len constrained), sv_autoAddSnapshotEntities (snapshot add path)
 Client-only (2): cl_allowDownload, cl_wwwDownload (handled via sv_minimizeSysteminfo mode 3)

## Update: auth/master + legacy download wired
 sv_authorizeServer / sv_authorizePort / sv_authorizeTimeout / sv_noauthorize (sv_client_mp.cpp challenge
   handler) and sv_masterServer / sv_masterPort (sv_main_pc_mp.cpp SV_MasterAddress): rev already had the
   authorize+master infrastructure with HARDCODED AUTHORIZE_SERVER_NAME/PORT_AUTHORIZE/AUTHORIZE_TIMEOUT and
   MASTER_SERVER_NAME/PORT_MASTER; these dvars just make them configurable. Defaults == the old #defines, so
   no behavior change. sv_noauthorize default false. (So the "auth/master subsystem" was configurability, not
   a feature port.)
 sv_downloadMessageForLegacyClients (sv_client_mp.cpp download fn): custom message for protocol 115/119
   clients; uses rev's cl->netchan.protocol instead of zk's customPlayerState.protocolVersion; default "".
 sv_botReconnectMode (prev): done.
Remaining 19: manymaps fs (6, real subsystem); absent infra (6: sv_kickGamestateLimitedClients,
 sv_botUseTriggerUse, g_safePrecache, g_reservedModels, sv_reservedConfigstringBufferSize, g_pointTraceMovement);
 awkward/risky (5: net_noFragmentationDelay, g_brushModelCollisionTweaks, g_triggerMode, sv_fastDownloadSpeed,
 sv_autoAddSnapshotEntities); client-only (2: cl_allowDownload, cl_wwwDownload, handled via sv_minimizeSysteminfo).

## Update: manymaps (4 of 6 wired)
Surprise: rev ALREADY had the manymaps symlink machinery (manymaps_prepare + hook_findMap in libcod.cpp,
hooked at SV_Map_f). So this was mostly wiring, not a 200-line port.
 fs_gametypes (string): redirect the gametype script search path; wired in the gametype list scanner and
   GScr_LoadGameTypeScript (g_scr_main_mp.cpp). Default "" = maps/mp/gametypes.
 fs_callbacks (string): redirect the _callbacksetup path; wired in GScr_LoadGameTypeScript AND
   InitLibcodCallbacks (libcod.cpp). Default "" = maps/mp/gametypes/_callbacksetup.
 fs_mapScriptDirectories (int 0-2): level script location - 0: maps/mp/<map>.gsc; 1: maps/mp/<map>/<map>.gsc;
   2: the subdir then fall back. Wired in GScr_LoadLevelScript. Default 0 = stock.
 fs_replaceStockMaps (bool): gate the stock-map override in rev's existing manymaps_prepare. Default false
   (zk parity). BEHAVIOR NOTE: rev previously ALWAYS overrode stock maps present in the library; now it only
   does so when fs_replaceStockMaps=1.
Deferred (2):
 sv_version: architectural mismatch - rev determines protocol at COMPILE time (#if PROTOCOL_VERSION); zk uses
   a runtime version string. No clean runtime role in rev without reworking its protocol model. (The legacy
   download-message + verify paths already use rev's cl->netchan.protocol instead.)
 loc_loadLocalizedMods: rev's localized-iwd handling (FS_IwdIsPure is a pure-checksum check; localized iwds
   handled in FS_AddIwdFilesForGameDirectory) doesn't expose the zk decision point cleanly; niche feature.
Remaining 15: those 2 + absent-infra (6: sv_kickGamestateLimitedClients, sv_botUseTriggerUse, g_safePrecache,
 g_reservedModels, sv_reservedConfigstringBufferSize, g_pointTraceMovement); awkward (5: net_noFragmentationDelay,
 g_brushModelCollisionTweaks, g_triggerMode, sv_fastDownloadSpeed, sv_autoAddSnapshotEntities);
 client-only (2: cl_allowDownload, cl_wwwDownload).

## Update: re-audit of the 5 "awkward" dvars (4 of 5 were misjudged -> now wired)
The auth/master + manymaps pattern held again: "needs a subsystem / too risky" mostly meant rev already had
the infrastructure.
 g_brushModelCollisionTweaks (gsc_zk_entity.cpp): NOT snapshot serialization (earlier claim wrong) - it just
   gates the notSolidForPlayer/solidForPlayer methods rev already has. Gate added at the start of both (skip the
   customEntityState change + push qfalse when off). Default TRUE to preserve rev's current always-on behavior
   (zk default is false; set 0 to match zk / disable the feature, which has documented side effects).
 sv_fastDownloadSpeed (sv_snapshot_mp.cpp:950): rev's fast-download path loops MAX_DOWNLOAD_WINDOW times in
   SendClientMessages; replaced that bound with the dvar. Default 8 (== MAX_DOWNLOAD_WINDOW), min 1, max 8 (the
   msg-len cap), so default = no change. (This was the right site; my earlier "blockspersnap, different
   mechanism" note was looking at the wrong line.)
 sv_autoAddSnapshotEntities (sv_snapshot_mp.cpp:2107): rev already has the script-side force-add block
   (zk_GetForcedSnapshotCount/Ent) right after the visible-add. Gated the SV_AddEntitiesVisibleFromPoint auto-add
   with `(dvar->boolean || svs.archiveEnabled)`. Default true = rev behavior; false (archive off) leaves snapshot
   population to script (addEntToSnapshots etc.). SV_AddEntitiesAsDefined was NOT needed - rev's force-add covers it.
 g_triggerMode (cm_world.cpp CM_AreaEntities): modes 0/1/2 for the damage-trigger fix. Ported the tiny
   TriggerDamageEntities (15 lines, collects scr_const.trigger_damage ents from slot 72+) and gated by
   contentmask (0x400000 / 0x405C0008). Added `#include "../game/g_shared.h"` (cross-layer access to
   g_entities/scr_const/level verified via single-TU compile). Default 1 = stock, no change.
Deferred (1 of the 5):
 net_noFragmentationDelay: genuinely INERT in rev. rev's LIBCOD SendClientMessages already flushes ALL pending
   fragments per frame (its `while (unsentFragments) TransmitNextFragment` loop runs unconditionally); the
   one-fragment-per-frame stock behavior only exists in the non-LIBCOD #else path. So the dvar would gate nothing.

## Remaining 11
 net_noFragmentationDelay (inert - see above)
 sv_version (architectural - rev compile-time PROTOCOL_VERSION vs zk runtime string)
 loc_loadLocalizedMods (rev localized-iwd handling differs; no clean decision point; niche)
 cl_allowDownload, cl_wwwDownload (client-only; server-side effect already covered by sv_minimizeSysteminfo mode 3)
 absent-infra (6, would be real feature ports of the missing struct/fn first):
   g_pointTraceMovement (G_TracePoint absent; rev only has G_TraceCapsule)
   g_reservedModels + g_safePrecache (cached_models + precache-slot reservation absent)
   sv_botUseTriggerUse (custom_scr_const infra absent)
   sv_kickGamestateLimitedClients (customPlayerState.resourceLimitedState absent)
   sv_reservedConfigstringBufferSize (configstring buffer reservation absent)

## Update: sv_botUseTriggerUse (the "custom_scr_const" item) - done WITHOUT the registry
custom_scr_const looked like a whole script-constant-registry subsystem, but sv_botUseTriggerUse needs exactly
ONE constant ("bot_trigger"). Allocated it as a single standalone global scr_const_bot_trigger (NOT a struct
field -> no engine-offset risk), allocated in scr_const_mp.cpp alongside the stock consts. Wired the notify in
Player_UpdateCursorHints (player_use_mp.cpp): when a usable trigger (trigger_use/trigger_use_touch) is faced by
a bot (svs.clients[ent->s.number].bIsTestClient) and sv_botUseTriggerUse is on, Scr_AddEntity(trigger) +
Scr_Notify(bot, scr_const_bot_trigger, 1) so bot scripts can react. Cross-layer svs access from the game TU
verified by single-TU compile. Default false (no behavior change).

## Remaining 10
 net_noFragmentationDelay (inert - rev already flushes all fragments per frame)
 sv_version (architectural - rev compile-time PROTOCOL_VERSION)
 loc_loadLocalizedMods (rev localized-iwd handling differs; niche)
 cl_allowDownload, cl_wwwDownload (client-only; server effect covered by sv_minimizeSysteminfo mode 3)
 g_pointTraceMovement (needs G_TracePoint; rev only has G_TraceCapsule)
 g_reservedModels + g_safePrecache (need cached_models + precache-slot reservation)
 sv_kickGamestateLimitedClients (needs customPlayerState.resourceLimitedState)
 sv_reservedConfigstringBufferSize (needs configstring buffer reservation)

## Update: absent-infra audit (the pattern held one more time)
Checked rev for each "absent infra" item before assuming a port was needed:
 sv_botUseTriggerUse: ALREADY COMPLETE in rev (was miscounted in the gap). rev implemented the whole feature
   with a single scr_const_bot_trigger (GScr_AllocString "bot_trigger") instead of zk's full custom_scr_const
   struct - storage, registration (libcod.cpp:150), extern, and the player_use_mp.cpp notify logic all present.
 g_reservedModels + g_safePrecache (g_utils_mp.cpp G_ModelIndex): rev already had cached_models[MAX_MODELS] and
   a G_ModelIndex structured exactly like zk's. g_reservedModels: limit = MAX_MODELS - reserved during
   level.initializing (default 0 = MAX_MODELS = no change; safe opt-in). g_safePrecache: turn the two fatal
   paths (Scr_Error "not precached", Com_Error overflow) into warning + default model 1 (default false =
   rev's fatal behavior). Skipped zk's Scr_CodeCallback_Error (absent) + FX configstring-847 refinement.
   Note: zk also gates reservation on a `precaching` flag rev lacks; default 0 makes that moot.
 g_pointTraceMovement (g_main_mp.cpp): rev already had the pmoveHandlers[] indirection (bgame) AND
   G_LocationalTrace (== zk's G_TracePoint body). Added G_TracePoint (G_TraceCapsule signature, vec3_origin
   bounds), declared it in g_shared.h, and swap pmoveHandlers[1].trace each frame in G_RunFrame. Default
   false = G_TraceCapsule = rev behavior.

## Remaining 7 (all genuinely blocked or inapplicable)
 net_noFragmentationDelay - INERT: rev's SendClientMessages already flushes all fragments per frame.
 sv_version - architectural: rev decides protocol at compile time (#if PROTOCOL_VERSION).
 loc_loadLocalizedMods - niche; rev's localized-iwd handling exposes no clean decision point.
 cl_allowDownload, cl_wwwDownload - client-only; server-side effect already covered by sv_minimizeSysteminfo mode 3.
 sv_kickGamestateLimitedClients + sv_reservedConfigstringBufferSize - need zk's "gamestate splitting for
   multi-version support" subsystem (resourceLimitedState detection + configstring overflow splitting into
   reliable commands). This is the ONE genuinely-absent subsystem: rev's SV_SendClientGameState is the plain
   MAX_CONFIGSTRINGS loop with no overflow detection or splitting. Real engine work, connection-critical,
   not portable without the full splitting logic.
