#ifndef _GSC_HPP_
#define _GSC_HPP_

#include "../qcommon/qcommon.h"
#include "../game/g_shared.h"
#include "cod2x_state.hpp"
#include "gsc_http.hpp"
#include "gsc_match.hpp"
#include "gsc_websocket.hpp"

#define COD2_MAX_STRINGLENGTH MAX_STRING_CHARS

/* default stuff */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdarg.h> // va_args
#include <unistd.h> //link, unlink, usleep
#include <dirent.h> // dir stuff
#include <stddef.h> // offsetof
#include <sys/stat.h> // fsize
#include <time.h>  // getsystemtime
#include <sys/time.h> // milliseconds
#include <ctype.h> // isdigit

#if LIBCOD_COMPILE_BOTS == 1
#include "gsc_bots.hpp"
#endif

#if LIBCOD_COMPILE_ENTITY == 1
#include "gsc_entity.hpp"
#endif

#if LIBCOD_COMPILE_EXEC == 1
#include "gsc_exec.hpp"
#endif

#if LIBCOD_COMPILE_LEVEL == 1
#include "gsc_level.hpp"
#endif

#if LIBCOD_COMPILE_MEMORY == 1
#include "gsc_memory.hpp"
#endif

#if LIBCOD_COMPILE_MYSQL == 1
#include "gsc_mysql.hpp"
#endif

#if LIBCOD_COMPILE_MYSQL_VORON == 1
#include "gsc_mysql_voron.hpp"
#endif

#if LIBCOD_COMPILE_PLAYER == 1
#include "gsc_player.hpp"
#endif

#if LIBCOD_COMPILE_SQLITE == 1
#include "gsc_sqlite.hpp"
#endif

#if LIBCOD_COMPILE_UTILS == 1
#include "gsc_utils.hpp"
#endif

#if LIBCOD_COMPILE_WEAPONS == 1
#include "gsc_weapons.hpp"
#endif

#ifdef EXTRA_INCLUDES_INC
#include "extra/includes.hpp"
#endif

#define STACK_UNDEFINED 0
#define STACK_OBJECT 1
#define STACK_STRING 2
#define STACK_LOCALIZED_STRING 3
#define STACK_VECTOR 4
#define STACK_FLOAT 5
#define STACK_INT 6
#define STACK_RAWPOINTER 7
#define STACK_CODEPOS 8
#define STACK_PRECODEPOS 9
#define STACK_FUNCTION 10
#define STACK_STACK 11
#define STACK_ANIMATION 12
#define STACK_DEVELOPER_CODEPOS 13
#define STACK_INCLUDE_CODEPOS 14
#define STACK_THREAD_LIST 15
#define STACK_THREAD_1 16
#define STACK_THREAD_2 17
#define STACK_THREAD_3 18
#define STACK_THREAD_4 19
#define STACK_STRUCT 20
#define STACK_REMOVED_ENTITY 21
#define STACK_ENTITY 22
#define STACK_ARRAY 23
#define STACK_REMOVED_THREAD 24

#define stackPushUndefined Scr_AddUndefined
#define stackPushBool Scr_AddBool
#define stackPushInt Scr_AddInt
#define stackPushPointer Scr_AddPointer
#define stackPushFloat Scr_AddFloat
#define stackPushString Scr_AddString
#define stackPushVector Scr_AddVector
#define stackPushEntity Scr_AddEntity
#define stackPushArray Scr_MakeArray
#define stackPushArrayLast Scr_AddArray
#define stackPushObject Scr_AddObject

#ifndef Q_vsnprintf
int Q_vsnprintf(char *s0, size_t size, const char *fmt, va_list args);
#endif

int stackGetParamType(int param);
const char *stackGetParamTypeAsString(int param);

int stackGetParams(const char *params, ...);
void stackError(const char *format, ...);

int stackGetParamInt(int param, int *value);
int stackGetParamPointer(int param, void **value);
int stackGetParamFunction(int param, int *value);
int stackGetParamString(int param, const char **value);
int stackGetParamConstString(int param, unsigned int *value);
int stackGetParamVector(int param, vec3_t value);
int stackGetParamFloat(int param, float *value);
int stackGetParamObject(int param, unsigned int *value);

xfunction_t Scr_GetCustomFunction(const char **fname, qboolean *fdev);
xmethod_t Scr_GetCustomMethod(const char **fname, qboolean *fdev);

uint64_t Sys_Milliseconds64(void);

