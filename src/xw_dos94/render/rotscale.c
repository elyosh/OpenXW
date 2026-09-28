#include "xw_dos94/render/rotscale.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/trace2.h"
#include <string.h>
static const uint16_t tangent091[137] = {
	0,     366,   731,   1097,  1463,  1828,  2194,  2561,  2927,  3293,  3660,  4027,  4395,  4762,
	5131,  5499,  5868,  6237,  6607,  6977,  7348,  7720,  8092,  8464,  8838,  9212,  9586,  9962,
	10338, 10715, 11093, 11471, 11851, 12231, 12613, 12995, 13379, 13763, 14149, 14536, 14924, 15313,
	15703, 16095, 16488, 16882, 17277, 17674, 18073, 18473, 18874, 19277, 19682, 20088, 20496, 20906,
	21317, 21731, 22146, 22563, 22982, 23403, 23826, 24251, 24678, 25107, 25539, 25973, 26409, 26848,
	27289, 27732, 28178, 28627, 29078, 29532, 29989, 30449, 30911, 31377, 31845, 32317, 32791, 33269,
	33751, 34235, 34723, 35215, 35710, 36209, 36711, 37217, 37727, 38242, 38760, 39282, 39809, 40340,
	40875, 41415, 41960, 42509, 43063, 43622, 44186, 44755, 45330, 45910, 46495, 47086, 47683, 48286,
	48895, 49509, 50131, 50758, 51392, 52033, 52681, 53336, 53999, 54668, 55345, 56030, 56723, 57424,
	58134, 58852, 59578, 60314, 61059, 61813, 62577, 63351, 64135, 64929, 65535,
};
static const uint16_t tangent110[121] = {
	0,     442,   885,   1327,  1770,  2212,  2655,  3098,  3542,  3985,  4429,  4873,  5318,  5763,
	6208,  6654,  7100,  7547,  7995,  8443,  8891,  9341,  9791,  10242, 10693, 11146, 11599, 12054,
	12509, 12965, 13422, 13880, 14340, 14800, 15261, 15724, 16188, 16654, 17120, 17588, 18058, 18528,
	19001, 19474, 19950, 20427, 20906, 21386, 21868, 22352, 22838, 23326, 23815, 24307, 24800, 25296,
	25794, 26294, 26796, 27301, 27808, 28317, 28829, 29344, 29860, 30380, 30902, 31427, 31955, 32486,
	33019, 33556, 34096, 34639, 35185, 35734, 36287, 36843, 37403, 37966, 38533, 39103, 39678, 40256,
	40838, 41425, 42015, 42610, 43209, 43812, 44420, 45033, 45650, 46272, 46899, 47532, 48169, 48811,
	49459, 50112, 50771, 51436, 52106, 52783, 53465, 54154, 54849, 55551, 56259, 56974, 57697, 58426,
	59162, 59906, 60658, 61417, 62185, 62961, 63744, 64537, 65338,
};

static uint16_t fold(uint16_t angle) {
	angle &= 0x7FFF;
	return angle >= 0x4000 ? (uint16_t)(0x8000 - angle) : angle;
}

static uint16_t tangent(uint16_t angle, uint16_t* type) {
	*type = angle >= 0x2200 ? 4 : 0;
	return *type ? tangent110[(0x4000 - angle) >> 6] : tangent091[angle >> 6];
}

