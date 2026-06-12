#include "gsc_zk_entity.hpp"

#if LIBCOD_COMPILE_ENTITY == 1

#include "gsc_zk_custom_state.hpp"

// local helper (zk used a global IsNullVector)
static qboolean zk_IsNullVector_ent(const vec3_t v)
{
	return ( v[0] == 0.0f && v[1] == 0.0f && v[2] == 0.0f );
}


/* Ported from zk_libcod gsc_entity.cpp. Logic unchanged; all fields verified
 * present in rev's gentity_s (clipmask, r.mins/maxs/contents/ownerNum,
 * pTurretInfo, tagInfo->parent->s.number). */

void gsc_zk_entity_getclipmask(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	stackPushInt(ent->clipmask);
}

void gsc_zk_entity_setclipmask(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	ent->clipmask = Scr_GetInt(0);
	stackPushBool(qtrue);
}

void gsc_zk_entity_getvmax(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	stackPushVector(entity->r.maxs);
}

void gsc_zk_entity_getvmin(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	stackPushVector(entity->r.mins);
}

void gsc_zk_entity_isturret(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->pTurretInfo )
		stackPushBool(qtrue);
	else
		stackPushBool(qfalse);
}

void gsc_zk_entity_islinkedto(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->tagInfo )
	{
		stackPushEntity(&g_entities[ent->tagInfo->parent->s.number]);
	}
	else
	{
		stackPushUndefined();
	}
}

void gsc_zk_entity_getturretowner(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->pTurretInfo )
	{
		if ( ent->r.ownerNum == ENTITYNUM_NONE )
		{
			stackPushUndefined();
		}
		else
		{
			stackPushEntity(&g_entities[ent->r.ownerNum]);
		}
	}
	else
	{
		stackError("gsc_zk_entity_getturretowner() entity is not a turret");
		stackPushUndefined();
	}
}

void gsc_zk_entity_setlight(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		int r, g, b, i;

		if ( !stackGetParams("iiii", &r, &g, &b, &i) )
		{
			stackError("gsc_zk_entity_setlight() argument is undefined or has a wrong type");
			stackPushUndefined();
			return;
		}

		if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
		if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
		if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
		if ( i < 0 ) i = 0; else if ( i > 255 ) i = 255;

		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
		stackPushBool(qtrue);
	}
	else
	{
		stackError("gsc_zk_entity_setlight() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_hastag(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];
	unsigned int tagId;
	const char *tagName;

	tagId = Scr_GetConstLowercaseString(0);
	tagName = SL_ConvertToString(tagId);
	if ( !*tagName )
		tagId = 0;

	if ( !SV_DObjExists(ent) )
	{
		if ( !ent->model )
		{
			stackError("gsc_zk_entity_hastag() entity has no model");
			stackPushInt(0);
			return;
		}
		stackError("gsc_zk_entity_hastag() entity model '%s' is invalid", G_ModelName(ent->model));
		stackPushInt(0);
		return;
	}
	if ( tagId && SV_DObjGetBoneIndex(ent, tagId) < 0 )
	{
		SV_DObjDumpInfo(ent);
		stackError("gsc_zk_entity_hastag() tag '%s' does not exist in model '%s'", tagName, G_ModelName(ent->model));
		stackPushInt(0);
		return;
	}

	stackPushInt(1);
}

void gsc_zk_entity_gettagorigin(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];
	vec3_t origin;
	unsigned int tagId;
	const char *tagName;

	tagId = Scr_GetConstLowercaseString(0);
	tagName = SL_ConvertToString(tagId);
	if ( !*tagName )
		tagId = 0; // Defaults to origin

	if ( !G_DObjGetWorldTagPos(ent, tagId, origin) )
	{
		stackError("gsc_zk_entity_gettagorigin() could not find tag '%s' on model '%s'", tagName, G_ModelName(ent->model));
		stackPushUndefined();
		return;
	}

	stackPushVector(origin);
}

// ---- item / grenade entity accessors ----

void gsc_zk_entity_getgrenadefusetime(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->handler != ENT_HANDLER_GRENADE )
	{
		stackError("gsc_zk_entity_getgrenadefusetime() entity is not a grenade");
	}
	else
	{
		float remainingFuseTime = ent->nextthink - level.time;

		if ( remainingFuseTime < 0.0 )
			stackPushFloat(0.0);
		else
			stackPushFloat(remainingFuseTime / 1000.0);
	}
}

