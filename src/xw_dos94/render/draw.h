#ifndef XW_DOS94_DRAW_H
#define XW_DOS94_DRAW_H
#include "xw_dos94/render/transfm2.h"
#include <stdint.h>
/* Submit one selected component with the caller's parent, transform and detail state. */
void Dos94_DRAW_drawcomponent(uint16_t model, uint16_t component, Dos94EyePoint eye);
void Dos94_DRAW_drawcomplexobject(uint16_t index);
void Dos94_DRAW_drawcraft(uint16_t index, uint16_t model, const uint16_t* order, unsigned count);
void Dos94_ANIM_drawverysimpleobject(uint16_t index);
void Dos94_DRAW_drawlaser(uint16_t index);
void Dos94_DRAW_drawhyperstar(uint16_t index);
void Dos94_ANIM_sort_and_draw_bitmaps(void);
void Dos94_DRAW_drawbackdropimage(uint16_t model, int16_t x, int16_t y, int16_t angle);
void Dos94_static_drawstaticobject(uint16_t index);
#endif
