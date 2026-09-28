#ifndef XW_RUNTIME_INTEGRATION_DEBRIEF_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_DEBRIEF_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/actor.h>
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
void XwDebrief_IgnoreActorEvent(Actor* actor, int time);
int16_t XwDebrief_DrawBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								 int16_t refresh);
int16_t XwDebrief_DrawStatistics(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								 int16_t refresh);
void XwDebrief_DrawPageButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
