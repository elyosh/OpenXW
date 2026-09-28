#ifndef XW_RUNTIME_INTEGRATION_CUTSCENE_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_CUTSCENE_CALLBACKS_H

#include "xw/landru_config.h"
#include <landru/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

int16_t XwTitle_DrawCrawl(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

int16_t XwDsBoom_DrawBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

int16_t XwCutscene_DrawCloseOnRefresh(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh);

/* Shared Landru requires a short result; its actor loop ignores this result. */
int16_t XwCutscene_DrawClose(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

#ifdef __cplusplus
}
#endif
#endif
