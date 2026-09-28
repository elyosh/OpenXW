#include "xw/math/transfm2.h"

#include "xw/math/math2.h"
#include "xw/render/flight_view.h"

#include <stdlib.h>

// GLOBAL: XW 0x6377C8
int g_camMatR2_Y = 0;

// GLOBAL: XW 0x6377CC
int g_camMatR1_Z = 0;

// GLOBAL: XW 0x6377D0
int g_camMatR0_Z = 0;

// GLOBAL: XW 0x6377D4
int g_camMatR2_Z = 0;

// GLOBAL: XW 0x6377D8
int g_camMatR1_X = 0;

// GLOBAL: XW 0x6377DC
int g_camMatR0_X = 0;

// GLOBAL: XW 0x6377E0
int g_camMatR2_X = 0;

// GLOBAL: XW 0x6377E4
int g_camMatR1_Y = 0;

// GLOBAL: XW 0x6377E8
int g_camMatR0_Y = 0;

// FUNCTION: XW 0x429860
int transfm2_geteyex(int x, int y, int z) {
	uint32_t xTerm = (uint32_t)((uint64_t)(x * (int64_t)g_camMatR0_X) >> TRANSFM2_MATRIX_FRACTION_BITS);
	uint32_t yTerm = (uint32_t)((uint64_t)(y * (int64_t)g_camMatR0_Y) >> TRANSFM2_MATRIX_FRACTION_BITS);
	uint32_t zTerm = (uint32_t)((uint64_t)(z * (int64_t)g_camMatR0_Z) >> TRANSFM2_MATRIX_FRACTION_BITS);
	return (int32_t)(xTerm + yTerm + zTerm);
}

// FUNCTION: XW 0x4298A0
int transfm2_geteyey(int x, int y, int z) {
	uint32_t xTerm = (uint32_t)((uint64_t)((int64_t)g_camMatR1_X * x) >> TRANSFM2_MATRIX_FRACTION_BITS);
	uint32_t yTerm = (uint32_t)((uint64_t)((int64_t)g_camMatR1_Y * y) >> TRANSFM2_MATRIX_FRACTION_BITS);
	uint32_t zTerm = (uint32_t)((uint64_t)((int64_t)g_camMatR1_Z * z) >> TRANSFM2_MATRIX_FRACTION_BITS);
	return (int32_t)(xTerm + yTerm + zTerm);
}

// FUNCTION: XW 0x4298E0
int transfm2_geteyez(int x, int y, int z) {
	uint32_t xTerm = (uint32_t)((uint64_t)((int64_t)g_camMatR2_X * x) >> TRANSFM2_MATRIX_FRACTION_BITS);
	uint32_t yTerm = (uint32_t)((uint64_t)((int64_t)g_camMatR2_Y * y) >> TRANSFM2_MATRIX_FRACTION_BITS);
	uint32_t zTerm = (uint32_t)((uint64_t)((int64_t)g_camMatR2_Z * z) >> TRANSFM2_MATRIX_FRACTION_BITS);
	return (int32_t)(xTerm + yTerm + zTerm);
}

// FUNCTION: XW 0x429920
int transfm2_getscreencoordx(int viewX, unsigned int depth) { return transfm2_getscreenx(viewX, depth); }

// FUNCTION: XW 0x429940
int transfm2_getscreencoordy(int viewY, unsigned int depth) { return transfm2_getscreeny(viewY, depth); }

// FUNCTION: XW 0x429960
int transfm2_getscreenx(int viewX, unsigned int depth) {
	if (viewX < 0) {
		uint32_t magnitude = 0u - (uint32_t)viewX;
		uint64_t numerator = magnitude;
		uint32_t quotient;
		numerator <<= g_projPerspectiveShift & TRANSFM2_SHIFT_COUNT_MASK;
		numerator += (uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> TRANSFM2_WORD_BITS) < depth) {
			quotient = (uint32_t)(numerator / depth);
		} else {
			quotient = TRANSFM2_PROJECTION_OVERFLOW;
		}
		return (int32_t)((uint32_t)g_flightVpCenterX - quotient);
	} else {
		uint64_t numerator = (uint32_t)viewX;
		uint32_t quotient;
		numerator <<= g_projPerspectiveShift & TRANSFM2_SHIFT_COUNT_MASK;
		numerator += (uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> TRANSFM2_WORD_BITS) < depth) {
			quotient = (uint32_t)(numerator / depth);
		} else {
			quotient = TRANSFM2_PROJECTION_OVERFLOW;
		}
		return (int32_t)((uint32_t)g_flightVpCenterX + quotient);
	}
}

// FUNCTION: XW 0x4299F0
int transfm2_getscreeny(int viewY, unsigned int depth) {
	uint32_t projectedOffset;
	uint16_t aspect;
	if (viewY < 0) {
		uint32_t magnitude = 0u - (uint32_t)viewY;
		uint64_t numerator = magnitude;
		uint32_t quotient;
		numerator <<= g_projPerspectiveShift & TRANSFM2_SHIFT_COUNT_MASK;
		numerator += (uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> TRANSFM2_WORD_BITS) >= depth) {
			quotient = TRANSFM2_PROJECTION_OVERFLOW;
		} else {
			quotient = (uint32_t)(numerator / depth);
		}
		projectedOffset = 0u - quotient;
	} else {
		uint64_t numerator = (uint32_t)viewY;
		uint32_t quotient;
		numerator <<= g_projPerspectiveShift & TRANSFM2_SHIFT_COUNT_MASK;
		numerator += (uint32_t)g_projScaleHalfInt;
		if ((uint32_t)(numerator >> TRANSFM2_WORD_BITS) >= depth) {
			quotient = TRANSFM2_PROJECTION_OVERFLOW;
		} else {
			quotient = (uint32_t)(numerator / depth);
		}
		projectedOffset = quotient;
	}
	aspect = g_projAspectY;
	if (aspect != 0) {
		if ((int32_t)projectedOffset < 0) {
			projectedOffset = 0u - math2_longfraction(0u - projectedOffset, aspect);
		} else {
			projectedOffset = math2_longfraction(projectedOffset, aspect);
		}
	}
	return (int32_t)(projectedOffset + (uint32_t)g_projOffsetY + g_flightVpCenterY);
}
