#include "xw_runtime/integration/combat_callbacks.h"
#include "xw/frontend/combat.h"

void XwCombat_DrawArrowButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	combat_idraw_ArrowButton((PushButton*)input, frame, clip, refresh);
}

int16_t XwCombat_UpdateMonitor(Input* input, Rect* frame, Rect* clip, int16_t key, InputMouseEvent leftEvent,
							   InputMouseEvent rightEvent, int16_t x, int16_t y) {
	return combat_iupdate_Combat_Screen(input, frame, clip, key, (uint8_t)leftEvent, (uint8_t)rightEvent, x,
										y);
}
