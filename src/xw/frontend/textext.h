#ifndef XW_FRONTEND_TEXTEXT_H
#define XW_FRONTEND_TEXTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	TEXTEXT_PREFIX_CAPACITY = 64,
	TEXTEXT_CHARACTERS_PER_STEP = 2,
	TEXTEXT_FINAL_COLOR = 62,
	TEXTEXT_MAX_FADE_STEPS = 3,
	TEXTEXT_REVEAL_FINISH_DELAY = 2,
	TEXTEXT_CURSOR_LEFT_OFFSET = 2,
	TEXTEXT_CURSOR_RIGHT_OFFSET = 8,
	TEXTEXT_CURSOR_COLOR = 2
};

/* Declarations follow ascending original IDB address. */

/* 0x441240 */
void textext_Draw_Typewriter_Line(const char* text, uint16_t fontId, int16_t x, int16_t y,
								  int16_t revealChars);

#ifdef __cplusplus
}
#endif

#endif
