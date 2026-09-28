#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/sweep.h"
#include <string.h>

const Dos94EdgeEvent* Dos94Sweep_Event(uint16_t event) { return &Dos94_display->raster.events[event]; }

uint16_t Dos94Sweep_EventKey(uint16_t event) {
	const Dos94EdgeEvent* e = Dos94Sweep_Event(event);
	return (uint16_t)((e->x << 7) | (Dos94_display->raster.dos93 ? e->edge & 127 : e->light));
}

void Dos94Sweep_Begin(Dos94Sweep* s) {
	s->activeCount = s->leftCount = s->rightCount = s->sortedCount = 0;
	s->failed = Dos94_display->raster.failed;
}

static void parse_row(Dos94Sweep* s, uint16_t row) {
	s->slotCount = s->newActiveCount = s->newLeftCount = s->newRightCount = 0;
	Dos94Raster* r = &Dos94_display->raster;
	for (uint16_t index = r->rowHeads[row]; index != DOS_NO_EVENT; index = r->events[index].next) {
		const Dos94EdgeEvent* e = &r->events[index];
		if (e->kind == DOS_EVENT_POINT)
			s->slots[s->slotCount++] = index;
		else if (e->kind == DOS_EVENT_VERTICAL)
			s->newActive[s->newActiveCount++] = (Dos94ActiveEdge) { index, e->rows, e->lightFraction };
		else {
			uint32_t x = e->clip.kind == DOS_CLIP_X_Q7 ? e->clip.position >> 7 : e->clip.position;
			Dos94ClippedEdge edge = { index, x };
			if (e->clip.right)
				s->newRight[s->newRightCount++] = edge;
			else
				s->newLeft[s->newLeftCount++] = edge;
		}
	}
}

static uint16_t light_step(uint16_t event) {
	const Dos94EdgeEvent* e = Dos94Sweep_Event(event);
	Dos94RasterObject* object = e->object < 128 ? Dos94_display->raster.objectById[e->object] : NULL;
	return object ? object->edges[e->edge].halfLightIncY : 0;
}

static bool advance_active(Dos94ActiveEdge* edge) {
	Dos94EdgeEvent* e = &Dos94_display->raster.events[edge->event];
	if (!Dos94_display->raster.dos93 && e->object < 128) {
		uint16_t step = light_step(edge->event);
		unsigned fraction = edge->lightFraction + (uint8_t)step;
		edge->lightFraction = (uint8_t)fraction;
		uint8_t next = (uint8_t)(e->light + (step >> 8) + (fraction >> 8));
		if (next & 0x40)
			next = (uint8_t)~next;
		e->light = next & 63;
	}
	return --edge->rows != 0;
}

static void update_active(Dos94Sweep* s) {
	unsigned write = 0, newIndex = 0;
	for (unsigned i = 0; i < s->activeCount; ++i) {
		if (advance_active(&s->active[i]))
			s->active[write++] = s->active[i];
		else if (newIndex < s->newActiveCount)
			s->active[write++] = s->newActive[newIndex++];
	}
	while (newIndex < s->newActiveCount)
		s->active[write++] = s->newActive[newIndex++];
	s->activeCount = write;
	if (!s->newActiveCount)
		return;
	for (unsigned i = 1; i < write; ++i) {
		Dos94ActiveEdge edge = s->active[i];
		unsigned j = i;
		while (j && Dos94Sweep_EventKey(s->active[j - 1].event) > Dos94Sweep_EventKey(edge.event)) {
			s->active[j] = s->active[j - 1];
			--j;
		}
		s->active[j] = edge;
	}
}

static void sort_normal(Dos94Sweep* s) {
	unsigned count = 0;
	for (unsigned i = 0; i < s->sortedCount; ++i)
		if (s->sorted[i].slot < s->slotCount) {
			uint16_t slot = s->sorted[i].slot;
			s->sorted[count++] = (Dos94SortedEvent) { slot, s->slots[slot] };
		}
	for (unsigned slot = count; slot < s->slotCount; ++slot)
		s->sorted[count++] = (Dos94SortedEvent) { slot, s->slots[slot] };
	s->sortedCount = count;
	for (unsigned i = 1; i < count; ++i) {
		Dos94SortedEvent event = s->sorted[i];
		unsigned j = i;
		while (j && Dos94Sweep_EventKey(s->sorted[j - 1].event) > Dos94Sweep_EventKey(event.event)) {
			s->sorted[j] = s->sorted[j - 1];
			--j;
		}
		s->sorted[j] = event;
	}
}

/* DOS94 clipped continuation arithmetic, stored as native fields. */
static bool advance_clipped(Dos94ClippedEdge* edge) {
	Dos94EdgeEvent* e = &Dos94_display->raster.events[edge->event];
	Dos94ClippedState* state = &e->clip;
	state->rows = (uint8_t)(state->rows - 1);
	if (!state->rows)
		return false;
	if (state->kind == DOS_CLIP_Y_EXIT || state->kind == DOS_CLIP_Y_OUTSIDE) {
		uint16_t fraction = state->fraction;
		state->fraction = (uint16_t)(fraction - 256);
		if (fraction < 256) {
			edge->x += state->increase ? 1u : -1u;
			state->fraction = (uint16_t)(state->fraction + state->step);
		}
	} else {
		state->position = (state->position + (state->increase ? state->step : 0u - state->step)) & 0xFFFFFFu;
		edge->x = state->kind == DOS_CLIP_X_Q7 ? state->position >> 7 : state->position;
	}
	if (e->object < 128) {
		uint16_t light = (uint16_t)(e->light + 2u * light_step(edge->event));
		if ((int16_t)light < 0)
			light = (uint16_t)~light;
		e->light = light;
	}
	return true;
}

static unsigned update_clipped(Dos94ClippedEdge* edges, unsigned count, const Dos94ClippedEdge* incoming,
							   unsigned incomingCount) {
	unsigned write = 0;
	for (unsigned i = 0; i < count; ++i)
		if (advance_clipped(&edges[i]))
			edges[write++] = edges[i];
	memcpy(edges + write, incoming, incomingCount * sizeof *edges);
	return write + incomingCount;
}

void Dos94Sweep_Row(Dos94Sweep* s, uint16_t row) {
	parse_row(s, row);
	if (s->failed)
		return;
	sort_normal(s);
	update_active(s);
	s->leftCount = update_clipped(s->left, s->leftCount, s->newLeft, s->newLeftCount);
	s->rightCount = update_clipped(s->right, s->rightCount, s->newRight, s->newRightCount);
}
