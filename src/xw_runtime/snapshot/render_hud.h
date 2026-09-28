#ifndef XW_RENDER_HUD_H
#define XW_RENDER_HUD_H
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_cockpit_assets.h"
struct XwRadarBlip;
void XwHud_Target(uint16_t reference, uint16_t component, int extent, uint8_t color, bool overlay);
void XwHud_Reset(void);
void XwHud_Enable(bool enabled);
void XwHud_IndexedPalette(const uint32_t colors[256]);
/* Scope returns the previous pane; restore it on every exit, including cooperative yields. */
int XwHud_Push(int pane);
void XwHud_Pop(int previous);
void XwHud_ViewSelection(unsigned view);
void XwHud_View(unsigned view, const char* base, XwSnapRect aperture, bool mirrored);
void XwHud_Widget(unsigned element, unsigned value, unsigned segments, int step_x, int step_y, unsigned empty,
				  unsigned filled, unsigned kind);
void XwHud_Glyph(unsigned character, unsigned width, unsigned height);
void XwHud_Fill(int x0, int y0, int x1, int y1);
void XwHud_Sprite(const uint8_t* source, int x, int y, int transparent, int mirror);
void XwHud_Radar(struct XwRadarBlip* blips, unsigned count, bool completed);
void XwHud_Marker(bool cross, bool visible, int x, int y, unsigned color);
void XwHud_CopySurface(XwRenderSurface dst, XwRenderSurface src, bool complete);
void XwHud_ClearSurface(XwRenderSurface surface);
void XwHud_Present(XwRenderSnapshot* snapshot, XwRenderSurface surface, bool flip);
void XwHud_ExportDirect(XwRenderSnapshot* snapshot);
void XwHud_Save(const void* key, int x, int y, int width, int height);
void XwHud_Restore(const void* key);
void XwHud_Release(XwSnapCockpit* cockpit);
void XwHud_Copy(XwSnapCockpit* dst, const XwSnapCockpit* src);
#endif
