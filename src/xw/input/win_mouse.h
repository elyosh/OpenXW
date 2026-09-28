#ifndef XW_INPUT_WIN_MOUSE_H
#define XW_INPUT_WIN_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct XwMousePosition {
	int32_t x;
	int32_t y;
} XwMousePosition;

enum {
	WIN_MOUSE_BUTTON_LEFT = 0,
	WIN_MOUSE_BUTTON_RIGHT = 1,
	WIN_MOUSE_BUTTON_MIDDLE = 2,
	WIN_MOUSE_BUTTON_COUNT = 3,
	WIN_MOUSE_DEFAULT_MAX_X = 640,
	WIN_MOUSE_DEFAULT_MAX_Y = 480
};

extern int g_winMouseMaxX;
extern int g_winMouseMaxY;
extern int g_winMouseScaleX;
extern int g_winMouseScaleY;
extern int g_softwareCursorX;
extern int g_softwareCursorY;
extern int g_SoftwareCursor;
extern XwMousePosition g_winMousePrevPos;
extern XwMousePosition g_winMouseCursorPos;
extern XwMousePosition g_winMousePos;
extern int g_winMouseButtonDown[WIN_MOUSE_BUTTON_COUNT];
extern int g_winMouseButtonPressed[WIN_MOUSE_BUTTON_COUNT];
extern int g_winMouseButtonReleased[WIN_MOUSE_BUTTON_COUNT];
extern int g_winMouseMinX;
extern int g_winMouseMinY;

/* Declarations follow ascending original IDB address. */

/* 0x4AB840 */
void WinMouse_HideSystemCursor(void);

/* 0x4AB870 */
void WinMouse_GetPositionAndButtons(uint16_t* buttons, int16_t* x, int16_t* y);

/* 0x4AC5E0 */
void WinMouse_PollState(int* positionX, int* positionY, int* deltaX, int* deltaY, int* buttonDown,
						int* buttonPressed, int* buttonReleased);

/* 0x4AC7D0 */
int32_t WinMouse_SetPosition(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
