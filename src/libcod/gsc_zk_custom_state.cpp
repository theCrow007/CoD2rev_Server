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
	for ( int oi = 0; oi < MAX_OBJECTIVES; oi++ )
		customPlayerState[clientNum].objectives[oi].entNum = ENTITYNUM_NONE;

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

int zk_GetHoldingDownWeapon(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 0;
	return customPlayerState[clientNum].holdingDownWeapon;
}

// per-client brush-model solidity
qboolean zk_playerMovementTrace = qfalse;

qboolean zk_IsNonSolidForClient(int entNum, int clientNum)
{
	if ( entNum < 0 || entNum >= MAX_GENTITIES )
		return qfalse;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	if ( !customEntityState[entNum].notSolidBrushModel )
		return qfalse;
	return ( customEntityState[entNum].clientMask[clientNum >> 5] & ( 1 << ( clientNum & 0x1F ) ) ) ? qtrue : qfalse;
}

void zk_ClearNonSolidForClient(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;
	for ( int i = 0; i < MAX_GENTITIES; i++ )
	{
		customEntityState[i].clientMask[clientNum >> 5] &= ~( 1 << ( clientNum & 0x1F ) );
		if ( !customEntityState[i].clientMask[0] && !customEntityState[i].clientMask[1] )
			customEntityState[i].notSolidBrushModel = qfalse;
	}
}

int zk_GetBulletMask(int clientNum, int defaultMask)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return defaultMask;
	if ( customPlayerState[clientNum].overrideBulletMask )
		return customPlayerState[clientNum].bulletMask;
	if ( customPlayerState[clientNum].fireThroughWalls )
		return CONTENTS_BODY;
	return defaultMask;
}

qboolean zk_IsNotAllowingSpectators(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	return customPlayerState[clientNum].notAllowingSpectators;
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


qboolean zk_GetStepSizeOverride(int clientNum, qboolean prone, float *out)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	if ( prone )
	{
		if ( customPlayerState[clientNum].overrideProneStepSize )
		{
			*out = customPlayerState[clientNum].proneStepSize;
			return qtrue;
		}
	}
	else if ( customPlayerState[clientNum].overrideStepSize )
	{
		*out = customPlayerState[clientNum].stepSize;
		return qtrue;
	}
	return qfalse;
}


int zk_GetJumpHeightOverride(int clientNum, float *out)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 0;
	if ( customPlayerState[clientNum].overrideJumpHeight )
	{
		*out = (float)customPlayerState[clientNum].jumpHeight;
		return 1;
	}
	return 0;
}

int zk_GetJumpSlowdownOverride(int clientNum, int *out)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 0;
	if ( customPlayerState[clientNum].overrideJumpSlowdown )
	{
		*out = customPlayerState[clientNum].jumpSlowdown;
		return 1;
	}
	return 0;
}


void zk_AddEntToPlayerSnapshots(int clientNum, int entNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;
	int n = customPlayerState[clientNum].numForcedSnapshotEnts;
	for ( int i = 0; i < n; i++ )
	{
		if ( customPlayerState[clientNum].forcedSnapshotEnts[i] == entNum )
			return;
	}
	if ( n < ZK_MAX_SNAPSHOT_ENTITIES )
	{
		customPlayerState[clientNum].forcedSnapshotEnts[n] = entNum;
		customPlayerState[clientNum].numForcedSnapshotEnts++;
	}
}

void zk_RemoveEntFromPlayerSnapshots(int clientNum, int entNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;
	int n = customPlayerState[clientNum].numForcedSnapshotEnts;
	for ( int i = 0; i < n; i++ )
	{
		if ( customPlayerState[clientNum].forcedSnapshotEnts[i] == entNum )
		{
			for ( int j = i; j < n - 1; j++ )
				customPlayerState[clientNum].forcedSnapshotEnts[j] = customPlayerState[clientNum].forcedSnapshotEnts[j + 1];
			customPlayerState[clientNum].numForcedSnapshotEnts--;
			return;
		}
	}
}

int zk_GetForcedSnapshotCount(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 0;
	return customPlayerState[clientNum].numForcedSnapshotEnts;
}

int zk_GetForcedSnapshotEnt(int clientNum, int idx)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || idx < 0 || idx >= customPlayerState[clientNum].numForcedSnapshotEnts )
		return -1;
	return customPlayerState[clientNum].forcedSnapshotEnts[idx];
}


int zk_GetPlayerObjective(int clientNum, int objNum, objective_t *dest)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || objNum < 0 || objNum >= MAX_OBJECTIVES )
		return 0;
	if ( customPlayerState[clientNum].objectives[objNum].state == OBJST_EMPTY )
		return 0;
	*dest = customPlayerState[clientNum].objectives[objNum];
	return 1;
}


