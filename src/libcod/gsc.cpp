#include "gsc.hpp"
#include "gsc_zk_utils.hpp"
#include "gsc_zk_entity.hpp"
#include "gsc_zk_weapons.hpp"
#include "gsc_zk_level.hpp"
#include "gsc_zk_player.hpp"
#include "match.hpp"
#include "cod2rev_updater.hpp"

#ifdef LIBCOD

const char *stackGetParamTypeAsString(int param)
{
	if (param >= Scr_GetNumParam())
		return "UNDEFINED";

	VariableValue *var;
	var = Scr_GetValue(param);

	switch (var->type)
	{
	case 0:
		return "UNDEFINED";

	case 1:
		return "OBJECT";

	case 2:
		return "STRING";

	case 3:
		return "LOCALIZED_STRING";

	case 4:
		return "VECTOR";

	case 5:
		return "FLOAT";

	case 6:
		return "INT";

	case 7:
		return "CODEPOS";

	case 8:
		return "PRECODEPOS";

	case 9:
		return "FUNCTION";

	case 10:
		return "STACK";

	case 11:
		return "ANIMATION";

	case 12:
		return "DEVELOPER_CODEPOS";

	case 13:
		return "INCLUDE_CODEPOS";

	case 14:
		return "THREAD_LIST";

	case 15:
		return "THREAD_1";

	case 16:
		return "THREAD_2";

	case 17:
		return "THREAD_3";

	case 18:
		return "THREAD_4";

	case 19:
		return "STRUCT";

	case 20:
		return "REMOVED_ENTITY";

	case 21:
		return "ENTITY";

	case 22:
		return "ARRAY";

	case 23:
		return "REMOVED_THREAD";

	default:
		return "UNKNOWN TYPE";
	}
}

