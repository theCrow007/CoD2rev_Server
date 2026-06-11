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

#endif
