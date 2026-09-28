#include "xw/render/flight_view.h"
#include "xw_dos94/render/dos93_math.h"
#include "xw_dos94/render/dos93_trace.h"
#include "xw_dos94/render/raster.h"
#include "xw_dos94/render/trace_internal.h"

/* DOS94 compressed offscreen entries from 0x69F841 and 0x69FE0F. */
void Dos94Trace_Outside(Dos94TraceLine* l, bool right, bool yDominant) {
	bool up = l->startY > l->endY;
	uint8_t count = up ? l->startY - l->endY : l->endY - l->startY;
	if (!count)
		return;
	if (Dos94Assets_Version() == XW_GAME_VERSION_93) {
		if (!right)
			Dos94_TRACE2_entervertedge(up ? l->endY : l->startY, count, 0, 0, l->tag);
		return;
	}
	uint32_t step;
	bool large = false;
	if (yDominant)
		step = l->slope.whole >= 256 ? 0xFFFF : (l->slope.whole << 8) | (l->slope.fraction >> 8);
	else {
		large = l->slope.whole >= 512;
		step = large ? l->slope.whole : (l->slope.whole << 7) | (l->slope.fraction >> 9);
	}
	int direction = l->xDirection * (up ? -1 : 1);
	uint32_t position = (uint32_t)l->unclippedX;
	uint16_t light = l->light1;
	if (!yDominant && !large)
		position <<= 7;
	if (!right)
		position = 0u - position;
	if (up) {
		uint32_t delta = yDominant ? (step ? ((uint32_t)count << 8) / step : 0) : count * step;
		if (yDominant)
			delta = (uint32_t)(int32_t)(int16_t)delta;
		bool add = right ? l->xDirection > 0 : l->xDirection < 0;
		position += add ? delta : 0u - delta;
		if (!right && !yDominant && !large && (int32_t)position < 0)
			position = 0;
		light = (uint16_t)(light - count * l->lightIncY);
	}
	Dos94ClippedState state = { .kind = yDominant ? DOS_CLIP_Y_OUTSIDE
										: large   ? DOS_CLIP_X_INTEGER
												  : DOS_CLIP_X_Q7,
								.right = right,
								.increase = right ? direction > 0 : direction < 0,
								.step = (uint16_t)step,
								.fraction = (uint16_t)step,
								.rows = count,
								.position = position & 0xFFFFFFu };
	Dos94_TRACE2_enterclipped(up ? l->endY : l->startY, l->tag, light, state);
}

static void trace_y_direction(Dos94TraceLine* l, uint16_t x, uint16_t y, uint16_t span, uint16_t step,
							  uint16_t light, bool up, bool right) {
	Dos94TraceYEdge edge = { y, span, x, step, light, l->tag, l->lightIncX, l->lightIncY };
	if (Dos94Assets_Version() == XW_GAME_VERSION_93) {
		Dos93_TRACE2_yedge(edge, up, right);
		return;
	}
	if (up) {
		if (right)
			Dos94_TRACE2_yupright(edge);
		else
			Dos94_TRACE2_yupleft(edge);
	} else {
		if (right)
			Dos94_TRACE2_ydownright(edge);
		else
			Dos94_TRACE2_ydownleft(edge);
	}
}

/* DOS94 0x69F832. */
void Dos94_TRACE2_ydomedge(Dos94TraceLine* l) {
	int outside = Dos94_TRACE2_ydomclipy(l);
	if (outside) {
		Dos94Trace_Outside(l, outside > 0, true);
		return;
	}
	bool up = l->startY > l->endY;
	unsigned count = up ? l->startY - l->endY : l->endY - l->startY;
	if (!count)
		return;
	if (l->slope.whole >= g_flightVpHeight) {
		if (l->startX < 0 || l->startX >= g_flightVpWidth) {
			l->slope = (Dos94TraceSlope) { 255, 0xFFFF };
			Dos94Trace_Outside(l, l->startX >= g_flightVpWidth, true);
			return;
		}
		uint16_t light = l->light1;
		if (up)
			light = l->endY ? l->light2 : (uint16_t)(l->light1 - l->startY * l->lightIncY);
		Dos94_TRACE2_entervertedge(up ? l->endY : l->startY, count, l->startX, light, l->tag);
		return;
	}
	uint16_t step = (uint16_t)((l->slope.whole << 8) | (l->slope.fraction >> 8));
	uint16_t x = (uint16_t)l->startX;
	unsigned distance = 0;
	if (l->startX < 0) {
		distance = (uint16_t)-l->startX;
		x = 0;
	} else if (x > g_flightVpMaxX) {
		distance = x - g_flightVpMaxX;
		x = g_flightVpMaxX;
	}
	uint32_t entry = distance * step;
	if (entry >> 16) {
		Dos94Trace_Outside(l, x != 0, true);
		return;
	}
	uint16_t span = (uint16_t)(count << 8);
	if (entry >> 8) {
		if (span <= entry) {
			Dos94Trace_Outside(l, x != 0, true);
			return;
		}
		uint16_t light = (uint16_t)(l->light1 + (up ? -1 : 1) * (int)(entry >> 8) * l->lightIncY);
		uint16_t start = (uint16_t)(((uint16_t)l->startY << 8) + (up ? 0u - entry : entry));
		trace_y_direction(l, x, start, (uint16_t)(span - entry), step, light, up, x == 0);
		l->endY = (uint8_t)(start >> 8);
		if (up || x)
			l->light2 = light;
		Dos94Trace_Outside(l, x != 0, true);
		return;
	}
	trace_y_direction(l, x, (uint16_t)(((uint16_t)l->startY << 8) | (uint8_t)entry), span, step, l->light1,
					  up, l->xDirection > 0);
}

