#ifndef _GSC_ZK_UTILS_HPP_
#define _GSC_ZK_UTILS_HPP_

/*
 * Functions ported from ibuddieat/zk_libcod (gsc_utils.cpp).
 * Kept in a separate translation unit from rev's own gsc_utils so the
 * provenance stays clear and the port can proceed incrementally.
 * All bodies use only rev's existing helper layer (gsc.hpp).
 */
#include "gsc.hpp"

void gsc_zk_utils_abs();
void gsc_zk_utils_atan2();
void gsc_zk_utils_ceil();
void gsc_zk_utils_floor();
void gsc_zk_utils_chr();
void gsc_zk_utils_ord();
void gsc_zk_utils_tohex();
void gsc_zk_utils_fromhex();
void gsc_zk_utils_roundto();

/* batch 2 */
void gsc_zk_utils_error();
void gsc_zk_utils_executecommand();
void gsc_zk_utils_getmilliseconds();
void gsc_zk_utils_getmicroseconds();
void gsc_zk_utils_getcvarflags();
void gsc_zk_utils_getsystemtime();
void gsc_zk_utils_getlocaltime();
void gsc_zk_utils_fremove();
void gsc_zk_utils_loaddir();
void gsc_zk_utils_logprintconsole();
void gsc_zk_utils_getsurfacename();
void gsc_zk_utils_findconfigstringindex();
void gsc_zk_utils_findconfigstringindexoriginal();

#endif