/* DOS94 0x6A6C90: three independently cached, integer DDA modes. */
static void build_line(Dos94RotLine* line, uint16_t angle) {
	line->cached_angle = angle;
	line->sin_sign = angle & 0x8000;
	line->cos_sign = (uint16_t)(angle + 0x4000) & 0x8000;
	uint16_t quadrant = fold(angle);
	line->sin_quad = g_sinTable[(quadrant >> 6) & 511];
	line->cos_quad = g_sinTable[((quadrant + 0x4000) >> 6) & 511];
	line->x_flip = (angle >> 15) & 1;
	line->major_axis_flag = (((angle & 0x7FFF) >= 0x4000) != (line->x_flip != 0)) ? 2 : 0;
	uint16_t increment = tangent(quadrant, &line->case_type);
	uint16_t argument = line->case_type ? 0x4000 - quadrant : quadrant;
	line->angle_scale = g_sinTable[(256 - (argument >> 6)) & 511];
	uint16_t reciprocal = (uint16_t)(0x80000000u / line->angle_scale);
	line->reciprocal =
		line->case_type ? reciprocal : (uint16_t)(((uint32_t)reciprocal * 0xE8BAu + 0x8000) >> 16);
	line->scan_count = line->case_type ? g_flightVpHeight : g_flightVpWidth;
	line->dda[0] = line->dda[1] = 0;
	uint16_t fraction = 0x8000, minor = 0;
	for (unsigned i = 1; i < line->scan_count; ++i) {
		uint16_t previous = fraction;
		fraction = (uint16_t)(fraction + increment);
		if (fraction < previous)
			++minor;
		line->dda[2 * i] = line->case_type ? minor : i;
		line->dda[2 * i + 1] = line->case_type ? i : minor;
	}
	line->dx_abs = line->dda[2 * (line->scan_count - 1)];
	line->dy_abs = line->dda[2 * (line->scan_count - 1) + 1];
	line->num_run_lengths = 0;
	unsigned axis = line->case_type ? 0 : 1;
	for (unsigned i = 0; i < line->scan_count;) {
		unsigned start = i++;
		while (i < line->scan_count && line->dda[2 * i + axis] == line->dda[2 * start + axis])
			++i;
		line->run_lengths[line->num_run_lengths++] = i - start;
	}
	line->perp_update_inc = tangent(fold((uint16_t)(angle + 0x4000)), &line->perp_case_type);
	line->perp_flag = 0;
	if (line->case_type != line->perp_case_type)
		line->perpfrac_inc = (uint16_t)(((uint32_t)line->perp_update_inc * increment + 0x800000u) >> 24);
	else if ((angle > 0x2000 && angle < 0x6000) || (angle > 0xA000 && angle < 0xE000)) {
		uint16_t inverse = (uint16_t)(0x1000000u / line->perp_update_inc);
		line->perp_update_inc = (uint16_t)(inverse << 8);
		line->perp_flag = inverse >> 8;
		line->perpfrac_inc = increment >> 8;
	} else {
		line->perp_update_inc = 256;
		line->perp_flag = 256;
		line->perpfrac_inc = (uint16_t)(((uint32_t)increment * 0xD800u) >> 24);
	}
	line->packed_octant_a = (line->major_axis_flag >> 1) + line->x_flip;
	line->octant_case = line->case_type | line->major_axis_flag | line->x_flip;
	for (unsigned i = 0; i < line->scan_count; ++i) {
		int x = line->major_axis_flag ? -(int)line->dda[2 * i] : line->dda[2 * i];
		int y = line->x_flip ? line->dda[2 * i + 1] : -(int)line->dda[2 * i + 1];
		line->pixelOffsets[i] = x + y * Dos94_display->rowStride;
	}
}

void Dos94_ROTSCALE_preparefastdraw(uint16_t angle, uint16_t mode) {
	Dos94Rotation* r = &Dos94_display->rotation;
	if (r->viewportGeneration != Dos94_display->viewportGeneration) {
		for (unsigned i = 0; i < 3; ++i)
			build_line(&r->modes[i], 0);
		r->viewportGeneration = Dos94_display->viewportGeneration;
	}
	r->line = &r->modes[mode < 3 ? mode : 0];
	if (r->line->cached_angle != angle)
		build_line(r->line, angle);
}

void Dos94_ROTSCALE_preparecolor(const uint8_t palette[16]) {
	memcpy(Dos94_display->rotation.palette, palette, 16);
}

static uint16_t rounded_scale(uint16_t value, uint16_t factor) {
	return (uint16_t)(((uint32_t)value * factor + 128) >> 8);
}

static int16_t signed_trig(uint16_t value, uint16_t magnitude, bool negative) {
	uint16_t result = (uint16_t)(((uint32_t)value * magnitude + 0x8000) >> 16);
	return negative ? (int16_t)-result : (int16_t)result;
}

/* DOS94 0x6A7328: round each product independently. */
static void adjusted_corner(Dos94Rotation* r, int16_t x, int16_t y, int16_t* outX, int16_t* outY) {
	uint16_t sx = rounded_scale(x < 0 ? (uint16_t)-x : x, r->scale);
	uint16_t sy = rounded_scale(rounded_scale(y < 0 ? (uint16_t)-y : y, 282), r->scale);
	Dos94RotLine* l = r->line;
	*outX = (int16_t)(signed_trig(sx, l->cos_quad, (x < 0) != (l->cos_sign != 0)) +
					  signed_trig(sy, l->sin_quad, (y < 0) != (l->sin_sign != 0)));
	int16_t vertical = (int16_t)(signed_trig(sx, l->sin_quad, (x < 0) != (l->sin_sign != 0)) -
								 signed_trig(sy, l->cos_quad, (y < 0) != (l->cos_sign != 0)));
	uint16_t magnitude = rounded_scale(vertical < 0 ? (uint16_t)-vertical : vertical, 233);
	*outY = vertical < 0 ? (int16_t)-magnitude : (int16_t)magnitude;
}

