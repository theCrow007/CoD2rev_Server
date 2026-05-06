#pragma once

#include "../qcommon/qcommon.h"

extern dvar_t *sv_update;
extern dvar_t *sv_updateRestart;

void Cod2revUpdater_RegisterDvars();
void Cod2revUpdater_Frame();
void Cod2revUpdater_CheckNow();
void Cod2revUpdater_CheckOnMapChange();
void Cod2revUpdater_HandlePacketResponse(netadr_t from);

bool Cod2revUpdater_IsSafeText(const char *value);
bool Cod2revUpdater_IsAllowedUpdateUrl(const char *url);
bool Cod2revUpdater_IsValidDecimalSize(const char *value);
bool Cod2revUpdater_IsValidSha256(const char *value);

#ifdef COD2REV_UPDATER_TEST
bool Cod2revUpdater_ShouldSendRequestForTest(int now, int last_request_time, bool request_pending, bool force, bool server_running, bool update_enabled);
#endif
