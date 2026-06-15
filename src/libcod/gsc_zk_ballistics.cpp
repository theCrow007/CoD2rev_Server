#include "gsc.hpp"
#include "gsc_zk_custom_state.hpp"

#ifdef LIBCOD

/*
 * Bullet-drop projectile simulation (ported from zk_libcod).
 * These functions are the simulation engine; they are driven by the per-frame
 * advance loop (G_RunFrame hook) and the Bullet_Fire injection (Phase 3).
 */

extern unsigned char riflePriorityMap[];
extern unsigned char bulletPriorityMap[];
extern dvar_t *g_corpseHit;
extern unsigned int zk_const_bullet;

void zk_Bullet_Drop_Nextpos(float *end, droppingBullet_t *bullet)
{
	float timeDelta;
	float zDelta;

	VectorMA(bullet->position, bullet->velocity, bullet->direction, end);
	timeDelta = 1.0 / (float)sv_fps->current.integer;
	zDelta = (bullet->zVelocity * timeDelta) + (0.5 * (386.08858267717 * (timeDelta * timeDelta)));
	bullet->zVelocity = bullet->zVelocity - (386.08858267717 * timeDelta);
	end[2] += zDelta;
}

void zk_Bullet_Drop_Firstpos(float spread, float *end, const weaponParms *wp, float distance, droppingBullet_t *bullet)
{
	float timeDelta;
	float zDelta;

	Bullet_Endpos(spread, end, (weaponParms *)wp, distance, -1);
	timeDelta = 1.0 / (float)sv_fps->current.integer;
	zDelta = 0.5 * (386.08858267717 * (timeDelta * timeDelta));
	bullet->zVelocity = bullet->zVelocity - (386.08858267717 * timeDelta);
	end[2] += zDelta;
}

void zk_Bullet_Drop_Free(int clientNum, droppingBullet_t *bullet)
{
	customPlayerState[clientNum].droppingBulletsCount--;
	Com_DPrintf("zk_Bullet_Drop_Free: Done after %.2fs and %f units of distance, %d bullets still active\n",
		(float)(level.time - bullet->startTime) / 1000, bullet->distance, customPlayerState[clientNum].droppingBulletsCount);
	memset(bullet, 0, sizeof(droppingBullet_t));
}

gentity_t *zk_Bullet_Drop_Create_Visual(int clientNum, droppingBullet_t *bullet, float *start, float *end)
{
	gentity_t *ent;
	vec3_t dir;
	vec3_t angles;

	ent = G_Spawn();
	Scr_SetString(&ent->classname, zk_const_bullet);
	ent->r.contents = 0;
	ent->clipmask = 0;
	ent->s.eType = ET_GENERAL;
	ent->freeAfterEvent = 1;
	ent->eventTime = level.time + 50;
	G_SetModel(ent, G_ModelName(bullet->visualBulletModelIndex));
	G_DObjUpdate(ent);

	G_SetOrigin(ent, start);
	VectorSubtract(end, ent->r.currentOrigin, dir);
	vectoangles(dir, angles);
	G_SetAngle(ent, angles);

	ent->s.pos.trType = TR_LINEAR;
	ent->s.pos.trDuration = 50;
	ent->s.pos.trTime = level.time - 50;
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
	VectorScale(dir, 20.0, ent->s.pos.trDelta);
	SV_LinkEntity(ent);

	return ent;
}

void zk_Bullet_Drop_Update_Visual(droppingBullet_t *bullet, float *end)
{
	gentity_t *ent = bullet->visualBullet;
	vec3_t angles;

	ent->eventTime += 50;

	VectorCopy(bullet->position, ent->r.currentOrigin);
	vectoangles(bullet->direction, angles);
	G_SetAngle(ent, angles);

	ent->s.pos.trType = TR_LINEAR;
	ent->s.pos.trDuration = 50;
	ent->s.pos.trTime = level.time - 50;
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
	VectorScale(bullet->direction, 20.0, ent->s.pos.trDelta);
}

