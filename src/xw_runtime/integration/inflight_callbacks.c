#include "xw_runtime/integration/inflight_callbacks.h"
#include "xw/frontend/inflight_ui.h"

void XwInflight_HandleButton(Input* input, int time) {
	(void)time;
	InflightUI_HandleButton(input);
}

void XwInflight_ApplyShipSelection(Input* input, int time) {
	(void)time;
	InflightUI_ApplyShipSelection(input);
}

void XwInflight_DrawTextButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	InflightUI_DrawTextButton((PushButton*)input, frame, clip, refresh);
}

int16_t XwInflight_UpdateRepeatingButton(Input* input, Rect* frame, Rect* clip, int16_t key,
										 InputMouseEvent leftEvent, InputMouseEvent rightEvent, int16_t x,
										 int16_t y) {
	return InflightUI_UpdateRepeatingButton((PushButton*)input, frame, clip, key, leftEvent, rightEvent, x,
											y);
}
