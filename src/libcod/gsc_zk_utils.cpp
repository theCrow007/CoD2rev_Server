#include "gsc_zk_utils.hpp"

#if LIBCOD_COMPILE_UTILS == 1

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <strings.h>

/* Ported from zk_libcod gsc_utils.cpp. Logic unchanged; relies on rev's
 * stackGetParams / stackError / stackPush* helpers declared in gsc.hpp. */

void gsc_zk_utils_abs()
{
	float val;

	if ( !stackGetParams("f", &val) )
	{
		stackError("gsc_zk_utils_abs() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushFloat(fabs(val));
}

void gsc_zk_utils_atan2()
{
	float y;
	float x;

	if ( !stackGetParams("ff", &y, &x) )
	{
		stackError("gsc_zk_utils_atan2() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushFloat(atan2f(y, x));
}

void gsc_zk_utils_ceil()
{
	float val;

	if ( !stackGetParams("f", &val) )
	{
		stackError("gsc_zk_utils_ceil() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushFloat(ceilf(val));
}

void gsc_zk_utils_floor()
{
	float val;

	if ( !stackGetParams("f", &val) )
	{
		stackError("gsc_zk_utils_floor() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushFloat(floorf(val));
}

void gsc_zk_utils_chr()
{
	int input;

	if ( !stackGetParams("i", &input) )
	{
		stackError("gsc_zk_utils_chr() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	char output[2];
	if ( input == 0x25 )
	{
		output[0] = '.';
		output[1] = '\0';
	}
	else
	{
		Com_sprintf(output, 2, "%c", input & 0xFF);
	}

	stackPushString(output);
}

void gsc_zk_utils_ord()
{
	char *input;

	if ( !stackGetParams("s", &input) )
	{
		stackError("gsc_zk_utils_ord() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( strlen(input) != 1 )
	{
		stackError("gsc_zk_utils_ord() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushInt((unsigned char)*input);
}

void gsc_zk_utils_tohex()
{
	int input;

	if ( !stackGetParams("i", &input) )
	{
		stackError("gsc_zk_utils_tohex() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	char output[11];
	Com_sprintf(output, 11, "0x%X", input);

	stackPushString(output);
}

void gsc_zk_utils_fromhex()
{
	char *input;

	if ( !stackGetParams("s", &input) )
	{
		stackError("gsc_zk_utils_fromhex() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	int output;

	if ( sscanf(input, "%X", &output) != 1 )
	{
		stackError("gsc_zk_utils_fromhex() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushInt(output);
}

void gsc_zk_utils_roundto()
{
	float val;
	float precision;

	if ( !stackGetParams("ff", &val, &precision) )
	{
		stackError("gsc_zk_utils_roundto() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushFloat(roundf(val * precision) / precision);
}

/* ---- batch 2 ---- */

void gsc_zk_utils_error()
{
	int terminal;
	const char *message;

	message = Scr_GetString(0);
	terminal = 0;
	if ( Scr_GetNumParam() > 1 )
		terminal = Scr_GetInt(1);

	if ( !terminal )
		Scr_Error(message);
	else
		Com_Error(ERR_SCRIPT_DROP, "\x15%s\n", message);
}

void gsc_zk_utils_executecommand()
{
	char *str;

	if ( !stackGetParams("s", &str) )
	{
		stackError("gsc_zk_utils_executecommand() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	Cmd_ExecuteString(str);
	stackPushBool(qtrue);
}

void gsc_zk_utils_getmilliseconds()
{
	stackPushInt(Sys_Milliseconds() & 0x7FFFFFFF);
}

void gsc_zk_utils_getmicroseconds()
{
	struct timeval tv;
	unsigned long microseconds;

	gettimeofday(&tv, NULL);
	microseconds = 1000000 * tv.tv_sec + tv.tv_usec;
	stackPushInt(int(microseconds) & 0x7FFFFFFF);
}

void gsc_zk_utils_getcvarflags()
{
	const char *dvarName;
	dvar_t *dvar;

	dvarName = Scr_GetString(0);
	dvar = Dvar_FindVar(dvarName);
	if ( !dvar )
	{
		stackError("gsc_zk_utils_getcvarflags() dvar '%s' does not exist", dvarName);
		stackPushUndefined();
		return;
	}

	stackPushInt(dvar->flags);
}

void gsc_zk_utils_getsystemtime()
{
	time_t timer;
	stackPushInt(time(&timer));
}

void gsc_zk_utils_getlocaltime()
{
	time_t timer;
	struct tm *timeinfo;

	time(&timer);
	timeinfo = localtime(&timer);

	const char *timestring = asctime(timeinfo);
	char stripped_time[128];

	strncpy(stripped_time, timestring, sizeof(stripped_time));
	stripped_time[strlen(timestring) - 1] = '\0';

	stackPushString(stripped_time);
}

void gsc_zk_utils_fremove()
{
	char *file;

	if ( !stackGetParams("s", &file) )
	{
		stackError("gsc_zk_utils_fremove() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushInt(remove(file));
}

void gsc_zk_utils_loaddir()
{
	char *path, *dir;

	if ( !stackGetParams("ss", &path, &dir) )
	{
		stackError("gsc_zk_utils_loaddir() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	FS_AddIwdFilesForGameDirectory(path, dir);
	stackPushBool(qtrue);
}

void gsc_zk_utils_logprintconsole()
{
	char *str;

	if ( !stackGetParams("s", &str) )
	{
		stackError("gsc_zk_utils_logprintconsole() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	Com_Printf("%s", str);
	stackPushBool(qtrue);
}

void gsc_zk_utils_getsurfacename()
{
	int index;

	if ( !stackGetParams("i", &index) )
	{
		stackError("gsc_zk_utils_getsurfacename() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	stackPushString(Com_SurfaceTypeToName(index));
}

void gsc_zk_utils_findconfigstringindex()
{
	char *name;
	int min, max;

	if ( !stackGetParams("sii", &name, &min, &max) )
	{
		stackError("gsc_zk_utils_findconfigstringindex() one or more arguments is undefined or has a wrong type");
		return;
	}

	if ( min < 0 || max >= MAX_CONFIGSTRINGS )
	{
		stackError("gsc_zk_utils_findconfigstringindex() configstring index is out of range");
		stackPushUndefined();
		return;
	}

	for ( int i = 1; i < max; i++ )
	{
		const char *curitem = SV_GetConfigstringConst(min + i);

		if ( !*curitem )
			break;

		if ( !strcasecmp(name, curitem) )
		{
			stackPushInt(i + min);
			return;
		}
	}

	stackPushInt(0);
}

void gsc_zk_utils_findconfigstringindexoriginal()
{
	char *name;
	int min, max, create;

	if ( !stackGetParams("siii", &name, &min, &max, &create) )
	{
		stackError("gsc_zk_utils_findconfigstringindexoriginal() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	if ( min < 0 || max >= MAX_CONFIGSTRINGS )
	{
		stackError("gsc_zk_utils_findconfigstringindexoriginal() configstring index is out of range");
		stackPushUndefined();
		return;
	}

	stackPushInt(G_FindConfigstringIndex(name, min, max, create, "G_FindConfigstringIndex() from GSC"));
}

#endif