void zk_Bullet_Drop_Finalize_Visual(droppingBullet_t *bullet, qboolean remove)
{
	vec3_t angles;

	G_SetOrigin(bullet->visualBullet, bullet->position); // Sets TR_STATIONARY
	vectoangles(bullet->direction, angles);
	G_SetAngle(bullet->visualBullet, angles);

	if ( bullet->visualTime > 0 && !remove )
		bullet->visualBullet->eventTime = level.time + bullet->visualTime;
	else
		G_FreeEntity(bullet->visualBullet);
}

qboolean zk_Bullet_Fire_Drop(droppingBullet_t *bullet, const gentity_t *inflictor, gentity_t *attacker, float *start, float *end, float dmgScale, const weaponParms *wp, const gentity_t *weaponEnt)
{
	int surfaceType;
	int event;
	float dot;
	vec3_t dir;
	vec3_t origin;
	meansOfDeath_t meansOfDeath;
	int dflags;
	int damage;
	float dist;
	vec3_t temp;
	gentity_t *self;
	gentity_t *tempEnt;
	trace_t trace;
	int contentMask;
	unsigned char *priorityMap;
	qboolean noBulletImpacts;
	float scaleSquared;
	float dirScale;

	dflags = 0;

	if ( wp->weapDef->rifleBullet )
	{
		meansOfDeath = MOD_RIFLE_BULLET;
		dflags = 32;
	}
	else
	{
		meansOfDeath = MOD_PISTOL_BULLET;
	}

	if ( wp->weapDef->armorPiercing )
		dflags |= 2u;

	if ( wp->weapDef->rifleBullet )
		priorityMap = riflePriorityMap;
	else
		priorityMap = bulletPriorityMap;

	contentMask = MASK_SHOT;

	if ( bullet->lastHitEnt )
		G_LocationalTrace(&trace, start, end, bullet->lastHitEnt->s.number, contentMask, priorityMap);
	else
		G_LocationalTrace(&trace, start, end, inflictor->s.number, contentMask, priorityMap);
	Vec3Lerp(start, end, trace.fraction, origin);
	G_CheckHitTriggerDamage(attacker, start, origin, wp->weapDef->damage, meansOfDeath);
	self = &g_entities[trace.entityNum];
	VectorSubtract(end, start, dir);
	Vec3Normalize(dir);

	// Update bullet trajectory values
	bullet->distance += Vec3Distance(bullet->position, origin);
	VectorCopy(origin, bullet->position);
	VectorCopy(dir, bullet->direction);
	bullet->dmgScale = dmgScale;

	// Apply (linear) bullet drag
	bullet->velocity *= 1.0 - ((1.0 - bullet->drag) * trace.fraction);

	dot = DotProduct(dir, trace.normal) * -2.0;
	VectorMA(dir, dot, trace.normal, dir);

	// Bullet holes
	if ( ( trace.surfaceFlags & SURF_SKY ) == 0 && !self->client && trace.fraction < 1.0 )
	{
		if ( wp->weapDef->rifleBullet )
			event = EV_SHOTGUN_HIT;
		else
			event = EV_BULLET_HIT_LARGE;

		if ( attacker->s.number < 64 )
			noBulletImpacts = customPlayerState[attacker->s.number].noBulletImpacts;
		else
			noBulletImpacts = qfalse;

		if ( attacker->s.number < 64 && customPlayerState[attacker->s.number].fireThroughWalls )
		{
			if ( !noBulletImpacts )
			{
				gentity_t *hitEffect;
				hitEffect = G_TempEntity(origin, event);
				hitEffect->s.eventParm = DirToByte(trace.normal) & 0xFF;
				hitEffect->s.surfType = (trace.surfaceFlags >> 20) & 0x1F;
			}
		}
		else if ( !noBulletImpacts )
		{
			tempEnt = G_TempEntity(origin, event);
			tempEnt->s.eventParm = DirToByte(trace.normal);
			tempEnt->s.scale = DirToByte(dir);

			if ( self->s.eType == ET_PLAYER_CORPSE && g_corpseHit->current.boolean )
				surfaceType = 7;
			else
				surfaceType = (trace.surfaceFlags & 0x1F00000) >> 20;

			tempEnt->s.surfType = surfaceType;
			tempEnt->s.otherEntityNum = weaponEnt->s.number;
		}
	}

	// If glass was hit, keep the bullet alive
	if ( ( trace.contents & CONTENTS_GLASS ) != 0 )
	{
		VectorSubtract(end, start, dir);
		Vec3Normalize(dir);
		scaleSquared = DotProduct(trace.normal, dir);

		if ( -scaleSquared < 0.125 )
			dirScale = 0.0;
		else
			dirScale = 0.25 / -scaleSquared;

		VectorMA(origin, dirScale, dir, bullet->position);
		return qfalse;
	}
	else if ( self->takedamage )
	{
		// An entity that can take damage was hit
		if ( self != attacker )
		{
			VectorSubtract(start, origin, temp);
			dist = VectorLength(temp);
			damage = (int)((float)Bullet_GetDamage(wp, dist) * dmgScale);

			G_Damage(self, attacker, attacker, wp->forward, origin, damage, dflags, meansOfDeath, (hitLocation_t)trace.partGroup, bullet->timeOffset);

			bullet->lastHitEnt = self;

			if ( self->client )
			{
				if ( ( dflags & 0x20 ) != 0 && ( Dvar_GetInt("scr_friendlyfire") || !OnSameTeam(self, attacker) ) )
				{
					bullet->dmgScale = dmgScale * 0.5;
					return qfalse;
				}
			}
		}
	}

	// Bullet did not hit anything
	if ( trace.fraction == 1.0 && trace.entityNum == ENTITYNUM_NONE )
		return qfalse;

	return qtrue;
}

