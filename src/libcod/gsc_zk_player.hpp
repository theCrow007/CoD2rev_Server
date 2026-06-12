#ifndef _GSC_ZK_PLAYER_HPP_
#define _GSC_ZK_PLAYER_HPP_

/*
 * Player METHODS ported from ibuddieat/zk_libcod (gsc_player.cpp).
 * This is the "pure" slice - functions touching only real engine prims/fields,
 * no customPlayerState[] and no missing primitives.
 *
 * gsc_player breakdown (149 fns): ~60 depend on zk's customPlayerState[]
 * (hook subsystem), ~43 on gclient_s/playerState, ~35 on the server client_t,
 * ~11 pure. Of the pure set, several still need missing prims
 * (SV_AddEntToPlayerSnapshots, SV_RemoveEntFromPlayerSnapshots,
 * Scr_SetFogForPlayer, Scr_FxParamError) or missing helpers
 * (GetCurrentWeaponSlot, BG_GetWeaponSlotNameForIndex, playerCommand global)
 * and are deferred until those are implemented.
 */
#include "gsc.hpp"

void gsc_zk_player_earthquakeforplayer(scr_entref_t ref);
void gsc_zk_player_clienthasclientmuted(scr_entref_t ref);
void gsc_zk_player_playfxontagforplayer(scr_entref_t ref);
void gsc_zk_player_enablesilent(scr_entref_t ref);
void gsc_zk_player_disablesilent(scr_entref_t ref);
void gsc_zk_player_overridecontents(scr_entref_t ref);
void gsc_zk_player_setweaponspreadscale(scr_entref_t ref);
void gsc_zk_player_setmeleerangescale(scr_entref_t ref);
void gsc_zk_player_setmeleewidthscale(scr_entref_t ref);
void gsc_zk_player_setmeleeheightscale(scr_entref_t ref);
void gsc_zk_player_setspeed(scr_entref_t ref);
void gsc_zk_player_setgravity(scr_entref_t ref);
void gsc_zk_player_setturretspreadscale(scr_entref_t ref);
void gsc_zk_player_sethiddenfromscoreboard(scr_entref_t ref);
void gsc_zk_player_ishiddenfromscoreboard(scr_entref_t ref);
void gsc_zk_player_sethiddenfromserverstatus(scr_entref_t ref);
void gsc_zk_player_ishiddenfromserverstatus(scr_entref_t ref);
void gsc_zk_player_setping(scr_entref_t ref);
void gsc_zk_player_getspeed(scr_entref_t ref);
void gsc_zk_player_getgravity(scr_entref_t ref);
void gsc_zk_player_isreloading(scr_entref_t ref);
void gsc_zk_player_isfiring(scr_entref_t ref);
void gsc_zk_player_ismeleeing(scr_entref_t ref);
void gsc_zk_player_isthrowinggrenade(scr_entref_t ref);
void gsc_zk_player_getcurrentweaponammo(scr_entref_t ref);
void gsc_zk_player_getcurrentweaponclipammo(scr_entref_t ref);
void gsc_zk_player_getgroundentity(scr_entref_t ref);
void gsc_zk_player_getplayerstateflags(scr_entref_t ref);
void gsc_zk_player_isshellshocked(scr_entref_t ref);
void gsc_zk_player_ischatting(scr_entref_t ref);
void gsc_zk_player_getentertime(scr_entref_t ref);
void gsc_zk_player_getinactivitytime(scr_entref_t ref);
void gsc_zk_player_getweaponanimation(scr_entref_t ref);
void gsc_zk_player_getspectatorclient(scr_entref_t ref);
void gsc_zk_player_getclienthudelemcount(scr_entref_t ref);
void gsc_zk_player_isusingbinoculars(scr_entref_t ref);
void gsc_zk_player_canmantle(scr_entref_t ref);
void gsc_zk_player_getcurrentoffhandslotammo(scr_entref_t ref);
void gsc_zk_player_getjumpslowdowntimer(scr_entref_t ref);
void gsc_zk_player_noclip(scr_entref_t ref);
void gsc_zk_player_setcurrentweaponammo(scr_entref_t ref);
void gsc_zk_player_setcurrentweaponclipammo(scr_entref_t ref);
void gsc_zk_player_playscriptanimation(scr_entref_t ref);
void gsc_zk_player_processsuicide(scr_entref_t ref);
void gsc_zk_player_stopuseturret(scr_entref_t ref);
void gsc_zk_player_forceshot(scr_entref_t ref);

void gsc_zk_player_button_ads(scr_entref_t ref);
void gsc_zk_player_button_back(scr_entref_t ref);
void gsc_zk_player_button_forward(scr_entref_t ref);
void gsc_zk_player_button_frag(scr_entref_t ref);
void gsc_zk_player_button_holdbreath(scr_entref_t ref);
void gsc_zk_player_button_jump(scr_entref_t ref);
void gsc_zk_player_button_leanleft(scr_entref_t ref);
void gsc_zk_player_button_leanright(scr_entref_t ref);
void gsc_zk_player_button_left(scr_entref_t ref);
void gsc_zk_player_button_reload(scr_entref_t ref);
void gsc_zk_player_button_right(scr_entref_t ref);
void gsc_zk_player_button_smoke(scr_entref_t ref);
void gsc_zk_player_isbot(scr_entref_t ref);
void gsc_zk_player_getclientconnectstate(scr_entref_t ref);
void gsc_zk_player_getlastmsg(scr_entref_t ref);
void gsc_zk_player_getlastconnecttime(scr_entref_t ref);
void gsc_zk_player_addresstype(scr_entref_t ref);
void gsc_zk_player_getservercommandqueuesize(scr_entref_t ref);
void gsc_zk_player_getuserinfo(scr_entref_t ref);
void gsc_zk_player_setguid(scr_entref_t ref);
void gsc_zk_player_muteclient(scr_entref_t ref);
void gsc_zk_player_unmuteclient(scr_entref_t ref);
void gsc_zk_player_renameclient(scr_entref_t ref);
void gsc_zk_player_setuserinfo(scr_entref_t ref);
void gsc_zk_player_setconfigstringforplayer(scr_entref_t ref);
void gsc_zk_player_setnorthyawforplayer(scr_entref_t ref);
void gsc_zk_player_resetnextreliabletime(scr_entref_t ref);
void gsc_zk_player_connectionlesspackettoclient(scr_entref_t ref);
void gsc_zk_player_connectionlesspackettoserver(scr_entref_t ref);
void gsc_zk_player_setholdingweapondown(scr_entref_t ref);
void gsc_zk_player_setfirethroughwalls(scr_entref_t ref);
void gsc_zk_player_getbulletmask(scr_entref_t ref);
void gsc_zk_player_setbulletmask(scr_entref_t ref);
#endif
