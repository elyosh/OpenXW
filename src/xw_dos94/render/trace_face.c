#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/trace_internal.h"

static int16_t light_increment(int16_t a, int16_t b, uint32_t distance, int8_t direction, bool y) {
	int16_t difference = (int16_t)(b - a);
	uint16_t magnitude = difference < 0 ? (uint16_t)-difference : (uint16_t)difference;
	if (y)
		magnitude >>= 1;
	if (magnitude <= distance)
		return 0;
	uint16_t value = (uint16_t)distance ? magnitude / (uint16_t)distance : magnitude;
	if (y) {
		if (value >> 8)
			value &= 0xFFFE;
	} else
		value &= 0xFFFC;
	if ((difference < 0) != (direction < 0))
		value = (uint16_t)-value;
	return (int16_t)(y ? (uint16_t)(value * 2u) : value);
}

static void edge_slope(Dos94ProjectedEdge* e) {
	if (e->slopeCached)
		return;
	e->slopeCached = true;
	e->yDominant = (e->yDifference >> 1) >= e->xDifference;
	Dos94TraceSlope slope;
	if ((e->yDifference >> 1) == e->xDifference)
		slope = (Dos94TraceSlope) { 2, 0 };
	else
		slope = Dos94_math2_calcEdgeSlope(e->yDominant ? e->yDifference : e->xDifference,
										  e->yDominant ? e->xDifference : e->yDifference);
	e->slope = slope.whole;
	e->fraction = slope.fraction;
}

static uint16_t clipped_row(int32_t y) { return y < 0 ? 0 : y >= g_flightVpHeight ? g_flightVpHeight : y; }

/* DOS94 0x69E758/0x69EA8C: wholly offscreen mesh edges retain lighting
 * continuations; an unlit left edge needs only its X=0 opening/closing run. */
static void offscreen_face(Dos94ProjectedEdge* e, Dos94TraceLine* l, bool gouraud, bool right) {
	uint16_t first = clipped_row(l->first.y), last = clipped_row(l->second.y);
	Dos94ScreenPoint point = l->first;
	uint16_t light = l->light1;
	if (first == last)
		return;
	if (first > last) {
		uint16_t t = first;
		first = last;
		last = t;
		l->xDirection = -l->xDirection;
		point = l->second;
		light = l->light2;
	}
	if (!gouraud) {
		if (!right)
			Dos94_TRACE2_entervertedge(first, last - first, 0, 0, l->tag);
		return;
	}
	if (!first)
		light = (uint16_t)(light - (uint32_t)point.y * (uint32_t)(int32_t)l->lightIncY);
	edge_slope(e);
	uint32_t step = (e->slope << (e->yDominant ? 8 : 7)) | (e->fraction >> (e->yDominant ? 8 : 9));
	uint32_t position = right ? (uint32_t)point.x : 0u - (uint32_t)point.x;
	if (!e->yDominant)
		position <<= 7;
	if (!first) {
		uint32_t dy = 0u - (uint32_t)point.y;
		uint32_t delta = e->yDominant ? (step ? (uint32_t)(((uint64_t)dy << 8) / step) : 0) : dy * step;
		bool add = right ? l->xDirection > 0 : l->xDirection < 0;
		position += add ? delta : 0u - delta;
	}
	uint16_t encodedStep;
	Dos94ClipKind kind = e->yDominant ? DOS_CLIP_Y_OUTSIDE : DOS_CLIP_X_Q7;
	if (e->yDominant) {
		encodedStep = step >> 16 ? 0xFFFF : (uint16_t)step;
	} else {
		if (step >> 16) {
			kind = DOS_CLIP_X_INTEGER;
			encodedStep = (uint16_t)(step >> 7);
			position >>= 7;
		} else
			encodedStep = (uint16_t)step;
	}
	Dos94ClippedState state = { .kind = kind,
								.right = right,
								.increase = right ? l->xDirection > 0 : l->xDirection < 0,
								.step = encodedStep,
								.fraction = encodedStep,
								.rows = last - first,
								.position = position & 0xFFFFFFu };
	Dos94_TRACE2_enterclipped(first, l->tag, light, state);
}

static void submit_edge(Dos94ProjectedEdge* e, Dos94RasterObject* object, uint8_t edgeId, uint8_t faceId,
						uint16_t light1, uint16_t light2, bool gouraud) {
	Dos94Raster* r = &Dos94_display->raster;
	if ((e->flags & DOS94_EDGE_OFF_RIGHT) && !gouraud)
		return;
	++r->edgeIndex;
	object->edges[edgeId].face1 = faceId;
	object->edges[edgeId].face2 = 0;
	if (gouraud)
		r->lightIncY = light_increment((int16_t)light1, (int16_t)light2, e->yDifference, e->yDirection, true);
	object->edges[edgeId].halfLightIncY = gouraud ? (uint16_t)r->lightIncY >> 1 : 0;
	Dos94TraceLine line = { .first = e->first->screen,
							.second = e->second->screen,
							.light1 = light1,
							.light2 = light2,
							.tag = (uint16_t)((r->objectNumber << 8) | edgeId),
							.lightIncX = r->lightIncX,
							.lightIncY = r->lightIncY,
							.xDirection = e->xDirection,
							.yDirection = e->yDirection };
	if (e->flags & (DOS94_EDGE_OFF_LEFT | DOS94_EDGE_OFF_RIGHT)) {
		offscreen_face(e, &line, gouraud, (e->flags & DOS94_EDGE_OFF_RIGHT) != 0);
		return;
	}
	edge_slope(e);
	line.slope = (Dos94TraceSlope) { e->slope, e->fraction };
	if (e->yDominant) {
		if (gouraud)
			r->lightIncX =
				light_increment((int16_t)light1, (int16_t)light2, e->xDifference, e->xDirection, false);
		line.lightIncX = r->lightIncX;
		Dos94_TRACE2_ydomedge(&line);
	} else
		Dos94_TRACE2_xdomedge(&line);
}