void zk_Bullet_Fire_Drop_Think(int clientNum, droppingBullet_t *bullet)
{
	vec3_t end;

	zk_Bullet_Drop_Nextpos(end, bullet);
	if ( zk_Bullet_Fire_Drop(bullet, bullet->attacker, bullet->inflictor, bullet->position, end, bullet->dmgScale, &bullet->wp, bullet->attacker) )
	{
		if ( bullet->visualBullet )
			zk_Bullet_Drop_Finalize_Visual(bullet, qfalse);
		zk_Bullet_Drop_Free(clientNum, bullet);
	}
	else
	{
		if ( bullet->visualBullet )
			zk_Bullet_Drop_Update_Visual(bullet, end);
	}
}

void zk_Bullet_Fire_Drop_Think_AntiLag(int clientNum, droppingBullet_t *bullet)
{
	AntilagClientStore antilagStore;
	vec3_t end;

	G_AntiLagRewindClientPos(level.time - bullet->timeOffset, &antilagStore);
	zk_Bullet_Drop_Nextpos(end, bullet);
	if ( zk_Bullet_Fire_Drop(bullet, bullet->attacker, bullet->inflictor, bullet->position, end, bullet->dmgScale, &bullet->wp, bullet->attacker) )
	{
		if ( bullet->visualBullet )
			zk_Bullet_Drop_Finalize_Visual(bullet, qfalse);
		zk_Bullet_Drop_Free(clientNum, bullet);
	}
	else
	{
		if ( bullet->visualBullet )
			zk_Bullet_Drop_Update_Visual(bullet, end);
	}
	G_AntiLag_RestoreClientPos(&antilagStore);
}

extern dvar_t *g_bulletDrop;
extern dvar_t *g_bulletDropMaxTime;
extern dvar_t *g_antilag;

