#include "xw/render/flight_view.h"
#include "xw_dos94/render/dos93_math.h"
#include "xw_dos94/render/trace_internal.h"

/* DOS94 0x6A8877: normalize to a word divisor; retain the large-quotient
 * rounding path and its remainder in the fractional result. */
Dos94TraceSlope Dos94_math2_calcEdgeSlope(uint32_t n, uint32_t d) {
	if (!(n >> 16) && d >> 16) {
		uint32_t fraction =
			Dos94Assets_Version() == XW_GAME_VERSION_93 ? Dos93_math2_divide32u(n << 16, d) : (n << 16) / d;
		return (Dos94TraceSlope) { 0, (uint16_t)fraction };
	}
	if (!n)
		return (Dos94TraceSlope) { 0, 0 };
	if (n >> 16) {
		if (d >> 24) {
			d >>= 8;
			n >>= 8;
		}
		while (d >> 16) {
			d >>= 1;
			n >>= 1;
		}
	}
	if (!d)
		return (Dos94TraceSlope) { 0x7FFFFFFF, 0 };
	if (d == 1)
		return (Dos94TraceSlope) { n, 0 };
	uint32_t quotient = n / d, remainder = n % d;
	if ((n >> 16) >= d)
		return (Dos94TraceSlope) { quotient + (remainder > d / 2), (uint16_t)remainder };
	return (Dos94TraceSlope) { quotient, (uint16_t)((remainder << 16) / d) };
}

static uint32_t divide32(uint32_t n, uint32_t d) {
	return Dos94Assets_Version() == XW_GAME_VERSION_93 ? Dos93_math2_divide32u(n, d) : d ? n / d : n;
}

static uint32_t divide_fixed(uint32_t n, uint32_t d) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93)
		return Dos93_math2_DivideByFixed16(n, d);
	return (n >> 16) < d ? (uint32_t)(((uint64_t)n << 16) / d) : n << 16;
}

static void swap_light(Dos94TraceLine* l) {
	uint16_t t = l->light1;
	l->light1 = l->light2;
	l->light2 = t;
}

static uint8_t clip_row(int32_t y) {
	return y < 0 ? 0 : y >= g_flightVpHeight ? g_flightVpHeight : (uint8_t)y;
}

/* Common branch selection in DOS94 0x69DAEF and 0x69DE94. Each returned
 * branch has its own integer-width and lighting adjustment below. */
static int select_endpoint(Dos94TraceLine* l, const Dos94ScreenPoint** point) {
	Dos94ScreenPoint a = l->first, b = l->second;
	bool aInside = a.y >= 0 && a.y < g_flightVpHeight;
	bool bInside = b.y >= 0 && b.y < g_flightVpHeight;
	if (aInside || bInside) {
		bool reverse = !aInside || (bInside && b.y < a.y);
		*point = reverse ? &l->second : &l->first;
		l->startY = (uint8_t)(*point)->y;
		l->endY = clip_row(reverse ? a.y : b.y);
		if (reverse) {
			swap_light(l);
			l->xDirection = -l->xDirection;
		}
		return 0;
	}
	/* These distinctions follow the original high-word branches, including
	 * their short arithmetic for an endpoint whose high word equals -1. */
	bool reverse;
	int mode;
	if (!(a.y >> 16)) {
		reverse = false;
		mode = 3;
	} else if ((b.y >> 16) < 0) {
		reverse = true;
		mode = (b.y >> 16) == -1 ? 2 : 1;
	} else if (b.y >> 16) {
		reverse = false;
		mode = (a.y >> 16) == -1 ? 2 : 1;
	} else {
		reverse = true;
		mode = 3;
	}
	*point = reverse ? &l->second : &l->first;
	if (reverse) {
		swap_light(l);
		l->xDirection = -l->xDirection;
	}
	l->startY = mode == 3 ? g_flightVpHeight : 0;
	l->endY = mode == 3 ? 0 : g_flightVpHeight;
	return mode;
}

static uint32_t clip_distance(Dos94TraceLine* l, const Dos94ScreenPoint* p, int mode) {
	uint32_t distance = mode == 3   ? (uint16_t)((uint16_t)p->y - g_flightVpHeight)
						: mode == 2 ? (uint16_t)-(uint16_t)p->y
									: 0u - (uint32_t)p->y;
	uint32_t lightDistance = mode == 1 ? distance : distance & ~1u;
	uint16_t change = (uint16_t)(lightDistance * (uint32_t)(int32_t)l->lightIncY);
	l->light1 = (uint16_t)(mode == 3 ? l->light1 - change : l->light1 + change);
	return distance;
}

