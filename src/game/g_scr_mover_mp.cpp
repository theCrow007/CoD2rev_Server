#include "../qcommon/qcommon.h"
#include "g_shared.h"
#include "g_scr_mover_times.h"

#define X 0
#define Y 1
#define Z 2

scr_method_t scriptent_methods[] =
{
	{ "moveto", ScriptEntCmd_MoveTo, qfalse, },
	{ "movex", ScriptEntCmd_MoveX, qfalse, },
	{ "movey", ScriptEntCmd_MoveY, qfalse, },
	{ "movez", ScriptEntCmd_MoveZ, qfalse, },
	{ "movegravity", ScriptEntCmd_GravityMove, qfalse, },
	{ "rotateto", ScriptEntCmd_RotateTo, qfalse, },
	{ "rotatepitch", ScriptEntCmd_RotatePitch, qfalse, },
	{ "rotateyaw", ScriptEntCmd_RotateYaw, qfalse, },
	{ "rotateroll", ScriptEntCmd_RotateRoll, qfalse, },
	{ "rotatevelocity", ScriptEntCmd_RotateVelocity, qfalse, },
	{ "solid", ScriptEntCmd_Solid, qfalse, },
	{ "notsolid", ScriptEntCmd_NotSolid, qfalse, },
};

/*
===============
ScriptEnt_GetMethod
===============
*/
void (*ScriptEnt_GetMethod( const char **pName ))( scr_entref_t )
{
	for ( int i = 0; i < ARRAY_COUNT(scriptent_methods); i++ )
	{
		if ( !strcmp(*pName, scriptent_methods[i].name) )
		{
			*pName = scriptent_methods[i].name;
			return scriptent_methods[i].call;
		}
	}

	return NULL;
}

/*
===============
ScriptEntCmdGetCommandTimes
===============
*/
void ScriptEntCmdGetCommandTimes( float *pfTotalTime, float *pfAccelTime, float *pfDecelTime )
{
	*pfTotalTime = Scr_GetFloat(1);

	if ( *pfTotalTime <= 0 )
	{
		Scr_ParamError(1, "total time must be positive");
	}

	int iNumParms = Scr_GetNumParam();

	if ( iNumParms >= 3 )
	{
		*pfAccelTime = Scr_GetFloat(2);

		if ( *pfAccelTime < 0 )
		{
			Scr_ParamError(2, "accel time must be nonnegative");
		}

		if ( iNumParms >= 4 )
		{
			*pfDecelTime = Scr_GetFloat(3);

			if ( *pfDecelTime < 0 )
			{
				Scr_ParamError(3, "decel time must be nonnegative");
			}
		}
		else
		{
			*pfDecelTime = 0;
		}
	}
	else
	{
		*pfAccelTime = 0;
		*pfDecelTime = 0;
	}

	if ( *pfAccelTime + *pfDecelTime > *pfTotalTime )
	{
		Scr_Error("accel time plus decel time is greater than total time");
	}
}

/*
===============
InitScriptMover
===============
*/
void InitScriptMover( gentity_t *pSelf )
{
	if ( level.spawnVar.spawnVarsValid )
	{
		float light;
		vec3_t color;
		qboolean lightSet, colorSet;

		// if the "color" or "light" keys are set, setup constantLight
		lightSet = G_SpawnFloat( "light", "100", &light );
		colorSet = G_SpawnVector( "color", "1 1 1", color );
		if ( lightSet || colorSet )
		{
			int r, g, b, i;

			r = color[0] * 255;
			if ( r > 255 )
			{
				r = 255;
			}
			g = color[1] * 255;
			if ( g > 255 )
			{
				g = 255;
			}
			b = color[2] * 255;
			if ( b > 255 )
			{
				b = 255;
			}
			i = light / 4;
			if ( i > 255 )
			{
				i = 255;
			}
			pSelf->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
		}
	}

	pSelf->handler = ENT_HANDLER_SCRIPT_MOVER;
	pSelf->r.svFlags = 0;
	pSelf->s.eType = ET_SCRIPTMOVER;

	VectorCopy(pSelf->r.currentOrigin, pSelf->s.pos.trBase);
	pSelf->s.pos.trType = TR_STATIONARY;

	VectorCopy(pSelf->r.currentAngles, pSelf->s.apos.trBase);
	pSelf->s.apos.trType = TR_STATIONARY;

	pSelf->flags |= FL_SUPPORTS_LINKTO;
}

