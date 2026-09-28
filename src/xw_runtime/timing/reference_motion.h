#ifndef XW_RUNTIME_TIMING_REFERENCE_MOTION_H
#define XW_RUNTIME_TIMING_REFERENCE_MOTION_H

#include "xw/flight/object/create.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwReferenceMotionState {
	int32_t position[3];
	uint64_t ticks;
	bool valid;
} XwReferenceMotionState;

void XwReferenceMotion_Save(XwReferenceMotionState out[XW_OBJECT_COUNT]);
void XwReferenceMotion_Restore(const XwReferenceMotionState state[XW_OBJECT_COUNT]);

/* Flight-owner thread only; independent of renderer lifetime and generations. */
void XwReferenceMotion_Clear(void);
void XwReferenceMotion_SeedWorld(void);
/* Replacement invalidates history; repositioning seeds the final authoritative location. */
void XwReferenceMotion_Reset(uint16_t slot);
void XwReferenceMotion_Reposition(uint16_t slot);
/* Called after all reference aiming consumers, before dynamics/movement. */
void XwReferenceMotion_CommitBoundary(void);
/* Native returns adjacent-frame displacement; unlocked returns one reference interval. */
int32_t XwReferenceMotion_Axis(uint16_t slot, unsigned axis);

#ifdef __cplusplus
}
#endif
#endif