scr_function_t scriptFunctions[] =
{
	{"http_fetch", gsc_http_fetch, 0},
	{"matchUploadData", gsc_match_uploadData, 0},
	{"matchSetData", gsc_match_setData, 0},
	{"matchGetData", gsc_match_getData, 0},
	{"matchRedownloadData", gsc_match_redownloadData, 0},
	{"matchClearData", gsc_match_clearData, 0},
	{"matchIsActivated", gsc_match_isActivated, 0},
	{"matchCancel", gsc_match_cancel, 0},
	{"matchFinish", gsc_match_finish, 0},
	{"websocket_connect", gsc_websocket_connect, 0},
	{"websocket_sendText", gsc_websocket_sendText, 0},
	{"websocket_close", gsc_websocket_close, 0},

#if LIBCOD_COMPILE_EXEC == 1
	{"exec", gsc_exec, 0},
	{"exec_async_create", gsc_exec_async_create, 0},
	{"exec_async_create_nosave", gsc_exec_async_create_nosave, 0},
	{"exec_async_checkdone", gsc_exec_async_checkdone, 0},
#endif

#if LIBCOD_COMPILE_LEVEL == 1
	// --- ported from zk_libcod (gsc_zk_level.cpp) ---
	{"getMovers", gsc_zk_level_getmovers, 0},
	{"getEntityCount", gsc_zk_level_getentitycount, 0},
	{"setNorthYaw", gsc_zk_level_setnorthyaw, 0},
	{"getSavePersist", gsc_zk_level_getsavepersist, 0},
	{"setSavePersist", gsc_zk_level_setsavepersist, 0},
	// aliases: zk names -> rev's identical configstring-index finders
	{"findConfigStringIndex", gsc_zk_utils_findconfigstringindex, 0},
	{"findConfigStringIndexOriginal", gsc_zk_utils_findconfigstringindexoriginal, 0},
	// --- end zk port ---
	{"getnumberofstaticmodels", gsc_level_getnumberofstaticmodels, 0},
	{"getstaticmodelname", gsc_level_getstaticmodelname, 0},
	{"getstaticmodelorigin", gsc_level_getstaticmodelorigin, 0},
#endif

#if LIBCOD_COMPILE_MEMORY == 1
	{"memory_malloc", gsc_memory_malloc, 0},
	{"memory_free", gsc_memory_free, 0},
	{"memory_int_get", gsc_memory_int_get, 0},
	{"memory_int_set", gsc_memory_int_set, 0},
	{"memory_memset", gsc_memory_memset, 0},
	{"binarybuffer_new", gsc_binarybuffer_new, 0},
	{"binarybuffer_free", gsc_binarybuffer_free, 0},
	{"binarybuffer_seek", gsc_binarybuffer_seek, 0},
	{"binarybuffer_write", gsc_binarybuffer_write, 0},
	{"binarybuffer_read", gsc_binarybuffer_read, 0},
#endif

#if LIBCOD_COMPILE_MYSQL == 1
	{"mysql_init", gsc_mysql_init, 0},
	{"mysql_real_connect", gsc_mysql_real_connect, 0},
	{"mysql_close", gsc_mysql_close, 0},
	{"mysql_query", gsc_mysql_query, 0},
	{"mysql_errno", gsc_mysql_errno, 0},
	{"mysql_error", gsc_mysql_error, 0},
	{"mysql_affected_rows", gsc_mysql_affected_rows, 0},
	{"mysql_store_result", gsc_mysql_store_result, 0},
	{"mysql_num_rows", gsc_mysql_num_rows, 0},
	{"mysql_num_fields", gsc_mysql_num_fields, 0},
	{"mysql_field_seek", gsc_mysql_field_seek, 0},
	{"mysql_fetch_field", gsc_mysql_fetch_field, 0},
	{"mysql_fetch_row", gsc_mysql_fetch_row, 0},
	{"mysql_free_result", gsc_mysql_free_result, 0},
	{"mysql_real_escape_string", gsc_mysql_real_escape_string, 0},
	{"mysql_async_create_query", gsc_mysql_async_create_query, 0},
	{"mysql_async_create_query_nosave", gsc_mysql_async_create_query_nosave, 0},
	{"mysql_async_getdone_list", gsc_mysql_async_getdone_list, 0},
	{"mysql_async_getresult_and_free", gsc_mysql_async_getresult_and_free, 0},
	{"mysql_async_initializer", gsc_mysql_async_initializer, 0},
	{"mysql_reuse_connection", gsc_mysql_reuse_connection, 0},
#endif

#if LIBCOD_COMPILE_PLAYER == 1
	{"kick2", gsc_kick_slot, 0},
#endif

#if LIBCOD_COMPILE_SQLITE == 1
	{"sqlite_open", gsc_sqlite_open, 0},
	{"sqlite_query", gsc_sqlite_query, 0},
	{"sqlite_close", gsc_sqlite_close, 0},
	{"sqlite_escape_string", gsc_sqlite_escape_string, 0},
	{"sqlite_databases_count", gsc_sqlite_databases_count, 0},
	{"sqlite_tasks_count", gsc_sqlite_tasks_count, 0},
	{"async_sqlite_initialize", gsc_async_sqlite_initialize, 0},
	{"async_sqlite_create_query", gsc_async_sqlite_create_query, 0},
	{"async_sqlite_create_query_nosave", gsc_async_sqlite_create_query_nosave, 0},
	{"async_sqlite_checkdone", gsc_async_sqlite_checkdone, 0},
#endif

#if LIBCOD_COMPILE_UTILS == 1
	// --- ported from zk_libcod (gsc_zk_utils.cpp) ---
	{"abs", gsc_zk_utils_abs, 0},
	{"atan2", gsc_zk_utils_atan2, 0},
	{"ceil", gsc_zk_utils_ceil, 0},
	{"floor", gsc_zk_utils_floor, 0},
	{"chr", gsc_zk_utils_chr, 0},
	{"ord", gsc_zk_utils_ord, 0},
	{"tohex", gsc_zk_utils_tohex, 0},
	{"fromhex", gsc_zk_utils_fromhex, 0},
	{"roundto", gsc_zk_utils_roundto, 0},
	{"error", gsc_zk_utils_error, 0},
	{"executecommand", gsc_zk_utils_executecommand, 0},
	{"getmilliseconds", gsc_zk_utils_getmilliseconds, 0},
	{"getmicroseconds", gsc_zk_utils_getmicroseconds, 0},
	{"getcvarflags", gsc_zk_utils_getcvarflags, 0},
	{"getsystemtime", gsc_zk_utils_getsystemtime, 0},
	{"getlocaltime", gsc_zk_utils_getlocaltime, 0},
	{"fremove", gsc_zk_utils_fremove, 0},
	{"loaddir", gsc_zk_utils_loaddir, 0},
	{"logprintconsole", gsc_zk_utils_logprintconsole, 0},
	{"getsurfacename", gsc_zk_utils_getsurfacename, 0},
	{"pow", gsc_utils_exponent, 0}, // alias: rev's exponent == zk's pow
	// --- end zk port ---
	{"printf", gsc_utils_printf, 0},
	{"printoutofband", gsc_utils_outofbandprint, 0},
	{"getarraykeys", gsc_utils_getarraykeys, 0},
	{"getascii", gsc_utils_getAscii, 0},
	{"toupper", gsc_utils_toupper, 0},
	{"system", gsc_utils_system, 0},
	{"exponent", gsc_utils_exponent, 0},
	{"round", gsc_utils_round, 0},
	{"file_link", gsc_utils_file_link, 0},
	{"file_unlink", gsc_utils_file_unlink, 0},
	{"file_exists", gsc_utils_file_exists, 0},
	{"fs_loaddir", gsc_utils_FS_LoadDir, 0},
	{"gettype", gsc_utils_getType, 0},
	{"float", gsc_utils_float, 0},
	{"cmd_executestring", gsc_utils_ExecuteString, 0},
	{"sendgameservercommand", gsc_utils_sendgameservercommand, 0},
	{"scandir", gsc_utils_scandir, 0},
	{"fopen", gsc_utils_fopen, 0},
	{"fread", gsc_utils_fread, 0},
	{"fwrite", gsc_utils_fwrite, 0},
	{"fclose", gsc_utils_fclose, 0},
	{"fsize", gsc_utils_fsize, 0},
	{"ftime", gsc_utils_ftime, 0},
	{"sprintf", gsc_utils_sprintf, 0},
	{"getsystemtime", gsc_utils_getsystemtime, 0},
	{"getserverstarttime", gsc_utils_getserverstarttime, 0},
	{"getlocaltime", gsc_utils_getlocaltime, 0},
	{"g_findconfigstringindex", gsc_G_FindConfigstringIndex, 0},
	{"g_findconfigstringindexoriginal", gsc_G_FindConfigstringIndexOriginal, 0},
	{"getconfigstring", gsc_get_configstring, 0},
	{"setconfigstring", gsc_set_configstring, 0},
	{"makelocalizedstring", gsc_make_localized_string, 0},
	{"sqrt", gsc_utils_sqrt, 0},
	{"sqrtinv", gsc_utils_sqrtInv, 0},
	{"getlasttestclientnumber", gsc_utils_getlasttestclientnumber, 0},
	{"getmaxmsglen", gsc_utils_getmaxmsglen, 0},
	{"bullethiteffect", gsc_utils_bullethiteffect, 0},
	{"vectorscale", gsc_utils_vectorscale, 0},
	{"remove_file", gsc_utils_remove_file, 0},
	{"putchar", gsc_utils_putchar, 0},
	{"remotecommand", gsc_utils_remotecommand, 0},
#endif

#if LIBCOD_COMPILE_WEAPONS == 1
	// --- ported from zk_libcod (gsc_zk_weapons.cpp) ---
	{"isSemiAutoWeapon", gsc_zk_weapons_issemiautoweapon, 0},
	{"getWeaponRaiseTime", gsc_zk_weapons_getweaponraisetime, 0},
	{"setWeaponRaiseTime", gsc_zk_weapons_setweaponraisetime, 0},
	{"getWeaponFuseTime", gsc_zk_weapons_getweaponfusetime, 0},
	{"setWeaponFuseTime", gsc_zk_weapons_setweaponfusetime, 0},
	{"getWeaponMoveSpeedScale", gsc_zk_weapons_getmovespeedscale, 0},
	{"setWeaponMoveSpeedScale", gsc_zk_weapons_setmovespeedscale, 0},
	{"getWeaponDisplayName", gsc_zk_weapons_getweapondisplayname, 0},
	// --- end zk port ---
	{"getweaponmaxammo", gsc_weapons_getweaponmaxammo, 0},
	{"setweaponmaxammo", gsc_weapons_setweaponmaxammo, 0},
	{"getweaponclipsize", gsc_weapons_getweaponclipsize, 0},
	{"setweaponclipsize", gsc_weapons_setweaponclipsize, 0},
	{"getweapondamage", gsc_weapons_getweapondamage, 0},
	{"setweapondamage", gsc_weapons_setweapondamage, 0},
	{"getweaponmeleedamage", gsc_weapons_getweaponmeleedamage, 0},
	{"setweaponmeleedamage", gsc_weapons_setweaponmeleedamage, 0},
	{"getweaponfiretime", gsc_weapons_getweaponfiretime, 0},
	{"setweaponfiretime", gsc_weapons_setweaponfiretime, 0},
	{"getweaponmeleetime", gsc_weapons_getweaponmeleetime, 0},
	{"setweaponmeleetime", gsc_weapons_setweaponmeleetime, 0},
	{"getweaponreloadtime", gsc_weapons_getweaponreloadtime, 0},
	{"setweaponreloadtime", gsc_weapons_setweaponreloadtime, 0},
	{"getweaponreloademptytime", gsc_weapons_getweaponreloademptytime, 0},
	{"setweaponreloademptytime", gsc_weapons_setweaponreloademptytime, 0},
	{"getweaponcookable", gsc_weapons_getweaponcookable, 0},
	{"setweaponcookable", gsc_weapons_setweaponcookable, 0},
	{"getweaponhitlocmultiplier", gsc_weapons_getweaponhitlocmultiplier, 0},
	{"setweaponhitlocmultiplier", gsc_weapons_setweaponhitlocmultiplier, 0},
	{"getloadedweapons", gsc_weapons_getloadedweapons, 0},
#endif

#ifdef EXTRA_FUNCTIONS_INC
#include "extra/functions.hpp"
#endif

	{NULL, NULL, 0} /* terminator */
};

