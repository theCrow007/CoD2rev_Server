#include "gsc_zk_entity.hpp"

#if LIBCOD_COMPILE_ENTITY == 1

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

#endif
