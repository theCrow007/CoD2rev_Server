#ifndef _GSC_ZK_LEVEL_HPP_
#define _GSC_ZK_LEVEL_HPP_

/*
 * Level functions ported from ibuddieat/zk_libcod (gsc_level.cpp).
 * Deferred: getSavePersist/setSavePersist (zk reads/writes level.savepersist
 * directly; rev manages this via G_SetSavePersist() instead - reconcile storage
 * before porting). getNumberOfStaticModels/etc. already exist in rev.
 */
#include "gsc.hpp"

void gsc_zk_level_getmovers();
void gsc_zk_level_getentitycount();
void gsc_zk_level_setnorthyaw();
void gsc_zk_level_getsavepersist();
void gsc_zk_level_setsavepersist();

#endif
