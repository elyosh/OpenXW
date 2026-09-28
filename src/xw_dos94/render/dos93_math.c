#include "xw_dos94/render/dos93_math.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"

static uint32_t magnitude(int32_t value) { return value < 0 ? 0u - (uint32_t)value : (uint32_t)value; }

static uint32_t divide_word(uint32_t numerator, uint16_t denominator) {
	if (denominator <= 1)
		return numerator;
	return numerator / denominator + (numerator % denominator > (denominator >> 1));
}

/* 0x6A80CB/0x6A812A: estimate with a rounded-up divisor, then correct the
 * residual in base 65536 or 256. Equality has its own early-return branch. */
static uint32_t divide_corrected(uint32_t numerator, uint32_t denominator, unsigned shift) {
	uint32_t mask = (1u << shift) - 1;
	uint32_t divisor = (denominator >> shift) + 1;
	uint32_t correction = (1u << shift) - (denominator & mask);
	uint16_t quotient = 0;
	for (;;) {
		uint32_t upper = numerator >> shift;
		uint32_t part = upper / divisor;
		quotient = (uint16_t)(quotient + part);
		numerator = ((upper % divisor) << shift) + (numerator & mask) + part * correction;
		upper = numerator >> shift;
		if (upper > divisor)
			continue;
		if (upper == divisor || numerator >= (denominator >> 1))
			++quotient;
		return quotient;
	}
}

/* DOS93 0x6A804B: preserve branch-specific rounding and reduced precision. */
uint32_t Dos93_math2_divide32u(uint32_t numerator, uint32_t denominator) {
	uint16_t high = denominator >> 16, low = (uint16_t)denominator;
	if (!(numerator >> 16))
		return high ? 0 : divide_word(numerator, low);
	if (!low) {
		if (!high)
			return numerator;
		return divide_word((numerator + 0x8000u) >> 16, high);
	}
	if (!high)
		return divide_word(numerator, low);
	if ((numerator >> 16) == high)
		return 1;
	if ((numerator >> 16) < high) {
		uint64_t twice = (uint64_t)numerator * 2;
		return twice > UINT32_MAX || (twice >> 16) >= high;
	}
	if (high >= 255)
		return divide_corrected(numerator, denominator, 16);
	if (!(low & 255))
		return divide_word((numerator + 128u) >> 8, denominator >> 8);
	return divide_corrected(numerator, denominator, 8);
}

/* 0x6A81D1/0x6A8226: corrected quotient with separately truncated products. */
static uint32_t fixed_quotient(uint32_t numerator, uint16_t divisor, uint16_t correction,
							   uint16_t* residual) {
	uint32_t quotient = 0;
	if ((numerator >> 16) >= divisor) {
		quotient = numerator / divisor;
		numerator = (uint32_t)(((uint64_t)quotient * correction) >> 16) + numerator % divisor;
	}
	for (;;) {
		uint16_t part = (uint16_t)(numerator / divisor), remainder = numerator % divisor;
		quotient += part;
		numerator = ((uint32_t)part * correction >> 16) + remainder;
		if (numerator <= divisor) {
			*residual = numerator;
			return quotient;
		}
	}
}

uint32_t Dos93_math2_DivideByFixed16(uint32_t numerator, uint32_t denominator) {
	if (!(numerator >> 16))
		return Dos93_math2_divide32u(numerator << 16, denominator);
	if (!(uint16_t)denominator)
		return (denominator >> 16) <= 1 ? numerator : numerator / (denominator >> 16);
	unsigned shift = (denominator >> 16) >= 255 ? 16 : 8;
	uint16_t divisor = (uint16_t)((denominator >> shift) + 1);
	if (!divisor)
		return UINT32_MAX;
	uint16_t correction = (uint16_t)(0u - (denominator << (16 - shift))), residual;
	uint32_t quotient = fixed_quotient(numerator, divisor, correction, &residual);
	if (shift == 16)
		return quotient;
	uint32_t tail = (uint32_t)residual << 8;
	uint16_t part = (uint16_t)(tail / divisor), remainder = tail % divisor;
	unsigned carry = (((uint32_t)(uint8_t)part * (correction >> 8)) >> 8) + (remainder >> 8);
	/* The byte exchanges rotate, retaining the old high byte on overflow. */
	return ((quotient << 8) | (quotient >> 24)) + (uint8_t)part +
		   (carry > 255 || (uint8_t)carry > (divisor >> 8));
}

