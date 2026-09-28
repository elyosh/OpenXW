#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/trace_internal.h"

static int16_t gradient(int16_t first, int16_t second, uint32_t distance, int direction) {
	int16_t difference = (int16_t)(second - first);
	uint16_t value = (difference < 0 ? (uint16_t)-difference : (uint16_t)difference) >> 1;
	if (value <= distance)
		return 0;
	if ((uint16_t)distance)
		value /= (uint16_t)distance;
	if (value >> 8)
		value &= 0xFFFE;
	if ((difference < 0) != (direction < 0))
		value = (uint16_t)-value;
	return (int16_t)(uint16_t)(2u * value);
}

static int32_t add(int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); }

static bool y_visible(Dos94ScreenPoint a, Dos94ScreenPoint b) {
	return !(a.y < 0 && b.y < 0) && !(a.y >= g_flightVpHeight && b.y >= g_flightVpHeight);
}

static void line_edge(Dos94ScreenPoint a, Dos94ScreenPoint b, uint32_t dx, uint32_t dy, uint16_t tag,
					  uint16_t light1, uint16_t light2, int xSign, int ySign, bool forceX) {
	Dos94Raster* r = &Dos94_display->raster;
	bool yDominant = !forceX && (dy >> 1) >= dx;
	Dos94TraceSlope slope = (dy >> 1) == dx && !forceX
								? (Dos94TraceSlope) { 2, 0 }
								: Dos94_math2_calcEdgeSlope(yDominant ? dy : dx, yDominant ? dx : dy);
	Dos94TraceLine line = { .first = a,
							.second = b,
							.slope = slope,
							.tag = tag,
							.light1 = light1,
							.light2 = light2,
							.lightIncX = r->lightIncX,
							.lightIncY = r->lightIncY,
							.xDirection = xSign,
							.yDirection = ySign };
	if (yDominant)
		Dos94_TRACE2_ydomedge(&line);
	else
		Dos94_TRACE2_xdomedge(&line);
}

static void cap(Dos94ScreenPoint point, uint16_t thickness, uint16_t light, int16_t increment, uint16_t tag) {
	if (Dos94_display->raster.dos93) {
		if (point.x >= g_flightVpWidth)
			return;
		if (point.x < 0)
			point.x = 0;
	}
	if (point.y >= g_flightVpHeight)
		return;
	int32_t end = add(point.y, thickness);
	if (end < 0)
		return;
	uint16_t start = point.y < 0 ? 0 : point.y;
	if (end > g_flightVpHeight)
		end = g_flightVpHeight;
	if (point.y < 0)
		light = (uint16_t)(light - (uint32_t)point.y * (uint32_t)(int32_t)increment);
	Dos94_TRACE2_entervertedge(start, (uint16_t)(end - start), (uint16_t)point.x, light, tag);
}

static void clip_x(Dos94ScreenPoint* point, uint32_t dx, uint32_t dy, int ySign, int16_t increment,
				   uint16_t* light, int16_t* original, uint16_t* temporary) {
	if (point->x >= 0 && point->x <= g_flightVpWidth)
		return;
	uint32_t distance = point->x < 0 ? 0u - (uint32_t)point->x : (uint32_t)point->x - g_flightVpWidth;
	uint32_t delta = dx ? (uint32_t)(((uint64_t)distance * dy) / dx) : 0;
	if (ySign < 0)
		delta = 0u - delta;
	point->y = add(point->y, (int32_t)delta);
	uint16_t adjustment = (uint16_t)(delta * (uint32_t)(int32_t)increment);
	*light = (uint16_t)(*light + adjustment);
	*original = (int16_t)(*original + adjustment);
	*temporary = (uint16_t)(*temporary + adjustment);
	point->x = point->x < 0 ? 0 : g_flightVpWidth;
}

