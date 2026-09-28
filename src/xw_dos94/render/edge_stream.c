#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/raster.h"
#include <string.h>

/* Frame counters and logical DOS budgets are independent of native record sizes. */
void Dos94_XTRANS2_initxtrans(void) {
	Dos94Raster* r = &Dos94_display->raster;
	memset(r->rowHeads, 0xff, sizeof r->rowHeads);
	memset(r->rowTails, 0xff, sizeof r->rowTails);
	memset(r->rowPageBytes, 0, sizeof r->rowPageBytes);
	r->dos93 = Dos94Assets_Version() == XW_GAME_VERSION_93;
	r->pageLimit = r->dos93 ? 256 : 320;
	r->pageBytes = r->dos93 ? 126 : 124;
	r->objectLimit = r->dos93 ? 0x1F00 : 0x3F00;
	r->flatLimit = r->dos93 ? 119 : 111;
	r->markFirst = r->dos93 ? 249 : 240;
	r->eventCount = r->pageCount = 0;
	r->failed = false;
	r->nextObject = r->parentObject = r->numMarks = 0;
	r->objectNumber = r->edgeIndex = r->flatObjectNumber = 1;
	g_flightVpCenterX = g_flightVpWidth / 2;
	g_flightVpCenterY = g_flightVpHeight / 2;
	g_flightVpMaxX = g_flightVpWidth - 1;
	g_flightVpMaxY = g_flightVpHeight - 1;
}

static void append_event(uint16_t row, Dos94EdgeEvent event, unsigned logicalBytes) {
	Dos94Raster* r = &Dos94_display->raster;
	if (row >= g_flightVpHeight || r->failed)
		return;
	unsigned used = r->rowPageBytes[row], pages = r->pageCount;
	if (r->rowHeads[row] == DOS_NO_EVENT)
		++pages;
	/* Both revisions split multi-record entries across their logical page end. */
	while (used + logicalBytes > r->pageBytes) {
		logicalBytes -= r->pageBytes - used;
		used = 0;
		++pages;
	}
	if (pages > r->pageLimit || r->eventCount == DOS_EDGE_EVENTS) {
		r->failed = true;
		return;
	}
	r->pageCount = pages;
	r->rowPageBytes[row] = used + logicalBytes;
	uint16_t index = r->eventCount++;
	event.next = DOS_NO_EVENT;
	r->events[index] = event;
	if (r->rowTails[row] != DOS_NO_EVENT)
		r->events[r->rowTails[row]].next = index;
	else
		r->rowHeads[row] = index;
	r->rowTails[row] = index;
}

static Dos94EdgeEvent make_event(uint16_t tag, uint16_t x, uint8_t light, Dos94EventKind kind) {
	Dos94Raster* r = &Dos94_display->raster;
	/* Source mesh edge identities are below128 in all supplied DOS93 models. */
	return (Dos94EdgeEvent) {
		.x = x & 511, .light = r->dos93 ? 0 : light, .object = tag >> 8, .edge = (uint8_t)tag, .kind = kind
	};
}

void Dos94_TRACE2_enterevent(uint16_t row, uint16_t tag, uint16_t x, uint8_t light) {
	append_event(row, make_event(tag, x, light, DOS_EVENT_POINT), Dos94_display->raster.dos93 ? 3 : 4);
}

/* A Q10 light enters the DOS94 run as a Q9 fraction plus six intensity bits. */
void Dos94_TRACE2_entervertedge(uint16_t top, uint16_t count, uint16_t x, uint16_t light, uint16_t tag) {
	if (!count)
		return;
	Dos94EdgeEvent event = make_event(tag, x, (uint8_t)(light >> 9), DOS_EVENT_VERTICAL);
	event.rows = (uint8_t)count ? (uint8_t)count : 256;
	event.lightFraction = (uint8_t)(light >> 1);
	append_event(top, event, Dos94_display->raster.dos93 ? 6 : 8);
}

void Dos94_TRACE2_enterclipped(uint16_t row, uint16_t tag, uint16_t light, Dos94ClippedState state) {
	Dos94EdgeEvent event = make_event(tag, 0, 0, DOS_EVENT_CLIPPED);
	event.light = light;
	event.clip = state;
	append_event(row, event, 12);
}
