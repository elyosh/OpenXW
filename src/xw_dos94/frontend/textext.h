#ifndef XW_DOS94_FRONTEND_TEXTEXT_H
#define XW_DOS94_FRONTEND_TEXTEXT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void Dos94_DrawTypewriterLine(const char* text, uint16_t fontId, int16_t x, int16_t y, int16_t revealChars);
#ifdef __cplusplus
}
#endif
#endif
