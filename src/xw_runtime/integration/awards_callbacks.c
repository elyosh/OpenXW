#include "xw_runtime/integration/awards_callbacks.h"
#include "xw/frontend/award_box.h"
#include "xw/frontend/awards_ui.h"
#include "xw/frontend/tourdesk.h"

int16_t XwAwardBox_UpdateButton(Input* input, Rect* frame, Rect* clip, int16_t key, InputMouseEvent leftEvent,
								InputMouseEvent rightEvent, int16_t x, int16_t y) {
	return AwardBox_iupdate_Button((PushButton*)input, frame, clip, key, leftEvent, rightEvent, x, y);
}

void XwAwardsUI_DrawTextButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	AwardsUI_DrawTextButton((PushButton*)input, frame, clip, refresh);
}

void XwTourDesk_DrawSelectionButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	tourdesk_idraw_SelectionButton((PushButton*)input, frame, clip, refresh);
}
