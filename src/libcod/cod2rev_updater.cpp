#include "cod2rev_updater.hpp"

#ifdef LIBCOD

#include "../qcommon/cmd.h"
#include "../server/server.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef _WIN32
#include <limits.h>
#include <unistd.h>
#endif

#define COD2REV_UPDATE_PRODUCT "CoD2rev_Server"
#define COD2REV_UPDATE_VERSION "1.0"
#define COD2REV_UPDATE_ROLE "server"
#define COD2REV_UPDATE_HOST "master.cod2x.me"
#define COD2REV_UPDATE_PORT 20720

#if defined(__i386__) || defined(_M_IX86)
#define COD2REV_UPDATE_PLATFORM "linux-i386"
#elif defined(__x86_64__) || defined(_M_X64)
#define COD2REV_UPDATE_PLATFORM "linux-x64"
#else
#define COD2REV_UPDATE_PLATFORM "linux"
#endif

bool Cod2revUpdater_IsSafeText(const char *value)
{
	if (!value || !*value)
		return false;

	for (const char *p = value; *p; p++)
	{
		unsigned char c = (unsigned char)*p;

		if (c < 32 || c > 126)
			return false;

		switch (*p)
		{
		case '\'':
		case '"':
		case '\\':
		case '$':
		case '`':
		case ';':
		case '&':
		case '|':
		case '<':
		case '>':
		case '(':
		case ')':
			return false;
		default:
			break;
		}
	}

	return true;
}

bool Cod2revUpdater_IsAllowedUpdateUrl(const char *url)
{
	if (!Cod2revUpdater_IsSafeText(url))
		return false;

	return strncmp(url, "https://", 8) == 0 || strncmp(url, "http://", 7) == 0;
}

bool Cod2revUpdater_IsValidDecimalSize(const char *value)
{
	if (!value || !*value)
		return false;

	if (value[0] == '0')
		return false;

	for (const char *p = value; *p; p++)
	{
		if (!isdigit((unsigned char)*p))
			return false;
	}

	return true;
}

bool Cod2revUpdater_IsValidSha256(const char *value)
{
	if (!value || strlen(value) != 64)
		return false;

	for (const char *p = value; *p; p++)
	{
		if (!isxdigit((unsigned char)*p))
			return false;
	}

	return true;
}

#ifndef COD2REV_UPDATER_TEST

dvar_t *sv_update;
dvar_t *sv_updateRestart;

static netadr_t updater_address;
static bool updater_address_resolved;
static bool updater_request_sent;
static bool updater_restart_queued;
static bool updater_ready_notice_printed;
static int updater_wait_notice_time;

#ifndef _WIN32
static char updater_ready_marker_path[PATH_MAX + 32];
#endif

enum
{
	COD2REV_UPDATE_RESTART_NEVER,
	COD2REV_UPDATE_RESTART_NOW,
	COD2REV_UPDATE_RESTART_WHEN_EMPTY
};

static bool Cod2revUpdater_ResolveServer()
{
	if (updater_address_resolved)
		return true;

	Com_DPrintf("Resolving AutoUpdate Server %s...\n", COD2REV_UPDATE_HOST);

	if (!NET_StringToAdr(COD2REV_UPDATE_HOST, &updater_address))
	{
		Com_Printf("CoD2rev AutoUpdate: failed to resolve %s.\n", COD2REV_UPDATE_HOST);
		return false;
	}

	updater_address.port = BigShort(COD2REV_UPDATE_PORT);
	updater_address_resolved = true;

	Com_DPrintf("CoD2rev AutoUpdate resolved to %s\n", NET_AdrToString(updater_address));
	return true;
}

static bool Cod2revUpdater_SendRequest()
{
	if (!Cod2revUpdater_ResolveServer())
		return false;

	Com_Printf("-----------------------------------\n");
	Com_Printf("CoD2rev AutoUpdate: checking for updates...\n");

	const char *payload = va("getUpdateInfo2 \"%s\" \"%s\" \"%s\" \"%s\"",
	                         COD2REV_UPDATE_PRODUCT,
	                         COD2REV_UPDATE_VERSION,
	                         COD2REV_UPDATE_PLATFORM,
	                         COD2REV_UPDATE_ROLE);

	bool status = NET_OutOfBandPrint(NS_SERVER, updater_address, payload);

	Com_Printf("-----------------------------------\n");
	return status;
}

#ifndef _WIN32
static bool Cod2revUpdater_GetExecutablePath(char *path, size_t path_size)
{
	if (!path || path_size == 0)
		return false;

	ssize_t length = readlink("/proc/self/exe", path, path_size - 1);

	if (length <= 0 || (size_t)length >= path_size)
		return false;

	path[length] = '\0';
	return Cod2revUpdater_IsSafeText(path);
}

