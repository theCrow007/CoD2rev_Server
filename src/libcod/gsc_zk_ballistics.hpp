#ifndef _GSC_ZK_BALLISTICS_HPP
#define _GSC_ZK_BALLISTICS_HPP

#include "gsc_zk_custom_state.hpp"

void zk_Bullet_Drop_Nextpos(float *end, droppingBullet_t *bullet);
void zk_Bullet_Drop_Firstpos(float spread, float *end, const weaponParms *wp, float distance, droppingBullet_t *bullet);
void zk_Bullet_Drop_Free(int clientNum, droppingBullet_t *bullet);
gentity_t *zk_Bullet_Drop_Create_Visual(int clientNum, droppingBullet_t *bullet, float *start, float *end);
void zk_Bullet_Drop_Update_Visual(droppingBullet_t *bullet, float *end);
void zk_Bullet_Drop_Finalize_Visual(droppingBullet_t *bullet, qboolean remove);
qboolean zk_Bullet_Fire_Drop(droppingBullet_t *bullet, const gentity_t *inflictor, gentity_t *attacker, float *start, float *end, float dmgScale, const weaponParms *wp, const gentity_t *weaponEnt);
void zk_Bullet_Fire_Drop_Think(int clientNum, droppingBullet_t *bullet);
void zk_Bullet_Fire_Drop_Think_AntiLag(int clientNum, droppingBullet_t *bullet);

void zk_RunDroppingBullets(void);
qboolean zk_TryFireDroppingBullet(gentity_t *attacker, float spread, weaponParms *wp, gentity_t *weaponEnt, int gametime);
#endif
