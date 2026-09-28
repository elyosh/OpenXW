#ifndef XW_FLIGHT_OBJECT_DYNAMIX_H
#define XW_FLIGHT_OBJECT_DYNAMIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	DYNAMIX_NEGATIVE_SPEED_DELTA = 0x8000,
	DYNAMIX_BASE_ACCELERATION_Q16 = 0x4000,
	DYNAMIX_SURFACE_ACCELERATION_MULTIPLIER = 3
};

enum {
	DYNAMIX_DIVE_LEVEL_ALTITUDE = 256,
	DYNAMIX_FIGHTER_DESCENT_SCALE = 3,
	DYNAMIX_OTHER_DESCENT_SCALE = 2,
	DYNAMIX_DIVE_LEVELLED = 2
};

enum {
	DYNAMIX_ENGINE_WEIGHT_COUNT = 5,
	DYNAMIX_PITCH_NEGATIVE_WRAP = 0xE000,
	DYNAMIX_CLIMB_DIVE_ACTIVE = 1,
	DYNAMIX_POWER_BALANCED_TOTAL = 4,
	DYNAMIX_Y_WING_POWER_SPEED_Q16 = 0x1000,
	DYNAMIX_POWER_SPEED_Q16 = 0x2000,
	DYNAMIX_DECELERATION = 20,
	DYNAMIX_SURFACE_DECELERATION = 60,
	DYNAMIX_HYPERSPACE_INITIAL_ACCELERATION = 50,
	DYNAMIX_HYPERSPACE_MIDDLE_ACCELERATION = 200,
	DYNAMIX_HYPERSPACE_FINAL_ACCELERATION = 500
};

extern const uint16_t g_engineAverageFactorQ16ByCount[DYNAMIX_ENGINE_WEIGHT_COUNT];
extern uint16_t g_dynamicsCraftTypeIndex;

/* Declarations follow ascending original IDB address. */

/* 0x408FF0 */
void dynamix_planedynamics(void);

/* 0x4095C0 */
void dynamix_adjustvelocity(uint16_t objectIndex, int16_t targetSpeed, int16_t allowDeceleration,
							uint16_t throttleFraction);

/* 0x4096F0 */
void dynamix_addvelocity(uint16_t objectIndex, uint16_t acceleration);

/* 0x409760 */
void dynamix_subvelocity(uint16_t objectIndex, uint16_t deceleration);

/* 0x4097D0 */
void dynamix_pulloutdive(uint16_t objectIndex);

#ifdef __cplusplus
}
#endif

#endif
