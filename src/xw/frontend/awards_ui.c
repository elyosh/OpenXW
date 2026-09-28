#include "xw/frontend/awards_ui.h"

#include <landru/font.h>
#include <landru/style.h>

// FUNCTION: XW 0x457450
void AwardsUI_DrawTextButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	size_t offset;
	int16_t remaining;
	int16_t color;
	const char* label;
	const char* labels;
	(void)clip;
	if (refresh == 0) {
		return;
	}
	offset = 0;
	labels = button->labels;
	for (remaining = button->labelIndex; labels[offset] != '\0' && remaining != 0; --remaining) {
		for (; labels[offset++] != '\0';) {
		}
	}
	label = &labels[offset];
	xstyle_Style_Paint_Border(frame, button->pressed);
	color = button->pressed ? AWARDS_UI_BUTTON_PRESSED_COLOR : AWARDS_UI_BUTTON_COLOR;
	xfont_Print_Centered_Text(label, frame, AWARDS_UI_BUTTON_FONT, color);
}
