#ifndef XW_DOS94_ROTSCALE_H
#define XW_DOS94_ROTSCALE_H
#include "xw_dos94/assets/models.h"

typedef struct Dos94RotLine {
	uint16_t cached_angle, sin_quad, cos_quad, sin_sign, cos_sign;
	uint16_t angle_scale, reciprocal, scan_count, x_flip, major_axis_flag, packed_octant_a, octant_case;
	uint16_t case_type, perp_case_type, perp_update_inc, perp_flag, perpfrac_inc;
	uint16_t dx_abs, dy_abs, num_run_lengths;
	uint16_t dda[642], run_lengths[320];
	int32_t pixelOffsets[320];
} Dos94RotLine;

typedef struct Dos94Rotation {
	Dos94RotLine modes[3];
	Dos94RotLine* line;
	uint32_t viewportGeneration;
	uint8_t palette[16];
	int16_t startdrawpoint, firstvispoint, lastvispoint, firstxincoffset, lastxincoffset;
	int16_t firstyincoffset, lastyincoffset, linestartx, linestarty, lineendx, lineendy, plotx, ploty;
	uint16_t perpendfrac, perpendflag, scaleX, scaleY, scale, reverse;
} Dos94Rotation;

void Dos94_ROTSCALE_preparefastdraw(uint16_t angle, uint16_t mode);
void Dos94_ROTSCALE_preparecolor(const uint8_t palette[16]);
bool Dos94_ROTSCALE_rotatescaleimage(int16_t x, int16_t y, uint16_t scale, Dos94ByteView image);
bool Dos94Rot_Start(Dos94Rotation* rotation);
bool Dos94Rot_Step(Dos94Rotation* rotation);
#endif
