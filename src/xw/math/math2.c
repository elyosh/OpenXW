#include "xw/math/math2.h"

#include "xw/flight/flight_display.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw_runtime/timing/host_clock.h"

#include <stdlib.h>

// GLOBAL: XW 0x4DA880
uint16_t g_gameRandomSeed = 0x2357;

// GLOBAL: XW 0x4DA884
uint16_t g_auxRandomSeed = 0x2357;

// GLOBAL: XW 0x4DA888
XwRadarBoundarySample g_radarLowResBoundary[MATH2_RADAR_BOUNDARY_COUNT] = {
	{ 0, 18 },  { 1, 18 },  { 2, 18 },  { 3, 18 },  { 4, 17 },  { 5, 17 },  { 6, 17 },  { 7, 17 },
	{ 8, 16 },  { 9, 16 },  { 10, 16 }, { 10, 15 }, { 11, 15 }, { 12, 15 }, { 12, 14 }, { 13, 14 },
	{ 14, 14 }, { 14, 13 }, { 15, 13 }, { 15, 12 }, { 16, 12 }, { 16, 11 }, { 17, 11 }, { 17, 10 },
	{ 18, 10 }, { 18, 9 },  { 18, 8 },  { 19, 8 },  { 19, 7 },  { 19, 6 },  { 20, 6 },  { 20, 5 },
	{ 21, 4 },  { 21, 3 },  { 21, 2 },  { 21, 1 },  { 21, 0 }
};

// GLOBAL: XW 0x4DA8D8
XwRadarBoundarySample g_radarActiveBoundary[MATH2_RADAR_BOUNDARY_COUNT] = {
	{ 0, 18 },  { 1, 18 },  { 2, 18 },  { 3, 18 },  { 4, 17 },  { 5, 17 },  { 6, 17 },  { 7, 17 },
	{ 8, 16 },  { 9, 16 },  { 10, 16 }, { 10, 15 }, { 11, 15 }, { 12, 15 }, { 12, 14 }, { 13, 14 },
	{ 14, 14 }, { 14, 13 }, { 15, 13 }, { 15, 12 }, { 16, 12 }, { 16, 11 }, { 17, 11 }, { 17, 10 },
	{ 18, 10 }, { 18, 9 },  { 18, 8 },  { 19, 8 },  { 19, 7 },  { 19, 6 },  { 20, 6 },  { 20, 5 },
	{ 21, 4 },  { 21, 3 },  { 21, 2 },  { 21, 1 },  { 21, 0 }
};

// GLOBAL: XW 0x4DA924
unsigned int g_radarBoundaryResolutionMode = RTSVGA2_MODE_13H;

// GLOBAL: XW 0x561984
uint16_t g_auxRandomOutput = 0;

// GLOBAL: XW 0x561988
uint16_t g_gameRandomOutput = 0;

// GLOBAL: XW 0x5BE9BC
uint16_t g_mathDivideFractionQ16;

// GLOBAL: XW 0x63A8E4
int16_t g_radarProjectedY = 0;

// GLOBAL: XW 0x63A8E6
int16_t g_radarProjectedX = 0;

// FUNCTION: XW 0x49E9B0
int math2_ABoverC32(int multiplicand, int multiplier, int divisor) {
	int negativeResult = 0;
	if (multiplicand < 0) {
		multiplicand = (int)(0u - (uint32_t)multiplicand);
		negativeResult = 1;
	}
	if (multiplier < 0) {
		multiplier = (int)(0u - (uint32_t)multiplier);
		negativeResult = !negativeResult;
	}
	if (divisor < 0) {
		divisor = (int)(0u - (uint32_t)divisor);
		negativeResult = !negativeResult;
	}
	if (negativeResult) {
		uint64_t product = (uint64_t)(uint32_t)multiplier * (uint32_t)multiplicand;
		uint32_t highWord = (uint32_t)(product >> MATH2_PRODUCT_HIGH_WORD_SHIFT);
		uint32_t quotient = highWord < (uint32_t)divisor ? (uint32_t)(product / (uint32_t)divisor)
														 : MATH2_QUOTIENT_OVERFLOW_MAGNITUDE;
		multiplicand = (int)(0u - quotient);
	} else {
		uint64_t product = (uint64_t)(uint32_t)multiplier * (uint32_t)multiplicand;
		uint32_t highWord = (uint32_t)(product >> MATH2_PRODUCT_HIGH_WORD_SHIFT);
		uint32_t quotient = highWord < (uint32_t)divisor ? (uint32_t)(product / (uint32_t)divisor)
														 : MATH2_QUOTIENT_OVERFLOW_MAGNITUDE;
		multiplicand = (int)quotient;
	}
	return multiplicand;
}

