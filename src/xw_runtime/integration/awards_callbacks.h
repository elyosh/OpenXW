#ifndef XW_RUNTIME_INTEGRATION_AWARDS_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_AWARDS_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
int16_t XwAwardBox_UpdateButton(Input* input, Rect* frame, Rect* clip, int16_t key, InputMouseEvent leftEvent,
								InputMouseEvent rightEvent, int16_t x, int16_t y);
void XwTourDesk_DrawSelectionButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void XwAwardsUI_DrawTextButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
