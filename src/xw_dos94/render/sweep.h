#ifndef XW_DOS94_SWEEP_H
#define XW_DOS94_SWEEP_H
#include "xw_dos94/render/raster.h"

enum { DOS94_SWEEP_EVENTS = DOS_EDGE_EVENTS };

typedef struct Dos94ActiveEdge {
	uint16_t event;
	uint16_t rows;
	uint8_t lightFraction;
} Dos94ActiveEdge;

typedef struct Dos94ClippedEdge {
	uint16_t event;
	uint32_t x;
} Dos94ClippedEdge;

typedef struct Dos94SortedEvent {
	uint16_t slot, event;
} Dos94SortedEvent;

typedef struct Dos94Sweep {
	Dos94ActiveEdge active[DOS94_SWEEP_EVENTS], newActive[DOS94_SWEEP_EVENTS];
	Dos94ClippedEdge left[DOS94_SWEEP_EVENTS], right[DOS94_SWEEP_EVENTS];
	Dos94ClippedEdge newLeft[DOS94_SWEEP_EVENTS], newRight[DOS94_SWEEP_EVENTS];
	Dos94SortedEvent sorted[DOS94_SWEEP_EVENTS];
	uint16_t slots[DOS94_SWEEP_EVENTS];
	unsigned activeCount, newActiveCount, leftCount, rightCount, newLeftCount, newRightCount, sortedCount,
		slotCount;
	bool failed;
} Dos94Sweep;

void Dos94Sweep_Begin(Dos94Sweep* sweep);
void Dos94Sweep_Row(Dos94Sweep* sweep, uint16_t row);
const Dos94EdgeEvent* Dos94Sweep_Event(uint16_t event);
uint16_t Dos94Sweep_EventKey(uint16_t event);
void Dos94_XTRANS2_drawxtrans(void);
#endif
