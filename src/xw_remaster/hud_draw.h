#ifndef XW_REMASTER_HUD_DRAW_H
#define XW_REMASTER_HUD_DRAW_H
#include "xw_remaster/hud_assets.h"
#include "xw_remaster/render_math.h"
#include <aeron/scene/draw_list2d.h>

typedef struct XwHudDraw {
	AeronDrawList2D* list;
	const XwRenderSnapshot* snapshot;
	const XwCockpitDefinition* definition;
	XwLayoutTransform layout;
	float offset_x, offset_y;
} XwHudDraw;

void XwHudDraw_Color(const XwHudDraw* draw, unsigned color, bool indexed, float rgba[4]);
void XwHudDraw_Fill(const XwHudDraw* draw, XwSnapRect rect, XwSnapRect clip, unsigned color, bool indexed);
bool XwHudDraw_Base(const XwHudDraw* draw);
bool XwHudDraw_Image(const XwHudDraw* draw, XwHudImageKey key, XwSnapRect rect, XwSnapRect clip,
					 bool mirrored);
bool XwHudDraw_Glyph(const XwHudDraw* draw, const XwSnapGlyph* glyph);
bool XwHudDraw_Instruments(const XwHudDraw* draw);
void XwHudDraw_Target(const XwHudDraw* draw, const XwRenderView* view);
#endif