/*
===============
ScriptEntCmd_NotSolid
===============
*/
void ScriptEntCmd_NotSolid( scr_entref_t entref )
{
	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
		return;
	}

	if ( pSelf->classname == scr_const.script_origin )
	{
		Com_DPrintf("cannot use the solid/notsolid commands on a script_origin entity( number %i )\n", pSelf->s.number);
		return;
	}

	pSelf->r.contents = 0;

	if ( pSelf->classname != scr_const.script_model )
	{
		pSelf->s.eFlags |= EF_NONSOLID_BMODEL;
	}

	SV_LinkEntity(pSelf);
}

/*
===============
ScriptEntCmd_Solid
===============
*/
void ScriptEntCmd_Solid( scr_entref_t entref )
{
	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
		return;
	}

	if ( pSelf->classname == scr_const.script_origin )
	{
		Com_DPrintf("cannot use the solid/notsolid commands on a script_origin entity( number %i )\n", pSelf->s.number);
		return;
	}

	if ( pSelf->classname == scr_const.script_model )
	{
		pSelf->r.contents = CONTENTS_MISSILECLIP | CONTENTS_CLIPSHOT;
	}
	else
	{
		pSelf->r.contents = CONTENTS_SOLID;
		pSelf->s.eFlags &= ~EF_NONSOLID_BMODEL;
	}

	SV_LinkEntity(pSelf);
}

/*
===============
SP_script_origin
===============
*/
void SP_script_origin( gentity_t *pSelf )
{
	InitScriptMover(pSelf);
	pSelf->r.contents = 0;
	SV_LinkEntity(pSelf);

	if ( pSelf->s.constantLight )
	{
		pSelf->s.eFlags |= EF_NODRAW;
		return;
	}

	pSelf->r.svFlags |= SVF_NOCLIENT;
}

/*
===============
SP_script_brushmodel
===============
*/
void SP_script_brushmodel( gentity_t *pSelf )
{
	SV_SetBrushModel(pSelf);
	InitScriptMover(pSelf);
	pSelf->r.contents = CONTENTS_SOLID;

	SV_LinkEntity(pSelf);
}

/*
===============
ScriptEntCmd_RotateVelocity
===============
*/
void ScriptEntCmd_RotateVelocity( scr_entref_t entref )
{
	vec3_t vSpeed;
	float fDecelTime;
	float fAccelTime;
	float fTotalTime;

	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
		return;
	}

	Scr_GetVector(0, vSpeed);

	ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
	ScriptMover_RotateSpeed(pSelf, vSpeed, fTotalTime, fAccelTime, fDecelTime);
}

/*
===============
ScriptEnt_RotateAxis
===============
*/
void ScriptEnt_RotateAxis( scr_entref_t entref, int iAxis )
{
	vec3_t vRot;
	float fDecelTime;
	float fAccelTime;
	float fTotalTime;
	float fMove;

	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
		return;
	}

	fMove = Scr_GetFloat(0);

	ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
	VectorCopy(pSelf->r.currentAngles, vRot);

	vRot[iAxis] += fMove;
	ScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

/*
===============
ScriptEntCmd_RotateTo
===============
*/
void ScriptEntCmd_RotateTo( scr_entref_t entref )
{
	vec3_t vRot;
	vec3_t vDest;
	float fDecelTime;
	float fAccelTime;
	float fTotalTime;

	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
		return;
	}

	Scr_GetVector(0, vDest);

	ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);

	for ( int i = 0; i < 3; i++ )
	{
		vRot[i] = AngleSubtract(vDest[i], pSelf->r.currentAngles[i]) + pSelf->r.currentAngles[i];
	}

	ScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

/*
===============
ScriptEnt_MoveAxis
===============
*/
void ScriptEnt_MoveAxis( scr_entref_t entref, int iAxis )
{
	vec3_t vPos;
	float fDecelTime;
	float fAccelTime;
	float fTotalTime;
	float fMove;

	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
		return;
	}

	fMove = Scr_GetFloat(0);

	ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
	VectorCopy(pSelf->r.currentOrigin, vPos);

	vPos[iAxis] += fMove;
	ScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

