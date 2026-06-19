# zk_libcod dvar port plan (rev / CoD2rev_Server)

Coverage: **37 of 93 registered**. Tier 1 COMPLETE (17/17). Tier 2: 8 done, rest reclassified.

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

## Tier 2 status
Done (wired, building x64):
 g_forceRate / g_forceSnaps (SV_UserinfoChanged override); sv_limitLocalRcon (rcon rate-limit skip for
 loopback, via NET_IsLocalAddress); g_resetSlide (jump pm_flags clear, bg_jump.cpp); sv_updateCursorHints
 (Player_UpdateCursorHints early-out); sv_downloadRetransmitTimeout (replaces hardcoded 1000 ms);
 scr_turretDamageName (gates turret weapon-name substitution, g_combat); g_turretMissingTagTerminalError
 (missing [tag_player] -> Com_Error vs non-fatal warning, g_client).
Defaults preserve rev behavior except sv_limitLocalRcon (loopback rcon no longer rate-limited by default).

Deferred - NOT clean toggles; each needs a mini feature-port or the underlying feature is absent in rev:
 - jump_carryMoverVelocity: needs g_entities + customEntityState velocity reach from bgame (layering).
 - g_sendEmtpyOffhandEvents: EV_EMPTY_OFFHAND / PM_SendEmtpyOffhandEvent path not present in rev.
 - g_droppedWeaponsNeglectBots: dropped-weapon removal-by-distance loop not located in rev.
 - sv_isLookingAtOnDemand: needs on-demand IsLookingAt path (rev has per-frame Player_UpdateLookAtEntity).
 - sv_botUseTriggerUse: needs custom_scr_const.bot_trigger + the nested use/cursor site.
 - sv_downloadNotifications / sv_downloadMessageAtMap: zk-added download notification messages (absent).
 - g_brushModelCollisionTweaks: brush-model eFlags/contents collision changes (snapshot path, involved).
 - g_noMoverBlockage: needs a new CodeCallback_MoverBlockage callback.
 - sv_verifyIwds: iwd checksum verification feature.
 - sv_kickGamestateLimitedClients: needs customPlayerState.resourceLimitedState gamestate-overflow feature.
 - sv_genericServerErrorMessage: needs the generic server-error message site.

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

