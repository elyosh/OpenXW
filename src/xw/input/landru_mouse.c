#include "xw/input/landru_mouse.h"

#include "xw/input/mouse.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// FUNCTION: XW 0x49E5A0
int32_t LandruMouse_SetPosition(int16_t x, int16_t y) {
	int positionY = y;
	int positionX = x;
	return Mouse_SetPosition(positionX, positionY);
}

// FUNCTION: XW 0x4AB940
void LandruMouse_SetBoundsStub(int16_t minimum, int16_t maximum) {
	(void)minimum;
	(void)maximum;
	nullsub_SharedNoOp();
}
