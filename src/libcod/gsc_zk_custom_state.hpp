#ifndef _GSC_ZK_CUSTOM_STATE_HPP_
#define _GSC_ZK_CUSTOM_STATE_HPP_

/*
 * Custom per-player / per-entity state infrastructure ported from
 * ibuddieat/zk_libcod (declarations.hpp). This is the foundation for the
 * ~80 zk functions that store extra state and have it consumed by engine
 * hooks (gravity/velocity, visibility, silent, no-earthquakes, overrides...).
 *
 * NOTE: customPlayerState_t here is a TRIMMED CORE of zk's full struct. Fields
 * belonging to self-contained subsystems are intentionally omitted until those
 * subsystems are ported, to avoid dragging in their dependencies:
 *   - objectives[16]            (needs objective_t)
 *   - animation/fps/frames      (custom-animation subsystem)
 *   - talkerIcons[64]           (talker-icon subsystem)
 *   - droppingBullets[]         (needs droppingBullet_t, MAX_DROPPING_BULLETS)
 *   - voice fields              (COMPILE_CUSTOM_VOICE)
 *   - jump fields               (COMPILE_JUMP)
 *   - resourceLimitedState      (needs resourceLimitedState_t)
 *   - snapshotEntities          (needs snapshotEntityNumbers_t)
 * Add fields here as each feature slice is ported. Since nothing in rev depends
 * on zk's exact layout, growing this struct is safe.
 */
#include "gsc.hpp"

typedef enum
{
	GRAVITY_NONE,
	GRAVITY_NO_BOUNCE,
	GRAVITY_BOUNCE,
	GRAVITY_NUM
} customGravityType_t;

typedef enum
{
	CUSTOM_TEAM_NONE,
	CUSTOM_TEAM_AXIS,
	CUSTOM_TEAM_ALLIES,
	CUSTOM_TEAM_SPECTATOR,
	CUSTOM_TEAM_AXIS_ALLIES,
	CUSTOM_TEAM_AXIS_SPECTATOR,
	CUSTOM_TEAM_ALLIES_SPECTATOR,
	CUSTOM_TEAM_ALL
} customTeam_t;

// button masks (usercmd buttons) used by bot input + use-button features
#define ZK_KEY_MASK_USE         0x8
#define ZK_KEY_MASK_USERELOAD   0x20
#define ZK_KEY_MASK_ADS_MODE    0x1000
#define ZK_KEY_MASK_FRAG        0x10000
#define ZK_KEY_MASK_SMOKE       0x20000

typedef struct customPlayerState_s
{
	// --- overrides ---
	qboolean overrideContents;
	int contents;
	qboolean overridePing;
	int ping;
	qboolean overrideStatusPing;
	int statusPing;
	qboolean overrideBulletMask;
	int bulletMask;
	qboolean overrideStepSize;
	float stepSize;
	qboolean overrideProneStepSize;
	float proneStepSize;

	// --- melee / fire / spread scaling ---
	float meleeHeightScale;
	float meleeRangeScale;
	float meleeWidthScale;
	int fireThroughWalls;
	float fireRangeScale;
	float turretSpreadScale;
	float weaponSpreadScale;

	// --- simple toggles ---
	qboolean noPickup;
	qboolean noPickupHintString;
	qboolean noEarthquakes;
	qboolean noBulletImpacts;
	qboolean silent;
	customTeam_t collisionTeam;
	qboolean hiddenFromScoreboard;
	qboolean hiddenFromServerStatus;
	qboolean notAllowingSpectators;

	// --- movement values ---
	int speed;
	int gravity;
	int previousButtons;
	unsigned int weapon;

	// --- use-button feature ---
	qboolean activateOnUseButtonRelease;
	qboolean heldUseButton;

	// --- proxy address bookkeeping ---
	netadr_t realAddress;

	// --- bot input (consumed by the bot usercmd hook) ---
	int botButtons;
	int botWeapon;
	char botForwardMove;
	char botRightMove;
} customPlayerState_t;

typedef struct customEntityState_s
{
	customGravityType_t gravityType;
	qboolean collideModels;
	vec3_t velocity;
	float maxVelocity;
	qboolean angledGravity;
	float parallelBounce;
	float perpendicularBounce;
	qboolean convertedTrigger;
	qboolean notSolidBrushModel; // If true, not solid for at least one player
	int clientMask[2];
} customEntityState_t;

extern customPlayerState_t customPlayerState[MAX_CLIENTS];
extern customEntityState_t customEntityState[MAX_GENTITIES];

// lifecycle - call from rev's ClientConnect / G_InitGentity / startup
void zk_ResetCustomPlayerState(int clientNum);
void zk_ResetCustomEntityState(int entnum);
void zk_InitCustomState(void);

// feature accessors (consumed by native engine call-site hooks)
qboolean zk_IsPlayerSilent(int clientNum);
qboolean zk_IsHiddenFromScoreboard(int clientNum);
qboolean zk_IsHiddenFromServerStatus(int clientNum);
qboolean zk_GetPingOverride(int clientNum, int *ping);
qboolean zk_GetStatusPingOverride(int clientNum, int *ping);
qboolean zk_GetPlayerContentsOverride(int clientNum, int *contents);
float zk_GetWeaponSpreadScale(int clientNum);
float zk_GetTurretSpreadScale(int clientNum);
void zk_ApplyMeleeScales(int clientNum, float *range, float *width, float *height);
void zk_ApplyPlayerSpeedGravity(int clientNum, int *speed, int *gravity);

#endif
