#include "xw/input/win_mouse.h"

#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/landru_config.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/input/system_cursor.h"
#include "xw_runtime/platform/main_window.h"

#include <landru/error.h>

// GLOBAL: XW 0x4DED24
int g_winMouseMaxX = WIN_MOUSE_DEFAULT_MAX_X;

// GLOBAL: XW 0x4DED28
int g_winMouseMaxY = WIN_MOUSE_DEFAULT_MAX_Y;

// GLOBAL: XW 0x4DED2C
int g_winMouseScaleX = 1;

// GLOBAL: XW 0x4DED30
int g_winMouseScaleY = 1;

// GLOBAL: XW 0x4FC550
int g_softwareCursorX = 0;

// GLOBAL: XW 0x4FC554
int g_softwareCursorY = 0;

// GLOBAL: XW 0x56687C
int g_SoftwareCursor = 0;

// GLOBAL: XW 0x5668A8
XwMousePosition g_winMousePrevPos = { 0, 0 };

// GLOBAL: XW 0x5668B0
XwMousePosition g_winMouseCursorPos = { 0, 0 };

// GLOBAL: XW 0x5668B8
XwMousePosition g_winMousePos = { 0, 0 };

// GLOBAL: XW 0x5668C0
int g_winMouseButtonDown[WIN_MOUSE_BUTTON_COUNT] = { 0, 0, 0 };

// GLOBAL: XW 0x5668CC
int g_winMouseButtonPressed[WIN_MOUSE_BUTTON_COUNT] = { 0, 0, 0 };

// GLOBAL: XW 0x5668D8
int g_winMouseButtonReleased[WIN_MOUSE_BUTTON_COUNT] = { 0, 0, 0 };

// GLOBAL: XW 0x5668E4
int g_winMouseMinX = 0;

// GLOBAL: XW 0x5668E8
int g_winMouseMinY = 0;

// FUNCTION: XW 0x4AB840
void WinMouse_HideSystemCursor(void) {
#ifdef XW_MODERN
	if (g_SoftwareCursor == 0)
		XwPort_SetSystemCursorDisplayCount(-1);
#else
	int displayCount;
	unsigned int remainingHideCalls;
	if (g_SoftwareCursor == 0) {
		displayCount = XwPort_ShowSystemCursor(0);
		if (displayCount >= 0) {
			remainingHideCalls = (unsigned int)displayCount + 1;
			do {
				XwPort_ShowSystemCursor(0);
				--remainingHideCalls;
			} while (remainingHideCalls != 0);
		}
	}
#endif
}

// FUNCTION: XW 0x4AB870
void WinMouse_GetPositionAndButtons(uint16_t* buttons, int16_t* x, int16_t* y) {
	int positionX;
	int positionY;
	int deltaY;
	int deltaX;
	int buttonDown[WIN_MOUSE_BUTTON_COUNT];
	int buttonReleased[WIN_MOUSE_BUTTON_COUNT];
	int buttonPressed[WIN_MOUSE_BUTTON_COUNT];
	WinMouse_PollState(&positionX, &positionY, &deltaX, &deltaY, buttonDown, buttonPressed, buttonReleased);
	g_softwareCursorX = positionX;
	g_softwareCursorY = positionY;
#ifndef XW_MODERN
	if (g_landruDoublePixelsEnabled != 0) {
		positionX = positionX * g_landruLogicalWidth / (2 * g_landruLogicalWidth);
		{
			int doubledHeight = 2 * g_landruLogicalHeight;
			int topMargin = (FLIGHT_DISPLAY_HEIGHT - doubledHeight) >> 1;
			positionY = (positionY - topMargin) * g_landruLogicalHeight / doubledHeight;
		}
	}
#endif
	*x = (int16_t)positionX;
	*y = (int16_t)positionY;
	*buttons =
		(uint16_t)(buttonDown[WIN_MOUSE_BUTTON_LEFT] |
				   (2 * (buttonDown[WIN_MOUSE_BUTTON_RIGHT] | (2 * buttonDown[WIN_MOUSE_BUTTON_MIDDLE]))));
}