/*
===============
ScriptEntCmd_GravityMove
===============
*/
void ScriptEntCmd_GravityMove( scr_entref_t entref )
{
	vec3_t velocity;

	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
	}

	Scr_GetVector(0, velocity);
	ScriptMover_GravityMove(pSelf, velocity, Scr_GetFloat(1));
}

/*
===============
ScriptEntCmd_MoveTo
===============
*/
void ScriptEntCmd_MoveTo( scr_entref_t entref )
{
	vec3_t vPos;
	float fDecelTime;
	float fAccelTime;
	float fTotalTime;

	if ( entref.classnum != CLASS_NUM_ENTITY )
	{
		Scr_ObjectError("not an entity");
		return;
	}

	assert(entref.entnum < MAX_GENTITIES);
	gentity_t *pSelf = &g_entities[entref.entnum];

	if ( pSelf->classname != scr_const.script_brushmodel && pSelf->classname != scr_const.script_model && pSelf->classname != scr_const.script_origin )
	{
		Scr_ObjectError(va("entity %i is not a script_brushmodel, script_model, or script_origin", entref.entnum));
	}

	Scr_GetVector(0, vPos);

	ScriptEntCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
	ScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

/*
===============
ScriptEntCmd_RotateRoll
===============
*/
void ScriptEntCmd_RotateRoll( scr_entref_t entref )
{
	ScriptEnt_RotateAxis(entref, ROLL);
}

/*
===============
ScriptEntCmd_RotateYaw
===============
*/
void ScriptEntCmd_RotateYaw( scr_entref_t entref )
{
	ScriptEnt_RotateAxis(entref, YAW);
}

/*
===============
ScriptEntCmd_RotatePitch
===============
*/
void ScriptEntCmd_RotatePitch( scr_entref_t entref )
{
	ScriptEnt_RotateAxis(entref, PITCH);
}

/*
===============
ScriptEntCmd_MoveZ
===============
*/
void ScriptEntCmd_MoveZ( scr_entref_t entref )
{
	ScriptEnt_MoveAxis(entref, Z);
}

/*
===============
ScriptEntCmd_MoveY
===============
*/
void ScriptEntCmd_MoveY( scr_entref_t entref )
{
	ScriptEnt_MoveAxis(entref, Y);
}

/*
===============
ScriptEntCmd_MoveX
===============
*/
void ScriptEntCmd_MoveX( scr_entref_t entref )
{
	ScriptEnt_MoveAxis(entref, X);
}

/*
===============
SP_script_model
===============
*/
void SP_script_model( gentity_t *pSelf )
{
	G_DObjUpdate(pSelf);
	InitScriptMover(pSelf);

	pSelf->r.svFlags |= SVF_MODEL;
	pSelf->r.contents = CONTENTS_MISSILECLIP | CONTENTS_CLIPSHOT;

	SV_LinkEntity(pSelf);
}

/*
===============
Reached_ScriptMover
===============
*/
void Reached_ScriptMover( gentity_t *pEnt )
{
	qboolean bMoveFinished;

	if ( pEnt->s.pos.trType != TR_STATIONARY && pEnt->s.pos.trTime + pEnt->s.pos.trDuration <= level.time )
	{
		bMoveFinished = ScriptMover_UpdateMove(
		                    &pEnt->s.pos,
		                    pEnt->r.currentOrigin,
		                    pEnt->mover.speed,
		                    pEnt->mover.midTime,
		                    pEnt->mover.decelTime,
		                    pEnt->mover.pos1,
		                    pEnt->mover.pos2,
		                    pEnt->mover.pos3);

		BG_EvaluateTrajectory(&pEnt->s.pos, level.time, pEnt->r.currentOrigin);
		SV_LinkEntity(pEnt);

		if ( bMoveFinished )
		{
			Scr_Notify(pEnt, scr_const.movedone, 0);
		}
	}

	if ( pEnt->s.apos.trType != TR_STATIONARY && pEnt->s.apos.trTime + pEnt->s.apos.trDuration <= level.time )
	{
		bMoveFinished = ScriptMover_UpdateMove(
		                    &pEnt->s.apos,
		                    pEnt->r.currentAngles,
		                    pEnt->mover.aSpeed,
		                    pEnt->mover.aMidTime,
		                    pEnt->mover.aDecelTime,
		                    pEnt->mover.apos1,
		                    pEnt->mover.apos2,
		                    pEnt->mover.apos3);

		BG_EvaluateTrajectory(&pEnt->s.apos, level.time, pEnt->r.currentAngles);
		SV_LinkEntity(pEnt);

		if ( bMoveFinished )
		{
			pEnt->r.currentAngles[0] = AngleNormalize180(pEnt->r.currentAngles[0]);
			pEnt->r.currentAngles[1] = AngleNormalize360(pEnt->r.currentAngles[1]);
			pEnt->r.currentAngles[2] = AngleNormalize180(pEnt->r.currentAngles[2]);

			Scr_Notify(pEnt, scr_const.rotatedone, 0);
		}
	}
}

/*
===============
ScriptMover_SetupMoveSpeed
===============
*/
void ScriptMover_SetupMoveSpeed( trajectory_t *pTr, const float *vSpeed, float fTotalTime, float fAccelTime, float fDecelTime,
                                 vec3_t vCurrPos, float *pfSpeed, float *pfMidTime, float *pfDecelTime,
                                 vec3_t vPos1, vec3_t vPos2, vec3_t vPos3 )
{
	trajectory_t tr;

	if ( pTr->trType != TR_STATIONARY )
	{
		BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
	}

	if ( fAccelTime == 0 && fDecelTime == 0 )
	{
		pTr->trTime = level.time;
		pTr->trDuration = ScriptMover_TimeToMsec(fTotalTime);

		*pfMidTime = fTotalTime;
		*pfDecelTime = 0;

		VectorCopy(vCurrPos, pTr->trBase);
		VectorCopy(vSpeed, pTr->trDelta);

		assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
		pTr->trType = TR_LINEAR_STOP;

		BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
		BG_EvaluateTrajectory(pTr, level.time + pTr->trDuration, vPos3);

		return;
	}

	*pfMidTime = fTotalTime - fAccelTime - fDecelTime;
	*pfDecelTime = fDecelTime;
	*pfSpeed = VectorLength(vSpeed);

	if ( fAccelTime == 0 )
	{
		VectorCopy(vCurrPos, vPos1);

		if ( *pfMidTime == 0 )
		{
			pTr->trTime = level.time;
			pTr->trDuration = ScriptMover_TimeToMsec(*pfDecelTime);

			VectorCopy(vCurrPos, pTr->trBase);
			VectorCopy(vSpeed, pTr->trDelta);

			assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
			pTr->trType = TR_DECCELERATE;
		}
		else
		{
			pTr->trTime = level.time;
			pTr->trDuration = ScriptMover_TimeToMsec(*pfMidTime);

			VectorCopy(vCurrPos, pTr->trBase);
			VectorCopy(vSpeed, pTr->trDelta);

			assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
			pTr->trType = TR_LINEAR_STOP;
		}
	}
	else
	{
		pTr->trTime = level.time;
		pTr->trDuration = ScriptMover_TimeToMsec(fAccelTime);

		VectorCopy(vCurrPos, pTr->trBase);
		VectorCopy(vSpeed, pTr->trDelta);

		assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
		pTr->trType = TR_ACCELERATE;

		BG_EvaluateTrajectory(pTr, level.time + pTr->trDuration, vPos1);
	}

	VectorMA(vPos1, *pfMidTime, vSpeed, vPos2);

	if ( *pfDecelTime == 0 )
	{
		VectorCopy(vPos2, vPos3);
	}
	else
	{
		tr.trType = TR_DECCELERATE;
		tr.trTime = level.time;

		tr.trDuration = ScriptMover_TimeToMsec(*pfDecelTime);

		VectorCopy(vPos2, tr.trBase);
		VectorCopy(vSpeed, tr.trDelta);

		assert(!IS_NAN((tr.trDelta)[0]) && !IS_NAN((tr.trDelta)[1]) && !IS_NAN((tr.trDelta)[2]));

		BG_EvaluateTrajectory(&tr, level.time + tr.trDuration, vPos3);
	}

	BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
}

/*
===============
ScriptMover_SetupMove
===============
*/
void ScriptMover_SetupMove( trajectory_t *pTr, const vec3_t vPos, float fTotalTime, float fAccelTime, float fDecelTime,
                            vec3_t vCurrPos, float *pfSpeed, float *pfMidTime, float *pfDecelTime,
                            vec3_t vPos1, vec3_t vPos2, vec3_t vPos3 )
{
	vec3_t vMaxSpeed;
	vec3_t vMove;

	VectorSubtract(vPos, vCurrPos, vMove);

	if ( pTr->trType != TR_STATIONARY )
	{
		BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
	}

	if ( fAccelTime == 0 && fDecelTime == 0 )
	{
		pTr->trTime = level.time;
		pTr->trDuration = ScriptMover_TimeToMsec(fTotalTime);

		*pfMidTime = fTotalTime;
		*pfDecelTime = 0;

		VectorCopy(vPos, vPos3);
		VectorCopy(vCurrPos, pTr->trBase);

		assert(pTr->trDuration);
		VectorScale(vMove, 1000.0f / pTr->trDuration, pTr->trDelta);
		assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
		pTr->trType = TR_LINEAR_STOP;

		BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
		return;
	}

	*pfMidTime = fTotalTime - fAccelTime - fDecelTime;
	*pfDecelTime = fDecelTime;

	float fDist = VectorLength(vMove);
	assert((2.0f * fTotalTime) - fAccelTime - fDecelTime);
	*pfSpeed = (fDist + fDist) / (fTotalTime + fTotalTime - fAccelTime - fDecelTime);

	Vec3NormalizeTo(vMove, vMaxSpeed);
	VectorScale(vMaxSpeed, *pfSpeed, vMaxSpeed);

	if ( fAccelTime == 0 )
	{
		VectorCopy(vCurrPos, vPos1);

		if ( *pfMidTime == 0 )
		{
			pTr->trTime = level.time;
			pTr->trDuration = ScriptMover_TimeToMsec(*pfDecelTime);

			VectorCopy(vCurrPos, pTr->trBase);
			VectorCopy(vMaxSpeed, pTr->trDelta);

			assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
			pTr->trType = TR_DECCELERATE;
		}
		else
		{
			pTr->trTime = level.time;
			pTr->trDuration = ScriptMover_TimeToMsec(*pfMidTime);

			VectorCopy(vCurrPos, pTr->trBase);
			VectorScale(vMaxSpeed, *pfMidTime, vMove);

			assert(pTr->trDuration);
			VectorScale(vMove, 1000.0f / pTr->trDuration, pTr->trDelta);
			assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
			pTr->trType = TR_LINEAR_STOP;
		}
	}
	else
	{
		pTr->trTime = level.time;
		pTr->trDuration = ScriptMover_TimeToMsec(fAccelTime);

		VectorCopy(vCurrPos, pTr->trBase);
		VectorCopy(vMaxSpeed, pTr->trDelta);

		assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
		pTr->trType = TR_ACCELERATE;

		BG_EvaluateTrajectory(pTr, level.time + pTr->trDuration, vPos1);
	}

	VectorMA(vPos1, *pfMidTime, vMaxSpeed, vPos2);
	VectorCopy(vPos, vPos3);

	BG_EvaluateTrajectory(pTr, level.time, vCurrPos);
}

/*
===============
ScriptMover_UpdateMove
===============
*/
qboolean ScriptMover_UpdateMove( trajectory_t *pTr, vec3_t vCurrPos, float fSpeed, float fMidTime, float fDecelTime,
                                 const vec3_t vPos1, const vec3_t vPos2, const vec3_t vPos3 )
{
	vec3_t vMove;
	int trDuration = ScriptMover_TimeToMsec(fMidTime);

	if ( pTr->trType == TR_ACCELERATE && trDuration > 0 )
	{
		pTr->trTime = level.time;
		pTr->trDuration = trDuration;

		VectorCopy(vPos1, pTr->trBase);
		VectorSubtract(vPos2, vPos1, vMove);

		assert(trDuration);
		VectorScale(vMove, 1000.0f / trDuration, pTr->trDelta);

		assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
		pTr->trType = TR_LINEAR_STOP;

		return qfalse;
	}

	if ( (pTr->trType == TR_ACCELERATE && trDuration <= 0 || pTr->trType == TR_LINEAR_STOP) && fDecelTime > 0 )
	{
		pTr->trTime = level.time;
		pTr->trDuration = ScriptMover_TimeToMsec(fDecelTime);

		VectorCopy(vPos2, pTr->trBase);
		VectorSubtract(vPos3, vPos2, vMove);

		Vec3Normalize(vMove);

		VectorScale(vMove, fSpeed, vMove);
		VectorCopy(vMove, pTr->trDelta);

		assert(!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2]));
		pTr->trType = TR_DECCELERATE;

		return qfalse;
	}

	if ( pTr->trType == TR_GRAVITY )
		BG_EvaluateTrajectory(pTr, level.time, pTr->trBase);
	else
		VectorCopy(vPos3, pTr->trBase);

	pTr->trTime = level.time;
	pTr->trType = TR_STATIONARY;

	return qtrue;
}

