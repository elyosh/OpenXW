#include "xw_runtime/integration/debrief_callbacks.h"
#include "xw/frontend/debrief.h"

void XwDebrief_IgnoreActorEvent(Actor* actor, int time) {
	(void)actor;
	Cutscene_IgnoreSoundEvent(NULL, time);
}

int16_t XwDebrief_DrawBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								 int16_t refresh) {
	debrief_draw_Background(actor, frame, clip, x, y, refresh);
	return 0;
}

int16_t XwDebrief_DrawStatistics(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								 int16_t refresh) {
	debrief_draw_Statistics(actor, frame, clip, x, y, refresh);
	return 0;
}

void XwDebrief_DrawPageButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	debrief_idraw_PageButton((PushButton*)input, frame, clip, refresh);
}
