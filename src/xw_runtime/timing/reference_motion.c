/* Reference-position sampling adapted from OpenXvT's offline flight timing. */
#include "xw_runtime/timing/reference_motion.h"
#include "xw/flight/object/create.h"
#include "xw_runtime/timing/flight_timing.h"
#include <limits.h>
#include <string.h>

typedef XwReferenceMotionState ReferenceMotion;
static ReferenceMotion history[XW_OBJECT_COUNT];

void XwReferenceMotion_Save(XwReferenceMotionState out[XW_OBJECT_COUNT]) {
	memcpy(out, history, sizeof history);
}

void XwReferenceMotion_Restore(const XwReferenceMotionState state[XW_OBJECT_COUNT]) {
	memcpy(history, state, sizeof history);
}

void XwReferenceMotion_Clear(void) { memset(history, 0, sizeof history); }

void XwReferenceMotion_Reset(uint16_t slot) {
	if (slot < XW_OBJECT_COUNT)
		history[slot] = (ReferenceMotion) { 0 };
}

void XwReferenceMotion_Reposition(uint16_t slot) {
	if (slot >= XW_OBJECT_COUNT)
		return;
	const ObjectRecord* object = &g_objectTable[slot];
	if (!XwFlightTiming_IsUnlocked() || !object->objectType) {
		XwReferenceMotion_Reset(slot);
		return;
	}
	history[slot] = (ReferenceMotion) {
		.position = { object->worldX, object->worldY, object->worldZ },
		.ticks = XwFlightTiming_CompletedMovementTicks(),
		.valid = true,
	};
}

void XwReferenceMotion_SeedWorld(void) {
	XwReferenceMotion_Clear();
	if (XwFlightTiming_IsUnlocked())
		for (uint16_t slot = 0; slot < XW_OBJECT_COUNT; ++slot)
			XwReferenceMotion_Reposition(slot);
}

void XwReferenceMotion_CommitBoundary(void) {
	if (!XwFlightTiming_IsUnlocked() || !XwFlightTiming_ReferenceDue())
		return;
	for (uint16_t slot = 0; slot < XW_OBJECT_COUNT; ++slot)
		XwReferenceMotion_Reposition(slot);
}

int32_t XwReferenceMotion_Axis(uint16_t slot, unsigned axis) {
	if (slot >= XW_OBJECT_COUNT || axis >= 3)
		return 0;
	const ObjectRecord* object = &g_objectTable[slot];
	const int32_t position[3] = { object->worldX, object->worldY, object->worldZ };
	if (!XwFlightTiming_IsUnlocked()) {
		const int32_t previous[3] = { object->prevWorldX, object->prevWorldY, object->prevWorldZ };
		return (int32_t)((uint32_t)position[axis] - (uint32_t)previous[axis]);
	}
	if (!object->objectType) {
		XwReferenceMotion_Reset(slot);
		return 0;
	}
	const ReferenceMotion* sample = &history[slot];
	uint64_t now = XwFlightTiming_CompletedMovementTicks();
	if (!sample->valid || now <= sample->ticks || now - sample->ticks > INT64_MAX)
		return 0;
	/* Current positions precede this frame's move pass, so exclude its elapsed ticks. */
	int64_t numerator = ((int64_t)position[axis] - sample->position[axis]) * XwFlightTiming_ReferenceTicks();
	int64_t delta = numerator / (int64_t)(now - sample->ticks);
	return delta < INT32_MIN ? INT32_MIN : delta > INT32_MAX ? INT32_MAX : (int32_t)delta;
}
