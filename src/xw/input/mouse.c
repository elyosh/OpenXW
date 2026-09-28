#include "xw/input/mouse.h"

#include "xw/input/win_mouse.h"

// FUNCTION: XW 0x49E580
void Mouse_ReadPositionAndButtons(uint16_t* outButtons, int16_t* outX, int16_t* outY) {
	WinMouse_GetPositionAndButtons(outButtons, outX, outY);
}

// FUNCTION: XW 0x4AB920
int32_t Mouse_SetPosition(int16_t x, int16_t y) { return WinMouse_SetPosition(x, y); }
