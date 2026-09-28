#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/ordering.h"
#include "xw_dos94/render/sweep.h"
#include "xw_runtime/runtime/port.h"
#include <string.h>

typedef struct SpanEndpoint {
	uint16_t x, clippedIndex;
	uint8_t light;
	bool clipped;
} SpanEndpoint;

typedef struct ActiveMesh {
	uint8_t faces[16], count, currentFace;
	SpanEndpoint left[16], currentLeft;
} ActiveMesh;

typedef struct Visibility {
	ActiveMesh mesh[128];
	uint16_t stack[256], stackCount, current;
	int16_t sortedTail;
	bool active[256], pending;
	uint16_t row, start, maskCursor, maskEnd;
	int8_t mask;
	unsigned normalCursor, activeCursor;
	uint16_t eventObject, eventEdge, eventX;
	uint8_t eventLight;
} Visibility;

static uint16_t in_front(Visibility* v, uint16_t a, uint16_t b) {
	return Dos94_xtrans2_getinfront(a, b, a < 128 ? v->mesh[a].currentFace : 0,
									b < 128 ? v->mesh[b].currentFace : 0, v->active[128]);
}

static uint16_t nearest(Visibility* v) {
	if (!v->stackCount)
		return 0;
	uint16_t id = v->stack[v->stackCount - 1];
	for (unsigned i = v->stackCount - 1; i; i--)
		id = in_front(v, id, v->stack[i - 1]);
	for (unsigned i = 0; i < v->stackCount; ++i)
		if (v->stack[i] == id) {
			--v->stackCount;
			if (i != v->stackCount) {
				v->stack[i] = v->stack[v->stackCount];
				v->sortedTail = 1;
			}
			if (v->sortedTail > 0)
				--v->sortedTail;
			break;
		}
	return id;
}

static bool matches_face(uint16_t event, uint16_t object, uint8_t face) {
	const Dos94EdgeEvent* e = Dos94Sweep_Event(event);
	if (e->object != object)
		return false;
	const Dos94RasterObject* mesh = Dos94_display->raster.objectById[object];
	const Dos94RasterEdge* edge = &mesh->edges[e->edge];
	return edge->face1 == face || edge->face2 == face;
}

static void right_endpoint(Visibility* v, uint16_t* x, uint8_t* light) {
	Dos94Sweep* s = &Dos94_display->sweep;
	unsigned id = v->current;
	uint8_t face = v->mesh[id].currentFace;
	const Dos94RasterObject* object = Dos94_display->raster.objectById[id];
	const Dos94RasterEdge* edge = &object->edges[(uint8_t)v->eventEdge];
	if (id == v->eventObject && (edge->face1 == face || edge->face2 == face)) {
		*x = v->eventX;
		*light = v->eventLight & 63;
		return;
	}
	unsigned normal = v->normalCursor, active = v->activeCursor;
	while (normal < s->sortedCount || active < s->activeCount) {
		if (normal < s->sortedCount) {
			uint16_t event = s->sorted[normal++].event;
			if (matches_face(event, id, face)) {
				const Dos94EdgeEvent* e = Dos94Sweep_Event(event);
				*x = e->x;
				*light = e->light & 63;
				return;
			}
		}
		if (active < s->activeCount) {
			uint16_t event = s->active[active++].event;
			if (matches_face(event, id, face)) {
				const Dos94EdgeEvent* e = Dos94Sweep_Event(event);
				*x = e->x;
				*light = e->light & 63;
				return;
			}
		}
	}
	*x = 0xFFFF;
	*light = 0;
	for (unsigned i = 0; i < s->rightCount; ++i)
		if (matches_face(s->right[i].event, id, face)) {
			*x = s->right[i].x > 0xFFFF ? 0xFFFF : (uint16_t)s->right[i].x;
			*light = (Dos94Sweep_Event(s->right[i].event)->light >> 9) & 63;
			return;
		}
}

