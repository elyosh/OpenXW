#ifndef XW_MATH_TRIG2_H
#define XW_MATH_TRIG2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	TRIG2_SINE_TABLE_COUNT = 513,
	TRIG2_SINE_PEAK_INDEX = 256,
	TRIG2_ARCCOS_FRACTION_BITS = 8,
	TRIG2_ARCCOS_ANGLE_SHIFT = 2,
	TRIG2_ARCTAN_TABLE_COUNT = 258,
	TRIG2_RADIUS_CORRECTION_COUNT = 257,
	TRIG2_ARCTAN_DIAGONAL_INDEX = 256,
	TRIG2_ARCTAN_NORMALIZE_SHIFT = 8,
	TRIG2_ARCTAN_HIGH_BYTE_MASK = -0x01000000,
	TRIG2_ARCTAN_FRACTION_MASK = 0xFF00,
	TRIG2_QUARTER_TURN = 0x4000,
	TRIG2_ANGLE_SIGN_BIT = 0x8000,
	TRIG2_SINE_INDEX_SHIFT = 6,
	TRIG2_SINE_INDEX_MASK = 0x1FF,
	TRIG2_INTERPOLATION_FRACTION_SHIFT = 10,
	TRIG2_PRODUCT_FRACTION_BITS = 16,
	TRIG2_PRODUCT_ROUNDING_BIAS = 0x8000
};

extern const float g_trigRadiansPerAngle16;
extern const float g_trigSignedQ15Magnitude;
extern uint16_t g_sinTable[TRIG2_SINE_TABLE_COUNT];
extern const uint16_t g_trig2ArctanTable[TRIG2_ARCTAN_TABLE_COUNT];
extern const uint16_t g_trig2RadiusCorrectionTable[TRIG2_RADIUS_CORRECTION_COUNT];
extern int g_moveDeltaX;
extern int16_t g_trig2Yaw;
extern int g_trig2PolarDistance;
extern int trig2_xoffset;
extern int g_moveDeltaY;
extern uint16_t trig2_phi;
extern int trig2_rho;
extern uint16_t g_trig2ArctanAxesSwapped;
extern uint16_t g_trig2NegativeX;
extern uint16_t g_trig2NegativeY;
extern uint16_t trig2_signz;
extern int trig2_yoffset;
extern int g_moveDeltaZ;
extern unsigned int g_trig2MajorAxisMagnitude;
extern int trig2_zoffset;
extern uint16_t trig2_theta;
extern int16_t g_trig2Pitch;
extern uint16_t g_trig2PlaneAngle;

/* Declarations follow ascending original IDB address. */

/* 0x4A9C90 */
int64_t trig2_getsignedsin(int16_t angle);

/* 0x4A9CB0 */
int trig2_calcsineofangle(unsigned int angle);

/* 0x4A9D10 */
int16_t trig2_w_arccos(int cosineQ15);

/* 0x4A9D20 */
int16_t trig2_arccos(int cosineQ15);

/* 0x4A9E30 */
/* Uses the signed low word of value. Returns the unsigned 16-bit result bits. */
unsigned int trig2_sinewordmult(int value, unsigned int angle);

/* 0x4A9E80 */
int trig2_sinedwordmult(int value, uint16_t angle);

/* 0x4A9EE0 */
int64_t trig2_getsignedcos(int16_t angle);

/* 0x4A9F00 */
/* Uses the signed low word of value. Returns the unsigned 16-bit result bits. */
unsigned int trig2_cosinewordmult(int value, int angle);

/* 0x4A9F60 */
int trig2_cosinedwordmult(int value, uint16_t angle);

/* 0x4A9FD0 */
void trig2_UpdateCartesianOffsets(void);

/* 0x4AA040 */
void trig2_movexyz(uint16_t distance, int16_t yaw, uint16_t pitch);

/* 0x4AA0A0 */
uint16_t trig2_ctop(int dx, int dy, int dz);

/* 0x4AA190 */
void trig2_calcangleplanedistance(int absX, int absY);

/* 0x4AA210 */
void trig2_calcarctan(int absX, signed int absY, uint16_t* outAngle, uint16_t* outRatioIndex);

/* 0x4AA300 */
int trig2_arctan(int dy, int dx);

#ifdef __cplusplus
}
#endif

#endif