void zk_ApplyEarthquakeClientMask(int *clientMask)
{
	for ( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if ( customPlayerState[i].noEarthquakes )
		{
			if ( i > 31 )
				clientMask[1] |= 1 << ( i - 32 );
			else
				clientMask[0] |= 1 << i;
		}
	}
}


extern void SV_QueueVoicePacket(int talkerNum, int clientNum, VoicePacket_t *voicePacket);

void zk_RunTalkerIcons(void)
{
	VoicePacket_t fakeVoicePacket;
	fakeVoicePacket.data[0] = 0xFF; // magic: empty/fake voice payload
	fakeVoicePacket.dataSize = 1;

	for ( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if ( svs.clients[i].state < CS_CONNECTED )
			continue;
		for ( int j = 0; j < MAX_CLIENTS; j++ )
		{
			if ( !customPlayerState[i].talkerIcons[j] )
				continue;
			if ( svs.clients[j].state < CS_CONNECTED )
				continue;
			// do not fake the icon if that player is genuinely talking right now
			int durationSinceLastTalk = level.time - level.clients[j].lastVoiceTime;
			if ( durationSinceLastTalk >= 0 && g_voiceChatTalkingDuration->current.integer > durationSinceLastTalk )
				continue;
			fakeVoicePacket.talker = j;
			SV_QueueVoicePacket(j, i, &fakeVoicePacket);
		}
	}
}

void zk_ClearTalkerIconsForClient(int dropped)
{
	if ( dropped < 0 || dropped >= MAX_CLIENTS )
		return;
	for ( int i = 0; i < MAX_CLIENTS; i++ )
		customPlayerState[i].talkerIcons[dropped] = 0;
}


int zk_GetPlayerAnimationOverride(int clientNum, int animNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 0;
	// keep the engine's death animations (30..51) intact; only override others
	if ( customPlayerState[clientNum].animation && ( animNum < 30 || animNum > 51 ) )
		return customPlayerState[clientNum].animation;
	return 0;
}

// ---- per-player/team collision (collisionTeam) ----
extern unsigned int GScr_AllocString(const char *s);
unsigned int zk_const_axis_allies = 0;
unsigned int zk_const_bullet = 0;

void zk_InitCollisionConsts(void)
{
	zk_const_axis_allies = GScr_AllocString("axis_allies");
	zk_const_bullet = GScr_AllocString("bullet");
}

qboolean zk_SkipCollision(gentity_t *client1, gentity_t *client2)
{
	int id1 = client1 - g_entities;
	int id2 = client2 - g_entities;

	if ( id1 < MAX_CLIENTS && id2 < MAX_CLIENTS && client1->client && client2->client
		&& client1->client->sess.connected == CON_CONNECTED
		&& client2->client->sess.connected == CON_CONNECTED )
	{
		if ( customPlayerState[id1].collisionTeam == CUSTOM_TEAM_NONE || customPlayerState[id2].collisionTeam == CUSTOM_TEAM_NONE )
			return qtrue;
		if ( customPlayerState[id1].collisionTeam == CUSTOM_TEAM_AXIS && client2->client->sess.cs.team != TEAM_AXIS )
			return qtrue;
		if ( customPlayerState[id1].collisionTeam == CUSTOM_TEAM_ALLIES && client2->client->sess.cs.team != TEAM_ALLIES )
			return qtrue;
		if ( customPlayerState[id2].collisionTeam == CUSTOM_TEAM_AXIS && client1->client->sess.cs.team != TEAM_AXIS )
			return qtrue;
		if ( customPlayerState[id2].collisionTeam == CUSTOM_TEAM_ALLIES && client1->client->sess.cs.team != TEAM_ALLIES )
			return qtrue;
	}
	return qfalse;
}

// ---- ballistics accessors (noBulletImpacts + fireRangeScale) ----
float zk_GetFireRangeScale(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return 1.0f;
	return customPlayerState[clientNum].fireRangeScale;
}

qboolean zk_GetNoBulletImpacts(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
		return qfalse;
	return customPlayerState[clientNum].noBulletImpacts;
}

// ---- activate-on-use-button-release accessors (Player_UpdateActivate hook) ----
qboolean zk_GetActivateOnUseButtonRelease(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) return qfalse;
	return customPlayerState[clientNum].activateOnUseButtonRelease;
}

qboolean zk_GetHeldUseButton(int clientNum)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) return qfalse;
	return customPlayerState[clientNum].heldUseButton;
}

void zk_SetHeldUseButton(int clientNum, qboolean value)
{
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) return;
	customPlayerState[clientNum].heldUseButton = value;
}

#endif