static void output_span(Visibility* v, uint16_t end) {
	if (v->mask < 0 || end <= v->start)
		return;
	Dos94Raster* r = &Dos94_display->raster;
	if (end > g_flightVpWidth)
		end = g_flightVpWidth;
	uint8_t material = r->backgroundColor;
	if (v->current >= 128)
		material = r->flat[v->current - 128].color;
	else if (v->current) {
		Dos94RasterObject* object = r->objectById[v->current];
		material = object->material[v->mesh[v->current].currentFace];
		if (object->gouraud[v->mesh[v->current].currentFace]) {
			uint16_t rightX;
			uint8_t rightLight;
			right_endpoint(v, &rightX, &rightLight);
			SpanEndpoint left = v->mesh[v->current].currentLeft;
			uint16_t leftX = left.x;
			uint8_t leftLight = left.light & 63;
			if (left.clipped) {
				Dos94ClippedEdge* edge = &Dos94_display->sweep.left[left.clippedIndex];
				leftX = 0;
				if (edge->x > 0xFFFF)
					leftLight = rightLight;
				else {
					uint32_t total = rightX + edge->x;
					uint32_t ratio = !edge->x         ? 0xFFFF
									 : total > 0xFFFF ? ((uint32_t)(rightX >> 1) << 16) / (total >> 1)
													  : ((uint32_t)rightX << 16) / total;
					int16_t delta = (int16_t)(Dos94Sweep_Event(edge->event)->light - (rightLight << 9));
					uint16_t magnitude = delta < 0 ? (uint16_t)-delta : (uint16_t)delta;
					uint16_t change = (uint16_t)((ratio * magnitude) >> 16);
					leftLight =
						(uint8_t)((uint16_t)((rightLight << 9) + (delta < 0 ? -change : change)) >> 9);
				}
			}
			Dos94_XTRANS2_outputGouraudSpan(v->row, v->start, end, material, leftX, leftLight, rightX,
											rightLight);
			v->start = end;
			return;
		}
	}
	Dos94_XTRANS2_outputCachedSpan(v->row, v->start, end, material);
	v->start = end;
}

static void change_current(Visibility* v, uint16_t current, uint16_t x) {
	if (current == v->current)
		return;
	output_span(v, x);
	v->current = current;
}

static void open_object(Visibility* v, uint16_t object, uint16_t x) {
	v->active[object] = true;
	if (!v->pending && (!v->current || in_front(v, v->current, object) != v->current)) {
		if (v->current) {
			v->stack[v->stackCount++] = v->current;
			++v->sortedTail;
		}
		change_current(v, object, x);
	} else {
		v->sortedTail = 0;
		v->stack[v->stackCount++] = object;
	}
	if (object == 128 && v->stackCount > 1 && !v->pending) {
		uint16_t candidate = nearest(v);
		if (in_front(v, v->current, candidate) != v->current) {
			v->stack[v->stackCount++] = v->current;
			change_current(v, candidate, x);
		} else
			v->stack[v->stackCount++] = candidate;
	}
}

static void close_object(Visibility* v, uint16_t object, uint16_t x) {
	v->active[object] = false;
	if (v->current == object) {
		output_span(v, x);
		if (v->sortedTail > 0) {
			--v->sortedTail;
			v->current = v->stack[--v->stackCount];
		} else if (v->mask < 0) {
			v->current = 0;
			v->pending = v->stackCount != 0;
		} else
			v->current = nearest(v);
		return;
	}
	for (unsigned i = 0; i < v->stackCount; ++i)
		if (v->stack[i] == object) {
			--v->stackCount;
			if (i != v->stackCount) {
				v->stack[i] = v->stack[v->stackCount];
				v->sortedTail = 1;
			}
			if (v->sortedTail > 0)
				--v->sortedTail;
			break;
		}
	if (object == 128 && v->current && !v->pending) {
		output_span(v, x);
		v->stack[v->stackCount++] = v->current;
		v->current = nearest(v);
	}
}

