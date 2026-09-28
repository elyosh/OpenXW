#ifndef XW_DOS93_MATH_H
#define XW_DOS93_MATH_H

#include "xw_dos94/render/transfm2.h"

typedef struct Dos93PolarCoordinates {
	int16_t yaw, pitch;
	uint32_t distance;
} Dos93PolarCoordinates;

uint32_t Dos93_math2_divide32u(uint32_t numerator, uint32_t denominator);
uint32_t Dos93_math2_DivideByFixed16(uint32_t numerator, uint32_t denominator);
int32_t Dos93_transfm2_geteye(const int16_t row[3], const Dos94EyePoint* point);
int32_t Dos93_TRANSFM2_getscreenx(int32_t x, uint32_t depth);
int32_t Dos93_TRANSFM2_getscreeny(int32_t y, uint32_t depth);
void Dos93_trig2_calcarctan(uint32_t first, uint32_t second, uint16_t* angle, uint16_t* ratioIndex);
uint16_t Dos93_trig2_arctan(int32_t first, int32_t second);
Dos93PolarCoordinates Dos93_trig2_ctop(int32_t x, int32_t y, int32_t z);

#endif
