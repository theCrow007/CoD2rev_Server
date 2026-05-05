#pragma once

#include "../universal/q_shared.h"
#include "../universal/dvar.h"

#define COD2X_VERSION_PROTOCOL 6

enum cod2x_client_mode_t
{
	COD2X_CLIENT_OPTIONAL = 0,
	COD2X_CLIENT_REQUIRED = 1,
};

struct cod2x_client_state_t
{
	bool hasCod2x;
	int protocol;
	char hwid2[33];
	int hwidHash;
};

extern cod2x_client_state_t cod2x_clients[MAX_CLIENTS];
extern dvar_t *sv_cod2xClientMode;
extern dvar_t *g_cod2x;

int Cod2x_HwidHash(const char *hwid2);
bool Cod2x_IsValidHwid2(const char *hwid2);
void Cod2x_ClearClientState(int clientNum);
void Cod2x_SetClientState(int clientNum, int protocol, const char *hwid2);
void Cod2x_RegisterDvars();
