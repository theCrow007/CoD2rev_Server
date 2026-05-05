#pragma once

enum scriptMoverTimeError_t
{
	SCRIPT_MOVER_TIME_OK,
	SCRIPT_MOVER_TIME_TOTAL_TOO_SHORT,
	SCRIPT_MOVER_TIME_ACCEL_TOO_SHORT,
	SCRIPT_MOVER_TIME_DECEL_TOO_SHORT,
	SCRIPT_MOVER_TIME_MID_TOO_SHORT
};

inline int ScriptMover_TimeToMsec( float seconds )
{
	const int msec = seconds * 1000.0f;

	if ( seconds > 0.0f && msec <= 0 )
	{
		return 1;
	}

	return msec;
}

inline scriptMoverTimeError_t ScriptMover_CommandTimeError( float totalTime, float accelTime, float decelTime )
{
	if ( ScriptMover_TimeToMsec(totalTime) <= 0 )
	{
		return SCRIPT_MOVER_TIME_TOTAL_TOO_SHORT;
	}

	if ( accelTime > 0.0f && ScriptMover_TimeToMsec(accelTime) <= 0 )
	{
		return SCRIPT_MOVER_TIME_ACCEL_TOO_SHORT;
	}

	if ( decelTime > 0.0f && ScriptMover_TimeToMsec(decelTime) <= 0 )
	{
		return SCRIPT_MOVER_TIME_DECEL_TOO_SHORT;
	}

	const float midTime = totalTime - accelTime - decelTime;
	if ( midTime > 0.0f && ScriptMover_TimeToMsec(midTime) <= 0 )
	{
		return SCRIPT_MOVER_TIME_MID_TOO_SHORT;
	}

	return SCRIPT_MOVER_TIME_OK;
}

inline bool ScriptMover_CommandTimesValid( float totalTime, float accelTime, float decelTime )
{
	return ScriptMover_CommandTimeError(totalTime, accelTime, decelTime) == SCRIPT_MOVER_TIME_OK;
}
