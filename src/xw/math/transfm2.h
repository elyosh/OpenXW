#ifndef XW_MATH_TRANSFM2_H
#define XW_MATH_TRANSFM2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { TRANSFM2_MATRIX_FRACTION_BITS = 15 };

enum { TRANSFM2_WORD_BITS = 32, TRANSFM2_SHIFT_COUNT_MASK = 31, TRANSFM2_PROJECTION_OVERFLOW = 0x7FFFFF00 };

extern int g_camMatR2_Y;
extern int g_camMatR1_Z;
extern int g_camMatR0_Z;
extern int g_camMatR2_Z;
extern int g_camMatR1_X;
extern int g_camMatR0_X;
extern int g_camMatR2_X;
extern int g_camMatR1_Y;
extern int g_camMatR0_Y;

/* Declarations follow ascending original IDB address. */

/* 0x429860 */
int transfm2_geteyex(int x, int y, int z);

/* 0x4298A0 */
int transfm2_geteyey(int x, int y, int z);

/* 0x4298E0 */
int transfm2_geteyez(int x, int y, int z);

/* 0x429920 */
int transfm2_getscreencoordx(int viewX, unsigned int depth);

/* 0x429940 */
int transfm2_getscreencoordy(int viewY, unsigned int depth);

/* 0x429960 */
int transfm2_getscreenx(int viewX, unsigned int depth);

/* 0x4299F0 */
int transfm2_getscreeny(int viewY, unsigned int depth);

#ifdef __cplusplus
}
#endif

#endif
