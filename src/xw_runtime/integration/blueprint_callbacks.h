#ifndef XW_RUNTIME_INTEGRATION_BLUEPRINT_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_BLUEPRINT_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
void XwBlueprint_DrawNavigationButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
