#ifndef XW_RENDER_HUD_INTERNAL_H
#define XW_RENDER_HUD_INTERNAL_H
#include "xw_runtime/snapshot/render_hud.h"

typedef struct XwHudState {
	XwSnapCockpit cockpit;
	uint8_t glyph_pane[XW_SNAP_HUD_GLYPHS], paint_pane[XW_SNAP_HUD_PAINT], sprite_pane[XW_SNAP_HUD_SPRITES];
	uint64_t revision;
	uint32_t order;
	bool valid;
} XwHudState;

void XwHud_ClearSaved(void);
XwHudState* XwHud_Working(void);
int XwHud_Pane(void);
void XwHud_Changed(XwHudState* state);
void XwHud_Fail(XwHudState* state, const char* reason);
XwSnapRect XwHud_Clip(void);
uint16_t XwHud_Color(uint8_t index);
void XwHud_Erase(XwHudState* state, XwSnapRect rect);
void XwHud_CopyState(XwHudState* dst, const XwHudState* src);
#endif