/* DOS93 0x6A04B0/0x6A0585/0x6A065A: round magnitudes separately, then add. */
int32_t Dos93_transfm2_geteye(const int16_t row[3], const Dos94EyePoint* point) {
	const int32_t coordinates[3] = { point->x, point->y, point->z };
	uint32_t sum = 0;
	for (unsigned i = 0; i < 3; ++i) {
		uint16_t coefficient = (uint16_t)(2u * (row[i] < 0 ? -row[i] : row[i]));
		uint32_t term = (uint32_t)(((uint64_t)magnitude(coordinates[i]) * coefficient + 0x8000u) >> 16);
		sum += (row[i] < 0) != (coordinates[i] < 0) ? 0u - term : term;
	}
	return (int32_t)sum;
}

static uint32_t project_magnitude(int32_t coordinate, uint32_t depth) {
	uint32_t value = magnitude(coordinate);
	if (depth >> 16)
		return Dos93_math2_divide32u(value, depth >> 8);
	if (value >> 24) {
		value = Dos93_math2_divide32u(value, depth);
		if ((value >> 16) > 127)
			value = 0x007F0000u | (value & 0xFFFFu);
		return value << 8;
	}
	value = Dos93_math2_divide32u(value << 8, depth);
	/* Clamp the high word only; the low word remains the computed quotient. */
	return value & 0x80000000u ? 0x7FFF0000u | (value & 0xFFFFu) : value;
}

/* DOS93 0x6A122C. */
int32_t Dos93_TRANSFM2_getscreenx(int32_t x, uint32_t depth) {
	uint32_t projected = project_magnitude(x, depth);
	if (x < 0)
		projected = 0u - projected;
	return (int32_t)(projected + g_flightVpCenterX);
}

/* DOS93 0x6A12C3: aspect correction precedes sign restoration. */
int32_t Dos93_TRANSFM2_getscreeny(int32_t y, uint32_t depth) {
	uint32_t projected = (uint32_t)(((uint64_t)project_magnitude(y, depth) * 0xE8BAu + 0x8000u) >> 16);
	if (y < 0)
		projected = 0u - projected;
	return (int32_t)(projected + g_flightVpCenterY + (uint32_t)g_projOffsetY);
}

/* DOS93 0x6A718F: first/second is numerator/denominator, unlike the Windows ABI. */
void Dos93_trig2_calcarctan(uint32_t first, uint32_t second, uint16_t* angle, uint16_t* ratioIndex) {
	bool swapped = first > second;
	uint32_t minor = swapped ? second : first, major = swapped ? first : second;
	uint16_t index = 256;
	if (minor != major) {
		for (unsigned i = 0; i < 2 && !(major >> 24); ++i) {
			minor <<= 8;
			major <<= 8;
		}
		if ((minor >> 16) != (major >> 16))
			index = (uint16_t)((minor / (major >> 16) + 128u) >> 8);
	}
	*ratioIndex = index;
	*angle = swapped ? 0x4000 - g_trig2ArctanTable[index] : g_trig2ArctanTable[index];
}

/* DOS93 0x6A7220. */
uint16_t Dos93_trig2_arctan(int32_t first, int32_t second) {
	uint16_t angle, index;
	Dos93_trig2_calcarctan(magnitude(first), magnitude(second), &angle, &index);
	if (first < 0)
		angle = (uint16_t)-angle;
	if (second < 0)
		angle = 0x8000 - angle;
	return angle;
}

/* DOS93 0x6A715D: radius correction shares its authored table with DOS94/98. */
static uint32_t Dos93_trig2_ctoptwodim(uint32_t first, uint32_t second, uint16_t* angle) {
	uint16_t index;
	Dos93_trig2_calcarctan(first, second, angle, &index);
	uint32_t major = first > second ? first : second;
	return major + (uint32_t)(((uint64_t)major * g_trig2RadiusCorrectionTable[index] + 0x8000u) >> 16);
}

/* DOS93 0x6A7020: graphics callers consume yaw, pitch and radius explicitly. */
Dos93PolarCoordinates Dos93_trig2_ctop(int32_t x, int32_t y, int32_t z) {
	uint16_t angle;
	uint32_t radius = Dos93_trig2_ctoptwodim(magnitude(y), magnitude(x), &angle);
	if (y < 0)
		angle = (uint16_t)-angle;
	if (x < 0)
		angle = 0x8000 - angle;
	int16_t yaw = (int16_t)(0x4000 - angle);
	radius = Dos93_trig2_ctoptwodim(magnitude(z), radius, &angle);
	if (z < 0)
		angle = (uint16_t)-angle;
	return (Dos93PolarCoordinates) { yaw, (int16_t)(0x4000 - angle), radius };
}
