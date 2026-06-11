#include "gsc_zk_custom_state.hpp"

#ifdef LIBCOD

#include <string.h>

customPlayerState_t customPlayerState[MAX_CLIENTS];
customEntityState_t customEntityState[MAX_GENTITIES];

void zk_ResetCustomPlayerState(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;

	memset(&customPlayerState[clientNum], 0, sizeof(customPlayerState_t));

	// sensible non-zero defaults
	customPlayerState[clientNum].meleeHeightScale = 1.0f;
	customPlayerState[clientNum].meleeRangeScale  = 1.0f;
	customPlayerState[clientNum].meleeWidthScale  = 1.0f;
	customPlayerState[clientNum].fireRangeScale   = 1.0f;
	customPlayerState[clientNum].turretSpreadScale = 1.0f;
	customPlayerState[clientNum].weaponSpreadScale = 1.0f;
}

void zk_ResetCustomEntityState(int entnum)
{
	if ( entnum < 0 || entnum >= MAX_GENTITIES )
		return;

	memset(&customEntityState[entnum], 0, sizeof(customEntityState_t));
	customEntityState[entnum].gravityType = GRAVITY_NONE;
}

void zk_InitCustomState(void)
{
	for ( int i = 0; i < MAX_CLIENTS; i++ )
		zk_ResetCustomPlayerState(i);
	for ( int i = 0; i < MAX_GENTITIES; i++ )
		zk_ResetCustomEntityState(i);
}

qboolean zk_IsPlayerSilent(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	return customPlayerState[clientNum].silent;
}

qboolean zk_IsHiddenFromScoreboard(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	return customPlayerState[clientNum].hiddenFromScoreboard;
}

qboolean zk_IsHiddenFromServerStatus(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	return customPlayerState[clientNum].hiddenFromServerStatus;
}

qboolean zk_GetPingOverride(int clientNum, int *ping)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	if ( !customPlayerState[clientNum].overridePing )
		return qfalse;
	if ( ping )
		*ping = customPlayerState[clientNum].ping;
	return qtrue;
}

qboolean zk_GetStatusPingOverride(int clientNum, int *ping)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	if ( !customPlayerState[clientNum].overrideStatusPing )
		return qfalse;
	if ( ping )
		*ping = customPlayerState[clientNum].statusPing;
	return qtrue;
}

qboolean zk_GetPlayerContentsOverride(int clientNum, int *contents)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	if ( !customPlayerState[clientNum].overrideContents )
		return qfalse;
	if ( contents )
		*contents = customPlayerState[clientNum].contents;
	return qtrue;
}

float zk_GetWeaponSpreadScale(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 1.0f;
	return customPlayerState[clientNum].weaponSpreadScale;
}

float zk_GetTurretSpreadScale(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 1.0f;
	return customPlayerState[clientNum].turretSpreadScale;
}

void zk_ApplyMeleeScales(int clientNum, float *range, float *width, float *height)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;
	if ( range )  *range  *= customPlayerState[clientNum].meleeRangeScale;
	if ( width )  *width  *= customPlayerState[clientNum].meleeWidthScale;
	if ( height ) *height *= customPlayerState[clientNum].meleeHeightScale;
}

void zk_ApplyPlayerSpeedGravity(int clientNum, int *speed, int *gravity)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;
	if ( speed && customPlayerState[clientNum].speed > 0 )
		*speed = customPlayerState[clientNum].speed;
	if ( gravity && customPlayerState[clientNum].gravity > 0 )
		*gravity = customPlayerState[clientNum].gravity;
}

#endif