// Per-frame: advance every active dropping bullet. Called from G_RunFrame.
void zk_RunDroppingBullets(void)
{
	int i, j;
	client_t *client;

	if ( !g_bulletDrop->current.boolean )
		return;

	for ( i = 0, client = svs.clients; i < sv_maxclients->current.integer; i++, client++ )
	{
		if ( client->state < CS_CONNECTED || customPlayerState[i].droppingBulletsCount < 1 )
			continue;

		for ( j = 0; j < MAX_DROPPING_BULLETS; j++ )
		{
			droppingBullet_t *bullet = &customPlayerState[i].droppingBullets[j];
			if ( bullet->inUse )
			{
				// Enforce max. bullet lifetime
				if ( ( level.time - bullet->startTime ) > g_bulletDropMaxTime->current.integer )
				{
					if ( bullet->visualBullet )
						zk_Bullet_Drop_Finalize_Visual(bullet, qtrue);
					zk_Bullet_Drop_Free(i, bullet);
					continue;
				}

				if ( g_antilag->current.boolean )
					zk_Bullet_Fire_Drop_Think_AntiLag(i, bullet);
				else
					zk_Bullet_Fire_Drop_Think(i, bullet);
			}
		}
	}
}

// On fire: if bullet-drop is active for this shooter, queue a simulated bullet and
// return qtrue (skip the normal hitscan). Returns qfalse to let normal fire proceed.
qboolean zk_TryFireDroppingBullet(gentity_t *attacker, float spread, weaponParms *wp, gentity_t *weaponEnt, int gametime)
{
	float distance;
	vec3_t end;
	int i;
	int id;

	if ( !g_bulletDrop->current.boolean || !attacker->client )
		return qfalse;

	id = attacker->client->ps.clientNum;

	if ( !customPlayerState[id].droppingBulletsEnabled )
		return qfalse;

	if ( customPlayerState[id].droppingBulletsCount >= MAX_DROPPING_BULLETS )
	{
		Com_DPrintf("zk_TryFireDroppingBullet: too many bullets in flight for player %d, shot aborted\n", id);
		return qtrue; // shot consumed (aborted) - do not also hitscan
	}

	droppingBullet_t bullet;
	memset(&bullet, 0, sizeof(droppingBullet_t));

	bullet.drag = customPlayerState[id].droppingBulletDrag;

	distance = customPlayerState[id].droppingBulletVelocity / (float)sv_fps->current.integer;
	if ( distance > 8192.0 ) // Clamp to common stock value
		distance = 8192.0;

	zk_Bullet_Drop_Firstpos(spread, end, wp, distance, &bullet);

	bullet.visualBullet = NULL;
	if ( customPlayerState[id].droppingBulletVisuals )
	{
		bullet.visualBulletModelIndex = customPlayerState[id].droppingBulletVisualModelIndex;
		bullet.visualBullet = zk_Bullet_Drop_Create_Visual(id, &bullet, wp->muzzleTrace, end);
		bullet.visualTime = customPlayerState[id].droppingBulletVisualTime;
	}

	// If the bullet did not immediately hit an obstacle, queue it for next frame
	if ( !zk_Bullet_Fire_Drop(&bullet, weaponEnt, attacker, wp->muzzleTrace, end, 1.0, wp, weaponEnt) )
	{
		for ( i = 0; i < MAX_DROPPING_BULLETS; i++ )
		{
			if ( !customPlayerState[id].droppingBullets[i].inUse )
			{
				bullet.attacker = weaponEnt;
				bullet.distance = distance;
				bullet.inflictor = attacker;
				bullet.inUse = qtrue;
				bullet.startTime = level.time;
				bullet.timeOffset = level.time - gametime;
				bullet.velocity = distance;
				bullet.weaponEnt = weaponEnt;
				memcpy(&bullet.wp, wp, sizeof(weaponParms));
				memcpy(&customPlayerState[id].droppingBullets[i], &bullet, sizeof(droppingBullet_t));
				customPlayerState[id].droppingBulletsCount++;
				break;
			}
		}
	}
	else
	{
		if ( bullet.visualBullet )
			zk_Bullet_Drop_Finalize_Visual(&bullet, qfalse);
	}

	return qtrue;
}

#endif
