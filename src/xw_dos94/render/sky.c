#include "xw_dos94/render/sky.h"
#include "xw/flight/death_star.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/draw.h"
#include "xw_dos94/render/view.h"
#include <string.h>

static int16_t jitter_component(int16_t component, unsigned selector) {
	int16_t quarter = (int16_t)(component >> 7);
	switch (selector) {
		case 0:
			return component >> 6;
		case 1:
			return quarter;
		case 2:
			return (int16_t)-quarter;
		default:
			return (int16_t)(-2 * quarter);
	}
}

static bool project(int16_t x, int16_t y, uint16_t z, int16_t* outX, int16_t* outY) {
	uint16_t ax = x < 0 ? (uint16_t)-x : (uint16_t)x, ay = y < 0 ? (uint16_t)-y : (uint16_t)y;
	if ((ax >> 8) >= (z >> 8) || (ay >> 8) >= (z >> 8))
		return false;
	int16_t px = (int16_t)(((uint32_t)ax << 8) / z), py = (int16_t)(((uint32_t)ay << 8) / z);
	*outX = x < 0 ? (int16_t)-px : px;
	*outY = y < 0 ? (int16_t)-py : py;
	return true;
}

/* DOS94 0x6990D0: component steps begin at one, and jitter has 4^3 entries. */
void Dos94_backdrp2_backdrop(void) {
	Dos94Sky* s = &Dos94_display->sky;
	const int16_t (*matrix)[3] = Dos94_display->draw.camera.matrix;
	for (unsigned axis = 0; axis < 3; ++axis)
		for (unsigned row = 0; row < 3; ++row)
			for (unsigned i = 0; i < 16; ++i)
				s->steps[axis][row][i] = (int16_t)(((int32_t)matrix[row][axis] * (i + 1)) >> 5);
	for (unsigned z = 0; z < 4; ++z)
		for (unsigned y = 0; y < 4; ++y)
			for (unsigned x = 0; x < 4; ++x)
				for (unsigned row = 0; row < 3; ++row)
					s->jitter[z * 16 + y * 4 + x][row] =
						(int16_t)(jitter_component(matrix[row][0], x) + jitter_component(matrix[row][1], y) +
								  jitter_component(matrix[row][2], z));
	if (!g_backdropsEnabled || (g_deathStarSurfaceModeActive && matrix[2][2] <= (int16_t)0xA000))
		return;
	const unsigned wallAxis[3] = { 1, 0, 2 }, primary[3] = { 0, 1, 1 }, secondary[3] = { 2, 2, 0 };
	uint16_t counts[6] = { g_backdropPositiveYCount, g_backdropNegativeYCount, g_backdropPositiveXCount,
						   g_backdropNegativeXCount, g_backdropPositiveZCount, g_backdropNegativeZCount };
	unsigned cursor = 0;
	Dos94_display->raster.parentObject = 0x3000;
	for (unsigned wall = 0; wall < 3; ++wall) {
		unsigned axis = wallAxis[wall], first = primary[wall], second = secondary[wall];
		unsigned angleAxis = wall == 1 ? 1 : 0;
		int16_t angle = (int16_t)-Dos94_trig2_arctan(matrix[1][angleAxis], matrix[0][angleAxis]);
		bool negative = matrix[2][axis] < 0;
		unsigned count = counts[2 * wall + negative], start = cursor + (negative ? counts[2 * wall] : 0);
		for (unsigned i = 0; i < count && start + i < CREATE_BACKDROP_CAPACITY; ++i) {
			uint8_t packed = g_backdropPackedDirections[start + i];
			int16_t point[3];
			for (unsigned row = 0; row < 3; ++row) {
				int a = s->steps[first][row][packed & 7], b = s->steps[second][row][(packed >> 4) & 7];
				point[row] = (int16_t)((packed & 8 ? -a : a) + (packed & 128 ? -b : b) +
									   (negative ? -(matrix[row][axis] >> 2) : matrix[row][axis] >> 2));
			}
			int16_t x, y;
			if (point[2] >= 0 && project(point[0], point[1], (uint16_t)point[2], &x, &y))
				Dos94_DRAW_drawbackdropimage(g_backdropModelTypes[start + i],
											 (int16_t)(x + g_flightVpCenterX),
											 (int16_t)(g_flightVpCenterY - y - g_projOffsetY), angle);
		}
		cursor += counts[2 * wall] + counts[2 * wall + 1];
	}
}

/* DOS94 0x698968: draw over background, then erase unretained old stars. */
void Dos94_RTSVGA2_drawstars(void) {
	Dos94Sky* sky = &Dos94_display->sky;
	uint16_t current[768];
	unsigned currentCount = 0;
	memset(sky->present, 0, sizeof sky->present);
	const int16_t (*matrix)[3] = Dos94_display->draw.camera.matrix;
	int16_t corner[3];
	for (unsigned axis = 0; axis < 3; ++axis)
		corner[axis] = (int16_t)(((int32_t)(int16_t)-matrix[axis][0] + (int16_t)-matrix[axis][1] +
								  (int16_t)-matrix[axis][2]) >>
								 2);
	const unsigned first[3] = { 0, 0, 1 }, second[3] = { 1, 2, 2 };
	unsigned density = g_starDensity == 2 ? 2 : 1;
	uint8_t background = Dos94_display->raster.backgroundColor;
	for (unsigned plane = 0; plane < 3; ++plane) {
		int16_t base[3] = { corner[0], corner[1], corner[2] };
		unsigned seed = 0;
		for (unsigned row = 0; row < 16; row += density) {
			for (unsigned col = 0; col < 16; col += density, ++seed) {
				int16_t point[3];
				unsigned jitter = g_flightRandomBytePairs[seed].value0To63;
				for (unsigned axis = 0; axis < 3; ++axis)
					point[axis] = (int16_t)(base[axis] + sky->steps[first[plane]][axis][col] +
											sky->jitter[jitter & 63][axis]);
				if (point[2] < 0)
					for (unsigned axis = 0; axis < 3; ++axis)
						point[axis] = (int16_t)-point[axis];
				int16_t x, y;
				if (!project(point[0], point[1], (uint16_t)point[2], &x, &y))
					continue;
				x = (int16_t)(x + g_flightVpCenterX);
				y = (int16_t)(y + g_flightVpCenterY + g_projOffsetY);
				if (x < 0 || x >= g_flightVpWidth || y < 0 || y >= g_flightVpHeight)
					continue;
				unsigned offset = g_flightVpBaseOffset + 320u * y + x;
				if (Dos94_display->screen[offset] < background)
					continue;
				uint8_t brightness =
					(uint8_t)(g_flightRandomBytePairs[seed].value0To7 + g_legacyOscillatorValue);
				if (brightness > 7)
					brightness = 7;
				Dos94_display->screen[offset] = 248 + brightness;
				/* Offset zero is the original membership table's empty sentinel. */
				if (offset)
					sky->present[offset >> 3] |= (uint8_t)(1u << (offset & 7));
				current[currentCount++] = (uint16_t)offset;
			}
			for (unsigned axis = 0; axis < 3; ++axis)
				base[axis] = (int16_t)(corner[axis] + sky->steps[second[plane]][axis][row]);
		}
	}
	for (unsigned i = 0; i < sky->previousCount; ++i) {
		unsigned offset = sky->previous[i];
		if (Dos94_display->screen[offset] > background && !(sky->present[offset >> 3] & (1u << (offset & 7))))
			Dos94_display->screen[offset] = background;
	}
	memcpy(sky->previous, current, currentCount * sizeof current[0]);
	sky->previousCount = currentCount;
}
