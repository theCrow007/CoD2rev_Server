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

// ---- weapon ignore + default weapon (substitution only; the engine global-fallback
//      byte-patches zk used are not portable to rev's link-by-name binary) ----
#ifndef MAX_WEAPON_NAME_SIZE
#define MAX_WEAPON_NAME_SIZE 32
#endif
#define ZK_MAX_WEAPON_IGNORE_SIZE 20

static char zk_defaultWeapon[MAX_WEAPON_NAME_SIZE] = "defaultweapon_mp";
static char zk_ignoredWeapons[ZK_MAX_WEAPON_IGNORE_SIZE][MAX_WEAPON_NAME_SIZE];
static int zk_ignoredWeaponCount = 0;

static bool zk_IsOnIgnoreList(const char *weapon)
{
	for ( int i = 0; i < zk_ignoredWeaponCount; i++ )
		if ( strcmp(zk_ignoredWeapons[i], weapon) == 0 )
			return true;
	return false;
}

// consumed by BG_GetWeaponIndexForName: ignored weapon names resolve to the default
const char *zk_IgnoredWeaponRemap(const char *name)
{
	if ( zk_ignoredWeaponCount > 0 && name && zk_IsOnIgnoreList(name) )
		return zk_defaultWeapon;
	return name;
}

void gsc_zk_weapons_ignoreweapon()
{
	char *weapon;
	if ( !stackGetParams("s", &weapon) )
	{
		stackError("gsc_zk_weapons_ignoreweapon() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	if ( strlen(weapon) > ( MAX_WEAPON_NAME_SIZE - 1 ) )
	{
		stackError("gsc_zk_weapons_ignoreweapon() weapon name is too long");
		stackPushUndefined();
		return;
	}
	if ( zk_ignoredWeaponCount >= ZK_MAX_WEAPON_IGNORE_SIZE )
	{
		stackError("gsc_zk_weapons_ignoreweapon() exceeded max ignore size %d", ZK_MAX_WEAPON_IGNORE_SIZE);
		stackPushUndefined();
		return;
	}
	I_strncpyz(zk_ignoredWeapons[zk_ignoredWeaponCount], weapon, MAX_WEAPON_NAME_SIZE);
	zk_ignoredWeaponCount++;
	stackPushBool(qtrue);
}

void gsc_zk_weapons_resetignoredweapons()
{
	zk_ignoredWeaponCount = 0;
}

void gsc_zk_weapons_setdefaultweapon()
{
	char *weapon;
	if ( !stackGetParams("s", &weapon) )
	{
		stackError("gsc_zk_weapons_setdefaultweapon() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	if ( strlen(weapon) > ( MAX_WEAPON_NAME_SIZE - 1 ) )
	{
		stackError("gsc_zk_weapons_setdefaultweapon() weapon name is too long");
		stackPushUndefined();
		return;
	}
	if ( strcmp(zk_defaultWeapon, weapon) == 0 )
	{
		stackPushInt(2);
		return;
	}
	I_strncpyz(zk_defaultWeapon, weapon, MAX_WEAPON_NAME_SIZE);
	// NOTE: unlike zk, this does NOT byte-patch the engine's global default-weapon
	// fallback (G_SetupWeaponDef / BG_FillInAmmoItems); it only sets the weapon that
	// ignoreWeapon() substitutions resolve to.
	stackPushInt(1);
}

// ---- spawn grenade ----
void gsc_zk_weapons_spawngrenade(scr_entref_t ref)
{
	int id = ref.entnum;
	int args;
	gentity_t *owner;
	int weaponIndex;
	WeaponDef *weapon;
	vec3_t origin;
	vec3_t dir = {0, 0, 0};
	vec3_t velocity = {0, 0, 0};
	int fuseTime;
	gentity_t *grenade;

	args = Scr_GetNumParam();
	owner = &g_entities[id];

	if ( args < 2 || args > 5 )
	{
		stackError("gsc_zk_weapons_spawngrenade() wrong number of arguments");
		stackPushUndefined();
		return;
	}

	weaponIndex = BG_FindWeaponIndexForName(Scr_GetString(0));
	if ( weaponIndex <= 0 )
	{
		stackError("gsc_zk_weapons_spawngrenade() weapon not precached");
		stackPushUndefined();
		return;
	}

	weapon = BG_GetWeaponDef(weaponIndex);
	Scr_GetVector(1, origin);
	if ( args > 2 ) Scr_GetVector(2, dir);
	if ( args > 3 ) Scr_GetVector(3, velocity);
	if ( args > 4 )
		fuseTime = (int)(Scr_GetFloat(4) * 1000);
	else
		fuseTime = weapon->fuseTime;

	grenade = fire_grenade(owner, origin, dir, weaponIndex, fuseTime);
	Vec3Normalize(dir);
	VectorMA(grenade->s.pos.trDelta, DotProduct(velocity, dir), dir, grenade->s.pos.trDelta);

	stackPushEntity(grenade);
}

#endif
