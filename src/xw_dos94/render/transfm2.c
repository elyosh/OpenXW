#include "xw_dos94/render/transfm2.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/dos93_math.h"
#include <limits.h>

static int32_t add32(int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); }

static int32_t sub32(int32_t a, int32_t b) { return (int32_t)((uint32_t)a - (uint32_t)b); }

static uint32_t magnitude(int32_t value) { return value < 0 ? 0u - (uint32_t)value : (uint32_t)value; }

static int16_t highproduct(int16_t a, int16_t b) { return (int16_t)(((int32_t)a * b) >> 16); }

/* DOS94 0x6A0209/0x6A024B/0x6A028D: sum full products before shifting. */
int32_t Dos94_transfm2_geteye(const int16_t row[3], const Dos94EyePoint* point) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93)
		return Dos93_transfm2_geteye(row, point);
	return (int32_t)(((int64_t)row[0] * point->x + (int64_t)row[1] * point->y + (int64_t)row[2] * point->z) >>
					 15);
}

/* DOS94 0x6A01C0. The zero-denominator path still changes the stored depth. */
void Dos94_TRANSFM2_clipobjecteyez(Dos94EyePoint* origin, const Dos94EyePoint* endpoint) {
	int32_t numerator = (int32_t)(0u - (uint32_t)origin->z);
	int32_t denominator = sub32(endpoint->z, origin->z);
	if (denominator) {
		origin->x =
			add32(origin->x, (int32_t)((int64_t)sub32(endpoint->x, origin->x) * numerator / denominator));
		origin->y =
			add32(origin->y, (int32_t)((int64_t)sub32(endpoint->y, origin->y) * numerator / denominator));
	}
	origin->z = 1;
}

/* DOS94 0x6A02CF/0x6A0425: cache high words, indexed by remaining count.
 * Keep the circular product cache rather than resolving coordinate backlinks. */
bool Dos94_TRANSFM2_geteyecoords(Dos94Transform* t, const Dos94MeshView* mesh, unsigned shift,
								 Dos94EyePoint* output) {
	t->numEyeZPositive = 0;
	for (unsigned i = 0; i < mesh->vertexCount; ++i) {
		unsigned remaining = mesh->vertexCount - i;
		unsigned slot = remaining & 15;
		for (unsigned axis = 0; axis < 3; ++axis) {
			uint16_t value;
			if (!Dos94Models_ReadWord(mesh->payload, mesh->vertices + 6u * i + 2u * axis, &value))
				return false;
			if ((value >> 8) == 0x7F) {
				unsigned offset = (2 * remaining + (uint8_t)value) & 31;
				/* Valid coordinate backlinks address complete cached words. */
				if (offset & 1)
					return false;
				for (unsigned row = 0; row < 3; ++row)
					t->products[axis][row][slot] = t->products[axis][row][offset / 2];
			} else
				for (unsigned row = 0; row < 3; ++row)
					t->products[axis][row][slot] = highproduct(t->matrix[row][axis], (int16_t)value);
		}
		int32_t result[3];
		const int32_t origin[3] = { t->origin.x, t->origin.y, t->origin.z };
		for (unsigned row = 0; row < 3; ++row) {
			int16_t sum =
				(int16_t)(t->products[0][row][slot] + t->products[1][row][slot] + t->products[2][row][slot]);
			result[row] = add32(origin[row], (int32_t)sum * (shift ? 4 : 1));
		}
		output[i] = (Dos94EyePoint) { result[0], result[1], result[2] };
		if (result[2] >= 0)
			++t->numEyeZPositive;
	}
	return true;
}

/* DOS94 0x6A0BB3/0x6A0C6C/0x6A0D19. These entries retain the incoming Z count. */
bool Dos94_TRANSFM2_geteyecoordsZ0(Dos94Transform* t, const Dos94MeshView* mesh, unsigned shift,
								   Dos94EyePoint* output) {
	if (shift != 0 && shift != 8 && shift != 16)
		return false;
	for (unsigned i = 0; i < mesh->vertexCount; ++i) {
		uint16_t x, y;
		if (!Dos94Models_ReadWord(mesh->payload, mesh->vertices + 4u * i, &x) ||
			!Dos94Models_ReadWord(mesh->payload, mesh->vertices + 4u * i + 2, &y))
			return false;
		int32_t result[3];
		const int32_t origin[3] = { t->origin.x, t->origin.y, t->origin.z };
		for (unsigned row = 0; row < 3; ++row) {
			uint32_t sum = (uint32_t)((int32_t)t->matrix[row][0] * (int16_t)x) +
						   (uint32_t)((int32_t)t->matrix[row][1] * (int16_t)y) + 0x4000u;
			int16_t rounded = (int16_t)(sum >> 15);
			result[row] = add32(origin[row], (int32_t)((uint32_t)(int32_t)rounded << shift));
		}
		output[i] = (Dos94EyePoint) { result[0], result[1], result[2] };
		if (result[2] >= 0)
			++t->numEyeZPositive;
	}
	return true;
}

