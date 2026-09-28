#ifndef XW_RUNTIME_INTEGRATION_INPUT_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_INPUT_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
int16_t XwInput_UpdateNoOp(Input* input, Rect* frame, Rect* clip, int16_t key, InputMouseEvent leftEvent,
						   InputMouseEvent rightEvent, int16_t x, int16_t y);
void XwInput_UserNoOp(Input* input, int time);
#ifdef __cplusplus
}
#endif
#endif