xfunction_t Scr_GetCustomFunction(const char **fname, qboolean *fdev)
{
	for (int i = 0; scriptFunctions[i].name; i++)
	{
		if (strcasecmp(*fname, scriptFunctions[i].name))
			continue;

		scr_function_t func = scriptFunctions[i];

		*fname = func.name;
		*fdev = func.developer;

		return func.call;
	}

	return NULL;
}

scr_method_t scriptMethods[] =
{
#if LIBCOD_COMPILE_BOTS == 1
	{"setwalkdir", gsc_bots_set_walkdir, 0},
	{"setlean", gsc_bots_set_lean, 0},
	{"setbotstance", gsc_bots_set_stance, 0},
	{"thrownade", gsc_bots_thrownade, 0},
	{"fireweapon", gsc_bots_fireweapon, 0},
	{"meleeweapon", gsc_bots_meleeweapon, 0},
	{"reloadweapon", gsc_bots_reloadweapon, 0},
	{"adsaim", gsc_bots_adsaim, 0},
	{"switchtoweaponid", gsc_bots_switchtoweaponid, 0},
#endif

#if LIBCOD_COMPILE_ENTITY == 1
	{"setalive", gsc_entity_setalive, 0},
	{"setbounds", gsc_entity_setbounds, 0},
	// --- ported from zk_libcod (gsc_zk_entity.cpp) ---
	{"getClipmask", gsc_zk_entity_getclipmask, 0},
	{"setClipmask", gsc_zk_entity_setclipmask, 0},
	{"getVmax", gsc_zk_entity_getvmax, 0},
	{"getVmin", gsc_zk_entity_getvmin, 0},
	{"isTurret", gsc_zk_entity_isturret, 0},
	{"isLinkedTo", gsc_zk_entity_islinkedto, 0},
	{"getTurretOwner", gsc_zk_entity_getturretowner, 0},
	{"setLight", gsc_zk_entity_setlight, 0},
	{"hasTag", gsc_zk_entity_hastag, 0},
	{"getTagOrigin", gsc_zk_entity_gettagorigin, 0},
	// --- end zk port ---
#endif

#if LIBCOD_COMPILE_PLAYER == 1
	// --- ported from zk_libcod (gsc_zk_player.cpp) ---
	{"earthquakeForPlayer", gsc_zk_player_earthquakeforplayer, 0},
	{"clientHasClientMuted", gsc_zk_player_clienthasclientmuted, 0},
	{"playFxOnTagForPlayer", gsc_zk_player_playfxontagforplayer, 0},
	{"enableSilent", gsc_zk_player_enablesilent, 0},
	{"disableSilent", gsc_zk_player_disablesilent, 0},
	{"overrideContents", gsc_zk_player_overridecontents, 0},
	{"setWeaponSpreadScale", gsc_zk_player_setweaponspreadscale, 0},
	{"setTurretSpreadScale", gsc_zk_player_setturretspreadscale, 0},
	{"setHiddenFromScoreboard", gsc_zk_player_sethiddenfromscoreboard, 0},
	{"isHiddenFromScoreboard", gsc_zk_player_ishiddenfromscoreboard, 0},
	{"setHiddenFromServerStatus", gsc_zk_player_sethiddenfromserverstatus, 0},
	{"isHiddenFromServerStatus", gsc_zk_player_ishiddenfromserverstatus, 0},
	{"setPing", gsc_zk_player_setping, 0},
	{"getSpeed", gsc_zk_player_getspeed, 0},
	{"getGravity", gsc_zk_player_getgravity, 0},
	{"isReloading", gsc_zk_player_isreloading, 0},
	{"isFiring", gsc_zk_player_isfiring, 0},
	{"isMeleeing", gsc_zk_player_ismeleeing, 0},
	{"isThrowingGrenade", gsc_zk_player_isthrowinggrenade, 0},
	{"getCurrentWeaponAmmo", gsc_zk_player_getcurrentweaponammo, 0},
	{"getCurrentWeaponClipAmmo", gsc_zk_player_getcurrentweaponclipammo, 0},
	{"getGroundEntity", gsc_zk_player_getgroundentity, 0},
	{"getPlayerStateFlags", gsc_zk_player_getplayerstateflags, 0},
	{"isShellshocked", gsc_zk_player_isshellshocked, 0},
	{"isChatting", gsc_zk_player_ischatting, 0},
	{"getEnterTime", gsc_zk_player_getentertime, 0},
	{"getInactivityTime", gsc_zk_player_getinactivitytime, 0},
	{"getWeaponAnimation", gsc_zk_player_getweaponanimation, 0},
	{"getSpectatorClient", gsc_zk_player_getspectatorclient, 0},
	{"getClientHudElemCount", gsc_zk_player_getclienthudelemcount, 0},
	{"isUsingBinoculars", gsc_zk_player_isusingbinoculars, 0},
	{"canMantle", gsc_zk_player_canmantle, 0},
	{"getCurrentOffhandSlotAmmo", gsc_zk_player_getcurrentoffhandslotammo, 0},
	{"getJumpSlowdownTimer", gsc_zk_player_getjumpslowdowntimer, 0},
	{"noclip", gsc_zk_player_noclip, 0},
	{"setCurrentWeaponAmmo", gsc_zk_player_setcurrentweaponammo, 0},
	{"setCurrentWeaponClipAmmo", gsc_zk_player_setcurrentweaponclipammo, 0},
	{"playScriptAnimation", gsc_zk_player_playscriptanimation, 0},
	{"processSuicide", gsc_zk_player_processsuicide, 0},
	{"stopUseTurret", gsc_zk_player_stopuseturret, 0},
	{"forceShot", gsc_zk_player_forceshot, 0},
	{"aimButtonPressed", gsc_zk_player_button_ads, 0},
	{"backButtonPressed", gsc_zk_player_button_back, 0},
	{"forwardButtonPressed", gsc_zk_player_button_forward, 0},
	{"fragButtonPressed", gsc_zk_player_button_frag, 0},
	{"holdBreathButtonPressed", gsc_zk_player_button_holdbreath, 0},
	{"jumpButtonPressed", gsc_zk_player_button_jump, 0},
	{"leanLeftButtonPressed", gsc_zk_player_button_leanleft, 0},
	{"leanRightButtonPressed", gsc_zk_player_button_leanright, 0},
	{"leftButtonPressed", gsc_zk_player_button_left, 0},
	{"reloadButtonPressed", gsc_zk_player_button_reload, 0},
	{"rightButtonPressed", gsc_zk_player_button_right, 0},
	{"smokeButtonPressed", gsc_zk_player_button_smoke, 0},
	{"isBot", gsc_zk_player_isbot, 0},
	{"getClientConnectState", gsc_zk_player_getclientconnectstate, 0},
	{"getLastMsg", gsc_zk_player_getlastmsg, 0},
	{"getLastConnectTime", gsc_zk_player_getlastconnecttime, 0},
	{"getAddressType", gsc_zk_player_addresstype, 0},
	{"getServerCommandQueueSize", gsc_zk_player_getservercommandqueuesize, 0},
	{"getUserinfo", gsc_zk_player_getuserinfo, 0},
	{"setGuid", gsc_zk_player_setguid, 0},
	{"muteClient", gsc_zk_player_muteclient, 0},
	{"unmuteClient", gsc_zk_player_unmuteclient, 0},
	{"renameClient", gsc_zk_player_renameclient, 0},
	{"setUserinfo", gsc_zk_player_setuserinfo, 0},
	{"setConfigStringForPlayer", gsc_zk_player_setconfigstringforplayer, 0},
	{"setNorthYawForPlayer", gsc_zk_player_setnorthyawforplayer, 0},
	{"resetNextReliableTime", gsc_zk_player_resetnextreliabletime, 0},
	{"connectionlessPacketToClient", gsc_zk_player_connectionlesspackettoclient, 0},
	{"connectionlessPacketToServer", gsc_zk_player_connectionlesspackettoserver, 0},
	{"setHoldingWeaponDown", gsc_zk_player_setholdingweapondown, 0},
	{"setMeleeRangeScale", gsc_zk_player_setmeleerangescale, 0},
	{"setMeleeWidthScale", gsc_zk_player_setmeleewidthscale, 0},
	{"setMeleeHeightScale", gsc_zk_player_setmeleeheightscale, 0},
	// aliases: zk names -> rev's existing per-player speed/gravity
	// per-player speed/gravity: zk implementation (customPlayerState) is authoritative
	{"setSpeed", gsc_zk_player_setspeed, 0},
	{"setGravity", gsc_zk_player_setgravity, 0},
	// --- end zk port ---
	{"getIp", gsc_player_getip, 0},
	{"getstance", gsc_player_stance_get, 0},
	{"getStance", gsc_player_stance_get_cod2x, 0},
	{"setstance", gsc_player_stance_set, 0},
	{"setvelocity", gsc_player_velocity_set, 0},
	{"addvelocity", gsc_player_velocity_add, 0},
	{"getvelocity", gsc_player_velocity_get, 0},
	{"aimbuttonpressed", gsc_player_button_ads, 0},
	{"leftbuttonpressed", gsc_player_button_left, 0},
	{"rightbuttonpressed", gsc_player_button_right, 0},
	{"forwardbuttonpressed", gsc_player_button_forward, 0},
	{"backbuttonpressed", gsc_player_button_back, 0},
	{"leanleftbuttonpressed", gsc_player_button_leanleft, 0},
	{"leanrightbuttonpressed", gsc_player_button_leanright, 0},
	{"jumpbuttonpressed", gsc_player_button_jump, 0},
	{"reloadbuttonpressed", gsc_player_button_reload, 0},
	{"fragbuttonpressed", gsc_player_button_frag, 0},
	{"smokebuttonpressed", gsc_player_button_smoke, 0},
	{"getip", gsc_player_getip, 0},
	{"getHWID", gsc_player_gethwid, 0},
	{"getCDKeyHash", gsc_player_getcdkeyhash, 0},
	{"getAuthorizationStatus", gsc_player_getauthorizationstatus, 0},
	{"getViewOrigin", gsc_player_getvieworigin, 0},
	{"matchPlayerGetData", gsc_match_playerGetData, 0},
	{"matchPlayerSetData", gsc_match_playerSetData, 0},
	{"matchPlayerIsAllowed", gsc_match_playerIsAllowed, 0},
	{"getping", gsc_player_getping, 0},
	{"getspectatorclient", gsc_player_spectatorclient_get, 0},
	{"clientcommand", gsc_player_clientcommand, 0},
	{"getlastconnecttime", gsc_player_getlastconnecttime, 0},
	{"getlastmsg", gsc_player_getlastmsg, 0},
	{"getaddresstype", gsc_player_addresstype, 0},
	{"getclientstate", gsc_player_getclientstate, 0},
	{"renameclient", gsc_player_renameclient, 0},
	{"get_userinfo", gsc_player_get_userinfo, 0},
	{"set_userinfo", gsc_player_set_userinfo, 0},
	{"printoutofband", gsc_player_outofbandprint, 0},
	{"connectionlesspacket", gsc_player_connectionlesspacket, 0},
	{"clientuserinfochanged", gsc_player_clientuserinfochanged, 0},
	{"resetnextreliabletime", gsc_player_resetnextreliabletime, 0},
	{"setg_speed", gsc_zk_player_setspeed, 0},
	{"setg_gravity", gsc_zk_player_setgravity, 0},
	{"setweaponfiremeleedelay", gsc_player_setweaponfiremeleedelay, 0},
	{"setanim", gsc_player_set_anim, 0},
	{"getjumpslowdowntimer", gsc_player_getjumpslowdowntimer, 0},
	{"clearjumpstate", gsc_player_clearjumpstate, 0},
	{"getcooktime", gsc_player_getcooktime, 0},
	{"setguid", gsc_player_setguid, 0},
	{"clienthasclientmuted", gsc_player_clienthasclientmuted, 0},
	{"getlastgamestatesize", gsc_player_getlastgamestatesize, 0},
	{"getfps", gsc_player_getfps, 0},
	{"ismantling", gsc_player_ismantling, 0},
	{"isonladder", gsc_player_isonladder, 0},
	{"isusingturret", gsc_player_isusingturret, 0},
	{"isUsingTurret", gsc_player_isusingturret, 0},
	{"isbot", gsc_player_isbot, 0},
	{"disableitempickup", gsc_player_disableitempickup, 0},
	{"enableitempickup", gsc_player_enableitempickup, 0},
	{"getcurrentoffhandslotammo", gsc_player_getcurrentoffhandslotammo, 0},
	{"getprotocol", gsc_player_getprotocol, 0},
	{"islocalclient", gsc_player_islocalclient, 0},
#endif

#if LIBCOD_COMPILE_SQLITE == 1
	{"async_sqlite_create_entity_query", gsc_async_sqlite_create_entity_query, 0},
	{"async_sqlite_create_entity_query_nosave", gsc_async_sqlite_create_entity_query_nosave, 0},
#endif

#ifdef EXTRA_METHODS_INC
#include "extra/methods.hpp"
#endif

	{NULL, NULL, 0} /* terminator */
};