static void bound_contributions(const int16_t row[3], const int16_t bounds[6], bool reverseY,
								int16_t* minimum, int16_t* maximum) {
	int16_t lo = 0, hi = 0;
	for (unsigned axis = 0; axis < 3; ++axis) {
		int16_t a = bounds[axis], b = bounds[axis + 3];
		if (reverseY && axis == 1) {
			a = (int16_t)-a;
			b = (int16_t)-b;
		}
		a = highproduct(row[axis], a);
		b = highproduct(row[axis], b);
		lo = (int16_t)(lo + (a < b ? a : b));
		hi = (int16_t)(hi + (a > b ? a : b));
	}
	*minimum = lo;
	*maximum = hi;
}

/* DOS94 0x6A0593/0x6A0700: narrow each axis contribution and the sum. */
void Dos94_TRANSFM2_geteyeminmax(const Dos94Transform* t, const int16_t bounds[6], unsigned shift,
								 int32_t output[6]) {
	const int32_t origin[3] = { t->origin.x, t->origin.y, t->origin.z };
	for (unsigned axis = 0; axis < 3; ++axis) {
		int16_t lo, hi;
		bound_contributions(t->matrix[axis], bounds, false, &lo, &hi);
		output[axis] = add32(origin[axis], (int32_t)lo * (shift ? 4 : 1));
		output[axis + 3] = add32(origin[axis], (int32_t)hi * (shift ? 4 : 1));
	}
}

/* DOS94 0x6A089D/0x6A0A34: matrix columns are craft side, forward and up. */
void Dos94_TRANSFM2_getworldminmax(const int16_t matrix[3][3], const int16_t bounds[6], unsigned shift,
								   int16_t output[6]) {
	unsigned bits = shift ? 3 : 5;
	int rounding = shift ? 4 : 16;
	for (unsigned axis = 0; axis < 3; ++axis) {
		int16_t lo, hi;
		bound_contributions(matrix[axis], bounds, true, &lo, &hi);
		output[axis] = (int16_t)(output[axis] + ((int16_t)(lo + rounding) >> bits));
		output[axis + 3] = (int16_t)(output[axis + 3] + ((int16_t)(hi - rounding) >> bits));
	}
}

static uint32_t projection_magnitude(int32_t value, uint32_t depth) {
	uint32_t absolute = magnitude(value);
	/* CDQ follows NEG: INT32_MIN keeps its sign in the original numerator. */
	uint64_t numerator = (uint64_t)(int64_t)(int32_t)absolute << 8;
	if ((uint32_t)(numerator >> 32) < depth)
		return (uint32_t)(numerator / depth);
	if (numerator >> 32)
		return INT32_MAX;
	return (uint32_t)numerator;
}

/* DOS94 0x6A0E6A/0x6A0DE4. Depth zero keeps a small numerator undivided. */
int32_t Dos94_TRANSFM2_getscreenx(int32_t x, uint32_t depth) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93)
		return Dos93_TRANSFM2_getscreenx(x, depth);
	uint32_t projected = projection_magnitude(x, depth);
	if (x < 0)
		projected = 0u - projected;
	return (int32_t)(projected + g_flightVpCenterX);
}

/* DOS94 0x6A0F08/0x6A0DFA. */
int32_t Dos94_TRANSFM2_getscreeny(int32_t y, uint32_t depth) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93)
		return Dos93_TRANSFM2_getscreeny(y, depth);
	uint32_t projected = (uint32_t)(((uint64_t)projection_magnitude(y, depth) * 0xE8BAu + 0x8000u) >> 16);
	if (y < 0)
		projected = 0u - projected;
	return (int32_t)(projected + (uint32_t)(int32_t)(int16_t)g_flightVpCenterY + (uint32_t)g_projOffsetY);
}

/* DOS94 0x6A0FF9: normalize the divisor to a word before division. */
uint16_t Dos94_TRANSFM2_clipratio(int32_t negativeZ, int32_t positiveZ) {
	uint32_t denominator = (uint32_t)positiveZ - (uint32_t)negativeZ;
	uint64_t numerator = (uint64_t)(uint32_t)positiveZ << 16;
	while (denominator >> 16) {
		denominator >>= 1;
		numerator >>= 1;
	}
	if ((numerator >> 16) >= denominator)
		return 0xFFFF;
	return (uint16_t)(numerator / denominator);
}

