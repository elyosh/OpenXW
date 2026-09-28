#ifndef XW_RUNTIME_RUNTIME_FLIGHT_FRAME_H
#define XW_RUNTIME_RUNTIME_FLIGHT_FRAME_H

#include "xw_runtime/runtime/flight_input.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XwFlightFramePhase {
	XW_FRAME_IDLE,
	XW_FRAME_INPUT,
	XW_FRAME_SIMULATE,
	XW_FRAME_COLLISION,
	XW_FRAME_FINISH
} XwFlightFramePhase;

typedef struct XwFlightFrame {
	XwFlightFramePhase phase;
	int inputPending;
	XwFlightInput input;
	int rendered;
} XwFlightFrame;

/* Replay owns a separate frame so a suspended live input frame remains intact. */
int XwFlightFrame_TickState(XwFlightFrame* frame);
void XwFlightFrame_CancelState(XwFlightFrame* frame);
uint64_t XwFlightFrame_NextWakeDelayUs(const XwFlightFrame* frame);
/* Discard time spent in a modal owner without changing the admitted frame. */
void XwFlightFrame_ResumeClock(void);

/* Call at most once per host tick. Returns 1 when the admitted frame finishes,
 * 0 while waiting for time or a continuation. Service child Landru tasks before
 * resuming a suspended frame; do not run another simulation owner meanwhile. */
int XwFlightFrame_Tick(void);
int XwFlightFrame_IsPending(void);
/* The input owner sets this while its pause/modal/replay continuation waits. */
void XwFlightFrame_SetInputPending(int pending);
void XwFlightFrame_UpdateInput(void);
/* Cancel only after clearing child tasks and cancelling the input owner. */
void XwFlightFrame_Cancel(void);

#ifdef __cplusplus
}
#endif
#endif
