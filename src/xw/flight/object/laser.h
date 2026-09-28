#ifndef XW_FLIGHT_OBJECT_LASER_H
#define XW_FLIGHT_OBJECT_LASER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct WarheadGuidanceState WarheadGuidanceState;
typedef struct XwWeaponHardpoint XwWeaponHardpoint;

enum {
	LASER_LOCK_ANGLE_DISTANCE_LIMIT = 0x10000,
	LASER_LOCK_MIN_ANGLE_SCORE = 4,
	LASER_LOCK_NEAR_ANGLE_SCORE = 72,
	LASER_LOCK_DISTANCE_SHIFT = 12,
	LASER_LOCK_ANGLE_SCALE = 4,
	LASER_LOCK_RANGE = 101805,
	LASER_LOCK_LARGE_RANGE = 244332,
	LASER_LOCK_SECONDS = 5,
	LASER_LOCK_ACQUIRING = 1,
	LASER_LOCK_COMPLETE = 2,
	LASER_RECHARGE_NEUTRAL = 2,
	LASER_RECHARGE_MEDIUM = 3,
	LASER_RECHARGE_MAX = 4,
	LASER_SHIELD_LOW_FRACTION = 0x4000,
	LASER_SHIELD_HIGH_FRACTION = 0xC000,
	LASER_CHARGE_LOW = 32,
	LASER_CHARGE_HIGH = 96,
	LASER_CHARGE_MAX = 127,
	LASER_SHIELD_RECHARGE_STEP = 20,
	LASER_CHARGE_RECHARGE_STEP = 2,
	LASER_SHIELD_DISTRIBUTE_FRONT = 0,
	LASER_SHIELD_DISTRIBUTE_REAR = 2,
	LASER_FRONT_SHIELD = 0,
	LASER_REAR_SHIELD = 1,
	LASER_BURST_FRAME_DELAY = 2
};

enum {
	LASER_CANNON_SUBSYSTEM_MASK = 0x10,
	LASER_LINK_SINGLE = 1,
	LASER_LINK_PAIR = 2,
	LASER_LINK_ALL = 3,
	LASER_PAIR_SLOT_MASK = 1,
	LASER_PAIR_SLOT_STEP = 2,
	LASER_HIGH_CHARGE_THRESHOLD = 64,
	LASER_PLAYER_CHARGE_COST = 4,
	LASER_SHOT_SOUND_LIMIT = 2,
	LASER_COOLDOWN_PER_SHOT = 78,
	LASER_COOLDOWN_BASE = 2
};

enum {
	LASER_LAUNCHER_SIDE_MASK = 0x80,
	LASER_WARHEAD_COOLDOWN_SECONDS = 2,
	LASER_WARHEAD_MESSAGE_TYPE_BASE = 141,
	LASER_MAX_HOMING_TIER = 6,
	LASER_INCOMING_MISSILE_SOUND = 0x27,
	LASER_INCOMING_MISSILE_VOICE_FIRST = 0x2B,
	LASER_INCOMING_MISSILE_VOICE_SECOND = 0x43,
	LASER_INCOMING_ALERT_ACTIVE = 1,
	LASER_INCOMING_ALERT_SECONDS = 12
};

enum {
	LASER_PROJECTILE_FAMILY = 1,
	LASER_PROJECTILE_INITIAL_AGE = 1,
	LASER_PROJECTILE_FIRST_TYPE = 143,
	LASER_LAUNCH_BASIS_SHIFT = 15
};

enum { LASER_WARHEAD_GUIDANCE_COUNT = 48, LASER_PROJECTILE_TYPE_COUNT = 8 };

extern const int16_t g_projectileSpeedByType[LASER_PROJECTILE_TYPE_COUNT];
extern const int16_t g_projectileBaseDamageByType[LASER_PROJECTILE_TYPE_COUNT];
extern const int16_t g_projectileLifetimeSecondsByType[LASER_PROJECTILE_TYPE_COUNT];
extern const int16_t g_projectileLaunchOffsetByType[LASER_PROJECTILE_TYPE_COUNT];

/* Original IDB size: 3 bytes. */
struct WarheadGuidanceState {
	/* IDB +0x0: Homing level: zero disables homing, initializer clamps at 6. */
	uint8_t homingTier;
	/* IDB +0x1: Target object index; initializer uses 0xFFFF when no target. AI threat predicates compare
	 * with g_paiObjectIndex. */
	uint16_t targetObjIdx;
};

/* Original IDB size: 7 bytes. */
struct XwWeaponHardpoint {
	/* IDB +0x0 */
	int16_t x;
	/* IDB +0x2 */
	int16_t z;
	/* IDB +0x4 */
	int16_t y;
	/* IDB +0x6: Turret: bits 0-1 yaw center in quarter turns, bits 2-3 yaw half-width minus one in eighth
	 * turns, bits 4-5 pitch center, bits 6-7 pitch half-width minus one. Non-turret slots use this byte as
	 * initial ammunition count. */
	uint8_t firingArcOrAmmoCount;
};

extern WarheadGuidanceState g_warheadGuidanceTable[LASER_WARHEAD_GUIDANCE_COUNT];

/* Declarations follow ascending original IDB address. */

/* 0x410510 */
void laser_weaponsfire(void);

/* 0x410A40 */
void laser_chargeshields(uint16_t shieldIndex, int16_t delta);

/* 0x410AC0 */
void laser_fireplayerweapon(void);

/* 0x410CA0 */
void laser_firelasersystem(uint16_t sourceObjectIndex, int laserGroup);

/* 0x410FD0 */
void laser_firerocketsystem(uint16_t objectIndex, uint16_t launcherIndex);

/* 0x411130 */
uint16_t laser_firemissile(uint16_t objectIndex, uint16_t weaponSlotIndex, uint16_t projectileTypeId,
						   uint16_t launcherIndex);

/* 0x4112D0 */
uint16_t laser_createprojectile(uint16_t sourceObjectIndex, uint16_t weaponSlotIndex,
								uint16_t projectileObjectType);

#ifdef __cplusplus
}
#endif

#endif