static void trace_x_direction(Dos94TraceLine* l, uint16_t x, uint16_t slope, uint16_t light, uint8_t y,
							  uint8_t rows, bool skip, bool up, bool right) {
	Dos94TraceXEdge edge = { x, slope, light, l->tag, l->lightIncY, y, rows, skip };
	if (Dos94Assets_Version() == XW_GAME_VERSION_93) {
		Dos93_TRACE2_xedge(edge, up, right);
		return;
	}
	if (up) {
		if (right)
			Dos94_TRACE2_xupright(edge);
		else
			Dos94_TRACE2_xupleft(edge);
	} else {
		if (right)
			Dos94_TRACE2_xdownright(edge);
		else
			Dos94_TRACE2_xdownleft(edge);
	}
}

/* Large integer slopes use the original narrow X=0 closure cases at 0x69FBC3. */
static void very_large_x(Dos94TraceLine* l) {
	Dos94ScreenPoint a = l->first, b = l->second;
	int32_t ah = a.x >> 16, bh = b.x >> 16;
	bool leftCross = false;
	if (!ah)
		leftCross = l->xDirection < 0;
	else if (ah == -1 && !bh)
		leftCross = l->xDirection > 0;
	else if (!bh)
		leftCross = l->xDirection > 0;
	if (leftCross) {
		Dos94ScreenPoint top = l->yDirection > 0 ? a : b, bottom = l->yDirection > 0 ? b : a;
		if (top.y >= g_flightVpHeight || bottom.y < 0)
			return;
		uint16_t first = top.y < 0 ? 0 : top.y;
		uint16_t end = bottom.y >= g_flightVpHeight ? g_flightVpHeight : bottom.y;
		/* Nonzero top retains BX, the low word of the incoming integer slope. */
		uint16_t light = (uint16_t)l->slope.whole;
		if (!first)
			light = (uint16_t)((l->yDirection > 0 ? l->light1 : l->light2) -
							   (uint32_t)top.y * (uint32_t)(int32_t)l->lightIncY);
		Dos94_TRACE2_entervertedge(first, (uint16_t)(end - first), 0, light, l->tag);
		return;
	}
	if (!ah || !bh || bh == -1 || (ah == -1 && bh))
		return;
	Dos94ScreenPoint p = bh < 0 ? b : a;
	uint32_t numerator = 0u - (uint32_t)p.x;
	uint32_t count = Dos94Assets_Version() == XW_GAME_VERSION_93
						 ? Dos93_math2_divide32u(numerator, l->slope.whole)
					 : l->slope.whole ? numerator / l->slope.whole
									  : numerator;
	bool subtract = bh < 0 ? l->yDirection > 0 : l->yDirection < 0;
	uint32_t top = (uint32_t)p.y - (subtract ? count : 0);
	if ((subtract && top >> 16) || (uint16_t)top >= g_flightVpHeight)
		return;
	uint16_t light = (bh < 0) == subtract ? l->light1 : l->light2;
	Dos94_TRACE2_entervertedge((uint16_t)top, (uint16_t)count, 0, light, l->tag);
}

/* DOS94 0x69FBBA. */
void Dos94_TRACE2_xdomedge(Dos94TraceLine* l) {
	if (l->slope.whole >> 16) {
		very_large_x(l);
		return;
	}
	int outside = Dos94_TRACE2_xdomclipy(l);
	if (outside) {
		Dos94Trace_Outside(l, outside > 0, false);
		return;
	}
	bool up = l->startY > l->endY;
	uint8_t count = up ? l->startY - l->endY : l->endY - l->startY;
	if (!count)
		return;
	uint16_t step =
		l->slope.whole >= 512 ? 0xFFFF : (uint16_t)((l->slope.whole << 7) | (l->slope.fraction >> 9));
	uint16_t x = (uint16_t)((uint16_t)l->startX << 7);
	if (l->entryDistance >> 8) {
		Dos94Trace_Outside(l, x != 0, false);
		return;
	}
	uint8_t skip = (uint8_t)l->entryDistance;
	if (skip) {
		--skip;
		if (count <= skip) {
			Dos94Trace_Outside(l, x != 0, false);
			return;
		}
		uint16_t light = (uint16_t)(l->light1 + (up ? -1 : 1) * skip * l->lightIncY);
		uint8_t start = (uint8_t)(l->startY + (up ? -skip : skip));
		trace_x_direction(l, x, step, light, start, count - skip, true, up, x == 0);
		l->endY = start;
		if (up)
			l->light2 = light;
		Dos94Trace_Outside(l, x != 0, false);
		return;
	}
	trace_x_direction(l, x, step, l->light1, l->startY, count, false, up, l->xDirection > 0);
}