// FUNCTION: XW 0x49EA30
unsigned int math2_fraction(uint16_t value, uint16_t fractionQ16) {
	unsigned int scaledValue = value;
	if (fractionQ16 != MATH2_FRACTION_FULL) {
		scaledValue = (scaledValue * fractionQ16 + MATH2_FRACTION_ROUNDING) >> MATH2_FRACTION_BITS;
	}
	return scaledValue;
}

// FUNCTION: XW 0x49EA60
unsigned int math2_longfraction(unsigned int value, uint16_t fractionQ16) {
	if (fractionQ16 == MATH2_FRACTION_FULL) {
		return value;
	}
	return fractionQ16 * (value >> MATH2_FRACTION_BITS) +
		   ((fractionQ16 * (unsigned int)(uint16_t)value + MATH2_FRACTION_ROUNDING) >> MATH2_FRACTION_BITS);
}

// FUNCTION: XW 0x49EAA0
uint16_t math2_divide(uint16_t numerator, uint16_t denominator) {
	uint16_t quotient;
	uint16_t remainder;

	quotient = numerator / denominator;
	remainder = numerator - quotient * denominator;
	g_mathDivideFractionQ16 = ((unsigned int)remainder << 16) / denominator;
	return quotient;
}

// FUNCTION: XW 0x49EAF0
unsigned int math2_percentage(uint16_t numerator, uint16_t denominator) {
	if (numerator != denominator) {
		if (denominator == 0) {
			return 0;
		}
		if (numerator < denominator) {
			return ((unsigned int)numerator << MATH2_FRACTION_BITS) / denominator;
		}
	}
	return MATH2_FRACTION_FULL;
}

// FUNCTION: XW 0x49EB30
unsigned int math2_longpercentage(unsigned int numerator, unsigned int denominator) {
	if (numerator != denominator && denominator != 0 && numerator < denominator) {
		while (numerator > MATH2_FRACTION_FULL || denominator > MATH2_FRACTION_FULL) {
			numerator >>= 1;
			denominator >>= 1;
		}
		return (numerator << MATH2_FRACTION_BITS) / denominator;
	}
	return MATH2_FRACTION_FULL;
}

// FUNCTION: XW 0x49EB70
int math2_getrandom__auxiliary(void) {
	int stepsRemaining;
	int result;
	for (stepsRemaining = MATH2_RANDOM_BITS; stepsRemaining != 0; --stepsRemaining) {
		uint8_t lowTap = g_auxRandomSeed & MATH2_RANDOM_FEEDBACK_MASK;
		uint8_t highTap = (g_auxRandomSeed >> MATH2_RANDOM_BYTE_BITS) & MATH2_RANDOM_HIGH_BYTE_MASK;
		uint16_t feedback = highTap ^ (uint16_t)(lowTap * 2);
		int emittedBit = (g_auxRandomSeed & (1 << MATH2_RANDOM_HIGH_BIT)) != 0;
		g_auxRandomSeed = g_auxRandomSeed * 2 + (feedback != 0);
		result = g_auxRandomOutput * 2 + emittedBit;
		g_auxRandomOutput = result;
	}
	return result;
}

// FUNCTION: XW 0x49EBD0
int math2_getrandom(void) {
	int stepsRemaining;
	int result;
	for (stepsRemaining = MATH2_RANDOM_BITS; stepsRemaining != 0; --stepsRemaining) {
		uint8_t lowTap = g_gameRandomSeed & MATH2_RANDOM_FEEDBACK_MASK;
		uint8_t highTap = (g_gameRandomSeed >> MATH2_RANDOM_BYTE_BITS) & MATH2_RANDOM_HIGH_BYTE_MASK;
		uint16_t feedback = highTap ^ (uint16_t)(lowTap * 2);
		int emittedBit = (g_gameRandomSeed & (1 << MATH2_RANDOM_HIGH_BIT)) != 0;
		g_gameRandomSeed = g_gameRandomSeed * 2 + (feedback != 0);
		result = g_gameRandomOutput * 2 + emittedBit;
		g_gameRandomOutput = result;
	}
	return result;
}

// FUNCTION: XW 0x49EC30
void math2_SeedFromClock(void) {
	g_auxRandomSeed = timeGetTime();
	g_gameRandomSeed = math2_getrandom__auxiliary();
}