extern dvar_t *sv_master[MAX_MASTER_SERVERS];
extern dvar_t *fs_library;
extern dvar_t *g_playerCollision;
extern dvar_t *g_playerEject;
extern dvar_t *sv_allowRcon;
extern dvar_t *sv_downloadMessage;
extern dvar_t *sv_fastDownload;
extern dvar_t *sv_cracked;
extern dvar_t *sv_kickbots;
extern dvar_t *sv_kickMessages;
extern dvar_t *sv_botKickMessages;
extern dvar_t *sv_disconnectMessages;
extern dvar_t *sv_timeoutMessages;
extern dvar_t *sv_wwwDlDisconnectedMessages;
extern dvar_t *sv_logRcon;
extern dvar_t *logTimestamps;
extern dvar_t *logfileName;
extern dvar_t *sv_logHeartbeats;
extern dvar_t *g_logPickup;
extern dvar_t *logErrors;
extern dvar_t *con_coloredPrints;
extern dvar_t *logfileRotate;
extern dvar_t *g_debugCallbacks;
extern dvar_t *g_debugEvents;
extern dvar_t *g_debugStaticModels;
extern dvar_t *g_forceRate;
extern dvar_t *g_forceSnaps;
extern dvar_t *sv_limitLocalRcon;
extern dvar_t *g_resetSlide;
extern dvar_t *sv_updateCursorHints;
extern dvar_t *sv_downloadRetransmitTimeout;
extern dvar_t *scr_turretDamageName;
extern dvar_t *g_turretMissingTagTerminalError;
extern dvar_t *g_droppedWeaponsNeglectBots;
extern dvar_t *jump_carryMoverVelocity;
extern dvar_t *sv_isLookingAtOnDemand;
extern dvar_t *sv_verifyIwds;
extern dvar_t *g_spawnMapWeapons;
extern dvar_t *g_spawnMapTurrets;
extern dvar_t *g_playerCollisionEjectDuration;
extern dvar_t *g_playerCollisionEjectDamageAllowed;
extern dvar_t *sv_maxSnapshotEntities;
extern dvar_t *g_spectateBots;
extern dvar_t *sv_downloadMessageAtMap;
extern dvar_t *sv_downloadNotifications;
extern dvar_t *sv_minimizeSysteminfo;
extern dvar_t *sv_botReconnectMode;
extern dvar_t *sv_authorizeServer;
extern dvar_t *sv_authorizePort;
extern dvar_t *sv_authorizeTimeout;
extern dvar_t *sv_noauthorize;
extern dvar_t *sv_masterServer;
extern dvar_t *sv_masterPort;
extern dvar_t *sv_downloadMessageForLegacyClients;
extern dvar_t *fs_gametypes;
extern dvar_t *fs_callbacks;
extern dvar_t *fs_mapScriptDirectories;
extern dvar_t *fs_replaceStockMaps;
extern dvar_t *g_brushModelCollisionTweaks;
extern dvar_t *sv_fastDownloadSpeed;
extern dvar_t *sv_autoAddSnapshotEntities;
extern dvar_t *g_triggerMode;
extern dvar_t *g_reservedModels;
extern dvar_t *g_safePrecache;
extern dvar_t *g_pointTraceMovement;
extern dvar_t *sv_botUseTriggerUse;
extern unsigned short scr_const_bot_trigger;
extern dvar_t *sv_genericServerErrorMessage;
extern dvar_t *g_noMoverBlockage;
extern dvar_t *g_sendEmtpyOffhandEvents;
extern int codecallback_moverblockage;
extern dvar_t *jump_bounceEnable;
extern dvar_t *g_mantleBlockEnable;
extern dvar_t *g_fixedWeaponSpreads;
extern dvar_t *g_dropGrenadeOnDeath;

void RegisterLibcodDvars();
int hook_findMap(const char *qpath, void **buffer);
bool SV_IsBannedIp(netadr_t adr);
void hook_ClientCommand(int clientNum);
void InitLibcodCallbacks();
void SV_MasterHeartbeat_libcod(const char *hbname);
void SV_ResetPureClient_libcod(client_t *cl);
void SV_CalcPings_libcod( void );
void SV_CheckTimeouts_libcod( void );
void SV_BotUserMove_libcod(client_t *client);
void SV_ClientThink_libcod(client_t *cl, usercmd_t *cmd);
const char* NET_AdrToStringNoPort( netadr_t a );
void Cod2x_Frame();
void Cod2x_Shutdown();
void Cod2x_CheckUpdateNow();
void Cod2x_CheckUpdateOnMapChange();
bool Cod2x_BeforeMapChangeOrRestart(bool shutdown);
void Cod2x_OnStartGameType();

void PM_ProjectVelocity(const float *velIn, const float *normal, float *velOut);
void SV_AddLibcodCommands();
const char *SV_ModifyConfigstringIwdChkSum( client_t *client, int index );
VariableValue* Scr_GetValue(unsigned int param);

#endif
