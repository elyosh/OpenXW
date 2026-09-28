#ifndef XW_RUNTIME_INTEGRATION_MAINMENU_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_MAINMENU_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/actor.h>
#ifdef __cplusplus
extern "C" {
#endif
int16_t XwMainMenu_DrawTitle(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