xmethod_t Scr_GetCustomMethod(const char **fname, qboolean *fdev)
{
	for (int i = 0; scriptMethods[i].name; i++)
	{
		if (strcasecmp(*fname, scriptMethods[i].name))
			continue;

		scr_method_t func = scriptMethods[i];

		*fname = func.name;
		*fdev = func.developer;

		return func.call;
	}

	return NULL;
}

int stackGetParamType(int param)
{
	if (param >= Scr_GetNumParam())
		return STACK_UNDEFINED;

	VariableValue *var;
	var = Scr_GetValue(param);

	return var->type;
}

void stackError(const char *format, ...)
{
	char errorMessage[COD2_MAX_STRINGLENGTH];
	va_list va;

	va_start(va, format);
	Q_vsnprintf(errorMessage, sizeof(errorMessage), format, va);
	va_end(va);

	errorMessage[COD2_MAX_STRINGLENGTH - 1] = '\0';

	Scr_Error(errorMessage);
}

int stackGetParams(const char *params, ...)
{
	va_list args;
	va_start(args, params);

	int errors = 0;

	for (size_t i = 0; i < strlen(params); i++)
	{
		switch (params[i])
		{
		case ' ': // ignore param
			break;

		case 'i':
		{
			int *tmp = va_arg(args, int *);
			if ( ! stackGetParamInt(i, tmp))
			{
				Com_DPrintf("\nstackGetParams() Param %i is not an int\n", i);
				errors++;
			}
			break;
		}

		case 'v':
		{
			float *tmp = va_arg(args, float *);
			if ( ! stackGetParamVector(i, tmp))
			{
				Com_DPrintf("\nstackGetParams() Param %i is not a vector\n", i);
				errors++;
			}
			break;
		}

		case 'f':
		{
			float *tmp = va_arg(args, float *);
			if ( ! stackGetParamFloat(i, tmp))
			{
				Com_DPrintf("\nstackGetParams() Param %i is not a float\n", i);
				errors++;
			}
			break;
		}

		case 's':
		{
			const char **tmp = va_arg(args, const char **);
			if ( ! stackGetParamString(i, tmp))
			{
				Com_DPrintf("\nstackGetParams() Param %i is not a string\n", i);
				errors++;
			}
			break;
		}

		case 'c':
		{
			unsigned int *tmp = va_arg(args, unsigned int *);
			if ( ! stackGetParamConstString(i, tmp))
			{
				Com_DPrintf("\nstackGetParams() Param %i is not a const string\n", i);
				errors++;
			}
			break;
		}

		default:
			errors++;
			Com_DPrintf("\nUnknown identifier [%s] passed to stackGetParams()\n", params[i]);
			break;
		}
	}

	va_end(args);
	return errors == 0; // success if no errors
}