/* DOS94 0x69DAEF. The return sign describes horizontal rejection. */
int Dos94_TRACE2_ydomclipy(Dos94TraceLine* l) {
	const Dos94ScreenPoint* point;
	int mode = select_endpoint(l, &point);
	uint32_t whole = l->slope.whole;
	/* DOS93's short, very-steep outside branches retain the incoming direction. */
	if (Dos94Assets_Version() == XW_GAME_VERSION_93 && (whole >> 16) && mode >= 2 && point == &l->second)
		l->xDirection = -l->xDirection;
	uint32_t x = (uint32_t)point->x;
	if (mode) {
		uint32_t distance = clip_distance(l, point, mode), delta = 0;
		if (whole >> 16) {
			if (mode == 1)
				delta = divide32(distance, whole);
			l->slope.whole = 0xFFFF;
		} else {
			uint32_t divisor = (whole << 16) | l->slope.fraction;
			delta = mode == 1 ? divide_fixed(distance, divisor) : divide32(distance << 16, divisor);
		}
		x += l->xDirection < 0 ? 0u - delta : delta;
	} else if (whole >> 16)
		l->slope.whole = 0xFFFF;
	l->unclippedX = (int32_t)x;
	int16_t high = (int16_t)(x >> 16);
	uint16_t low = (uint16_t)x;
	if (high < 0) {
		if (l->xDirection < 0 || high != -1 || low <= (uint16_t)-g_flightVpHeight)
			return -1;
	} else if (high)
		return 1;
	else if (!low) {
		if (l->xDirection < 0)
			return -1;
	} else {
		if (low > (uint16_t)(g_flightVpWidth + g_flightVpHeight))
			return 1;
		if (l->xDirection > 0 && low >= g_flightVpWidth)
			return 1;
	}
	l->startX = (int16_t)low;
	return 0;
}

static int classify_x_start(Dos94TraceLine* l) {
	uint32_t x = (uint32_t)l->unclippedX;
	uint16_t whole = (uint16_t)l->slope.whole;
	int16_t high = (int16_t)(x >> 16);
	uint32_t distance;
	int side;
	if (high < 0) {
		if (l->xDirection < 0)
			return -1;
		distance = 0u - x;
		side = -1;
		if (distance >> 24 || (distance >> 8) > whole)
			return -1;
	} else {
		if (!high && (uint16_t)x < g_flightVpWidth) {
			if (!x && l->xDirection < 0)
				return -1;
			l->startX = (int16_t)x;
			l->entryDistance = 0;
			return 0;
		}
		if (l->xDirection > 0)
			return 1;
		distance = x - g_flightVpMaxX;
		side = 1;
		if (!high) {
			if (!(whole >> 8) && (distance >> 8) > whole)
				return 1;
		} else if (distance >> 24 || (distance >> 8) >= whole)
			return 1;
	}
	uint32_t numerator, denominator = ((uint32_t)whole << 16) | l->slope.fraction;
	if (whole >> 8) {
		numerator = distance << 8;
		denominator >>= 8;
	} else
		numerator = (distance << 16) | (distance >> 16);
	l->entryDistance = (uint16_t)(divide32(numerator, denominator) + 1);
	l->startX = side < 0 ? 0 : g_flightVpMaxX;
	return 0;
}

/* DOS94 0x69DE94: fixed multiplication, not the Y-dominant division. */
int Dos94_TRACE2_xdomclipy(Dos94TraceLine* l) {
	const Dos94ScreenPoint* point;
	int mode = select_endpoint(l, &point);
	uint32_t x = (uint32_t)point->x;
	if (mode) {
		uint32_t distance = clip_distance(l, point, mode);
		uint32_t fixed = ((uint32_t)(uint16_t)l->slope.whole << 16) | l->slope.fraction;
		uint32_t delta = (uint32_t)(((uint64_t)distance * fixed) >> 16);
		x += l->xDirection < 0 ? 0u - delta : delta;
	}
	l->unclippedX = (int32_t)x;
	return classify_x_start(l);
}