static void scale_setup(Dos94Rotation* r) {
	uint16_t scale = (uint16_t)(((uint32_t)r->scale * r->line->angle_scale) >> 16);
	r->scaleX = r->line->case_type ? (uint16_t)(((uint32_t)scale * 233) >> 8) : scale;
	r->scaleY = (uint16_t)(scale + (((uint32_t)scale * r->line->perpfrac_inc) >> 8));
	if (!r->line->perp_case_type)
		r->scaleY = (uint16_t)(((uint32_t)r->scaleY * 282) >> 8);
}

static uint16_t dda_minor(Dos94RotLine* line, int index, unsigned axis) {
	if (index < 0)
		return axis ? line->perp_flag : line->perp_update_inc;
	if (index > 320)
		return 0;
	return line->dda[2 * index + axis];
}

static void update_perpendicular(Dos94Rotation* r) {
	if (r->perpendflag) {
		r->perpendflag = 0;
		return;
	}
	uint16_t previous = r->perpendfrac;
	r->perpendfrac = (uint16_t)(previous + r->line->perp_update_inc);
	bool carry = r->perpendfrac < previous;
	if (!r->line->perp_flag && !carry)
		return;
	int step = r->line->perp_flag && carry ? 2 : 1;
	int index = r->startdrawpoint;
	while (index < 0)
		index += r->line->scan_count;
	while (index >= r->line->scan_count)
		index -= r->line->scan_count;
	bool forward = r->line->case_type ? r->line->packed_octant_a != 1 : r->line->packed_octant_a == 1;
	unsigned axis = r->line->case_type ? 0 : 1;
	r->startdrawpoint = (int16_t)(r->startdrawpoint + (forward ? step : -step));
	if (dda_minor(r->line, index, axis) != dda_minor(r->line, index + (forward ? 1 : -1), axis))
		r->perpendflag = 1;
}

static void plot_run(Dos94Rotation* r, int start, int end, uint8_t color) {
	if (start < r->firstvispoint)
		start = r->firstvispoint;
	if (start < 0)
		start = 0;
	if (end > r->lastvispoint)
		end = r->lastvispoint + 1;
	if (end > r->line->scan_count)
		end = r->line->scan_count;
	int xs = r->line->major_axis_flag ? -1 : 1, ys = r->line->x_flip ? 1 : -1;
	int rowBase = (g_flightVpMaxY - r->linestarty) * Dos94_display->rowStride + r->linestartx;
	for (int i = start; i < end; ++i) {
		int x = r->linestartx + xs * r->line->dda[2 * i];
		int y = g_flightVpMaxY - r->linestarty + ys * r->line->dda[2 * i + 1];
		if (x >= 0 && x < g_flightVpWidth && y >= 0 && y < g_flightVpHeight)
			Dos94_display->logical[rowBase + r->line->pixelOffsets[i]] = color;
	}
}

/* DOS94 0x6A3861: fixed nibble runs and exclusive run endpoints. */
static bool rotate_rows(Dos94Rotation* r, const uint8_t* bytes, size_t size, uint8_t transparent) {
	if (!Dos94Rot_Start(r))
		return true;
	r->perpendfrac = 0;
	r->perpendflag = 1;
	uint32_t rowAccumulator = 0;
	for (size_t cursor = 5; cursor < size;) {
		unsigned count = bytes[cursor];
		if (count == 255)
			return true;
		if (cursor + 3 + count > size)
			return false;
		unsigned skip = bytes[cursor + (r->reverse & 0x8000 ? 2 : 1)];
		unsigned repeats = ((rowAccumulator + r->scaleY) >> 8) - (rowAccumulator >> 8);
		for (unsigned repeat = 0; repeat < (repeats ? repeats : 1); ++repeat) {
			uint32_t horizontal = skip * r->scaleX;
			int start = (int16_t)(r->startdrawpoint + (horizontal >> 8));
			for (unsigned i = 0; i < count; ++i) {
				unsigned index = r->reverse & 0x8000 ? count - 1 - i : i;
				uint8_t run = bytes[cursor + 3 + index];
				horizontal += (uint32_t)((run & 15) + 1) * r->scaleX;
				int end = (int16_t)(r->startdrawpoint + (horizontal >> 8));
				if ((run >> 4) != transparent && r->lastvispoint >= 0)
					plot_run(r, start, end, run >> 4);
				start = end;
			}
			if (repeats) {
				if (!Dos94Rot_Step(r))
					return true;
				update_perpendicular(r);
			}
		}
		rowAccumulator = (rowAccumulator + r->scaleY) & 0xFFFFFFu;
		cursor += 3 + count;
	}
	return false;
}

