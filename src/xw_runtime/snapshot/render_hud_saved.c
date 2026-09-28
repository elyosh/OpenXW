#include "xw_runtime/snapshot/render_hud_internal.h"
#include <string.h>

/* Original save boxes have finite lifetimes on the owner thread. No pixels or GPU readback. */
static struct {
	const void* key;
	XwSnapRect rect;
	XwHudState state;
} saved[4];

void XwHud_ClearSaved(void) {
	for (unsigned i = 0; i < 4; ++i) {
		XwHud_Release(&saved[i].state.cockpit);
		saved[i].key = NULL;
	}
}

void XwHud_Save(const void* key, int x, int y, int width, int height) {
	XwHudState* s = XwHud_Working();
	if (!s || !key || !s->valid)
		return;
	unsigned i = 0;
	while (i < 4 && saved[i].key && saved[i].key != key)
		++i;
	if (i == 4) {
		XwHud_Fail(s, "saved HUD owner capacity exceeded");
		return;
	}
	saved[i].key = key;
	saved[i].rect = (XwSnapRect) { x, y, width, height };
	XwHud_CopyState(&saved[i].state, s);
}

/* Box restores replace their finite pane regions; unaffected panes retain later changes. */
void XwHud_Restore(const void* key) {
	XwHudState* s = XwHud_Working();
	if (!s)
		return;
	unsigned i = 0;
	while (i < 4 && saved[i].key != key)
		++i;
	if (i == 4) {
		XwHud_Fail(s, "missing saved HUD owner");
		return;
	}
	XwHudState* old = &saved[i].state;
	XwSnapRect rect = saved[i].rect;
	XwHud_Erase(s, rect);
	/* Clip a temporary saved owner to the restored box by erasing its exterior. */
	XwHud_Erase(old, (XwSnapRect) { 0, 0, old->cockpit.screen_width, rect.y });
	XwHud_Erase(old, (XwSnapRect) { 0, rect.y + rect.height, old->cockpit.screen_width,
									old->cockpit.screen_height - rect.y - rect.height });
	XwHud_Erase(old, (XwSnapRect) { 0, rect.y, rect.x, rect.height });
	XwHud_Erase(old, (XwSnapRect) { rect.x + rect.width, rect.y,
									old->cockpit.screen_width - rect.x - rect.width, rect.height });
	XwSnapCockpit* dst = &s->cockpit;
	const XwSnapCockpit* src = &old->cockpit;
	if (dst->glyph_count + src->glyph_count > XW_SNAP_HUD_GLYPHS ||
		dst->paint_count + src->paint_count > XW_SNAP_HUD_PAINT ||
		dst->sprite_count + src->sprite_count > XW_SNAP_HUD_SPRITES) {
		XwHud_Fail(s, "restored HUD capacity exceeded");
		return;
	}
	uint32_t base = s->order;
	for (unsigned j = 0; j < src->glyph_count; ++j) {
		unsigned at = dst->glyph_count++;
		dst->glyphs[at] = src->glyphs[j];
		dst->glyphs[at].order += base;
		s->glyph_pane[at] = old->glyph_pane[j];
		XwRenderAssets_Retain(dst->glyphs[at].font);
	}
	for (unsigned j = 0; j < src->paint_count; ++j) {
		unsigned at = dst->paint_count++;
		dst->paint[at] = src->paint[j];
		dst->paint[at].order += base;
		s->paint_pane[at] = old->paint_pane[j];
	}
	for (unsigned j = 0; j < src->sprite_count; ++j) {
		unsigned at = dst->sprite_count++;
		dst->sprites[at] = src->sprites[j];
		dst->sprites[at].order += base;
		s->sprite_pane[at] = old->sprite_pane[j];
		XwRenderAssets_Retain(dst->sprites[at].image);
	}
	/* Panel decisions cannot run under the modal task owning this save lifetime. */
	memcpy(dst->widgets, src->widgets, sizeof dst->widgets);
	dst->radar = src->radar;
	s->order += old->order;
	XwHud_Changed(s);
	XwHud_Release(&old->cockpit);
	saved[i].key = NULL;
}
