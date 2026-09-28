#ifndef XW_FLIGHT_OBJECT_COLLIDE_H
#define XW_FLIGHT_OBJECT_COLLIDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	COLLIDE_NO_ATTACKER = 255,
	COLLIDE_PLAYER_IMPACT_SOUND = 18,
	COLLIDE_LETHAL_DAMAGE = 0x7FFF,
	COLLIDE_STATIC_LETHAL_EXTENT = 0x2000,
	COLLIDE_STARSHIP_DAMAGE_SHIFT = 4,
	COLLIDE_FREIGHTER_DAMAGE_SHIFT = 2,
	COLLIDE_SHIELD_WARNING_RANDOM_THRESHOLD = 0x2000,
	COLLIDE_SHIELD_WARNING_SOUND = 40,
	COLLIDE_ION_DAMAGE_PER_SUBSYSTEM = 200,
	COLLIDE_FRIENDLY_LOSS_SOUND = 38,
	COLLIDE_OTHER_LOSS_SOUND = 37,
	COLLIDE_DISABLED_VOICE = 72,
	COLLIDE_DESTROYED_VOICE = 74,
	COLLIDE_DISABLED_TIE_LIFETIME = 60,
	COLLIDE_ION_HIT_SOUND = 21,
	COLLIDE_HIT_FLASH_TICKS = 59,
	COLLIDE_SUBSYSTEM_DAMAGE_RANDOM_THRESHOLD = 0x4000,
	COLLIDE_COURSE_REQUIRED_HUD_FEATURE = 1,
	COLLIDE_SYSTEM_HIT_SOUND = 19,
	COLLIDE_HULL_HIT_SOUND = 20,
	COLLIDE_EXIT_RESCUED = 2,
	COLLIDE_EXIT_CAPTURED = 1,
	COLLIDE_EXIT_KILLED = 0,
	COLLIDE_PILOT_RESCUED = 0,
	COLLIDE_PILOT_CAPTURED = 1,
	COLLIDE_PILOT_KILLED = 2,
	COLLIDE_MUSIC_WINGMAN_LOSS = 11,
	COLLIDE_MUSIC_FRIENDLY_KILL = 12,
	COLLIDE_MUSIC_ENEMY_KILL = 9,
	COLLIDE_TUMBLE_RANDOM_MASK = 0x3FFF,
	COLLIDE_TUMBLE_MIN = 0x2000,
	COLLIDE_ANGLE_SIGN_BIT = 0x8000,
	COLLIDE_LARGE_LIFETIME_RANDOM_MASK = 7,
	COLLIDE_LARGE_LIFETIME_MIN = 8,
	COLLIDE_PLAYER_EXPLOSION_SCALE = 24,
	COLLIDE_CRAFT_EXPLOSION_SOUND = 13,
	COLLIDE_THREE_MESH_COUNT = 3,
	COLLIDE_FOUR_MESH_COUNT = 4,
	COLLIDE_BWING_MESH_COUNT = 6,
	COLLIDE_BWING_REVERSE_MESH = 4,
	COLLIDE_COMPONENT_DETACH_SOUND = 16,
	COLLIDE_COMPONENT_YAW_MIN = 2048,
	COLLIDE_CRAFT_LIFETIME_RANDOM_MASK = 15,
	COLLIDE_PLAYER_LIFETIME_RANDOM_MASK = 3,
	COLLIDE_PLAYER_LIFETIME_MIN = 3
};

enum {
	COLLIDE_ROUGH_DISTANCE_SECONDARY_SHIFT = 2,
	COLLIDE_SURFACE_SCAN_HEIGHT = 0x2000,
	COLLIDE_GATE_RETREAT_DISTANCE = 512,
	COLLIDE_GATE_HIT_SOUND = 20,
	COLLIDE_INSPECTION_LARGE_EXTENT = 3000,
	COLLIDE_INSPECTION_NORMAL_SCALE = 2,
	COLLIDE_INSPECTION_CAPITAL_SCALE = 4,
	COLLIDE_LONG_INSPECTION_CRAFT_TYPE = 2,
	COLLIDE_PLAYER_PROJECTILE_TARGET_LIMIT = 76,
	COLLIDE_FOLLOWER_PLAN = 54,
	COLLIDE_EXEMPT_MANEUVER_18 = 18,
	COLLIDE_EXEMPT_MANEUVER_21 = 21
};