void gsc_zk_entity_addgrenadefusetime(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->handler != ENT_HANDLER_GRENADE )
	{
		stackError("gsc_zk_entity_addgrenadefusetime() entity is not a grenade");
	}
	else
	{
		float time;

		if ( !stackGetParams("f", &time) )
		{
			stackError("gsc_zk_entity_addgrenadefusetime() one or more arguments is undefined or has a wrong type");
		}
		else
		{
			ent->nextthink += (int)((time * 1000) - ((int)(time * 1000) % FRAMETIME));
			if ( ent->nextthink < level.time )
				ent->nextthink = level.time;
		}
	}

	stackPushUndefined();
}

void gsc_zk_entity_getweaponitemammo(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];
	gitem_t *bg_item = &bg_itemlist[0];

	if ( entity->s.index < 1 || bg_numItems <= entity->s.index )
	{
		stackError("gsc_zk_entity_getitemquantity() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}

	bg_item += entity->s.index;
	if ( bg_item->giType == IT_WEAPON )
	{
		stackPushInt(entity->count);
	}
	else
	{
		stackError("gsc_zk_entity_getitemquantity() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}
}

void gsc_zk_entity_setweaponitemammo(scr_entref_t ref)
{
	int id = ref.entnum;
	int ammo;
	gentity_t *entity = &g_entities[id];
	gitem_t *bg_item = &bg_itemlist[0];

	if ( !stackGetParams("i", &ammo) )
	{
		stackError("gsc_zk_entity_setweaponitemammo() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( entity->s.index < 1 || bg_numItems <= entity->s.index )
	{
		stackError("gsc_zk_entity_setweaponitemammo() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}

	bg_item += entity->s.index;
	if ( bg_item->giType == IT_WEAPON )
	{
		entity->count = ammo;
		stackPushBool(qtrue);
	}
	else
	{
		stackError("gsc_zk_entity_setweaponitemammo() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}
}

void gsc_zk_entity_getweaponitemclipammo(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];
	gitem_t *bg_item = &bg_itemlist[0];

	if ( entity->s.index < 1 || bg_numItems <= entity->s.index )
	{
		stackError("gsc_zk_entity_getweaponitemclipammo() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}

	bg_item += entity->s.index;
	if ( bg_item->giType == IT_WEAPON )
	{
		stackPushInt(entity->item.clipAmmoCount);
	}
	else
	{
		stackError("gsc_zk_entity_getweaponitemclipammo() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}
}

void gsc_zk_entity_setweaponitemclipammo(scr_entref_t ref)
{
	int id = ref.entnum;
	int clipammo;
	gentity_t *entity = &g_entities[id];
	gitem_t *bg_item = &bg_itemlist[0];

	if ( !stackGetParams("i", &clipammo) )
	{
		stackError("gsc_zk_entity_setweaponitemclipammo() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( entity->s.index < 1 || bg_numItems <= entity->s.index )
	{
		stackError("gsc_zk_entity_setweaponitemclipammo() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}

	bg_item += entity->s.index;
	if ( bg_item->giType == IT_WEAPON )
	{
		entity->item.clipAmmoCount = clipammo;
		stackPushBool(qtrue);
	}
	else
	{
		stackError("gsc_zk_entity_setweaponitemclipammo() must be called on a weapon entity");
		stackPushUndefined();
		return;
	}
}

void gsc_zk_entity_getitemquantity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];
	gitem_t *bg_item = &bg_itemlist[0];

	if ( entity->s.index < 1 || bg_numItems <= entity->s.index )
	{
		stackError("gsc_zk_entity_getitemquantity() must be called on an ammo or health entity");
		stackPushUndefined();
		return;
	}

	bg_item += entity->s.index;
	if ( bg_item->giType == IT_AMMO || bg_item->giType == IT_HEALTH )
	{
		stackPushInt(bg_item->quantity);
	}
	else
	{
		stackError("gsc_zk_entity_getitemquantity() must be called on an ammo or health entity");
		stackPushUndefined();
		return;
	}
}

void gsc_zk_entity_setitemquantity(scr_entref_t ref)
{
	int id = ref.entnum;
	int quantity;
	gentity_t *entity = &g_entities[id];
	gitem_t *bg_item = &bg_itemlist[0];

	if ( !stackGetParams("i", &quantity) )
	{
		stackError("gsc_zk_entity_setitemquantity() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( entity->s.index < 1 || bg_numItems <= entity->s.index )
	{
		stackError("gsc_zk_entity_setitemquantity() must be called on an ammo or health entity");
		stackPushUndefined();
		return;
	}

	bg_item += entity->s.index;
	if ( bg_item->giType == IT_AMMO || bg_item->giType == IT_HEALTH )
	{
		bg_item->quantity = quantity;
		stackPushBool(qtrue);
	}
	else
	{
		stackError("gsc_zk_entity_setitemquantity() must be called on an ammo or health entity");
		stackPushUndefined();
		return;
	}
}

// ---- per-client brush-model solidity ----

void gsc_zk_entity_notsolidforplayer(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	if ( !entity->r.bmodel )
	{
		stackError("gsc_zk_entity_notsolidforplayer() entity %i does not have brush models", id);
		stackPushUndefined();
		return;
	}

	gentity_t *player = Scr_GetEntity(0);
	int id2 = player->s.number;
	if ( id2 >= MAX_CLIENTS )
		Scr_ParamError(0, va("entity %i is not a player", id2));

	customEntityState[id].clientMask[id2 >> 5] |= 1 << (id2 & 0x1F);
	customEntityState[id].notSolidBrushModel = qtrue;

	stackPushBool(qtrue);
}

void gsc_zk_entity_solidforplayer(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *entity = &g_entities[id];

	if ( !entity->r.bmodel )
	{
		stackError("gsc_zk_entity_solidforplayer() entity %i does not have brush models", id);
		stackPushUndefined();
		return;
	}

	gentity_t *player = Scr_GetEntity(0);
	int id2 = player->s.number;
	if ( id2 >= MAX_CLIENTS )
		Scr_ParamError(0, va("entity %i is not a player", id2));

	customEntityState[id].clientMask[id2 >> 5] &= ~(1 << (id2 & 0x1F));
	if ( !customEntityState[id].clientMask[0] && !customEntityState[id].clientMask[1] )
		customEntityState[id].notSolidBrushModel = qfalse;

	stackPushBool(qtrue);
}

// ---- entity gravity/velocity/bounce GSC interface ----

void Scr_DisableGravity(gentity_t *ent)
{
	int id = ent - g_entities;

	customEntityState[id].gravityType = GRAVITY_NONE;
	customEntityState[id].collideModels = qfalse;
	ent->s.eFlags &= ~EF_BOUNCE;
	ent->clipmask = CONTENTS_NONE;
	ent->physicsObject = 0;
	ent->s.groundEntityNum = ENTITYNUM_NONE;
	G_SetOrigin(ent, ent->r.currentOrigin);
	G_SetAngle(ent, ent->r.currentAngles);
}

void gsc_zk_entity_enablegravity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		int collideModels = qtrue;
		qboolean angledGravity = qtrue;

		if ( Scr_GetNumParam() > 0 && Scr_GetInt(0) == 0 )
		{
			collideModels = qfalse;
		}

		if ( Scr_GetNumParam() > 1 && Scr_GetInt(1) == 0 )
		{
			angledGravity = qfalse;
		}

		customEntityState[id].gravityType = GRAVITY_NO_BOUNCE;
		customEntityState[id].collideModels = collideModels;
		customEntityState[id].angledGravity = angledGravity;
		customEntityState[id].maxVelocity = 8192.0;
		ent->clipmask = MASK_SHOT | CONTENTS_PLAYERCLIP | CONTENTS_CANSHOTCLIP;
		ent->physicsObject = 1;
		memset(&ent->s.pos, 0, sizeof(trajectory_t));
		if ( customEntityState[id].angledGravity )
		{
			ent->s.apos.trType = TR_LINEAR;
			ent->s.apos.trTime = level.time;
			vectoangles(ent->r.currentAngles, ent->s.apos.trBase);
		}

		stackPushBool(qtrue);
	}
	else
	{
		stackError("gsc_zk_entity_enablegravity() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_disablegravity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			Scr_DisableGravity(ent);

			stackPushBool(qtrue);
		}
		else
		{
			stackPushBool(qfalse);
		}
	}
	else
	{
		stackError("gsc_zk_entity_disablegravity() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_isgravityenabled(scr_entref_t ref)
{
	int id = ref.entnum;

	if ( customEntityState[id].gravityType )
		stackPushBool(qtrue);
	else
		stackPushBool(qfalse);
}

void gsc_zk_entity_enablebounce(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			float parallelBounce = 0.5; // default from frag_grenade_german_mp
			float perpendicularBounce = 0.25; // default from frag_grenade_german_mp

			if ( Scr_GetNumParam() > 0 )
			{
				parallelBounce = Scr_GetFloat(0);
				if ( Scr_GetNumParam() > 1 )
					perpendicularBounce = Scr_GetFloat(1);
			}

			customEntityState[id].gravityType = GRAVITY_BOUNCE;
			customEntityState[id].parallelBounce = parallelBounce;
			customEntityState[id].perpendicularBounce = perpendicularBounce;
			ent->s.eFlags |= EF_BOUNCE;

			stackPushBool(qtrue);
		}
		else
		{
			stackPushBool(qfalse);
		}
	}
	else
	{
		stackError("gsc_zk_entity_enablebounce() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_disablebounce(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			customEntityState[id].gravityType = GRAVITY_NO_BOUNCE;
			ent->s.eFlags &= ~EF_BOUNCE;

			stackPushBool(qtrue);
		}
		else
		{
			stackPushBool(qfalse);
		}
	}
	else
	{
		stackError("gsc_zk_entity_disablebounce() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_getentityvelocity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			stackPushVector(customEntityState[id].velocity); // ent->s.pos.trDelta returns only the added velocity
		}
		else
		{
			stackPushUndefined();
		}
	}
	else
	{
		stackError("gsc_zk_entity_getentityvelocity() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_setentityvelocity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			vec3_t velocity;

			Scr_GetVector(0, velocity);
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
			VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
			VectorCopy(velocity, ent->s.pos.trDelta);
			if ( customEntityState[id].angledGravity )
			{
				ent->s.apos.trType = TR_LINEAR;
				ent->s.apos.trTime = level.time;
				vectoangles(ent->r.currentAngles, ent->s.apos.trBase);
			}
			if ( !zk_IsNullVector_ent(velocity) )
				ent->s.groundEntityNum = ENTITYNUM_NONE;

			stackPushBool(qtrue);
		}
		else
		{
			stackPushBool(qfalse);
		}
	}
	else
	{
		stackError("gsc_zk_entity_setentityvelocity() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_addentityvelocity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			vec3_t velocity;

			Scr_GetVector(0, velocity);
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
			VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
			VectorAdd(customEntityState[id].velocity, velocity, ent->s.pos.trDelta);
			if ( customEntityState[id].angledGravity )
			{
				ent->s.apos.trType = TR_LINEAR;
				ent->s.apos.trTime = level.time;
				vectoangles(ent->r.currentAngles, ent->s.apos.trBase);
			}
			if ( !zk_IsNullVector_ent(velocity) )
				ent->s.groundEntityNum = ENTITYNUM_NONE;

			stackPushBool(qtrue);
		}
		else
		{
			stackPushBool(qfalse);
		}
	}
	else
	{
		stackError("gsc_zk_entity_addentityvelocity() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_getmaxentityvelocity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			stackPushFloat(customEntityState[id].maxVelocity);
		}
		else
		{
			stackPushUndefined();
		}
	}
	else
	{
		stackError("gsc_zk_entity_getmaxentityvelocity() entity is not a script_model");
		stackPushUndefined();
	}
}

void gsc_zk_entity_setmaxentityvelocity(scr_entref_t ref)
{
	int id = ref.entnum;
	gentity_t *ent = &g_entities[id];

	if ( ent->classname == scr_const.script_model )
	{
		if ( customEntityState[id].gravityType )
		{
			customEntityState[id].maxVelocity = Scr_GetFloat(0);
			if ( customEntityState[id].maxVelocity < 0.0 )
				customEntityState[id].maxVelocity = 0.0;

			stackPushBool(qtrue);
		}
		else
		{
			stackPushBool(qfalse);
		}
	}
	else
	{
		stackError("gsc_zk_entity_setmaxentityvelocity() entity is not a script_model");
		stackPushUndefined();
	}
}

#endif
