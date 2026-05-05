#include "cod2x_state.hpp"

#include <ctype.h>
#include <stdint.h>
#include <string.h>

cod2x_client_state_t cod2x_clients[MAX_CLIENTS];
dvar_t *sv_cod2xClientMode;
dvar_t *g_cod2x;

int Cod2x_HwidHash(const char *hwid2)
{
	uint32_t hash = 2166136261u;

	if (!hwid2)
		return 1;

	while (*hwid2)
	{
		hash ^= (unsigned char)(*hwid2++);
		hash *= 16777619u;
	}

	if (hash == 0)
		hash = 1;

	return (int)hash;
}

bool Cod2x_IsValidHwid2(const char *hwid2)
{
	if (!hwid2 || strlen(hwid2) != 32)
		return false;

	for (int i = 0; i < 32; i++)
	{
		if (!isxdigit((unsigned char)hwid2[i]))
			return false;
	}

	return true;
}

void Cod2x_ClearClientState(int clientNum)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
		return;

	memset(&cod2x_clients[clientNum], 0, sizeof(cod2x_clients[clientNum]));
}

void Cod2x_SetClientState(int clientNum, int protocol, const char *hwid2)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
		return;

	Cod2x_ClearClientState(clientNum);

	cod2x_clients[clientNum].hasCod2x = protocol > 0 && Cod2x_IsValidHwid2(hwid2);
	cod2x_clients[clientNum].protocol = protocol;

	if (cod2x_clients[clientNum].hasCod2x)
	{
		strncpy(cod2x_clients[clientNum].hwid2, hwid2, sizeof(cod2x_clients[clientNum].hwid2) - 1);
		cod2x_clients[clientNum].hwidHash = Cod2x_HwidHash(hwid2);
	}
}

void Cod2x_RegisterDvars()
{
#ifndef COD2X_STATE_TEST
	sv_cod2xClientMode = Dvar_RegisterInt("sv_cod2xClientMode", COD2X_CLIENT_OPTIONAL, COD2X_CLIENT_OPTIONAL, COD2X_CLIENT_REQUIRED, DVAR_ARCHIVE | DVAR_CHANGEABLE_RESET);
	g_cod2x = Dvar_RegisterInt("g_cod2x", COD2X_VERSION_PROTOCOL, 0, COD2X_VERSION_PROTOCOL, DVAR_SYSTEMINFO | DVAR_CHANGEABLE_RESET);
#endif
}
