#pragma once

#include "../qcommon/qcommon.h"

extern dvar_t *sv_update;
extern dvar_t *sv_updateRestart;

void Cod2revUpdater_RegisterDvars();
void Cod2revUpdater_Frame();
void Cod2revUpdater_HandlePacketResponse(netadr_t from);

bool Cod2revUpdater_IsSafeText(const char *value);
bool Cod2revUpdater_IsAllowedUpdateUrl(const char *url);
bool Cod2revUpdater_IsValidDecimalSize(const char *value);
bool Cod2revUpdater_IsValidSha256(const char *value);
