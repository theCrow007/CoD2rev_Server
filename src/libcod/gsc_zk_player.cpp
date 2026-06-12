#include "gsc_zk_player.hpp"

#if LIBCOD_COMPILE_PLAYER == 1

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gsc_zk_custom_state.hpp"

#ifndef MAX_STRINGLENGTH
#define MAX_STRINGLENGTH 1024
#endif

/* Ported from zk_libcod gsc_player.cpp. All enums/fields verified in rev:
 * EV_EARTHQUAKE, EV_PLAY_FX_ON_TAG, s.angles2/time/otherEntityNum/
 * attackerEntityNum, SV_ClientHasClientMuted. tag_name reconciled to
 * const char* (rev's SL_ConvertToString returns const char*). */

void gsc_zk_player_earthquakeforplayer(scr_entref_t ref)
{
	int id = ref.entnum;
	float scale;
	float duration;
	vec3_t source;
	float radius;

	if ( !stackGetParams("ffvf", &scale, &duration, &source, &radius) )
	{
		stackError("gsc_zk_player_earthquakeforplayer() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_earthquakeforplayer() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	duration = floorf((duration * 1000.0) + 0.5);

	if ( scale <= 0.0 )
		Scr_ParamError(0, "Scale must be greater than 0");

	if ( (int)duration < 1 )
		Scr_ParamError(1, "duration must be greater than 0");

	if ( radius <= 0.0 )
		Scr_ParamError(3, "Radius must be greater than 0");

	gentity_t *ent = G_TempEntity(source, EV_EARTHQUAKE);
	ent->s.angles2[0] = scale;
	ent->s.time = (int)duration;
	ent->s.angles2[1] = radius;
	ent->s.otherEntityNum = id + 1;

	stackPushBool(qtrue);
}

void gsc_zk_player_clienthasclientmuted(scr_entref_t ref)
{
	int id = ref.entnum;
	int id2;

	if ( !stackGetParams("i", &id2) )
	{
		stackError("gsc_zk_player_clienthasclientmuted() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_clienthasclientmuted() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( id2 >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_clienthasclientmuted() entity %i is not a player", id2);
		stackPushUndefined();
		return;
	}

	stackPushInt(SV_ClientHasClientMuted(id, id2));
}

void gsc_zk_player_playfxontagforplayer(scr_entref_t ref)
{
	int id = ref.entnum;
	int argc;
	int index;
	gentity_t *ent;
	unsigned int tag_id;
	const char *tag_name;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_playfxontagforplayer() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	argc = Scr_GetNumParam();
	if ( argc != 3 )
		Scr_Error("USAGE: playFxOnTagForPlayer <effect id from loadFx> <entity> <tag name>");

	index = Scr_GetInt(0);
	if ( index < 1 || 0x3F < index )
		Scr_ParamError(0, va("effect id %i is invalid\n", index));

	ent = Scr_GetEntity(1);
	if ( ent->model == 0 )
		Scr_ParamError(1, "cannot play fx on entity with no model");

	tag_id = Scr_GetConstLowercaseString(2);
	tag_name = SL_ConvertToString(tag_id);
	if ( strchr(tag_name, '"') != 0 )
		Scr_ParamError(2, "cannot use \" characters in tag names\n");

	if ( SV_DObjGetBoneIndex(ent, tag_id) < 0 )
	{
		SV_DObjDumpInfo(ent);
		Scr_ParamError(2, va("tag '%s' does not exist on entity with model '%s'", tag_name, G_ModelName(ent->model)));
	}

	/* Reusing the attackerEntityNum field (otherwise only used at obituary
	 * TempEntities) so it is archived, for correct killcam data. */
	ent->s.attackerEntityNum = 1 + id;
	G_AddEvent(ent, EV_PLAY_FX_ON_TAG, G_FindConfigstringIndex(va("%02d%s", index, tag_name), 0x38E, 0x100, 1, NULL));

	stackPushBool(qtrue);
}

void gsc_zk_player_enablesilent(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_enablesilent() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	qboolean old_setting = customPlayerState[id].silent;
	if ( !old_setting )
	{
		SV_GameSendServerCommand(id, SV_CMD_CAN_IGNORE, "v cg_footsteps \"0\"");
		SV_GameSendServerCommand(id, SV_CMD_CAN_IGNORE, "v bg_foliagesnd_minspeed \"999\"");
		SV_GameSendServerCommand(id, SV_CMD_CAN_IGNORE, "v bg_foliagesnd_maxspeed \"999\"");
	}
	customPlayerState[id].silent = qtrue;

	stackPushInt(old_setting);
}

void gsc_zk_player_disablesilent(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_disablesilent() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	qboolean old_setting = customPlayerState[id].silent;
	if ( old_setting )
	{
		SV_GameSendServerCommand(id, SV_CMD_CAN_IGNORE, "v cg_footsteps \"1\"");
		SV_GameSendServerCommand(id, SV_CMD_CAN_IGNORE, "v bg_foliagesnd_minspeed \"40\"");
		SV_GameSendServerCommand(id, SV_CMD_CAN_IGNORE, "v bg_foliagesnd_maxspeed \"180\"");
	}
	customPlayerState[id].silent = qfalse;

	stackPushInt(old_setting);
}

void gsc_zk_player_overridecontents(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_overridecontents() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int contents = customPlayerState[id].contents;

	if ( Scr_GetNumParam() > 0 )
	{
		if ( Scr_GetType(0) == VAR_UNDEFINED )
		{
			customPlayerState[id].overrideContents = qfalse;
			customPlayerState[id].contents = 0;
		}
		else if ( Scr_GetType(0) == VAR_INTEGER )
		{
			customPlayerState[id].overrideContents = qtrue;
			customPlayerState[id].contents = Scr_GetInt(0);
		}
		else
		{
			stackError("gsc_zk_player_overridecontents() first argument has a wrong type");
			stackPushUndefined();
			return;
		}
	}
	else
	{
		stackError("gsc_zk_player_overridecontents() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushInt(contents);
}

static void zk_set_scale_helper(float *field, const char *fname)
{
	float old_scale, new_scale;
	if ( !stackGetParams("f", &new_scale) )
	{
		stackError("%s() argument is undefined or has a wrong type", fname);
		stackPushUndefined();
		return;
	}
	old_scale = *field;
	if ( new_scale < 0 )
		new_scale = 0.0f;
	*field = new_scale;
	stackPushFloat(old_scale);
}

void gsc_zk_player_setweaponspreadscale(scr_entref_t ref)
{
	if ( ref.entnum >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setweaponspreadscale() entity %i is not a player", ref.entnum);
		stackPushUndefined();
		return;
	}
	zk_set_scale_helper(&customPlayerState[ref.entnum].weaponSpreadScale, "gsc_zk_player_setweaponspreadscale");
}

void gsc_zk_player_setmeleerangescale(scr_entref_t ref)
{
	if ( ref.entnum >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setmeleerangescale() entity %i is not a player", ref.entnum);
		stackPushUndefined();
		return;
	}
	zk_set_scale_helper(&customPlayerState[ref.entnum].meleeRangeScale, "gsc_zk_player_setmeleerangescale");
}

void gsc_zk_player_setmeleewidthscale(scr_entref_t ref)
{
	if ( ref.entnum >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setmeleewidthscale() entity %i is not a player", ref.entnum);
		stackPushUndefined();
		return;
	}
	zk_set_scale_helper(&customPlayerState[ref.entnum].meleeWidthScale, "gsc_zk_player_setmeleewidthscale");
}

void gsc_zk_player_setmeleeheightscale(scr_entref_t ref)
{
	if ( ref.entnum >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setmeleeheightscale() entity %i is not a player", ref.entnum);
		stackPushUndefined();
		return;
	}
	zk_set_scale_helper(&customPlayerState[ref.entnum].meleeHeightScale, "gsc_zk_player_setmeleeheightscale");
}

void gsc_zk_player_setturretspreadscale(scr_entref_t ref)
{
	if ( ref.entnum >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setturretspreadscale() entity %i is not a player", ref.entnum);
		stackPushUndefined();
		return;
	}
	zk_set_scale_helper(&customPlayerState[ref.entnum].turretSpreadScale, "gsc_zk_player_setturretspreadscale");
}

void gsc_zk_player_setspeed(scr_entref_t ref)
{
	int id = ref.entnum;
	int speed;

	if ( !stackGetParams("i", &speed) )
	{
		stackError("gsc_zk_player_setspeed() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setspeed() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}
	if ( speed < 0 )
	{
		stackError("gsc_zk_player_setspeed() param must be equal or above zero");
		stackPushUndefined();
		return;
	}

	customPlayerState[id].speed = speed;
	stackPushBool(qtrue);
}

void gsc_zk_player_setgravity(scr_entref_t ref)
{
	int id = ref.entnum;
	int gravity;

	if ( !stackGetParams("i", &gravity) )
	{
		stackError("gsc_zk_player_setgravity() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setgravity() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}
	if ( gravity < 0 )
	{
		stackError("gsc_zk_player_setgravity() param must be equal or above zero");
		stackPushUndefined();
		return;
	}

	customPlayerState[id].gravity = gravity;
	stackPushBool(qtrue);
}

void gsc_zk_player_sethiddenfromscoreboard(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_sethiddenfromscoreboard() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	customPlayerState[id].hiddenFromScoreboard = Scr_GetInt(0) ? qtrue : qfalse;
	stackPushBool(qtrue);
}

void gsc_zk_player_ishiddenfromscoreboard(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_ishiddenfromscoreboard() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	stackPushBool(customPlayerState[id].hiddenFromScoreboard);
}

void gsc_zk_player_sethiddenfromserverstatus(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_sethiddenfromserverstatus() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	qboolean hidden = Scr_GetInt(0);
	customPlayerState[id].hiddenFromServerStatus = hidden;

	stackPushBool(qtrue);
}

void gsc_zk_player_ishiddenfromserverstatus(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_ishiddenfromserverstatus() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	stackPushBool(customPlayerState[id].hiddenFromServerStatus);
}

void gsc_zk_player_setping(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setping() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( Scr_GetNumParam() > 0 )
	{
		if ( Scr_GetType(0) == VAR_UNDEFINED )
		{
			customPlayerState[id].overridePing = qfalse;
			customPlayerState[id].ping = 0;
		}
		else if ( Scr_GetType(0) == VAR_INTEGER )
		{
			customPlayerState[id].overridePing = qtrue;
			customPlayerState[id].ping = Scr_GetInt(0);
		}
		else
		{
			stackError("gsc_zk_player_setping() first argument has a wrong type");
			stackPushUndefined();
			return;
		}

		if ( Scr_GetNumParam() > 1 )
		{
			if ( Scr_GetType(1) == VAR_UNDEFINED )
			{
				customPlayerState[id].overrideStatusPing = qfalse;
				customPlayerState[id].statusPing = 0;
			}
			else if ( Scr_GetType(1) == VAR_INTEGER )
			{
				customPlayerState[id].overrideStatusPing = qtrue;
				customPlayerState[id].statusPing = Scr_GetInt(1);
			}
			else
			{
				stackError("gsc_zk_player_setping() second argument has a wrong type");
				stackPushUndefined();
				return;
			}
		}
		else
		{
			// only one arg: status ping mirrors the general ping
			customPlayerState[id].overrideStatusPing = qtrue;
			customPlayerState[id].statusPing = Scr_GetInt(0);
		}
	}
	else
	{
		stackError("gsc_zk_player_setping() needs at least one argument");
		stackPushUndefined();
		return;
	}

	stackPushBool(qtrue);
}

// ---- gclient/playerState readers ----

void gsc_zk_player_getspeed(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	if ( entity->client == NULL )
	{
		stackError("gsc_zk_player_getspeed() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	stackPushInt(entity->client->ps.speed);
}

void gsc_zk_player_getgravity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	if ( entity->client == NULL )
	{
		stackError("gsc_zk_player_getgravity() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	stackPushInt(entity->client->ps.gravity);
}

void gsc_zk_player_isreloading(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_isreloading() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	stackPushBool( ps->weaponstate >= WEAPON_RELOADING && ps->weaponstate <= WEAPON_RELOAD_END );
}

void gsc_zk_player_isfiring(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_isfiring() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *ent = &g_entities[id];
	stackPushBool(ent->client->ps.weaponstate == WEAPON_FIRING);
}

void gsc_zk_player_ismeleeing(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_ismeleeing() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *ent = &g_entities[id];
	stackPushBool(ent->client->ps.weaponstate == WEAPON_MELEE_FIRE);
}

void gsc_zk_player_isthrowinggrenade(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_isthrowinggrenade() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	stackPushBool( ps->weaponstate >= WEAPON_OFFHAND_INIT && ps->weaponstate <= WEAPON_OFFHAND_END );
}

void gsc_zk_player_getcurrentweaponammo(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getcurrentweaponammo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int ammo;
	gentity_t *player = &g_entities[id];

	if ( !G_IsPlaying(player) )
	{
		stackPushInt(0);
		return;
	}

	if ( !BG_WeaponIsClipOnly(player->s.weapon) )
		ammo = player->client->ps.ammo[BG_AmmoForWeapon(player->s.weapon)];
	else
		ammo = player->client->ps.ammoclip[BG_ClipForWeapon(player->s.weapon)];

	stackPushInt(ammo);
}

void gsc_zk_player_getcurrentweaponclipammo(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getcurrentweaponclipammo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int ammo;
	gentity_t *player = &g_entities[id];

	if ( !G_IsPlaying(player) )
	{
		stackPushInt(0);
		return;
	}

	int clipIndex = BG_ClipForWeapon(player->s.weapon);
	if ( !clipIndex )
		ammo = 0;
	else
		ammo = player->client->ps.ammoclip[clipIndex];

	stackPushInt(ammo);
}

void gsc_zk_player_getgroundentity(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getgroundentity() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *entity = &g_entities[id];
	if ( entity->client->ps.groundEntityNum == ENTITYNUM_NONE )
		stackPushUndefined();
	else
		stackPushEntity(&g_entities[entity->client->ps.groundEntityNum]);
}

void gsc_zk_player_getplayerstateflags(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getplayerstateflags() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *entity = &g_entities[id];
	stackPushInt(entity->client->ps.pm_flags);
}

void gsc_zk_player_isshellshocked(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_isshellshocked() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *player = &g_entities[id];
	gclient_t *client = player->client;
	stackPushBool( client->ps.shellshockIndex != 0 && level.time <= (client->ps.shellshockTime + client->ps.shellshockDuration) );
}

void gsc_zk_player_ischatting(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_ischatting() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *ent = &g_entities[id];
	stackPushBool( ( ent->client->buttons & BUTTON_TALK ) != 0 );
}

void gsc_zk_player_getentertime(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getentertime() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *entity = &g_entities[id];
	stackPushInt(entity->client->sess.enterTime);
}

void gsc_zk_player_getinactivitytime(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getinactivitytime() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *entity = &g_entities[id];
	stackPushInt(entity->client->inactivityTime);
}

void gsc_zk_player_getweaponanimation(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getweaponanimation() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	int anim = ps->weapAnim &= ~ANIM_TOGGLEBIT;

	if ( ( anim == WEAP_RECHAMBER || anim == WEAP_ADS_RECHAMBER ) && ps->weaponTime == 0 )
		anim = WEAP_IDLE;

	stackPushInt(anim);
}

void gsc_zk_player_getspectatorclient(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	if ( entity->client == NULL )
	{
		stackError("gsc_zk_player_getspectatorclient() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( entity->client->spectatorClient == -1 )
		stackPushUndefined();
	else
		stackPushEntity(&g_entities[entity->client->spectatorClient]);
}

void gsc_zk_player_getclienthudelemcount(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getclienthudelemcount() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int count = 0;
	game_hudelem_t *g_hudelem = g_hudelems;
	for ( int i = 0; i < 0x400; i++, g_hudelem++ )
	{
		if ( ( g_hudelem->elem.type != HE_TYPE_FREE ) && ( g_hudelem->clientNum == id ) )
			count++;
	}

	stackPushInt(count);
}

void gsc_zk_player_isusingbinoculars(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_isusingbinoculars() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	stackPushBool( ps->weaponstate >= WEAPON_BINOCULARS_INIT && ps->weaponstate <= WEAPON_BINOCULARS_END );
}

void gsc_zk_player_canmantle(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_canmantle() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	stackPushBool( ps->mantleState.flags & 8 ? qtrue : qfalse );
}

void gsc_zk_player_getcurrentoffhandslotammo(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getcurrentoffhandslotammo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	stackPushInt(ps->ammoclip[ps->offHandIndex - 1]);
}

void gsc_zk_player_getjumpslowdowntimer(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getjumpslowdowntimer() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);
	stackPushInt(ps->pm_time);
}

// ---- gclient/playerState setters ----

void gsc_zk_player_noclip(scr_entref_t ref)
{
	int id = ref.entnum;
	char *noclip;

	if ( !stackGetParams("s", &noclip) )
	{
		stackError("gsc_zk_player_noclip() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_noclip() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *entity = &g_entities[id];

	if ( !I_stricmp(noclip, "on") || atoi(noclip) )
		entity->client->noclip = qtrue;
	else if ( !I_stricmp(noclip, "off") || !I_stricmp(noclip, "0") )
		entity->client->noclip = qfalse;
	else
		entity->client->noclip = !entity->client->noclip;

	stackPushBool(qtrue);
}

void gsc_zk_player_setcurrentweaponammo(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setcurrentweaponammo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int ammo;

	if ( !stackGetParams("i", &ammo) )
	{
		stackError("gsc_zk_player_setcurrentweaponammo() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	gentity_t *player = &g_entities[id];
	int weaponIndex = player->client->ps.weapon;
	int clipIndex;
	int ammoIndex;

	if ( !G_IsPlaying(player) )
	{
		stackPushBool(qfalse);
		return;
	}

	if ( weaponIndex < 1 )
	{
		stackPushBool(qfalse);
		return;
	}

	if ( BG_WeaponIsClipOnly(weaponIndex) )
	{
		clipIndex = BG_ClipForWeapon(weaponIndex);
		if ( clipIndex )
		{
			if ( ammo >= 0 )
			{
				if ( ammo > BG_GetAmmoClipSize(clipIndex) )
					ammo = BG_GetAmmoClipSize(clipIndex);
			}
			else
				ammo = 0;
			player->client->ps.ammoclip[clipIndex] = ammo;
		}
	}
	else
	{
		ammoIndex = BG_AmmoForWeapon(weaponIndex);
		if ( ammoIndex )
		{
			if ( ammo >= 0 )
			{
				if ( ammo > BG_GetAmmoTypeMax(ammoIndex) )
					ammo = BG_GetAmmoTypeMax(ammoIndex);
			}
			else
				ammo = 0;
			player->client->ps.ammo[ammoIndex] = ammo;
		}
	}

	stackPushBool(qtrue);
}

void gsc_zk_player_setcurrentweaponclipammo(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setcurrentweaponclipammo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int ammo;

	if ( !stackGetParams("i", &ammo) )
	{
		stackError("gsc_zk_player_setcurrentweaponclipammo() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	gentity_t *player = &g_entities[id];

	if ( !G_IsPlaying(player) )
	{
		stackPushBool(qfalse);
		return;
	}

	if ( player->client->ps.weapon < 1 )
	{
		stackPushBool(qfalse);
		return;
	}

	int clipIndex = BG_ClipForWeapon(player->s.weapon);
	if ( !clipIndex )
	{
		stackPushBool(qfalse);
		return;
	}
	else
	{
		if ( ammo < 0 )
			ammo = 0;

		if ( ammo > BG_GetAmmoClipSize(clipIndex) )
			ammo = BG_GetAmmoClipSize(clipIndex);

		player->client->ps.ammoclip[clipIndex] = ammo;
	}

	stackPushBool(qtrue);
}

void gsc_zk_player_playscriptanimation(scr_entref_t ref)
{
	int id = ref.entnum;
	int scriptAnimEventType;
	int isContinue;
	int force;

	if ( !stackGetParams("iii", &scriptAnimEventType, &isContinue, &force) )
	{
		stackError("gsc_zk_player_playscriptanimation() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_playscriptanimation() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( scriptAnimEventType < 0 || scriptAnimEventType >= NUM_ANIM_EVENTTYPES )
	{
		stackError("gsc_zk_player_playscriptanimation() argument is not a valid scriptAnimEventType");
		stackPushUndefined();
		return;
	}

	gentity_t *entity = &g_entities[id];
	stackPushInt(BG_AnimScriptEvent(&entity->client->ps, (scriptAnimEventTypes_t)scriptAnimEventType, isContinue, force));
}

void gsc_zk_player_processsuicide(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_processsuicide() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *pSelf = &g_entities[id];

	pSelf->flags &= 0xFFFFFFFC;
	pSelf->health = 0;
	pSelf->client->ps.stats[STAT_HEALTH] = 0;

	player_die(pSelf, pSelf, pSelf, 100000, MOD_SUICIDE, 0, 0, HITLOC_NONE, 0);
}

void gsc_zk_player_stopuseturret(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	if ( entity->client == NULL )
	{
		stackError("gsc_zk_player_stopuseturret() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( entity->client->ps.pm_flags & PMF_PLAYER && entity->client->ps.eFlags & EF_TURRET_ACTIVE )
	{
		G_ClientStopUsingTurret(&level.gentities[entity->client->ps.viewlocked_entNum]);
		stackPushBool(qtrue);
	}
	else
		stackPushBool(qfalse);
}

void gsc_zk_player_forceshot(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_forceshot() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	gentity_t *player = &g_entities[id];
	gclient_t *client = player->client;
	qboolean onClientToo = qtrue;

	if ( Scr_GetNumParam() > 0 )
	{
		if ( Scr_GetType(0) == VAR_INTEGER )
			onClientToo = Scr_GetInt(0);
		else
		{
			stackError("gsc_zk_player_forceshot() one or more arguments is undefined or has a wrong type");
			stackPushUndefined();
			return;
		}
	}

	if ( !G_IsPlaying(player) )
	{
		stackPushBool(qfalse);
		return;
	}

	if ( client->ps.weapon < 1 )
	{
		stackPushBool(qfalse);
		return;
	}

	if ( !g_antilag->current.boolean )
		FireWeaponAntiLag(player, level.time);
	else
		FireWeaponAntiLag(player, client->lastServerTime);

	if ( onClientToo )
		PM_AddEvent(SV_GameClientNum(id), EV_FIRE_WEAPON);

	stackPushBool(qtrue);
}

// ---- server client_t accessors ----

void gsc_zk_player_button_ads(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_ads() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_ADS ? qtrue : qfalse);
}

void gsc_zk_player_button_back(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_back() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.forwardmove == BUTTON_BACK ? qtrue : qfalse);
}

void gsc_zk_player_button_forward(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_forward() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.forwardmove == BUTTON_FORWARD ? qtrue : qfalse);
}

void gsc_zk_player_button_frag(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_frag() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_FRAG ? qtrue : qfalse);
}

void gsc_zk_player_button_holdbreath(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_holdbreath() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_HOLDBREATH ? qtrue : qfalse);
}

void gsc_zk_player_button_jump(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_jump() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_JUMP ? qtrue : qfalse);
}

void gsc_zk_player_button_leanleft(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_leanleft() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_LEANLEFT ? qtrue : qfalse);
}

void gsc_zk_player_button_leanright(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_leanright() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_LEANRIGHT ? qtrue : qfalse);
}

void gsc_zk_player_button_left(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_left() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.rightmove == BUTTON_MOVELEFT ? qtrue : qfalse);
}

void gsc_zk_player_button_reload(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_reload() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_RELOAD ? qtrue : qfalse);
}

void gsc_zk_player_button_right(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_right() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.rightmove == BUTTON_MOVERIGHT ? qtrue : qfalse);
}

void gsc_zk_player_button_smoke(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_button_smoke() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->lastUsercmd.buttons & BUTTON_SMOKE ? qtrue : qfalse);
}

void gsc_zk_player_isbot(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_isbot() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushBool(client->bIsTestClient);
}

void gsc_zk_player_getclientconnectstate(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getclientconnectstate() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushInt(client->state);
}

void gsc_zk_player_getlastmsg(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getlastmsg() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushInt(svs.time - client->lastPacketTime);
}

void gsc_zk_player_getlastconnecttime(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getlastconnecttime() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushInt(client->lastConnectTime);
}

void gsc_zk_player_addresstype(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_addresstype() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushInt(client->netchan.remoteAddress.type);
}

void gsc_zk_player_getservercommandqueuesize(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getservercommandqueuesize() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	stackPushInt(client->reliableSequence - client->reliableAcknowledge);
}

void gsc_zk_player_getuserinfo(scr_entref_t ref)
{
	int id = ref.entnum;
	char *key;

	if ( !stackGetParams("s", &key) )
	{
		stackError("gsc_zk_player_getuserinfo() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getuserinfo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	const char *val = Info_ValueForKey(client->userinfo, key);

	if ( strlen(val) )
		stackPushString(val);
	else
		stackPushString("");
}

void gsc_zk_player_setguid(scr_entref_t ref)
{
	int id = ref.entnum;
	int guid;

	if ( !stackGetParams("i", &guid) )
	{
		stackError("gsc_zk_player_setguid() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setguid() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	client->guid = guid;

	stackPushBool(qtrue);
}

void gsc_zk_player_muteclient(scr_entref_t ref)
{
	int id = ref.entnum;
	int id2;

	if ( !stackGetParams("i", &id2) )
	{
		stackError("gsc_zk_player_muteclient() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_muteclient() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( id2 >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_muteclient() entity %i is not a player", id2);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	client->muteList[id2] = 1;

	stackPushBool(qtrue);
}

void gsc_zk_player_unmuteclient(scr_entref_t ref)
{
	int id = ref.entnum;
	int id2;

	if ( !stackGetParams("i", &id2) )
	{
		stackError("gsc_zk_player_unmuteclient() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_unmuteclient() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( id2 >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_unmuteclient() entity %i is not a player", id2);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	client->muteList[id2] = 0;

	stackPushBool(qtrue);
}

void gsc_zk_player_renameclient(scr_entref_t ref)
{
	int id = ref.entnum;
	char *name;

	if ( !stackGetParams("s", &name) )
	{
		stackError("gsc_zk_player_renameclient() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( strlen(name) > 31 )
	{
		stackError("gsc_zk_player_renameclient() player name is longer than 31 characters");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_renameclient() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	Info_SetValueForKey(client->userinfo, "name", name);
	strcpy(client->name, name);
	ClientUserinfoChanged(id);

	stackPushBool(qtrue);
}

void gsc_zk_player_setuserinfo(scr_entref_t ref)
{
	int id = ref.entnum;
	char *key, *value;

	if ( !stackGetParams("ss", &key, &value) )
	{
		stackError("gsc_zk_player_setuserinfo() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setuserinfo() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	Info_SetValueForKey(client->userinfo, key, value);

	stackPushBool(qtrue);
}

void gsc_zk_player_setconfigstringforplayer(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setconfigstringforplayer() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	char *value;
	int index;

	if ( !stackGetParams("is", &index, &value) )
	{
		stackError("gsc_zk_player_setconfigstringforplayer() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	char cmd[MAX_STRINGLENGTH];

	Com_sprintf(cmd, MAX_STRINGLENGTH, "d %i %s", index, value);
	SV_SendServerCommand(client, SV_CMD_RELIABLE, "%s", cmd);

	stackPushBool(qtrue);
}

void gsc_zk_player_setnorthyawforplayer(scr_entref_t ref)
{
	int id = ref.entnum;
	float fYaw;
	int len;
	client_t *client = &svs.clients[id];
	char cmd[MAX_STRINGLENGTH];

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setnorthyawforplayer() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( !stackGetParams("f", &fYaw) )
	{
		stackError("gsc_zk_player_setnorthyawforplayer() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	len = snprintf(NULL, 0, "%g", fYaw);
	if ( len < 0 || len >= ( MAX_STRINGLENGTH - 5 ) )
	{
		stackError("gsc_zk_player_setnorthyawforplayer() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	Com_sprintf(cmd, MAX_STRINGLENGTH, "d 11 %g", fYaw);
	SV_SendServerCommand(client, SV_CMD_RELIABLE, "%s", cmd);

	stackPushBool(qtrue);
}

void gsc_zk_player_resetnextreliabletime(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_resetnextreliabletime() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];

	client->nextReliableTime = 0;

	stackPushBool(qtrue);
}

void gsc_zk_player_connectionlesspackettoclient(scr_entref_t ref)
{
	int id = ref.entnum;
	char *cmd;

	if ( !stackGetParams("s", &cmd) )
	{
		stackError("gsc_zk_player_connectionlesspackettoclient() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_connectionlesspackettoclient() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	NET_OutOfBandPrint(NS_SERVER, client->netchan.remoteAddress, cmd);

	stackPushBool(qtrue);
}

void gsc_zk_player_connectionlesspackettoserver(scr_entref_t ref)
{
	int id = ref.entnum;
	char *cmd;

	if ( !stackGetParams("s", &cmd) )
	{
		stackError("gsc_zk_player_connectionlesspackettoserver() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_connectionlesspackettoserver() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	client_t *client = &svs.clients[id];
	byte bufData[MAX_MSGLEN];
	msg_t msg;

	MSG_Init(&msg, bufData, sizeof(bufData));
	MSG_WriteByte(&msg, svc_nop);
	MSG_WriteShort(&msg, 0);
	MSG_WriteLong(&msg, -1);
	MSG_WriteString(&msg, cmd);

	SV_ConnectionlessPacket(client->netchan.remoteAddress, &msg);

	stackPushBool(qtrue);
}

void gsc_zk_player_setholdingweapondown(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setholdingweapondown() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	playerState_t *ps = SV_GameClientNum(id);

	if ( Scr_GetNumParam() < 1 )
	{
		stackError("gsc_zk_player_setholdingweapondown() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !ps->weapon )
	{
		stackPushBool(qfalse);
		return;
	}

	WeaponDef *weapDef = BG_GetWeaponDef(ps->weapon);
	client_t *client = &svs.clients[id];

	extern void PM_StartWeaponAnim(playerState_t *ps, int anim);

	if ( Scr_GetInt(0) )
	{
		if ( customPlayerState[id].holdingDownWeapon )
		{
			stackPushBool(qfalse);
			return;
		}

		// Set new states, some again updated in PM_Weapon
		customPlayerState[id].holdingDownWeapon = ps->weapon;
		ps->weaponTime = weapDef->dropTime + client->ping;
		ps->weaponDelay = 0;
		ps->weaponstate = WEAPON_DROPPING;

		// Reset ADS
		if ( weapDef->adsOverlayReticle )
		{
			PM_ExitAimDownSight(ps);
			ps->fWeaponPosFrac = 0;
			ps->adsDelayTime = 0;
		}
		if ( !weapDef->adsFire )
		{
			ps->fWeaponPosFrac = 0;
			ps->adsDelayTime = 0;
		}

		// Reset grenade state
		if ( ps->pm_flags & PMF_FRAG )
		{
			ps->grenadeTimeLeft = 0;
			ps->pm_flags &= ~PMF_FRAG;
		}

		// Animations
		PM_AddEvent(ps, EV_PUTAWAY_WEAPON);
		PM_StartWeaponAnim(ps, WEAP_DROP);
		BG_AnimScriptEvent(ps, ANIM_ET_DROPWEAPON, 0, 1);
	}
	else
	{
		if ( customPlayerState[id].holdingDownWeapon )
		{
			ps->weaponTime = weapDef->raiseTime + client->ping;
			ps->weaponstate = WEAPON_RAISING;
			PM_AddEvent(ps, EV_RAISE_WEAPON);
			PM_StartWeaponAnim(ps, WEAP_RAISE);
			BG_AnimScriptEvent(ps, ANIM_ET_RAISEWEAPON, 0, 1);
		}
		customPlayerState[id].holdingDownWeapon = 0;
	}

	stackPushBool(qtrue);
}

// ---- bullet mask / fire-through-walls ----

void gsc_zk_player_setfirethroughwalls(scr_entref_t ref)
{
	int id = ref.entnum;
	int old_setting, new_setting;

	if ( !stackGetParams("i", &new_setting) )
	{
		stackError("gsc_zk_player_setfirethroughwalls() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setfirethroughwalls() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	old_setting = customPlayerState[id].fireThroughWalls;
	customPlayerState[id].fireThroughWalls = new_setting;

	stackPushInt(old_setting);
}

void gsc_zk_player_getbulletmask(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_getbulletmask() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	if ( customPlayerState[id].overrideBulletMask )
		stackPushInt(customPlayerState[id].bulletMask);
	else if ( customPlayerState[id].fireThroughWalls )
		stackPushInt(CONTENTS_BODY);
	else
		stackPushInt(MASK_SHOT);
}

void gsc_zk_player_setbulletmask(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( id >= MAX_CLIENTS )
	{
		stackError("gsc_zk_player_setbulletmask() entity %i is not a player", id);
		stackPushUndefined();
		return;
	}

	int old_setting;

	if ( Scr_GetNumParam() == 1 )
	{
		if ( customPlayerState[id].overrideBulletMask )
			old_setting = customPlayerState[id].bulletMask;
		else if ( customPlayerState[id].fireThroughWalls )
			old_setting = CONTENTS_BODY;
		else
			old_setting = MASK_SHOT;

		if ( Scr_GetType(0) == VAR_UNDEFINED )
		{
			customPlayerState[id].overrideBulletMask = qfalse;
			customPlayerState[id].bulletMask = 0;
		}
		else if ( Scr_GetType(0) == VAR_INTEGER )
		{
			customPlayerState[id].overrideBulletMask = qtrue;
			customPlayerState[id].bulletMask = Scr_GetInt(0);
		}
		else
		{
			stackError("gsc_zk_player_setbulletmask() argument has a wrong type");
			stackPushUndefined();
			return;
		}
	}
	else
	{
		stackError("gsc_zk_player_setbulletmask() needs exactly one argument");
		stackPushUndefined();
		return;
	}

	stackPushInt(old_setting);
}

#endif