static void Cod2revUpdater_DownloadAndSwapAsync(const char *current_version, const char *new_version, const char *url, const char *expected_size, const char *expected_sha256)
{
	char binary_path[PATH_MAX];
	char binary_new[PATH_MAX + 16];
	char binary_old[PATH_MAX + 64];
	char binary_log[PATH_MAX + 32];
	char binary_ready[PATH_MAX + 32];
	char cmd[4096];

	if (!Cod2revUpdater_GetExecutablePath(binary_path, sizeof(binary_path)))
	{
		Com_Printf("CoD2rev AutoUpdate: failed to resolve running binary path.\n");
		return;
	}

	if (!Cod2revUpdater_IsSafeText(current_version) ||
	    !Cod2revUpdater_IsSafeText(new_version) ||
	    !Cod2revUpdater_IsAllowedUpdateUrl(url) ||
	    !Cod2revUpdater_IsValidDecimalSize(expected_size) ||
	    !Cod2revUpdater_IsValidSha256(expected_sha256))
	{
		Com_Printf("CoD2rev AutoUpdate: rejected unsafe update response.\n");
		return;
	}

	Com_sprintf(binary_new, sizeof(binary_new), "%s.new", binary_path);
	Com_sprintf(binary_old, sizeof(binary_old), "%s.%s.old", binary_path, current_version);
	Com_sprintf(binary_log, sizeof(binary_log), "%s.update.log", binary_path);
	Com_sprintf(binary_ready, sizeof(binary_ready), "%s.update.ready", binary_path);

	if (!Cod2revUpdater_IsSafeText(binary_new) ||
	    !Cod2revUpdater_IsSafeText(binary_old) ||
	    !Cod2revUpdater_IsSafeText(binary_log) ||
	    !Cod2revUpdater_IsSafeText(binary_ready))
	{
		Com_Printf("CoD2rev AutoUpdate: binary path contains unsupported shell characters.\n");
		return;
	}

	Q_strncpyz(updater_ready_marker_path, binary_ready, sizeof(updater_ready_marker_path));

	Com_sprintf(cmd, sizeof(cmd),
	            "nohup sh -c 'set -e; { "
	            "echo \"==== Update Started at $(date \"+%%Y-%%m-%%d %%H:%%M:%%S\") ===============================================\"; "
	            "echo \"Current Version: %s\"; "
	            "echo \"New Version:     %s\"; "
	            "echo \"Url:             %s\"; "
	            "echo \"Expected Size:   %s\"; "
	            "echo \"Expected SHA256: %s\"; "
	            "echo \"Destination:     %s\"; "
	            "rm -f \"%s\" \"%s\"; "
	            "curl --connect-timeout 2 --max-time 20 --retry 3 --retry-delay 2 -o \"%s\" -L -s -S -f -w \"Downloaded: %%{size_download} bytes at %%{speed_download} bytes/sec in %%{time_total} seconds\\n\" \"%s\"; "
	            "test -s \"%s\"; "
	            "actual_size=$(wc -c < \"%s\"); "
	            "test \"$actual_size\" = \"%s\"; "
	            "printf \"%%s  %%s\\n\" \"%s\" \"%s\" | sha256sum -c -; "
	            "chmod +x \"%s\"; "
	            "mv -f \"%s\" \"%s\"; "
	            "mv -f \"%s\" \"%s\"; "
	            "printf \"%%s\\n\" \"%s\" > \"%s\"; "
	            "echo \"Update ready. Restart the server to load the new binary.\"; "
	            "echo \"==========================================================================================\"; "
	            "} >> \"%s\" 2>&1' </dev/null >/dev/null 2>&1 &",
	            current_version,
	            new_version,
	            url,
	            expected_size,
	            expected_sha256,
	            binary_new,
	            binary_new,
	            binary_old,
	            binary_new,
	            url,
	            binary_new,
	            binary_new,
	            expected_size,
	            expected_sha256,
	            binary_new,
	            binary_new,
	            binary_path,
	            binary_old,
	            binary_new,
	            binary_path,
	            new_version,
	            binary_ready,
	            binary_log);

	system(cmd);
}

static bool Cod2revUpdater_IsServerEmpty()
{
	if (!svs.clients || !sv_maxclients)
		return true;

	for (int i = 0; i < sv_maxclients->current.integer; i++)
	{
		if (svs.clients[i].state >= CS_CONNECTED)
			return false;
	}

	return true;
}

static void Cod2revUpdater_QueueRestart()
{
	if (updater_restart_queued)
		return;

	updater_restart_queued = true;
	remove(updater_ready_marker_path);

	Com_Printf("CoD2rev AutoUpdate: quitting so the supervisor can start the updated binary.\n");
	Cbuf_AddText("quit\n");
}

