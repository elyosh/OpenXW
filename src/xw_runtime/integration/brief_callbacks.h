#ifndef XW_RUNTIME_INTEGRATION_BRIEF_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_BRIEF_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
void XwBrief_DrawPlaybackButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
