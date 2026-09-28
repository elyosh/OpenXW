#ifndef XW_RUNTIME_INTEGRATION_INFLIGHT_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_INFLIGHT_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
void XwInflight_HandleButton(Input* input, int time);
void XwInflight_ApplyShipSelection(Input* input, int time);
void XwInflight_DrawTextButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
int16_t XwInflight_UpdateRepeatingButton(Input* input, Rect* frame, Rect* clip, int16_t key,
										 InputMouseEvent leftEvent, InputMouseEvent rightEvent, int16_t x,
										 int16_t y);
#ifdef __cplusplus
}
#endif
#endif
