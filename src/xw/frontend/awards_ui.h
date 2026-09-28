#ifndef XW_FRONTEND_AWARDS_UI_H
#define XW_FRONTEND_AWARDS_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/btnpush.h>
#include <landru/rect.h>
#include <stddef.h>
#include <stdint.h>

enum { AWARDS_UI_BUTTON_FONT = 2, AWARDS_UI_BUTTON_COLOR = 26, AWARDS_UI_BUTTON_PRESSED_COLOR = 38 };

/* Declarations follow ascending original IDB address. */

/* 0x457450 */
void AwardsUI_DrawTextButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);

#ifdef __cplusplus
}
#endif

#endif
