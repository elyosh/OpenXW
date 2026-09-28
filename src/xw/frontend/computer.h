#ifndef XW_FRONTEND_COMPUTER_H
#define XW_FRONTEND_COMPUTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/btnpush.h>
#include <landru/dialog.h>
#include <landru/input.h>
#include <landru/rect.h>
#include <stddef.h>
#include <stdint.h>

enum {
	COMPUTER_CONFIRM_TEXT_X = 8,
	COMPUTER_CONFIRM_TEXT_Y = 17,
	COMPUTER_CONFIRM_FONT = 0,
	COMPUTER_CONFIRM_TEXT_COLOR = 15
};

enum {
	COMPUTER_CONFIRM_WIDTH = 340,
	COMPUTER_CONFIRM_HEIGHT = 53,
	COMPUTER_CONFIRM_ALIGN_CENTER = 1,
	COMPUTER_CONFIRM_BUTTON_TOP = 7,
	COMPUTER_CONFIRM_BUTTON_BOTTOM = 46,
	COMPUTER_CONFIRM_YES_LEFT = 180,
	COMPUTER_CONFIRM_YES_RIGHT = 252,
	COMPUTER_CONFIRM_NO_LEFT = 260,
	COMPUTER_CONFIRM_NO_RIGHT = 332,
	COMPUTER_CONFIRM_MOUSE_X = 365,
	COMPUTER_CONFIRM_MOUSE_Y = 240,
	COMPUTER_CONFIRM_LABEL_CAPACITY = 6
};

extern const char g_computerConfirmPrompt[];
extern Input* g_computerConfirmYesButton;
extern char g_computerConfirmYesLabel[COMPUTER_CONFIRM_LABEL_CAPACITY];
extern char g_computerConfirmNoLabel[COMPUTER_CONFIRM_LABEL_CAPACITY];
extern Input* g_computerConfirmNoButton;

enum { COMPUTER_EXIT_YES = 1, COMPUTER_EXIT_NO = 2 };

/* Declarations follow ascending original IDB address. */

/* 0x4A94E0 */
void computer_ConfirmJoystickBindingReset(DialogSubResultHandler complete, void* context);

/* 0x4A9530 */
Input* computer_Build_Exit(const char* prompt);

/* 0x4A9650 */
int16_t computer_iupdate_ConfirmYes(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									int rightEvent, int16_t x, int16_t y);

/* 0x4A96B0 */
int16_t computer_iupdate_ConfirmNo(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								   int rightEvent, int16_t x, int16_t y);

/* 0x4A9710 */
void computer_idraw_Exit(Input* input, Rect* frame, Rect* clip, int16_t refresh);

/* 0x4A9750 */
void computer_iuser_Exit(Input* input, int time);

#ifdef __cplusplus
}
#endif

#endif
