#include "xw/frontend/textext.h"

#include "xw/landru_config.h"

#include <landru/font.h>
#include <landru/paint.h>
#include <landru/rect.h>
#include <string.h>

// FUNCTION: XW 0x441240
void textext_Draw_Typewriter_Line(const char* text, uint16_t fontId, int16_t x, int16_t y,
								  int16_t revealChars) {
	Rect cursorRect;
	char prefixBuffer[TEXTEXT_PREFIX_CAPACITY];
	int16_t textLength;
	int16_t prefixEnd;
	int16_t fadeSteps;
	int16_t color;
	int16_t prefixWidth;
	uint16_t fontHeight;
	if (revealChars >= 0) {
		textLength = strlen(text);
		strcpy(prefixBuffer, text);
		if (revealChars < textLength + TEXTEXT_REVEAL_FINISH_DELAY) {
			if (revealChars <= textLength) {
				prefixBuffer[revealChars] = '\0';
				prefixEnd = revealChars;
			} else {
				prefixEnd = textLength;
			}
			fadeSteps = textLength - revealChars + 1;
			if (fadeSteps > TEXTEXT_MAX_FADE_STEPS)
				fadeSteps = TEXTEXT_MAX_FADE_STEPS;
			prefixWidth = xfont_Get_String_Width_0(fontId, prefixBuffer);
			fontHeight = xfont_Get_FontID_Height(fontId);
			for (color = TEXTEXT_FINAL_COLOR - fadeSteps; color <= TEXTEXT_FINAL_COLOR;
				 ++color, --prefixEnd) {
				if (prefixEnd >= 0)
					prefixBuffer[prefixEnd] = '\0';
				xfont_Print_Clipped_Text(prefixBuffer, x, y, fontId, color);
			}
			xrect_Set_Rect(&cursorRect, prefixWidth + x + TEXTEXT_CURSOR_LEFT_OFFSET, y,
						   prefixWidth + x + TEXTEXT_CURSOR_RIGHT_OFFSET, y + fontHeight);
			if (revealChars < textLength)
				xpaint_Paint_Clipped_Rect(&cursorRect, TEXTEXT_CURSOR_COLOR);
		} else {
			strcpy(prefixBuffer, text);
			xfont_Print_Clipped_Text(prefixBuffer, x, y, fontId, TEXTEXT_FINAL_COLOR);
		}
	}
}