/* DOS94 0x69BE00. Width is offset on the minor axis, with separate end caps. */
void Dos94_DRAWLN2_tracelineedges(Dos94ScreenPoint* a, Dos94ScreenPoint* b, uint16_t thickness,
								  int16_t original1, int16_t original2, uint16_t tag) {
	int xs = b->x < a->x ? -1 : 1, ys = b->y < a->y ? -1 : 1;
	uint32_t dx = xs < 0 ? (uint32_t)a->x - (uint32_t)b->x : (uint32_t)b->x - (uint32_t)a->x;
	uint32_t dy = ys < 0 ? (uint32_t)a->y - (uint32_t)b->y : (uint32_t)b->y - (uint32_t)a->y;
	int32_t minX = xs < 0 ? b->x : a->x, maxX = xs < 0 ? a->x : b->x;
	int32_t minY = ys < 0 ? b->y : a->y, maxY = ys < 0 ? a->y : b->y;
	if (maxY < -(int32_t)thickness || minY > g_flightVpHeight + (int32_t)thickness ||
		maxX < -(int32_t)thickness || minX > g_flightVpWidth + (int32_t)thickness)
		return;
	Dos94Raster* r = &Dos94_display->raster;
	if (r->flatObjectNumber == r->flatLimit)
		return;
	++r->flatObjectNumber;
	if (r->dos93)
		original1 = original2 = 0;
	uint16_t light1 = original1 < 0 ? 0 : original1, light2 = original2 < 0 ? 0 : original2;
	uint16_t temporary1 = light1, temporary2 = light2;
	if (original1 == original2)
		original1 = original2 = original1 < 0 ? (int16_t)-original1 : 0;
	r->lineLight1 = original1;
	r->lineLight2 = original2;
	uint16_t half = (uint16_t)(thickness + 1) >> 1;
	if (dy > dx) {
		r->lightIncX = r->lightIncY = 0;
		if (original1 != original2)
			r->lightIncY = gradient(light1, light2, dy, ys);
		r->lineLightIncY = r->lightIncY;
		if (!y_visible(*a, *b))
			return;
		a->x = add(a->x, -half);
		b->x = add(b->x, -half);
		if ((a->x >= g_flightVpWidth && b->x >= g_flightVpWidth) ||
			(a->x <= -(int32_t)thickness && b->x <= -(int32_t)thickness))
			return;
		if (original1 != original2 && (dy >> 1) >= dx)
			r->lightIncX = gradient(light1, light2, dx, xs);
		if (r->dos93 && a->x < 0 && b->x < 0) {
			uint16_t first = a->y < 0 ? 0 : a->y > g_flightVpHeight ? g_flightVpHeight : a->y;
			uint16_t last = b->y < 0 ? 0 : b->y > g_flightVpHeight ? g_flightVpHeight : b->y;
			Dos94_TRACE2_entervertedge(first < last ? first : last,
									   first < last ? last - first : first - last, 0, 0, tag);
		} else
			line_edge(*a, *b, dx, dy, tag, light1, light2, xs, ys, false);
		a->x = add(a->x, thickness);
		b->x = add(b->x, thickness);
		light1 = original1 < 0 ? 0 : original1;
		light2 = original2 < 0 ? 0 : original2;
		line_edge(*a, *b, dx, dy, tag, light1, light2, xs, ys, false);
		return;
	}
	if ((a->x < 0 && b->x < 0) || (a->x >= g_flightVpWidth && b->x >= g_flightVpWidth))
		return;
	int16_t crossGradient = 0;
	if (original1 == original2) {
		crossGradient = gradient(light1, original1, thickness, 1);
		r->lightIncY = 0;
	} else
		r->lightIncY = gradient(light1, light2, dy, ys);
	r->lineLightIncY = crossGradient;
	if (!r->dos93) {
		clip_x(a, dx, dy, ys, r->lightIncY, &light1, &original1, &temporary1);
		clip_x(b, dx, dy, -ys, r->lightIncY, &light2, &original2, &temporary2);
	}
	r->lineLight1 = original1;
	r->lineLight2 = original2;
	a->y = add(a->y, -half);
	b->y = add(b->y, -half);
	bool firstVisible = y_visible(*a, *b);
	if (firstVisible && dy)
		line_edge(*a, *b, dx, dy, tag, light1, light2, xs, ys, true);
	cap(*a, thickness, temporary1, crossGradient, tag);
	cap(*b, thickness, temporary2, crossGradient, tag);
	a->y = add(a->y, thickness);
	b->y = add(b->y, thickness);
	if (!y_visible(*a, *b) || !dy)
		return;
	light1 = original1 < 0 ? 0 : original1;
	light2 = original2 < 0 ? 0 : original2;
	line_edge(*a, *b, dx, dy, tag, light1, light2, xs, ys, true);
}
