# zk_libcod dvar port plan (rev / CoD2rev_Server)

Coverage: **~44 of 93 registered** (12 pre-existing + 17 Tier 1 + 15 Tier 2).

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

