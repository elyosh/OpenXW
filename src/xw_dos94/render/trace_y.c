#include "xw/render/flight_view.h"
#include "xw_dos94/render/raster.h"
#include "xw_dos94/render/trace2.h"

static void clipped_y(Dos94TraceYEdge edge, uint16_t y, uint16_t remaining, uint16_t light, uint16_t x,
					  bool up, bool right, bool initial) {
	uint16_t top = up ? (uint16_t)(y - remaining) : y;
	if (!initial && !((uint16_t)(remaining + (uint8_t)top) >> 8))
		return;
	uint16_t distance = x;
	if (up) {
		if (!edge.slopeQ8)
			return;
		distance = remaining / edge.slopeQ8;
		int16_t change = (int16_t)((int16_t)distance * edge.lightIncX);
		light = (uint16_t)(right ? light + change : light - change);
	} else if (right)
		distance = (uint16_t)(x - g_flightVpWidth);
	Dos94ClippedState state = { .kind = DOS_CLIP_Y_EXIT,
								.right = right,
								.increase = !up,
								.step = edge.slopeQ8,
								.fraction = (uint16_t)(edge.slopeQ8 - (uint8_t)top),
								.rows = (uint16_t)(remaining + (uint8_t)top) >> 8,
								.position = (uint8_t)distance + (right ? g_flightVpWidth : 0) };
	Dos94_TRACE2_enterclipped(top >> 8, edge.tag, light, state);
}

static uint16_t light_step(int16_t increment, bool reverse, bool half) {
	if (reverse)
		return (uint16_t)(-(increment >> (half ? 2 : 1))) & 0x7FFF;
	return half ? (uint16_t)(increment >> 1) >> 1 : (uint16_t)increment >> 1;
}

/* DOS94 0x69C9B8–0x69D5A1: each X position contributes a vertical run.
 * The packed accumulator retains Y's eight fractional bits and light's guard bits. */
static void trace_y(Dos94TraceYEdge edge, bool up, bool right) {
	uint16_t x = edge.x;
	if (!right && !x) {
		clipped_y(edge, edge.startYQ8, edge.spanYQ8, edge.light, x, up, right, true);
		return;
	}
	uint32_t position = ((uint32_t)edge.startYQ8 << 16) | (edge.light >> 1);
	uint32_t remaining = (uint32_t)edge.spanYQ8 << 16;
	uint32_t fullStep = (uint32_t)edge.slopeQ8 << 16;
	bool first = true;
	for (;;) {
		uint32_t step = first ? fullStep >> 1 : fullStep;
		uint16_t increment = light_step(edge.lightIncX, up == right, first);
		if (up) {
			step = (step & 0xFFFF0000u) | increment;
			remaining = (remaining & 0xFFFF0000u) | increment;
			if (remaining < step) {
				step = remaining;
				increment = (uint16_t)((int16_t)(remaining >> 24) * edge.lightIncY) >> 1;
				step = (step & 0xFFFF0000u) | increment;
			}
		} else {
			/* The first downward run selects its geometric extent before adding light. */
			if (!first) {
				step = (step & 0xFFFF0000u) | increment;
				remaining = (remaining & 0xFFFF0000u) | increment;
			}
			if (remaining < step)
				step = remaining;
		}
		uint32_t before = position;
		uint32_t after;
		if (up)
			after = ((position | 0xC000u) - step) & 0xFFFF3FFFu;
		else
			after = position + step;
		uint16_t row = (uint16_t)((up ? after : before) >> 24);
		uint16_t count = (uint16_t)(up ? (before >> 24) - (after >> 24) : (after >> 24) - (before >> 24));
		if (!count)
			return;
		uint16_t light = (uint16_t)(up ? after : before);
		Dos94_TRACE2_entervertedge(row, count, x, (uint16_t)(light * 2u), edge.tag);
		if (up)
			position = after;
		else {
			if (first)
				step = (step & 0xFFFF0000u) | increment;
			position += step;
			if (position < before)
				return;
			position &= 0xFFFF3FFFu;
			if (position >= ((uint32_t)g_flightVpHeight << 24))
				return;
		}
		remaining = (remaining & 0xFFFF0000u) | (uint16_t)step;
		remaining -= step;
		if (!remaining)
			return;
		x = (uint16_t)(right ? x + 1 : x - 1);
		if ((!right && !x) || (right && x >= g_flightVpWidth)) {
			clipped_y(edge, (uint16_t)(position >> 16), (uint16_t)(remaining >> 16),
					  (uint16_t)((uint16_t)position * 2u), x, up, right, false);
			return;
		}
		first = false;
	}
}

void Dos94_TRACE2_ydownleft(Dos94TraceYEdge edge) { trace_y(edge, false, false); }

void Dos94_TRACE2_ydownright(Dos94TraceYEdge edge) { trace_y(edge, false, true); }

void Dos94_TRACE2_yupleft(Dos94TraceYEdge edge) { trace_y(edge, true, false); }

void Dos94_TRACE2_yupright(Dos94TraceYEdge edge) { trace_y(edge, true, true); }
