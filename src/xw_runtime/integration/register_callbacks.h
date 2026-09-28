#ifndef XW_RUNTIME_INTEGRATION_REGISTER_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_REGISTER_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
void XwRegister_DrawPilotButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void XwRegister_DrawPilotName(Input* input, Rect* frame, Rect* clip, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