static int32_t intersection_coordinate(int32_t negative, int32_t positive, uint16_t ratio) {
	int32_t difference = sub32(positive, negative);
	uint32_t step = (uint32_t)(((uint64_t)magnitude(difference) * ratio) >> 16);
	return (int32_t)((uint32_t)positive + (difference < 0 ? step : 0u - step));
}

/* DOS94 0x6A0FF9: clamp only the high word, retaining the low word. */
Dos94ScreenPoint Dos94_TRANSFM2_calczintersect(const Dos94EyePoint* negative, const Dos94EyePoint* positive) {
	uint16_t ratio = Dos94_TRANSFM2_clipratio(negative->z, positive->z);
	int32_t x = intersection_coordinate(negative->x, positive->x, ratio);
	int32_t y = intersection_coordinate(negative->y, positive->y, ratio);
	int32_t high = x >> 16;
	if (high <= -128)
		x = (int32_t)(0xFF800000u | ((uint32_t)x & 0xFFFFu));
	else if (high > 127)
		x = (int32_t)(0x007F0000u | ((uint32_t)x & 0xFFFFu));
	uint32_t yMagnitude = (uint32_t)(((uint64_t)magnitude(y) * 0xE8BAu + 0x8000u) >> 16);
	if ((yMagnitude >> 16) > 127)
		yMagnitude = 0x007F0000u | (yMagnitude & 0xFFFFu);
	yMagnitude <<= 8;
	if (y < 0)
		yMagnitude = 0u - yMagnitude;
	return (Dos94ScreenPoint) { (int32_t)(((uint32_t)x << 8) + g_flightVpCenterX),
								(int32_t)(yMagnitude + g_flightVpCenterY + (uint32_t)g_projOffsetY) };
}

void Dos94_TRANSFM2_resetbounds(Dos94ScreenBounds* b) {
	*b = (Dos94ScreenBounds) { .minX = INT32_MAX, .minY = INT32_MAX, .maxX = INT32_MIN, .maxY = INT32_MIN };
}

/* DOS94 0x6A0EC7/0x6A0F7F and the inline bounds updates at 0x6A10B3. */
void Dos94_TRANSFM2_screenbounds(Dos94ScreenBounds* b, Dos94ScreenPoint p, uint16_t i, bool clipped) {
	if (!b->initialized) {
		b->minX = b->maxX = p.x;
		b->minY = b->maxY = p.y;
		b->minXPoint = b->maxXPoint = b->minYPoint = b->maxYPoint = i;
		b->initialized = true;
	}

	if (p.x <= b->minX) {
		if (p.x == b->minX)
			++b->sameX;
		b->minX = p.x;
		b->minXPoint = i;
	} else if (p.x >= b->maxX) {
		b->maxX = p.x;
		b->maxXPoint = i;
	}
	if (p.y <= b->minY) {
		/* 0x6A11B3 repeats JA: equal high words increment even for a lower low word. */
		if (p.y == b->minY || (clipped && (p.y >> 16) == (b->minY >> 16)))
			++b->sameY;
		b->minY = p.y;
		b->minYPoint = i;
	} else if (p.y >= b->maxY) {
		b->maxY = p.y;
		b->maxYPoint = i;
	}
}

/* DOS94 0x6A0E10/0x6A0FC2: visit the preceding intersection before the following. */
uint16_t Dos94_TRANSFM2_getscreencoords(const Dos94EyePoint* input, uint8_t count, Dos94ScreenPoint* output,
										Dos94ScreenBounds* bounds) {
	uint16_t written = 0;
	for (unsigned i = 0; i < count; ++i) {
		if (input[i].z >= 0) {
			output[written] =
				(Dos94ScreenPoint) { Dos94_TRANSFM2_getscreenx(input[i].x, (uint32_t)input[i].z),
									 Dos94_TRANSFM2_getscreeny(input[i].y, (uint32_t)input[i].z) };
			Dos94_TRANSFM2_screenbounds(bounds, output[written], written, false);
			++written;
		} else {
			unsigned neighbors[2] = { (i + count - 1) % count, (i + 1) % count };
			for (unsigned n = 0; n < 2; ++n)
				if (input[neighbors[n]].z >= 0) {
					output[written] = Dos94_TRANSFM2_calczintersect(&input[i], &input[neighbors[n]]);
					Dos94_TRANSFM2_screenbounds(bounds, output[written], written, true);
					++written;
				}
		}
	}
	return written;
}