// FUNCTION: XW 0x49EC50
unsigned int math2_mphconvert(int16_t speed, uint16_t stepDivisor) {
	unsigned int scaled =
		(unsigned int)(MATH2_TRAVEL_MULTIPLIER * speed + MATH2_TRAVEL_ROUNDING) >> MATH2_TRAVEL_FRACTION_BITS;
	unsigned int travelDistance = scaled / stepDivisor;
	/* The original correction tests a bitwise AND, not the division remainder. */
	if ((scaled & stepDivisor) > (scaled >> 1))
		++travelDistance;
	return travelDistance;
}

// FUNCTION: XW 0x49EC90
void math2_getradarcoord(int viewX, int viewY, int absoluteDepth) {
	unsigned int resolutionMode = g_flightResolutionMode;
	int projectedX;
	int projectedY;
	uint16_t angle;
	uint16_t ratioIndex;
	if (g_radarBoundaryResolutionMode != resolutionMode) {
		int i;
		if (resolutionMode == RTSVGA2_MODE_13H) {
			int remaining;
			for (i = 0, remaining = MATH2_RADAR_BOUNDARY_COUNT; remaining != 0; ++i, --remaining) {
				g_radarActiveBoundary[i].maxX = g_radarLowResBoundary[i].maxX;
				g_radarActiveBoundary[i].maxY = g_radarLowResBoundary[i].maxY;
			}
		} else {
			int sampleAngle = 0;
			for (i = 0; sampleAngle < TRIG2_QUARTER_TURN; ++i) {
				g_radarActiveBoundary[i].maxX = trig2_sinewordmult(MATH2_RADAR_HIGH_RES_RADIUS, sampleAngle);
				g_radarActiveBoundary[i].maxY =
					trig2_cosinewordmult(MATH2_RADAR_HIGH_RES_RADIUS, sampleAngle);
				sampleAngle += MATH2_RADAR_ANGLE_STEP;
			}
			resolutionMode = g_flightResolutionMode;
		}
		g_radarBoundaryResolutionMode = resolutionMode;
	}
	projectedX = viewX;
	projectedY = viewY;
	if (viewX < 0)
		projectedX = (int)(0u - (uint32_t)viewX);
#ifdef XW_MODERN
	projectedX = (int)((uint32_t)projectedX << ((g_projPerspectiveShift - MATH2_RADAR_PERSPECTIVE_REDUCTION) &
												MATH2_SHIFT_COUNT_MASK));
#else
	projectedX = (int)((uint32_t)projectedX << (g_projPerspectiveShift - MATH2_RADAR_PERSPECTIVE_REDUCTION));
#endif
	if (absoluteDepth != 0)
		projectedX /= absoluteDepth;
	if (projectedX > INT16_MAX)
		projectedX = INT16_MAX;
	if (viewY < 0)
		projectedY = (int)(0u - (uint32_t)viewY);
#ifdef XW_MODERN
	projectedY = (int)((uint32_t)projectedY << ((g_projPerspectiveShift - MATH2_RADAR_PERSPECTIVE_REDUCTION) &
												MATH2_SHIFT_COUNT_MASK));
#else
	projectedY = (int)((uint32_t)projectedY << (g_projPerspectiveShift - MATH2_RADAR_PERSPECTIVE_REDUCTION));
#endif
	if (absoluteDepth != 0)
		projectedY /= absoluteDepth;
	if (projectedY > INT16_MAX)
		projectedY = INT16_MAX;
	g_radarProjectedX = projectedX;
	g_radarProjectedY = projectedY;
	trig2_calcarctan(projectedX, projectedY, &angle, &ratioIndex);
	{
		uint16_t boundaryIndex = (uint16_t)(TRIG2_QUARTER_TURN - angle) / MATH2_RADAR_ANGLE_STEP;
		int16_t clampedX = g_radarProjectedX;
		uint16_t maxX = g_radarActiveBoundary[boundaryIndex].maxX;
		uint16_t maxY;
		int16_t clampedY;
		if (maxX < clampedX) {
			clampedX = maxX;
			g_radarProjectedX = clampedX;
		}
		if (viewX < 0)
			g_radarProjectedX = -clampedX;
		maxY = g_radarActiveBoundary[boundaryIndex].maxY;
		clampedY = g_radarProjectedY;
		if (maxY < clampedY) {
			clampedY = maxY;
			g_radarProjectedY = clampedY;
		}
		if (viewY < 0)
			g_radarProjectedY = -clampedY;
	}
}