// FUNCTION: XW 0x4AC5E0
void WinMouse_PollState(int* positionX, int* positionY, int* deltaX, int* deltaY, int* buttonDown,
						int* buttonPressed, int* buttonReleased) {
	XwMousePosition point;
#ifndef XW_MODERN
	XwWin32Message message;
#endif
	if (g_quitRequested != 0)
		return;
#ifdef XW_MODERN
	if (XwPort_WindowQuitRequested()) {
		g_quitRequested = 1;
		xerror_Set_Landru_Exit(0);
		return;
	}
	XwPort_UpdateMouseButtons();
#else
	if (g_windowActive == 0 || PeekMessageA(&message, NULL, 0, 0, XW_WIN32_MESSAGE_NO_REMOVE)) {
		if (!GetMessageA(&message, NULL, 0, 0)) {
			g_quitRequested = 1;
			xerror_Set_Landru_Exit(0);
			return;
		}
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
#endif
	XwPort_GetSystemCursorPosition(&point);
	g_winMouseCursorPos = point;
	*deltaX = g_winMouseCursorPos.x - g_winMousePrevPos.x;
	*deltaY = g_winMouseCursorPos.y - g_winMousePrevPos.y;
	g_winMousePos.x += *deltaX;
	g_winMousePos.y += *deltaY;
	if (g_winMousePos.x < g_winMouseMinX)
		g_winMousePos.x = g_winMouseMinX;
	if (g_winMousePos.x > g_winMouseMaxX)
		g_winMousePos.x = g_winMouseMaxX;
	if (g_winMousePos.y < g_winMouseMinY)
		g_winMousePos.y = g_winMouseMinY;
	if (g_winMousePos.y > g_winMouseMaxY)
		g_winMousePos.y = g_winMouseMaxY;
	*positionX = g_winMousePos.x;
	*positionY = g_winMousePos.y;
	g_winMousePrevPos = g_winMouseCursorPos;
	buttonDown[WIN_MOUSE_BUTTON_LEFT] = g_winMouseButtonDown[WIN_MOUSE_BUTTON_LEFT];
	buttonDown[WIN_MOUSE_BUTTON_RIGHT] = g_winMouseButtonDown[WIN_MOUSE_BUTTON_RIGHT];
	buttonDown[WIN_MOUSE_BUTTON_MIDDLE] = g_winMouseButtonDown[WIN_MOUSE_BUTTON_MIDDLE];
	buttonPressed[WIN_MOUSE_BUTTON_LEFT] = g_winMouseButtonPressed[WIN_MOUSE_BUTTON_LEFT];
	buttonPressed[WIN_MOUSE_BUTTON_RIGHT] = g_winMouseButtonPressed[WIN_MOUSE_BUTTON_RIGHT];
	buttonPressed[WIN_MOUSE_BUTTON_MIDDLE] = g_winMouseButtonPressed[WIN_MOUSE_BUTTON_MIDDLE];
	buttonReleased[WIN_MOUSE_BUTTON_LEFT] = g_winMouseButtonReleased[WIN_MOUSE_BUTTON_LEFT];
	buttonReleased[WIN_MOUSE_BUTTON_RIGHT] = g_winMouseButtonReleased[WIN_MOUSE_BUTTON_RIGHT];
	buttonReleased[WIN_MOUSE_BUTTON_MIDDLE] = g_winMouseButtonReleased[WIN_MOUSE_BUTTON_MIDDLE];
	g_winMouseButtonPressed[WIN_MOUSE_BUTTON_LEFT] = 0;
	g_winMouseButtonPressed[WIN_MOUSE_BUTTON_RIGHT] = 0;
	g_winMouseButtonPressed[WIN_MOUSE_BUTTON_MIDDLE] = 0;
	g_winMouseButtonReleased[WIN_MOUSE_BUTTON_LEFT] = 0;
	g_winMouseButtonReleased[WIN_MOUSE_BUTTON_RIGHT] = 0;
	g_winMouseButtonReleased[WIN_MOUSE_BUTTON_MIDDLE] = 0;
	*deltaX *= g_winMouseScaleX;
	*deltaY *= g_winMouseScaleY;
	*positionX *= g_winMouseScaleX;
	*positionY *= g_winMouseScaleY;
}

// FUNCTION: XW 0x4AC7D0
int32_t WinMouse_SetPosition(int x, int y) {
	g_winMousePrevPos.x = x;
	g_winMouseCursorPos.x = x;
	g_winMousePos.x = x;
	g_winMousePrevPos.y = y;
	g_winMouseCursorPos.y = y;
	g_winMousePos.y = y;
	return XwPort_SetSystemCursorPosition(x, y);
}