/*
===============
ScriptMover_RotateSpeed
===============
*/
void ScriptMover_RotateSpeed( gentity_t *pEnt, const float *vRotSpeed, float fTotalTime, float fAccelTime, float fDecelTime )
{
	ScriptMover_SetupMoveSpeed(
	    &pEnt->s.apos,
	    vRotSpeed,
	    fTotalTime,
	    fAccelTime,
	    fDecelTime,
	    pEnt->r.currentAngles,
	    &pEnt->mover.aSpeed,
	    &pEnt->mover.aMidTime,
	    &pEnt->mover.aDecelTime,
	    pEnt->mover.apos1,
	    pEnt->mover.apos2,
	    pEnt->mover.apos3);

	SV_LinkEntity(pEnt);
}

/*
===============
ScriptMover_Rotate
===============
*/
void ScriptMover_Rotate( gentity_t *pEnt, const vec3_t vRot, float fTotalTime, float fAccelTime, float fDecelTime )
{
	ScriptMover_SetupMove(
	    &pEnt->s.apos,
	    vRot,
	    fTotalTime,
	    fAccelTime,
	    fDecelTime,
	    pEnt->r.currentAngles,
	    &pEnt->mover.aSpeed,
	    &pEnt->mover.aMidTime,
	    &pEnt->mover.aDecelTime,
	    pEnt->mover.apos1,
	    pEnt->mover.apos2,
	    pEnt->mover.apos3);

	SV_LinkEntity(pEnt);
}

