#ifndef XW_RUNTIME_RUNTIME_REPLAY_SCREEN_TASK_H
#define XW_RUNTIME_RUNTIME_REPLAY_SCREEN_TASK_H
#include "xw_runtime/timing/flight_timing.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Push playback; the caller resumes only after this task and its children end.
 * One replay iteration is admitted per host tick, including fast-forward. */
void XwReplayScreen_Begin(void);
/* Scope only the continuous viewer controls; never advance the world timing context. */
void XwReplayScreen_ResetCameraClock(void);
bool XwReplayScreen_BeginCameraControls(XwFlightClock* saved);
#ifdef __cplusplus
}
#endif
#endif
