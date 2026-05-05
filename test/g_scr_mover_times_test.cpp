#include <cassert>

#include "../src/game/g_scr_mover_times.h"

int main()
{
	assert(ScriptMover_TimeToMsec(0.001f) == 1);
	assert(ScriptMover_TimeToMsec(0.0005f) == 1);
	assert(ScriptMover_TimeToMsec(0.0f) == 0);

	assert(ScriptMover_CommandTimesValid(0.001f, 0.0f, 0.0f));
	assert(ScriptMover_CommandTimesValid(0.0005f, 0.0f, 0.0f));
	assert(ScriptMover_CommandTimesValid(1.0f, 0.0005f, 0.0f));
	assert(ScriptMover_CommandTimesValid(1.0f, 0.0f, 0.0005f));
	assert(ScriptMover_CommandTimesValid(0.0015f, 0.0f, 0.001f));
	assert(ScriptMover_CommandTimesValid(1.0f, 0.25f, 0.25f));
}
