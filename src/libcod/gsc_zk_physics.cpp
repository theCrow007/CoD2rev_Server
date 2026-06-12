// zk_libcod entity gravity physics integrator, ported onto CoD2rev_Server.
//
// Provides custom per-entity gravity/bounce physics for script_model entities,
// driven by customEntityState[].gravityType. Dispatched from G_RunFrameForEntity
// via zk_EntityHasGravity()/zk_RunEntityGravity().
//
// Faithful port of zk's G_RunGravityModelNoBounce / G_RunGravityModelWithBounce /
// G_BounceGravityModel. Two deliberate deviations, documented for parity tracking:
//   - The optional "land"/"bounce" script notifies are omitted (they require
//     registering custom script constants; the physics is unaffected, scripts
//     just don't receive the waittill callbacks).
//   - The EF_TAGCONNECT special case (Missile_TraceNoContents) is skipped; rev
//     lacks that primitive and tag-connected gravity entities are an edge case.

#include "gsc_zk_physics.hpp"

#if LIBCOD_COMPILE_ENTITY == 1

#include "gsc_zk_custom_state.hpp"
#include <math.h>

extern void G_MissileLandAngles(gentity_t *ent, trace_t *trace, vec3_t vAngles, qboolean bForceAlign);

// --- small vector helpers missing from rev ---

static void zk_VectorClampLength(vec3_t v, float maxLength)
{
	float len = VectorLength(v);
	if ( len > maxLength && len > 0.0f )
		VectorScale(v, maxLength / len, v);
}

static qboolean zk_IsNullVector(const vec3_t v)
{
	return ( v[0] == 0.0f && v[1] == 0.0f && v[2] == 0.0f );
}

// --- physics integrators (zk: G_RunItem / G_RunMissile / G_BounceMissile as base) ---

void G_RunGravityModelNoBounce(gentity_t *ent)
{
	vec3_t subOrigin;
	vec3_t lerpOrigin;
	trace_t trace;
	vec3_t origin;
	vec3_t maxLerpVector;

	if ( ( ( ( ent->s.groundEntityNum == ENTITYNUM_NONE ) || ( level.gentities[ent->s.groundEntityNum].s.pos.trType != TR_STATIONARY ) ) && ( ent->s.pos.trType != TR_GRAVITY ) ) &&
	     ( ( ( ent->spawnflags ^ 1 ) & 1 ) != 0 ) )
	{
		ent->s.pos.trType = TR_GRAVITY;
		ent->s.pos.trTime = level.time;
		VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
		VectorClear(ent->s.pos.trDelta);
	}
	if ( ( ( ent->s.pos.trType == TR_STATIONARY ) || ( ent->s.pos.trType == TR_GRAVITY_PAUSED ) ) || ( ent->tagInfo != NULL ) )
	{
		// stationary / paused / tag-linked: nothing to integrate
		return;
	}

	BG_EvaluateTrajectory(&ent->s.pos, level.time + 50, origin);

	if ( customEntityState[ent->s.number].maxVelocity > 0.0f )
	{
		VectorSubtract(origin, ent->r.currentOrigin, maxLerpVector);
		zk_VectorClampLength(maxLerpVector, customEntityState[ent->s.number].maxVelocity / (float)sv_fps->current.integer);
		VectorAdd(ent->r.currentOrigin, maxLerpVector, origin);
	}

	if ( Vec3DistanceSq(ent->r.currentOrigin, origin) < 0.1f )
		origin[2] = origin[2] - 1.0f;

	SV_Trace(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, ent->clipmask, customEntityState[ent->s.number].collideModels, NULL, 1);

	if ( trace.fraction < 1.0f )
	{
		Vec3Lerp(ent->r.currentOrigin, origin, trace.fraction, lerpOrigin);
		if ( ( trace.startsolid == 0 && ( trace.fraction < 0.01f ) ) && ( trace.normal[2] < 0.5f ) )
		{
			VectorSubtract(origin, ent->r.currentOrigin, subOrigin);
			VectorMA(origin, 1 - DotProduct(subOrigin, trace.normal), trace.normal, origin);
			SV_Trace(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, ent->clipmask, customEntityState[ent->s.number].collideModels, NULL, 1);
			Vec3Lerp(lerpOrigin, origin, trace.fraction, lerpOrigin);
		}
		ent->s.pos.trType = TR_LINEAR_STOP;
		ent->s.pos.trTime = level.time;
		ent->s.pos.trDuration = 50;
		VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
		VectorSubtract(lerpOrigin, ent->r.currentOrigin, ent->s.pos.trDelta);
		VectorScale(ent->s.pos.trDelta, 20.0f, ent->s.pos.trDelta);
		VectorCopy(lerpOrigin, ent->r.currentOrigin);
	}
	else
	{
		VectorCopy(origin, ent->r.currentOrigin);
	}
	SV_LinkEntity(ent);

	if ( ent->r.inuse != 0 && ( trace.fraction < 0.01f ) )
	{
		if ( trace.normal[2] > 0.0f )
		{
			if ( customEntityState[ent->s.number].angledGravity )
			{
				vec3_t angles;
				vec3_t axis[3]; // [0]=forward [1]=right [2]=up
				vec3_t fwd;

				// Build an orthonormal basis aligned to the surface normal,
				// matching zk's v1/v2/v3 cross-product sequence.
				VectorCopy(trace.normal, axis[2]);
				AngleVectors(ent->r.currentAngles, fwd, NULL, NULL);
				Vec3Cross(axis[2], fwd, axis[1]);
				Vec3Cross(axis[1], axis[2], axis[0]);
				AxisToAngles(axis, angles);
				G_SetAngle(ent, angles);
			}
			G_SetOrigin(ent, lerpOrigin);
			// (zk emits a "land" script notify here; omitted - see file header)
			ent->s.groundEntityNum = trace.entityNum;
			SV_LinkEntity(ent);
		}
	}
}