static void Cod2revUpdater_CheckRestartPolicy()
{
	if (!updater_ready_marker_path[0] || updater_restart_queued)
		return;

	FILE *marker = fopen(updater_ready_marker_path, "rb");

	if (!marker)
		return;

	fclose(marker);

	if (!updater_ready_notice_printed)
	{
		Com_Printf("CoD2rev AutoUpdate: update is installed on disk. Restart mode is %d.\n",
		           sv_updateRestart ? sv_updateRestart->current.integer : COD2REV_UPDATE_RESTART_NEVER);
		updater_ready_notice_printed = true;
	}

	if (!sv_updateRestart)
		return;

	switch (sv_updateRestart->current.integer)
	{
	case COD2REV_UPDATE_RESTART_NOW:
		Cod2revUpdater_QueueRestart();
		break;
	case COD2REV_UPDATE_RESTART_WHEN_EMPTY:
		if (Cod2revUpdater_IsServerEmpty())
		{
			Cod2revUpdater_QueueRestart();
		}
		else if (Com_Milliseconds() - updater_wait_notice_time > 30000)
		{
			updater_wait_notice_time = Com_Milliseconds();
			Com_Printf("CoD2rev AutoUpdate: waiting for empty server before restart.\n");
		}
		break;
	default:
		break;
	}
}
#endif

static void Cod2revUpdater_DownloadFile(const char *update_file, const char *new_version, const char *expected_size, const char *expected_sha256)
{
#ifdef _WIN32
	Com_Printf("CoD2rev AutoUpdate: server auto-update is only supported on Linux.\n");
#else
	Com_Printf("CoD2rev AutoUpdate: downloading '%s' in background.\n", update_file);
	Com_Printf("CoD2rev AutoUpdate: restart policy is sv_updateRestart %d.\n",
	           sv_updateRestart ? sv_updateRestart->current.integer : COD2REV_UPDATE_RESTART_NEVER);
	Cod2revUpdater_DownloadAndSwapAsync(COD2REV_UPDATE_VERSION, new_version, update_file, expected_size, expected_sha256);
#endif
}

void Cod2revUpdater_RegisterDvars()
{
	sv_update = Dvar_RegisterBool("sv_update", true, DVAR_CHANGEABLE_RESET);
	sv_updateRestart = Dvar_RegisterInt("sv_updateRestart", COD2REV_UPDATE_RESTART_NEVER, COD2REV_UPDATE_RESTART_NEVER, COD2REV_UPDATE_RESTART_WHEN_EMPTY, DVAR_CHANGEABLE_RESET);
}

void Cod2revUpdater_Frame()
{
#ifndef _WIN32
	Cod2revUpdater_CheckRestartPolicy();
#endif

	if (updater_request_sent)
		return;

	if (!sv_update || !sv_update->current.boolean)
		return;

	if (!com_sv_running || !com_sv_running->current.boolean)
		return;

	updater_request_sent = true;
	Cod2revUpdater_SendRequest();
}

void Cod2revUpdater_HandlePacketResponse(netadr_t from)
{
	if (!updater_address_resolved)
	{
		Com_DPrintf("CoD2rev AutoUpdate: ignoring response before request.\n");
		return;
	}

	Com_DPrintf("CoD2rev AutoUpdate response from %s\n", NET_AdrToString(from));

	if (!NET_CompareBaseAdr(updater_address, from))
	{
		Com_DPrintf("CoD2rev AutoUpdate: received update packet from unexpected IP.\n");
		return;
	}

	const char *update_available_text = Cmd_Argv(1);
	int update_available = atoi(update_available_text);

	Com_Printf("-----------------------------------\n");

	if (update_available)
	{
		const char *update_file = Cmd_Argv(2);
		const char *new_version = Cmd_Argv(3);
		const char *expected_size = Cmd_Argv(4);
		const char *expected_sha256 = Cmd_Argv(5);

		Com_Printf("CoD2rev AutoUpdate: update available: %s -> %s\n", COD2REV_UPDATE_VERSION, new_version);

		if (!Cod2revUpdater_IsAllowedUpdateUrl(update_file) ||
		    !Cod2revUpdater_IsSafeText(new_version) ||
		    !Cod2revUpdater_IsValidDecimalSize(expected_size) ||
		    !Cod2revUpdater_IsValidSha256(expected_sha256))
		{
			Com_Printf("CoD2rev AutoUpdate: rejected update because URL, size, or SHA-256 is invalid.\n");
		}
		else
		{
			Cod2revUpdater_DownloadFile(update_file, new_version, expected_size, expected_sha256);
		}
	}
	else
	{
		Com_Printf("CoD2rev AutoUpdate: no updates available.\n");
	}

	Com_Printf("-----------------------------------\n");
}

#endif

#endif
