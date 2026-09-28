#ifndef XW_RUNTIME_RUNTIME_REPLAY_EDIT_TASK_H
#define XW_RUNTIME_RUNTIME_REPLAY_EDIT_TASK_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Caller yields immediately and keeps text (maxLength + 1 bytes) alive until
 * this child task completes. The parent resumes after Enter; stack cancellation
 * restores autofill without running the parent continuation. */
void XwReplayEdit_Push(int16_t x, int16_t y, uint8_t maxLength, char* text, uint8_t backgroundColor);
#ifdef __cplusplus
}
#endif
#endif