/*
===============
ScriptMover_GravityMove
===============
*/
void ScriptMover_GravityMove( gentity_t *mover, const vec3_t velocity, float totalTime )
{
	assert(mover);
	assert(!IS_NAN((velocity)[0]) && !IS_NAN((velocity)[1]) && !IS_NAN((velocity)[2]));

	mover->s.pos.trTime = level.time;
	mover->s.pos.trDuration = ScriptMover_TimeToMsec(totalTime);

	VectorCopy(mover->r.currentOrigin, mover->s.pos.trBase);
	VectorCopy(velocity, mover->s.pos.trDelta);

	assert(!IS_NAN((mover->s.pos.trDelta)[0]) && !IS_NAN((mover->s.pos.trDelta)[1]) && !IS_NAN((mover->s.pos.trDelta)[2]));
	mover->s.pos.trType = TR_GRAVITY;

	BG_EvaluateTrajectory(&mover->s.pos, level.time, mover->r.currentOrigin);
	SV_LinkEntity(mover);
}

/*
===============
ScriptMover_Move
===============
*/
void ScriptMover_Move( gentity_s *pEnt, const vec3_t vPos, float fTotalTime, float fAccelTime, float fDecelTime )
{
	ScriptMover_SetupMove(
	    &pEnt->s.pos,
	    vPos,
	    fTotalTime,
	    fAccelTime,
	    fDecelTime,
	    pEnt->r.currentOrigin,
	    &pEnt->mover.speed,
	    &pEnt->mover.midTime,
	    &pEnt->mover.decelTime,
	    pEnt->mover.pos1,
	    pEnt->mover.pos2,
	    pEnt->mover.pos3);

	SV_LinkEntity(pEnt);
}
