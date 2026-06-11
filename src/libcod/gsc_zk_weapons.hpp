#ifndef _GSC_ZK_WEAPONS_HPP_
#define _GSC_ZK_WEAPONS_HPP_

/*
 * Weapon-def getters/setters ported from ibuddieat/zk_libcod (gsc_weapons.cpp).
 * Written in rev's own gsc_weapons idiom (BG_FindWeaponIndexForName +
 * isValidWeaponId + BG_GetWeaponDef). zk's WeaponDef field names were
 * reconciled to rev's: bSemiAuto->semiAuto, iRaiseTime->raiseTime,
 * iFuseTime->fuseTime, fMoveSpeedScale->moveSpeedScale, szDisplayName->displayName.
 *
 * Deferred: ignoreWeapon/resetIgnoredWeapons/setDefaultWeapon (zk globals + hooks),
 * spawnGrenade / *grenadeFuseTime (entity spawn / fuse tracking),
 * getWeaponItemAmmo / ...ClipAmmo (playerState ammo-array reconciliation).
 */
#include "gsc.hpp"

// rev's helper, defined (non-static) in gsc_weapons.cpp but not header-declared
qboolean isValidWeaponId(int id);

void gsc_zk_weapons_issemiautoweapon();
void gsc_zk_weapons_getweaponraisetime();
void gsc_zk_weapons_setweaponraisetime();
void gsc_zk_weapons_getweaponfusetime();
void gsc_zk_weapons_setweaponfusetime();
void gsc_zk_weapons_getmovespeedscale();
void gsc_zk_weapons_setmovespeedscale();
void gsc_zk_weapons_getweapondisplayname();

#endif
