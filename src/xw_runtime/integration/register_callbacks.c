#include "xw_runtime/integration/register_callbacks.h"
#include "xw/frontend/register.h"

void XwRegister_DrawPilotButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	register_idraw_Pilot_Button((PushButton*)input, frame, clip, refresh);
}

void XwRegister_DrawPilotName(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	register_idraw_Pilot_Name((REGISTER_RegStringButton*)input, frame, clip, refresh);
}
