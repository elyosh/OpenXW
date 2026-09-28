#include "xw_runtime/integration/mainmenu_callbacks.h"
#include "xw/frontend/mainmenu.h"

int16_t XwMainMenu_DrawTitle(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	mainmenu_draw_Title(actor, frame, clip, x, y, refresh);
	return 0;
}