/* DOS94 0x6A267F: toggle the two adjacent faces, with a sixteen-face admission cap. */
static void mesh_event(Visibility* v, unsigned id, unsigned edgeId, SpanEndpoint left, uint16_t x) {
	Dos94RasterObject* object = Dos94_display->raster.objectById[id];
	if (!object)
		return;
	ActiveMesh* state = &v->mesh[id];
	Dos94RasterEdge edge = object->edges[edgeId];
	uint8_t previous = state->currentFace, oldCount = state->count;
	uint8_t faces[2] = { edge.face1, edge.face2 };
	/* Flush before replacing the selected face or its left endpoint. */
	bool selected = false;
	for (unsigned n = 0; n < 2; ++n)
		if (faces[n] && (faces[n] <= previous || !oldCount))
			selected = true;
	if (selected && v->current == id)
		output_span(v, x);
	for (unsigned n = 0; n < 2; ++n) {
		uint8_t face = faces[n];
		if (!face)
			continue;
		unsigned i = 0;
		while (i < state->count && state->faces[i] != face)
			++i;
		if (i < state->count) {
			--state->count;
			memmove(state->faces + i, state->faces + i + 1, state->count - i);
			memmove(state->left + i, state->left + i + 1, (state->count - i) * sizeof state->left[0]);
		} else if (state->count < 16) {
			state->faces[state->count] = face;
			state->left[state->count++] = left;
		}
	}
	if (!state->count) {
		if (oldCount)
			close_object(v, id, x);
		return;
	}
	unsigned minimum = 0;
	for (unsigned i = 1; i < state->count; ++i)
		if (state->faces[i] < state->faces[minimum])
			minimum = i;
	state->currentFace = state->faces[minimum];
	state->currentLeft = state->left[minimum];
	if (!oldCount)
		open_object(v, id, x);
}

static void swap_mark_material(Dos94RasterObject* object, Dos94RasterMark* mark, unsigned layer) {
	uint8_t color = object->material[mark->face];
	bool gouraud = object->gouraud[mark->face];
	object->material[mark->face] = mark->materials[layer];
	object->gouraud[mark->face] = mark->gouraud[layer];
	mark->materials[layer] = color;
	mark->gouraud[layer] = gouraud;
}

static void marking_event(Visibility* v, unsigned id, unsigned layer, uint16_t x) {
	Dos94RasterMark* mark = &Dos94_display->raster.marks[id];
	Dos94RasterObject* object = Dos94_display->raster.objectById[mark->object];
	if (!object || !layer || layer > 16)
		return;
	uint8_t old = mark->count ? mark->layers[0] : 0;
	unsigned i = 0;
	while (i < mark->count && mark->layers[i] != layer)
		++i;
	if (i < mark->count) {
		--mark->count;
		memmove(mark->layers + i, mark->layers + i + 1, mark->count - i);
	} else if (mark->count < 16) {
		i = mark->count++;
		while (i && mark->layers[i - 1] < layer) {
			mark->layers[i] = mark->layers[i - 1];
			--i;
		}
		mark->layers[i] = layer;
	}
	uint8_t current = mark->count ? mark->layers[0] : 0;
	if (current == old)
		return;
	if (v->current == mark->object && v->mesh[mark->object].currentFace == mark->face)
		output_span(v, x);
	if (old)
		swap_mark_material(object, mark, old - 1);
	if (current)
		swap_mark_material(object, mark, current - 1);
}

static uint16_t mask_run(Visibility* v) {
	uint8_t* mask = Dos94_display->mask;
	if (v->maskCursor >= DOS94_MASK_BYTES)
		return g_flightVpWidth;
	uint16_t run = mask[v->maskCursor++];
	if (!run && v->maskCursor < DOS94_MASK_BYTES)
		run = 256 + mask[v->maskCursor++];
	return run;
}

static bool advance_mask(Visibility* v, uint16_t x) {
	while (x >= v->maskEnd) {
		if (v->mask >= 0)
			output_span(v, v->maskEnd);
		if (v->maskEnd >= g_flightVpWidth)
			return false;
		v->mask = -v->mask;
		if (v->mask >= 0 && v->pending) {
			v->current = nearest(v);
			v->pending = false;
		}
		uint16_t run = mask_run(v);
		v->maskEnd += run;
		if (v->mask < 0)
			v->start += run;
	}
	return true;
}