static Dos94ProjectedPoint* crossing_point(Dos94MeshProjection* p, const uint8_t* vertices, unsigned edge) {
	Dos94ProjectedEdge* e = &p->edges[vertices[2 * edge + 1]];
	Dos94ProjectedPoint* visible = p->vertexScreen[vertices[2 * edge]];
	if (!visible)
		visible = p->vertexScreen[vertices[2 * edge + 2]];
	return e->first == visible ? e->second : e->first;
}

/* DOS94 0x69EF92: join the first two transitions, in the original walk order. */
static void close_face(Dos94MeshProjection* p, Dos94RasterObject* object, const uint8_t* vertices,
					   uint8_t count, uint8_t faceId, bool gouraud) {
	Dos94Raster* r = &Dos94_display->raster;
	Dos94ProjectedPoint* points[2] = { NULL, NULL };
	unsigned found = 0;
	for (unsigned i = 0; i < count && found < 2; ++i) {
		if ((p->vertexScreen[vertices[2 * i]] != NULL) != (p->vertexScreen[vertices[2 * i + 2]] != NULL))
			points[found++] = crossing_point(p, vertices, i);
	}
	if (found != 2 || !points[0] || !points[1])
		return;
	Dos94ScreenPoint a = points[0]->screen, b = points[1]->screen;
	if (a.y == b.y || (a.y < 0 && b.y < 0) || (a.y >= g_flightVpHeight && b.y >= g_flightVpHeight))
		return;
	bool left = a.x <= 0 && b.x <= 0, right = a.x >= g_flightVpWidth && b.x >= g_flightVpWidth;
	if (right && !gouraud)
		return;
	if (!r->lastEdge) {
		unsigned index = 0;
		while (index + 1 < p->mesh.edgeCount && (p->edges[index].flags & 0x2A))
			++index;
		r->lastEdge = index;
	}
	unsigned slot = r->lastEdge;
	Dos94ProjectedEdge closing = { .first = points[0],
								   .second = points[1],
								   .xDirection = b.x < a.x ? -1 : 1,
								   .yDirection = b.y < a.y ? -1 : 1,
								   .flags = left    ? DOS94_EDGE_OFF_LEFT
											: right ? DOS94_EDGE_OFF_RIGHT
													: DOS94_EDGE_VISIBLE };
	closing.xDifference =
		closing.xDirection < 0 ? (uint32_t)a.x - (uint32_t)b.x : (uint32_t)b.x - (uint32_t)a.x;
	closing.yDifference =
		closing.yDirection < 0 ? (uint32_t)a.y - (uint32_t)b.y : (uint32_t)b.y - (uint32_t)a.y;
	/* Closure reuses a selected edge slot and does not consume a new edge index. */
	uint16_t edgeIndex = r->edgeIndex;
	submit_edge(&closing, object, (uint8_t)slot, faceId, (uint16_t)points[0]->light,
				(uint16_t)points[1]->light, gouraud);
	r->edgeIndex = edgeIndex;
	p->edges[slot].flags |= DOS94_EDGE_TRACED;
	r->lastEdge = 0;
}

/* DOS94 0x69E66D. Face IDs are one-based; shared edges attach the second face. */
void Dos94_TRACE2_drawface(Dos94MeshProjection* p, Dos94Lighting* l, const Dos94FaceView* face,
						   Dos94RasterObject* object, uint8_t faceId) {
	const uint8_t* stream = Dos94Assets_Data(face->stream);
	unsigned count = face->vertexCount;
	if (!stream || !count)
		return;
	const uint8_t* vertices = stream + 1;
	unsigned behind = 0;
	for (unsigned i = 0; i < count; ++i) {
		uint8_t id = vertices[2 * i + 1], first = vertices[2 * i], second = vertices[2 * i + 2];
		Dos94ProjectedEdge* e = &p->edges[id];
		if (!p->vertexScreen[first])
			++behind;
		unsigned rejected = DOS94_EDGE_OFF_Y | (Dos94_display->raster.dos93 ? DOS94_EDGE_OFF_RIGHT : 0);
		if (e->flags == 0x80 || e->flags == 0xA0 || (e->flags & rejected)) {
			if (!(e->flags & DOS94_EDGE_TRACED))
				Dos94_display->raster.lastEdge = id;
			continue;
		}
		if (e->flags & DOS94_EDGE_TRACED) {
			object->edges[id].face2 = faceId;
			continue;
		}
		e->flags |= DOS94_EDGE_TRACED;
		uint16_t light1 = (uint16_t)l->vertexLight[first], light2 = (uint16_t)l->vertexLight[second];
		if (p->vertexScreen[first] == e->second) {
			uint16_t t = light1;
			light1 = light2;
			light2 = t;
		}
		if (!e->first || !e->second) {
			p->failed = true;
			return;
		}
		submit_edge(e, object, id, faceId, light1, light2, l->gouraud);
	}
	if (behind && behind < count)
		close_face(p, object, vertices, count, faceId, l->gouraud);
}
