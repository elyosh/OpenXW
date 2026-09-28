#include "xw_runtime/integration/blueprint_callbacks.h"
#include "xw/frontend/blueprnt.h"

void XwBlueprint_DrawNavigationButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	blueprnt_DrawNavigationButton((PushButton*)input, frame, clip, refresh);
}
