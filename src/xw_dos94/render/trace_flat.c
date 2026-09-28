#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/trace_internal.h"

static uint16_t clip_y(int32_t y) { return y < 0 ? 0 : y >= g_flightVpHeight ? g_flightVpHeight : y; }

static void flat_edge(Dos94ScreenPoint a, Dos94ScreenPoint b, uint16_t tag) {
	if (a.y == b.y || (a.y < 0 && b.y < 0) || (a.y >= g_flightVpHeight && b.y >= g_flightVpHeight))
		return;
	if (a.x <= 0 && b.x <= 0) {
		uint16_t y1 = clip_y(a.y), y2 = clip_y(b.y);
		Dos94_TRACE2_entervertedge(y1 < y2 ? y1 : y2, y1 < y2 ? y2 - y1 : y1 - y2, 0, 0, tag);
		return;
	}
	if (a.x >= g_flightVpWidth && b.x >= g_flightVpWidth)
		return;
	uint32_t dx = b.x < a.x ? (uint32_t)a.x - (uint32_t)b.x : (uint32_t)b.x - (uint32_t)a.x;
	uint32_t dy = b.y < a.y ? (uint32_t)a.y - (uint32_t)b.y : (uint32_t)b.y - (uint32_t)a.y;
	if (!dx) {
		Dos94ScreenPoint top = a.y < b.y ? a : b;
		uint16_t first = (uint16_t)top.y;
		if (top.y >> 16) {
			dy += (uint32_t)top.y;
			first = 0;
		}
		uint16_t count = dy >> 8 ? g_flightVpHeight : dy;
		if (count)
			Dos94_TRACE2_entervertedge(first, count, top.x < 0 ? 0 : (uint16_t)top.x, 0, tag);
		return;
	}
	bool yDominant = (dy >> 1) >= dx;
	Dos94TraceSlope slope = (dy >> 1) == dx
								? (Dos94TraceSlope) { 2, 0 }
								: Dos94_math2_calcEdgeSlope(yDominant ? dy : dx, yDominant ? dx : dy);
	Dos94Raster* r = &Dos94_display->raster;
	Dos94TraceLine line = { .first = a,
							.second = b,
							.slope = slope,
							.tag = tag,
							.lightIncX = r->lightIncX,
							.lightIncY = r->lightIncY,
							.xDirection = b.x < a.x ? -1 : 1,
							.yDirection = b.y < a.y ? -1 : 1 };
	if (yDominant)
		Dos94_TRACE2_ydomedge(&line);
	else
		Dos94_TRACE2_xdomedge(&line);
}

/* DOS94 0x69E26D. Walk begins at the minimum-Y vertex, including the closing edge. */
bool Dos94_TRACE2_drawscreencoords(const Dos94ScreenPoint* points, uint16_t count, const Dos94ScreenBounds* b,
								   uint8_t color) {
	if (!count || b->maxX < 0 || b->maxY < 0 || b->minX >= g_flightVpWidth || b->minY >= g_flightVpHeight)
		return false;
	Dos94Raster* r = &Dos94_display->raster;
	if (r->flatObjectNumber == r->flatLimit)
		return false;
	unsigned id = r->flatObjectNumber++;
	r->flat[id].color = color;
	r->flat[id].component = (uint8_t)r->objectNumber;
	r->flat[id].parent = r->parentObject;
	r->flat[id].z = (int16_t)0x8001;
	uint16_t tag = (uint16_t)(((id + 128) << 8) | (uint8_t)r->layer);
	if ((uint8_t)count == (uint8_t)b->sameX) {
		uint16_t first = clip_y(b->minY), last = clip_y(b->maxY);
		uint16_t x = (uint16_t)points[b->maxYPoint].x;
		Dos94_TRACE2_entervertedge(first, (uint16_t)(last - first), x, 0, tag);
		Dos94_TRACE2_entervertedge(first, (uint16_t)(last - first), (uint16_t)(x + 1), 0, tag);
	} else if ((uint8_t)count == (uint8_t)b->sameY) {
		uint16_t y = (uint16_t)b->minY;
		Dos94_TRACE2_enterevent(y, tag, b->minX < 0 ? 0 : b->minX, 0);
		if (b->maxX >= 0 && b->maxX < g_flightVpWidth)
			Dos94_TRACE2_enterevent(y, tag, b->maxX, 0);
	} else
		for (unsigned i = 0; i < count; ++i) {
			unsigned first = (b->minYPoint + i) % count, second = (first + 1) % count;
			flat_edge(points[first], points[second], tag);
		}
	return true;
}