int stackGetParamInt(int param, int *value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type == STACK_FLOAT)
	{
		*value = var->u.floatValue;
		return 1;
	}

	if (var->type != STACK_INT)
		return 0;

	*value = var->u.intValue;

	return 1;
}

int stackGetParamFunction(int param, int *value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type != STACK_FUNCTION)
		return 0;

	*value = var->u.codePosValue - scrVarPub.programBuffer;

	return 1;
}

int stackGetParamString(int param, const char **value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type != STACK_STRING)
		return 0;

	*value = SL_ConvertToString(var->u.stringValue);

	return 1;
}

int stackGetParamConstString(int param, unsigned int *value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type != STACK_STRING)
		return 0;

	*value = var->u.stringValue;

	return 1;
}

int stackGetParamVector(int param, vec3_t value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type != STACK_VECTOR)
		return 0;

	VectorCopy(var->u.vectorValue, value);

	return 1;
}

int stackGetParamFloat(int param, float *value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type == STACK_INT)
	{
		*value = var->u.intValue;
		return 1;
	}

	if (var->type != STACK_FLOAT)
		return 0;

	*value = var->u.floatValue;

	return 1;
}

int stackGetParamObject(int param, unsigned int *value)
{
	if (param >= Scr_GetNumParam())
		return 0;

	VariableValue *var;
	var = Scr_GetValue(param);

	if (var->type != STACK_OBJECT)
		return 0;

	*value = var->u.pointerValue;

	return 1;
}

