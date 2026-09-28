#ifndef XW_RUNTIME_TIMING_FLIGHT_TIMING_H
#define XW_RUNTIME_TIMING_FLIGHT_TIMING_H

#include "xw/flight/mission/mission.h"
#include "xw_runtime/runtime/profile.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwFlightClock {
	uint16_t elapsed, scale;
} XwFlightClock;

/* Logical state only; the replay codec encodes fields rather than this struct's bytes. */
typedef struct XwFlightTimingState {
	XwFlightUpdateRate update_rate;
	uint16_t reference_ticks, phase, elapsed_ticks;
	bool advance_active, reference_due, movement_completed;
	uint64_t advance_serial, completed_movement_ticks;
	uint64_t animation_serial, animation_time_ticks;
	uint64_t mission_pose_serial[MISSION_OBJECT_COUNT];
	uint32_t dropped_periods;
} XwFlightTimingState;

/* Flight-owner thread only. The active profile supplies the pinned session policy. */
void XwFlightTiming_BeginSession(void);
void XwFlightTiming_EndSession(void);
/* New world only: resource/presentation rebuilds and modal returns preserve this state. */
void XwFlightTiming_ResetWorld(void);
bool XwFlightTiming_IsUnlocked(void);
uint16_t XwFlightTiming_StepTicks(void);
uint16_t XwFlightTiming_ReferenceTicks(void);

/* Begin after input; retain through collision yields; end on completion or cancellation. */
void XwFlightTiming_BeginAdvance(uint16_t elapsed);
void XwFlightTiming_MovementCompleted(void);
void XwFlightTiming_EndAdvance(void);
bool XwFlightTiming_ReferenceDue(void);
uint16_t XwFlightTiming_ReferenceElapsed(void);
/* Call only at an eligible visible object's rotation site; zero means no mutation. */
uint16_t XwFlightTiming_MissionPoseElapsed(unsigned slot);
uint64_t XwFlightTiming_AdvanceSerial(void);
uint64_t XwFlightTiming_CompletedMovementTicks(void);
XwFlightClock XwFlightTiming_EnterReference(void);
void XwFlightTiming_RestoreClock(XwFlightClock clock);

/* Called only when the post-movement animation body advances. */
void XwFlightTiming_AnimationEvent(void);
uint64_t XwFlightTiming_AnimationSerial(void);
uint64_t XwFlightTiming_AnimationTime(void);

void XwFlightTiming_Save(XwFlightTimingState* out);
/* Read-only validation also works before activation. */
bool XwFlightTiming_Check(const XwFlightTimingState* state, XwGameVersion version, XwFlightUpdateRate rate);
/* Requires the matching active profile/rate; failure leaves the timing owner unchanged. */
bool XwFlightTiming_Restore(const XwFlightTimingState* state);

#ifdef __cplusplus
}
#endif
#endif
