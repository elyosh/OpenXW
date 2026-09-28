#ifndef XW_RUNTIME_RUNTIME_REPLAY_SAVE_TASK_H
#define XW_RUNTIME_RUNTIME_REPLAY_SAVE_TASK_H
#include "xw/flight/hud/msg.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Returns zero while pending. The caller must yield to the child task and call
 * again from its continuation to consume the saved/cancelled/error message. */
XwFlightMessageId XwReplaySave_Run(void);

typedef enum XwReplayInputSaveState {
	XW_REPLAY_INPUT_SAVE_IDLE,
	XW_REPLAY_INPUT_SAVE_PENDING,
	XW_REPLAY_INPUT_SAVE_FINISHED
} XwReplayInputSaveState;

void XwReplayInput_WaitForSave(void);
int XwReplayInput_IsSavePending(void);
/* Called after child tasks and the owning replay surface lock are cleared. */
void XwReplayInput_CancelSave(void);
XwReplayInputSaveState XwReplayInput_ResumeSave(void);
#ifdef __cplusplus
}
#endif
#endif