enum {
	COLLIDE_CRAFT_MAX_DISTANCE = 0x40000,
	COLLIDE_CAPITAL_EXTENT_MULTIPLIER = 4,
	COLLIDE_CRAFT_LARGE_EXTENT = 2800,
	COLLIDE_BOX_QUARTER_SHIFT = 2
};

enum {
	COLLIDE_TIME_FRACTION_BITS = 8,
	COLLIDE_TIME_LAST_FRACTION = 255,
	COLLIDE_TIME_TO_Q15_SHIFT = 7,
	COLLIDE_HIT_FRACTION_BITS = 15
};

extern int g_collisionProbeWorldZ;
extern int g_collisionProbeWorldX;
extern int g_collisionProbeWorldY;
extern int g_collisionApproxDistance;
extern int g_collisionHitOffsetZ;
extern int g_collisionSweepStartZ;
extern int g_collisionHitOffsetY;
extern int g_collisionHitOffsetX;
extern int g_collisionSweepStartX;
extern int g_collisionSweepStartY;
extern int g_collisionSegmentStartWorldX;
extern int g_collisionSweepEndX;
extern int g_collisionSweepEndY;
extern int g_collisionSweepEndZ;
extern int g_collisionSegmentStartWorldY;
extern int g_collisionSegmentStartWorldZ;

extern int g_collisionScratchPoint1X;
extern int g_collisionScratchPoint1Y;
extern int g_collisionScratchPoint2X;
extern int g_collisionScratchPoint1Z;
extern int g_collisionScratchPoint2Y;
extern int g_collisionScratchPoint2Z;
extern uint8_t g_lastShieldDamageSide;

enum {
	COLLIDE_STATIC_VICTIM = 0xFFFF,
	COLLIDE_KILL_COUNT_SATURATION = 255,
	COLLIDE_KILL_VOICE_RANDOM_THRESHOLD = 0x4000
};

/* Declarations follow ascending original IDB address. */

/* 0x402E10 */
void collide_collisions(void);

/* 0x403820 */
int16_t collide_lasercraftcollide(uint16_t sourceObjIdx, uint16_t targetObjIdx);

/* 0x403A00 */
int16_t collide_checkboxcollision(int halfExtent);

/* 0x403F00 */
int16_t collide_targetinrange(uint16_t sourceObjectIndex, uint16_t targetObjectRef, uint16_t weaponSlot);

/* 0x404210 */
uint16_t collide_craftstarshipcollision(uint16_t sourceObjectIndex, int16_t predictionSeconds);

/* 0x404450 */
void collide_laserhitcraft(uint16_t projectileObjIdx, uint16_t craftObjIdx, int16_t hitMeshIndex);

/* 0x4046D0 */
int16_t collide_damagecraft(uint16_t victimObjIdx, int16_t hitMeshIndex, uint16_t sourceObjOrMissionPointRef,
							uint16_t shieldSide);

/* 0x405190 */
void collide_makeobjectexplosion(uint16_t objectIndex, uint8_t explosionObjectType);

/* 0x405210 */
unsigned int collide_roughdistance3du(unsigned int absDx, unsigned int absDy, unsigned int absDz);

/* 0x405260 */
int collide_roughdistance3d(int dx, int dy, int dz);

/* 0x4052C0 */
void collide_updatekills(uint16_t shooterObjectIndex, uint16_t victimObjectIndex, int16_t spaceObject);

/* 0x4053F0 */
void collide_updatehits(uint16_t projectileObjIdx, int16_t spacecraftHit);

#ifdef __cplusplus
}
#endif

#endif
