#include "xw_runtime/integration/cutscene_callbacks.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/scenes/ds_boom.h"
#include "xw/frontend/scenes/title.h"

int16_t XwCutscene_DrawClose(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Cutscene_DrawClose(actor, frame, clip, x, y, refresh);
	return 0;
}

int16_t XwDsBoom_DrawBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								int16_t refresh) {
	return (int16_t)DsBoom_draw_Background(actor, frame, clip, x, y, refresh);
}

int16_t XwCutscene_DrawCloseOnRefresh(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh) {
	return (int16_t)Cutscene_DrawCloseOnRefresh(actor, frame, clip, x, y, refresh);
}

int16_t XwTitle_DrawCrawl(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	title_draw_Title(actor, frame, clip, x, y, refresh);
	return 0;
}
