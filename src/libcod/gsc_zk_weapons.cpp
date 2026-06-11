#include "gsc_zk_weapons.hpp"

#if LIBCOD_COMPILE_WEAPONS == 1

/* Ported from zk_libcod gsc_weapons.cpp, adapted to rev's WeaponDef field
 * names and rev's isValidWeaponId/BG_GetWeaponDef helpers. */

void gsc_zk_weapons_issemiautoweapon()
{
	int id;
	const char *name;

	if ( stackGetParams("s", &name) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("i", &id) )
	{
		stackError("gsc_zk_weapons_issemiautoweapon() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	stackPushInt(weapon->semiAuto);
}

void gsc_zk_weapons_getweaponraisetime()
{
	int id;
	const char *name;

	if ( stackGetParams("s", &name) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("i", &id) )
	{
		stackError("gsc_zk_weapons_getweaponraisetime() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	stackPushInt(weapon->raiseTime);
}

void gsc_zk_weapons_setweaponraisetime()
{
	int id;
	const char *name;
	int time;

	if ( stackGetParams("si", &name, &time) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("ii", &id, &time) )
	{
		stackError("gsc_zk_weapons_setweaponraisetime() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	weapon->raiseTime = time;
	stackPushBool(qtrue);
}

void gsc_zk_weapons_getweaponfusetime()
{
	int id;
	const char *name;

	if ( stackGetParams("s", &name) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("i", &id) )
	{
		stackError("gsc_zk_weapons_getweaponfusetime() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	stackPushInt(weapon->fuseTime);
}

void gsc_zk_weapons_setweaponfusetime()
{
	int id;
	const char *name;
	int time;

	if ( stackGetParams("si", &name, &time) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("ii", &id, &time) )
	{
		stackError("gsc_zk_weapons_setweaponfusetime() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	weapon->fuseTime = time;
	stackPushBool(qtrue);
}

void gsc_zk_weapons_getmovespeedscale()
{
	int id;
	const char *name;

	if ( stackGetParams("s", &name) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("i", &id) )
	{
		stackError("gsc_zk_weapons_getmovespeedscale() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	stackPushFloat(weapon->moveSpeedScale);
}

void gsc_zk_weapons_setmovespeedscale()
{
	int id;
	const char *name;
	float scale;

	if ( stackGetParams("sf", &name, &scale) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("if", &id, &scale) )
	{
		stackError("gsc_zk_weapons_setmovespeedscale() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	weapon->moveSpeedScale = scale;
	stackPushBool(qtrue);
}

void gsc_zk_weapons_getweapondisplayname()
{
	int id;
	const char *name;

	if ( stackGetParams("s", &name) )
		id = BG_FindWeaponIndexForName(name);
	else if ( !stackGetParams("i", &id) )
	{
		stackError("gsc_zk_weapons_getweapondisplayname() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( !isValidWeaponId(id) )
	{
		stackPushUndefined();
		return;
	}

	WeaponDef *weapon = BG_GetWeaponDef(id);
	stackPushString(weapon->displayName);

	// mark the returned value as a localized (ISTRING) string, like zk does
	VariableValue *var;
	int param = 0;
	var = &scrVmPub.top[-param];
	var->type = VAR_ISTRING;
}

#endif
