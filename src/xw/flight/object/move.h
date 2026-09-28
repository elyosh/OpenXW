#ifndef XW_FLIGHT_OBJECT_MOVE_H
#define XW_FLIGHT_OBJECT_MOVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { MOVE_WORLD_COORDINATE_LIMIT = 0x1000000 };

enum {
	MOVE_COCKPIT_OFFSET_CRAFT_COUNT = 3,
	MOVE_DEFAULT_COCKPIT_UP = 176,
	MOVE_HOMING_RATE_COUNT = 16,
	MOVE_TORPEDO_HOMING_RATE_OFFSET = 7,
	MOVE_BOARD_MANEUVER = 18,
	MOVE_BOARD_CORRECTION_LIMIT = 250,
	MOVE_ROLL_IMPULSE_SCALE = 4,
	MOVE_FALLING_EFFECT_SOURCE = 0x2000,
	MOVE_FALLING_PITCH_RATE = 32,
	MOVE_FALLING_PITCH_LIMIT = 0x7FFF
};

struct ObjectRecord;
extern const uint16_t g_warheadHomingAngularRateByTier[MOVE_HOMING_RATE_COUNT];
extern const int16_t g_playerCockpitOffsetForwardByCraft[MOVE_COCKPIT_OFFSET_CRAFT_COUNT];
extern const int16_t g_playerCockpitOffsetUpByCraft[MOVE_COCKPIT_OFFSET_CRAFT_COUNT];
extern int16_t g_playerCockpitOffsetSideByCraft[MOVE_COCKPIT_OFFSET_CRAFT_COUNT];
extern struct ObjectRecord* g_movementCurrentObject;
typedef struct XwDockingOffsets XwDockingOffsets;

/* Original IDB size: 10 bytes. */
struct XwDockingOffsets {
	/* IDB +0x0: Local-forward docking coordinate of target craft. */
	int16_t targetForward;
	/* IDB +0x2: Target local-up contribution when target genus is 0/1; also contributes when a small craft
	 * approaches a larger target. */
	int16_t targetSmallUp;
	/* IDB +0x4: Target local-up contribution for larger-target docking geometry. */
	int16_t targetLargeUp;
	/* IDB +0x6: Local-up contribution subtracted for small-target alignment. */
	int16_t selfSmallUp;
	/* IDB +0x8: Local-up contribution subtracted for larger-target alignment. */
	int16_t selfLargeUp;
};

/* Declarations follow ascending original IDB address. */

/* 0x4115B0 */
void move_moveobjects(void);

/* 0x411C70 */
void move_updatexyz(struct ObjectRecord* object);

#ifdef __cplusplus
}
#endif

#endif