qboolean G_BounceGravityModel(gentity_t *ent, trace_t *trace)
{
	int contents;
	float length;
	qboolean bounce;
	vec3_t angle;
	vec3_t planeNormal;
	vec3_t velocity;
	float dot;

	contents = SV_PointContents(ent->r.currentOrigin, -1, CONTENTS_WATER);
	BG_EvaluateTrajectoryDelta(&ent->s.pos, 50 + (int)((float)(level.time - level.previousTime) * trace->fraction) + level.previousTime, velocity);
	dot = DotProduct(velocity, trace->normal);
	VectorMA(velocity, dot * -2.0f, trace->normal, ent->s.pos.trDelta);
	if ( 0.7f < trace->normal[2] )
		ent->s.groundEntityNum = trace->entityNum;

	if ( ( ent->s.eFlags & EF_BOUNCE ) != 0 )
	{
		length = VectorLength(velocity);
		if ( 0.0f < length && ( dot <= 0.0f ) )
		{
			VectorScale(ent->s.pos.trDelta, ( customEntityState[ent->s.number].perpendicularBounce - customEntityState[ent->s.number].parallelBounce ) * ( dot / -length ) + customEntityState[ent->s.number].parallelBounce, ent->s.pos.trDelta);
			if ( customEntityState[ent->s.number].maxVelocity > 0.0f )
				zk_VectorClampLength(ent->s.pos.trDelta, customEntityState[ent->s.number].maxVelocity);
		}

		if ( 0.7f < trace->normal[2] && VectorLength(ent->s.pos.trDelta) < 20.0f )
		{
			// (zk emits a "land" script notify here; omitted - see file header)
			G_SetOrigin(ent, ent->r.currentOrigin);
			G_MissileLandAngles(ent, trace, angle, 1);
			G_SetAngle(ent, angle);
			return qfalse;
		}
	}
	VectorScale(trace->normal, 0.1f, planeNormal);
	if ( 0.0f < planeNormal[2] )
		planeNormal[2] = 0.0f;
	VectorAdd(ent->r.currentOrigin, planeNormal, ent->r.currentOrigin);
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
	ent->s.pos.trTime = level.time;
	G_MissileLandAngles(ent, trace, angle, 0);
	VectorCopy(angle, ent->s.apos.trBase);
	ent->s.apos.trTime = level.time;
	if ( contents == 0 )
	{
		VectorSubtract(ent->s.pos.trDelta, velocity, velocity);
		bounce = ( 100.0f < VectorLength(velocity) ) ? qtrue : qfalse;
	}
	else
	{
		bounce = qfalse;
	}
	return bounce;
}

