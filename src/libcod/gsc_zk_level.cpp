#include "gsc_zk_level.hpp"

#if LIBCOD_COMPILE_LEVEL == 1

#include <stdio.h>

/* zk uses MAX_STRINGLENGTH (1024) as a sanity bound; rev doesn't define it. */
#ifndef MAX_STRINGLENGTH
#define MAX_STRINGLENGTH 1024
#endif

void gsc_zk_level_getmovers()
{
	gentity_t *ent = g_entities;
	int i;

	stackPushArray();
	for ( i = 0; i < level.num_entities; i++, ent++ )
	{
		if ( ent->s.eType == ET_SCRIPTMOVER )
		{
			stackPushEntity(ent);
			stackPushArrayLast();
		}
	}
}

void gsc_zk_level_getentitycount()
{
	int inUseOnly;
	gentity_t *ent;
	int i;
	int entities = 0;

	if ( !stackGetParams("i", &inUseOnly) )
		inUseOnly = 0;

	if ( inUseOnly )
	{
		ent = g_entities;
		for ( i = 0; i < level.num_entities; i++, ent++ )
		{
			if ( ent->r.inuse != 0 )
				entities++;
		}
		stackPushInt(entities);
	}
	else
	{
		stackPushInt(level.num_entities);
	}
}

void gsc_zk_level_setnorthyaw()
{
	float fYaw;

	if ( !stackGetParams("f", &fYaw) )
	{
		stackError("gsc_zk_level_setnorthyaw() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	int len = snprintf(NULL, 0, "%g", fYaw);
	if ( len <= 0 || len >= MAX_STRINGLENGTH )
	{
		stackError("gsc_zk_level_setnorthyaw() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	char *szYaw = (char *)Z_MallocInternal(len + 1);
	snprintf(szYaw, len + 1, "%g", fYaw);
	SV_SetConfigstring(11, szYaw); // 11 = CS_NORTHYAW
	Z_FreeInternal(szYaw);

	stackPushBool(qtrue);
}

void gsc_zk_level_getsavepersist()
{
	// rev stores this as level.savePersist, exposed via G_GetSavePersist()
	extern int G_GetSavePersist(void);
	stackPushBool(G_GetSavePersist());
}

void gsc_zk_level_setsavepersist()
{
	int save;

	if ( !stackGetParams("i", &save) )
	{
		stackError("gsc_zk_level_setsavepersist() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	extern void G_SetSavePersist(int savepersist);
	G_SetSavePersist(save);

	stackPushBool(qtrue);
}

#endif
