#ifndef _GSC_ZK_ENTITY_HPP_
#define _GSC_ZK_ENTITY_HPP_

/*
 * Entity METHODS ported from ibuddieat/zk_libcod (gsc_entity.cpp).
 * These take scr_entref_t and register in scriptMethods[].
 * Only functions that read real gentity_s fields are ported here; the
 * gravity/velocity/bounce/solidity cluster depends on zk's customEntityState[]
 * /customPlayerState[] globals + physics hooks and is deferred to the hook phase.
 */
#include "gsc.hpp"

void gsc_zk_entity_getclipmask(scr_entref_t ref);
void gsc_zk_entity_setclipmask(scr_entref_t ref);
void gsc_zk_entity_getvmax(scr_entref_t ref);
void gsc_zk_entity_getvmin(scr_entref_t ref);
void gsc_zk_entity_isturret(scr_entref_t ref);
void gsc_zk_entity_islinkedto(scr_entref_t ref);
void gsc_zk_entity_getturretowner(scr_entref_t ref);
void gsc_zk_entity_setlight(scr_entref_t ref);
void gsc_zk_entity_hastag(scr_entref_t ref);
void gsc_zk_entity_gettagorigin(scr_entref_t ref);

void gsc_zk_entity_getgrenadefusetime(scr_entref_t ref);
void gsc_zk_entity_addgrenadefusetime(scr_entref_t ref);
void gsc_zk_entity_getweaponitemammo(scr_entref_t ref);
void gsc_zk_entity_setweaponitemammo(scr_entref_t ref);
void gsc_zk_entity_getweaponitemclipammo(scr_entref_t ref);
void gsc_zk_entity_setweaponitemclipammo(scr_entref_t ref);
void gsc_zk_entity_getitemquantity(scr_entref_t ref);
void gsc_zk_entity_setitemquantity(scr_entref_t ref);
void gsc_zk_entity_enablegravity(scr_entref_t ref);
void gsc_zk_entity_disablegravity(scr_entref_t ref);
void gsc_zk_entity_isgravityenabled(scr_entref_t ref);
void gsc_zk_entity_enablebounce(scr_entref_t ref);
void gsc_zk_entity_disablebounce(scr_entref_t ref);
void gsc_zk_entity_getentityvelocity(scr_entref_t ref);
void gsc_zk_entity_setentityvelocity(scr_entref_t ref);
void gsc_zk_entity_addentityvelocity(scr_entref_t ref);
void gsc_zk_entity_getmaxentityvelocity(scr_entref_t ref);
void gsc_zk_entity_setmaxentityvelocity(scr_entref_t ref);
void gsc_zk_entity_notsolidforplayer(scr_entref_t ref);
void gsc_zk_entity_solidforplayer(scr_entref_t ref);
void gsc_zk_entity_getcontents(scr_entref_t ref);
void gsc_zk_entity_hidefromplayer(scr_entref_t ref);
#endif