void G_RunGravityModelWithBounce(gentity_t *ent)
{
	vec3_t lerpOrigin;
	trace_t trace2;
	trace_t trace;
	vec3_t origin;
	vec3_t maxLerpVector;
	qboolean bounce;

	if ( ( ent->s.pos.trType == TR_STATIONARY ) && ( ent->s.groundEntityNum != ENTITYNUM_WORLD ) )
	{
		VectorCopy(ent->r.currentOrigin, origin);
		origin[2] = origin[2] - 1.5f;
		G_MissileTrace(&trace, ent->r.currentOrigin, origin, ent->s.number, ent->clipmask);
		if ( trace.fraction == 1.0f )
		{
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
			ent->s.pos.trDuration = 0;
			VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
			VectorClear(ent->s.pos.trDelta);
		}
	}
	BG_EvaluateTrajectory(&ent->s.pos, level.time + 50, origin);

	if ( customEntityState[ent->s.number].maxVelocity > 0.0f )
	{
		VectorSubtract(origin, ent->r.currentOrigin, maxLerpVector);
		zk_VectorClampLength(maxLerpVector, customEntityState[ent->s.number].maxVelocity / (float)sv_fps->current.integer);
		VectorAdd(ent->r.currentOrigin, maxLerpVector, origin);
	}

	if ( I_fabs(ent->s.pos.trDelta[2]) <= 30 || SV_PointContents(ent->r.currentOrigin, -1, CONTENTS_WATER) )
		G_MissileTrace(&trace, ent->r.currentOrigin, origin, ent->s.number, ent->clipmask);
	else
		G_MissileTrace(&trace, ent->r.currentOrigin, origin, ent->s.number, ent->clipmask | CONTENTS_WATER);

	if ( ( trace.surfaceFlags & 0x1F00000 ) == SURF_WATER )
		G_MissileTrace(&trace, ent->r.currentOrigin, origin, ent->s.number, ent->clipmask);

	// (zk handles EF_TAGCONNECT here via Missile_TraceNoContents; omitted - see file header)

	Vec3Lerp(ent->r.currentOrigin, origin, trace.fraction, lerpOrigin);
	VectorCopy(lerpOrigin, ent->r.currentOrigin);

	if ( ( ( ent->s.eFlags & EF_BOUNCE ) != 0 ) && ( trace.fraction == 1.0f || ( trace.fraction < 1.0f && ( 0.7f < trace.normal[2] ) ) ) )
	{
		VectorCopy(ent->r.currentOrigin, origin);
		origin[2] = origin[2] - 1.5f;
		G_MissileTrace(&trace2, ent->r.currentOrigin, origin, ent->s.number, ent->clipmask);
		if ( ( trace2.fraction != 1.0f ) && ( trace2.entityNum == ENTITYNUM_WORLD ) )
		{
			trace.fraction = trace2.fraction;
			trace.normal[0] = trace2.normal[0];
			trace.normal[1] = trace2.normal[1];
			trace.normal[2] = trace2.normal[2];
			trace.surfaceFlags = trace2.surfaceFlags;
			trace.contents = trace2.contents;
			trace.material = trace2.material;
			trace.entityNum = trace2.entityNum;
			trace.partName = trace2.partName;
			trace.partGroup = trace2.partGroup;
			trace.allsolid = trace2.allsolid;
			trace.startsolid = trace2.startsolid;
			Vec3Lerp(ent->r.currentOrigin, origin, trace2.fraction, lerpOrigin);
			ent->s.pos.trBase[2] = ent->s.pos.trBase[2] + ( ( lerpOrigin[2] + 1.5f ) - ent->r.currentOrigin[2] );
			VectorCopy(lerpOrigin, ent->r.currentOrigin);
			ent->r.currentOrigin[2] = ent->r.currentOrigin[2] + 1.5f;
		}
	}
	SV_LinkEntity(ent);
	if ( trace.fraction != 1.0f )
	{
		bounce = G_BounceGravityModel(ent, &trace);
		(void)bounce;
		// (zk emits a "bounce" script notify here; omitted - see file header)
	}
}

// --- dispatch entry, called from G_RunFrameForEntity ---

qboolean zk_EntityHasGravity(int entnum)
{
	if ( entnum < 0 || entnum >= MAX_GENTITIES )
		return qfalse;
	return customEntityState[entnum].gravityType != 0;
}

void zk_RunEntityGravity(gentity_t *ent)
{
	vec3_t oldOrigin;
	int num = ent->s.number;

	VectorCopy(ent->r.currentOrigin, oldOrigin);

	if ( customEntityState[num].gravityType == GRAVITY_NO_BOUNCE )
		G_RunGravityModelNoBounce(ent);
	else if ( customEntityState[num].gravityType == GRAVITY_BOUNCE )
		G_RunGravityModelWithBounce(ent);

	VectorSubtract(ent->r.currentOrigin, oldOrigin, customEntityState[num].velocity);
	VectorScale(customEntityState[num].velocity, 20.0f, customEntityState[num].velocity);
}

#endif
