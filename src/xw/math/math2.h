#ifndef XW_MATH_MATH2_H
#define XW_MATH_MATH2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/render/rtsvga2.h"

#include <stddef.h>
#include <stdint.h>

extern uint16_t g_gameRandomSeed;
extern uint16_t g_auxRandomSeed;
extern uint16_t g_auxRandomOutput;
extern uint16_t g_gameRandomOutput;
extern uint16_t g_mathDivideFractionQ16;

enum { MATH2_PRODUCT_HIGH_WORD_SHIFT = 32, MATH2_QUOTIENT_OVERFLOW_MAGNITUDE = 0x7FFFFFFF };

enum {
	MATH2_RANDOM_BITS = 16,
	MATH2_RANDOM_HIGH_BIT = 15,
	MATH2_RANDOM_FEEDBACK_BIT = 6,
	MATH2_RANDOM_BYTE_BITS = 8,
	MATH2_RANDOM_HIGH_BYTE_MASK = 0x80,
	MATH2_RANDOM_FEEDBACK_MASK = 1 << MATH2_RANDOM_FEEDBACK_BIT
};

enum {
	MATH2_FRACTION_BITS = 16,
	MATH2_FRACTION_ROUNDING = 1 << (MATH2_FRACTION_BITS - 1),
	MATH2_FRACTION_FULL = (1 << MATH2_FRACTION_BITS) - 1
};

enum {
	MATH2_TRAVEL_MULTIPLIER = 4660,
	MATH2_TRAVEL_FRACTION_BITS = 8,
	MATH2_TRAVEL_ROUNDING = 1 << (MATH2_TRAVEL_FRACTION_BITS - 1)
};

enum {
	MATH2_RADAR_BOUNDARY_COUNT = 37,
	MATH2_RADAR_ANGLE_STEP = 443,
	MATH2_RADAR_HIGH_RES_RADIUS = 44,
	MATH2_RADAR_PERSPECTIVE_REDUCTION = 5,
	MATH2_SHIFT_COUNT_MASK = 31
};

extern XwRadarBoundarySample g_radarLowResBoundary[MATH2_RADAR_BOUNDARY_COUNT];
extern XwRadarBoundarySample g_radarActiveBoundary[MATH2_RADAR_BOUNDARY_COUNT];
extern unsigned int g_radarBoundaryResolutionMode;
extern int16_t g_radarProjectedY;
extern int16_t g_radarProjectedX;

/* Declarations follow ascending original IDB address. */

/* 0x49E9B0 */
int math2_ABoverC32(int multiplicand, int multiplier, int divisor);

/* 0x49EA30 */
unsigned int math2_fraction(uint16_t value, uint16_t fractionQ16);

/* 0x49EA60 */
unsigned int math2_longfraction(unsigned int value, uint16_t fractionQ16);

/* 0x49EAA0 */
uint16_t math2_divide(uint16_t numerator, uint16_t denominator);

/* 0x49EAF0 */
unsigned int math2_percentage(uint16_t numerator, uint16_t denominator);

/* 0x49EB30 */
unsigned int math2_longpercentage(unsigned int numerator, unsigned int denominator);

/* 0x49EB70 */
int math2_getrandom__auxiliary(void);

/* 0x49EBD0 */
int math2_getrandom(void);

/* 0x49EC30 */
void math2_SeedFromClock(void);

/* 0x49EC50 */
unsigned int math2_mphconvert(int16_t speed, uint16_t stepDivisor);

/* 0x49EC90 */
void math2_getradarcoord(int viewX, int viewY, int absoluteDepth);

#ifdef __cplusplus
}
#endif

#endif