/**
 * @brief Base time in seconds
 */
time_t sys_timeBase64 = 0;

/**
 * @brief Current time in ms, using sys_timeBase64 as origin
 */
uint64_t Sys_Milliseconds64(void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);

	if (!sys_timeBase64)
	{
		sys_timeBase64 = tp.tv_sec;
		return tp.tv_usec / 1000;
	}

	return (tp.tv_sec - sys_timeBase64) * 1000 + tp.tv_usec / 1000;
}

VariableValue* Scr_GetValue(unsigned int param)
{
	return &scrVmPub.top[int(-param)];
}

void Cod2x_Frame()
{
	Cod2revUpdater_Frame();
	gsc_http_frame();
	match_frame();
	gsc_websocket_frame();
}

void Cod2x_Shutdown()
{
	gsc_websocket_beforeMapChangeOrRestart(false, true, true);
	match_shutdown();
	gsc_http_shutdown();
}

void Cod2x_CheckUpdateNow()
{
	Cod2revUpdater_CheckNow();
}

void Cod2x_CheckUpdateOnMapChange()
{
	Cod2revUpdater_CheckOnMapChange();
}

bool Cod2x_BeforeMapChangeOrRestart(bool shutdown)
{
	if (!gsc_match_beforeMapChangeOrRestart(false, true, shutdown))
		return false;

	if (!gsc_websocket_beforeMapChangeOrRestart(false, true, shutdown))
		return false;

	return match_beforeMapChangeOrRestart(false, true, shutdown);
}

void Cod2x_OnStartGameType()
{
	match_onStartGameType();
	gsc_match_onStartGameType();
}

#endif
