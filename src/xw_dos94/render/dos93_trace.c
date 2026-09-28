#include "xw_dos94/render/dos93_trace.h"
#include "xw/render/flight_view.h"

static void close_y(uint16_t y, uint16_t span, uint16_t tag, bool up) {
	uint16_t end = (uint16_t)(up ? y - span : y + span);
	uint8_t count = up ? (y >> 8) - (end >> 8) : (end >> 8) - (y >> 8);
	Dos94_TRACE2_entervertedge((up ? end : y) >> 8, count, 0, 0, tag);
}

/* DOS93 TRACE2_ydownleft/right and yupleft/right. The residual high byte
 * controls termination; the final fractional carry emits an ordinary event. */
void Dos93_TRACE2_yedge(Dos94TraceYEdge edge, bool up, bool right) {
	uint16_t x = edge.x, y = edge.startYQ8, remaining = edge.spanYQ8;
	uint16_t step = edge.slopeQ8 >> 1;
	if (!right && !x) {
		close_y(y, remaining, edge.tag, up);
		return;
	}
	for (;;) {
		if (remaining < step)
			step = remaining;
		uint16_t before = y;
		y = (uint16_t)(up ? y - step : y + step);
		if (up ? before < step : (unsigned)before + step > UINT16_MAX || y >= (g_flightVpHeight << 8))
			step = remaining;
		uint8_t rows = up ? (before >> 8) - (y >> 8) : (y >> 8) - (before >> 8);
		/* These inline DOS emitters store a zero run byte too (256-row lifetime). */
		Dos94_TRACE2_entervertedge((up ? y : before) >> 8, rows ? rows : 256, x, 0, edge.tag);
		remaining = (uint16_t)(remaining - step);
		if (!(remaining >> 8)) {
			bool cross =
				up ? (uint8_t)y < (uint8_t)remaining : (unsigned)(uint8_t)y + (uint8_t)remaining > 255;
			if (cross) {
				x = (uint16_t)(right ? x + 1 : x - 1);
				uint8_t row = (uint8_t)((y >> 8) - (up ? 1 : 0));
				Dos94_TRACE2_enterevent(row, edge.tag, x, 0);
			}
			return;
		}
		x = (uint16_t)(right ? x + 1 : x - 1);
		if (right ? x >= g_flightVpWidth : !x) {
			if (!right)
				close_y(y, remaining, edge.tag, up);
			return;
		}
		step = edge.slopeQ8;
	}
}

/* DOS93 TRACE2_xdownleft/right and xupleft/right: X is Q7 throughout. */
void Dos93_TRACE2_xedge(Dos94TraceXEdge edge, bool up, bool right) {
	int row = (int)edge.startY - (up ? 1 : 0);
	if (row < 0)
		return;
	uint16_t x = edge.xQ7, step = edge.skipHalfStep ? 0 : edge.slopeQ7 >> 1;
	uint8_t remaining = edge.rowCount;
	for (;;) {
		bool outside = right ? (unsigned)x + step > UINT16_MAX : x < step;
		x = (uint16_t)(right ? x + step : x - step);
		if (right)
			outside |= x >= (g_flightVpWidth << 7);
		if (outside) {
			if (!right)
				Dos94_TRACE2_entervertedge(up ? (uint8_t)(row + 1 - remaining) : row, remaining, 0, 0,
										   edge.tag);
			return;
		}
		Dos94_TRACE2_enterevent(row, edge.tag, x >> 7, 0);
		if (!--remaining)
			return;
		row += up ? -1 : 1;
		if (row < 0 || row >= g_flightVpHeight)
			return;
		step = edge.slopeQ7;
	}
}