static void process_event(Visibility* v, const Dos94EdgeEvent* event, SpanEndpoint left, uint16_t x) {
	unsigned id = event->object, edge = event->edge;
	v->eventObject = id;
	v->eventEdge = edge;
	v->eventX = x;
	v->eventLight = event->light;
	if (id >= Dos94_display->raster.markFirst)
		marking_event(v, edge, (uint8_t)-id, x);
	else if (id >= 128) {
		if (v->active[id])
			close_object(v, id, x);
		else
			open_object(v, id, x);
	} else
		mesh_event(v, id, edge, left, x);
}

static void draw_row(Visibility* v) {
	Dos94Sweep* s = &Dos94_display->sweep;
	memset(v->active, 0, sizeof v->active);
	memset(v->mesh, 0, sizeof v->mesh);
	v->current = v->stackCount = 0;
	v->sortedTail = 0;
	v->pending = false;
	v->start = 0;
	v->normalCursor = v->activeCursor = 0;
	v->mask = (int8_t)Dos94_display->mask[v->maskCursor++];
	v->maskEnd = mask_run(v);
	if (v->mask < 0)
		v->start = v->maskEnd;
	if (v->start >= g_flightVpWidth)
		return;
	int8_t saved = v->mask;
	v->mask = -1;
	for (unsigned i = 0; i < s->leftCount; ++i) {
		process_event(v, Dos94Sweep_Event(s->left[i].event),
					  (SpanEndpoint) { .clippedIndex = i, .clipped = true }, 0);
	}
	v->mask = saved;
	if (v->mask >= 0 && v->pending) {
		v->current = nearest(v);
		v->pending = false;
	}
	while (v->normalCursor < s->sortedCount || v->activeCursor < s->activeCount) {
		bool normal =
			v->normalCursor < s->sortedCount &&
			(v->activeCursor == s->activeCount || Dos94Sweep_EventKey(s->sorted[v->normalCursor].event) <=
													  Dos94Sweep_EventKey(s->active[v->activeCursor].event));
		uint16_t event = normal ? s->sorted[v->normalCursor++].event : s->active[v->activeCursor++].event;
		const Dos94EdgeEvent* e = Dos94Sweep_Event(event);
		v->eventObject = e->object;
		v->eventEdge = e->edge;
		v->eventX = e->x;
		v->eventLight = e->light;
		if (!advance_mask(v, e->x))
			break;
		process_event(v, e, (SpanEndpoint) { .x = e->x, .light = e->light }, e->x);
	}
	v->eventObject = 0;
	if (v->pending) {
		v->current = nearest(v);
		v->pending = false;
	}
	advance_mask(v, g_flightVpWidth);
	for (unsigned i = 1; i <= Dos94_display->raster.numMarks; ++i) {
		Dos94RasterMark* mark = &Dos94_display->raster.marks[i];
		if (mark->count) {
			uint8_t layer = mark->layers[0];
			Dos94RasterObject* object = Dos94_display->raster.objectById[mark->object];
			swap_mark_material(object, mark, layer - 1);
			mark->count = 0;
		}
	}
}

/* DOS94 0x6A1A77: consume world events into the indexed VGA viewport. */
void Dos94_XTRANS2_drawxtrans(void) {
	Visibility v = { 0 };
	/* The cockpit loader and sweep share the DOS-owned mask region. */
	v.maskCursor = 0;
	Dos94Sweep_Begin(&Dos94_display->sweep);
	Dos94_XTRANS2_beginoutput();
	for (v.row = 0; v.row < g_flightVpHeight; ++v.row) {
		Dos94Sweep_Row(&Dos94_display->sweep, v.row);
		if (Dos94_display->sweep.failed || v.maskCursor >= DOS94_MASK_BYTES) {
			XwPort_Fail("DOS renderer event storage exhausted or invalid mask");
			break;
		}
		draw_row(&v);
	}
	Dos94_XTRANS2_endoutput();
}