/* DOS94 0x6A3625: palette conversion precedes the visibility events. */
static void scan_bitmap(int16_t minX, int16_t minY, int16_t maxX, int16_t maxY) {
	Dos94Raster* raster = &Dos94_display->raster;
	if (raster->flatObjectNumber >= raster->flatLimit)
		return;
	/* DOS93 0x6A2D82 expands the rotated bounds by two pixels. */
	int margin = raster->dos93 ? 2 : 4;
	int left = minX - margin, right = maxX + margin, top = g_flightVpMaxY - (maxY + margin),
		bottom = g_flightVpMaxY - (minY - margin);
	if (left >= g_flightVpWidth || right < 0 || top >= g_flightVpHeight || bottom < 0)
		return;
	if (left < 0)
		left = 0;
	if (top < 0)
		top = 0;
	if (right > g_flightVpWidth)
		right = g_flightVpWidth;
	if (bottom > g_flightVpHeight)
		bottom = g_flightVpHeight;
	if (left >= right || top >= bottom)
		return;
	unsigned id = raster->flatObjectNumber++;
	Dos94EyePoint world = Dos94_display->bitmaps.world;
	raster->flat[id] = (Dos94RasterFlat) {
		world.x >> 5, world.y >> 5, world.z >> 5, raster->parentObject, (uint8_t)raster->objectNumber, 0
	};
	uint16_t tag = (uint16_t)(((id + 128) << 8) | (uint8_t)raster->layer);
	for (int y = top; y < bottom; ++y) {
		uint8_t* row = Dos94_display->logical + y * Dos94_display->rowStride;
		for (int x = left; x < right;) {
			if (row[x] >= 64) {
				++x;
				continue;
			}
			int start = x;
			do {
				row[x] = Dos94_display->rotation.palette[row[x] & 15];
				++x;
			} while (x < right && row[x] < 64);
			Dos94_TRACE2_enterevent(y, tag, start, 0);
			Dos94_TRACE2_enterevent(y, tag, x, 0);
		}
	}
}

bool Dos94_ROTSCALE_rotatescaleimage(int16_t x, int16_t y, uint16_t scale, Dos94ByteView image) {
	const uint8_t* bytes = Dos94Assets_Data(image);
	if (!bytes || image.size < 5)
		return false;
	Dos94Rotation* r = &Dos94_display->rotation;
	if (!r->line)
		Dos94_ROTSCALE_preparefastdraw(0, 0);
	r->scale = scale;
	scale_setup(r);
	int16_t left = (int8_t)bytes[0], top = (int8_t)bytes[1], right = (int8_t)bytes[2],
			bottom = (int8_t)bytes[3];
	if (r->reverse & 0x8000) {
		int16_t t = left;
		left = -right;
		right = -t;
	}
	int16_t dx, dy;
	adjusted_corner(r, left, top, &dx, &dy);
	r->plotx = (int16_t)(x + dx);
	r->ploty = (int16_t)(y + dy);
	int16_t minX = r->plotx, maxX = r->plotx, minY = r->ploty, maxY = r->ploty;
	bool ok = rotate_rows(r, bytes, image.size, bytes[4]);
	int16_t corners[3][2] = { { right, top }, { right, bottom }, { left, bottom } };
	for (unsigned i = 0; i < 3; ++i) {
		adjusted_corner(r, corners[i][0], corners[i][1], &dx, &dy);
		dx = (int16_t)(dx + x);
		dy = (int16_t)(dy + y);
		if (dx < minX)
			minX = dx;
		if (dx > maxX)
			maxX = dx;
		if (dy < minY)
			minY = dy;
		if (dy > maxY)
			maxY = dy;
	}
	scan_bitmap(minX, minY, maxX, maxY);
	return ok;
}
