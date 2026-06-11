#include "gsc_zk_player.hpp"

#if LIBCOD_COMPILE_PLAYER == 1

#include <math.h>
#include <string.h>
#include "gsc_zk_custom_state.hpp"

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

#endif
