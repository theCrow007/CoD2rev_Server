#ifndef _GSC_ZK_PHYSICS_HPP_
#define _GSC_ZK_PHYSICS_HPP_

#include "gsc.hpp"

#if LIBCOD_COMPILE_ENTITY == 1

void G_RunGravityModelNoBounce(gentity_t *ent);
void G_RunGravityModelWithBounce(gentity_t *ent);
qboolean G_BounceGravityModel(gentity_t *ent, trace_t *trace);

// dispatch entry - called from rev's G_RunFrameForEntity
qboolean zk_EntityHasGravity(int entnum);
void zk_RunEntityGravity(gentity_t *ent);

#endif

#endif
