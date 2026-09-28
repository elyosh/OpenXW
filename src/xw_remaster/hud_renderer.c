/* Native-resolution composition follows OpenXvT's retained Aeron draw-list path. */
#include "xw_remaster/hud_renderer.h"
#include "xw_remaster/hud_draw.h"
#include <aeron/aeron.h>
#include <stdlib.h>
#include <string.h>

typedef struct HudKey {
	uint64_t mission, world, revision, palette, assets;
	XwRenderView view;
	XwSnapTargetBox target;
	int width, height;
} HudKey;

typedef struct Ordered {
	uint32_t order, index;
	uint8_t kind;
} Ordered;

static AeronDrawList2D *list, *markers;
static HudKey prepared;
static bool ready, valid;
void XwHudDraw_Cross(const XwHudDraw* draw);

static int Compare(const void* a, const void* b) {
	const Ordered *x = a, *y = b;
	return (x->order > y->order) - (x->order < y->order);
}

static bool Panes(const XwHudDraw* draw) {
	const XwSnapCockpit* c = &draw->snapshot->cockpit;
	Ordered order[XW_SNAP_HUD_GLYPHS + XW_SNAP_HUD_PAINT + XW_SNAP_HUD_SPRITES];
	unsigned n = 0;
	for (unsigned i = 0; i < c->glyph_count; ++i)
		order[n++] = (Ordered) { c->glyphs[i].order, i, 0 };
	for (unsigned i = 0; i < c->paint_count; ++i)
		order[n++] = (Ordered) { c->paint[i].order, i, 1 };
	for (unsigned i = 0; i < c->sprite_count; ++i)
		order[n++] = (Ordered) { c->sprites[i].order, i, 2 };
	qsort(order, n, sizeof order[0], Compare);
	for (unsigned i = 0; i < n; ++i) {
		unsigned index = order[i].index;
		if (order[i].kind == 0) {
			if (!XwHudDraw_Glyph(draw, &c->glyphs[index]))
				return false;
		} else if (order[i].kind == 1) {
			const XwSnapHudPaint* p = &c->paint[index];
			XwHudDraw_Fill(draw, (XwSnapRect) { p->x0, p->y0, p->x1 - p->x0, p->y1 - p->y0 }, p->clip,
						   p->color, false);
		} else {
			const XwSnapHudSprite* s = &c->sprites[index];
			if (!XwHudDraw_Image(
					draw, (XwHudImageKey) { s->image, s->frame, s->transparent_color, XW_HUD_IMAGE_PART },
					s->destination, s->clip, s->mirrored))
				return false;
		}
	}
	return true;
}

bool XwHudRenderer_Prepare(const XwRenderSnapshot* snapshot, const XwRenderView* view, int width,
						   int height) {
	ready = false;
	if (!snapshot || !snapshot->hud_valid || width <= 0 || height <= 0)
		return false;
	const XwSnapCockpit* c = &snapshot->cockpit;
	if (c->glyph_count > XW_SNAP_HUD_GLYPHS || c->paint_count > XW_SNAP_HUD_PAINT ||
		c->sprite_count > XW_SNAP_HUD_SPRITES)
		return false;
	if (!XwHudAssets_Select(snapshot)) {
		valid = false;
		return false;
	}
	HudKey key = { .mission = snapshot->key.mission_generation,
				   .world = snapshot->key.world_generation,
				   .revision = snapshot->key.hud_revision,
				   .palette = snapshot->appearance.palette_revision,
				   .assets = XwHudAssets_Generation(),
				   .target = snapshot->target_box,
				   .width = width,
				   .height = height };
	if (view)
		key.view = *view;
	if (valid && !memcmp(&key, &prepared, sizeof key)) {
		ready = true;
		return true;
	}
	valid = false;
	if (!list)
		list = AeronDrawList_Create(32768);
	if (!markers)
		markers = AeronDrawList_Create(16);
	if (!list || !markers)
		return false;
	XwHudDraw draw = { .list = list,
					   .snapshot = snapshot,
					   .definition = XwCockpitAssets_Definition(c->definition) };
	if (!draw.definition ||
		!XwRenderMath_Layout(c->screen_width, c->screen_height, width, height, &draw.layout))
		return false;
	draw.offset_x = (width - c->screen_width * draw.layout.scale_x) / 2;
	draw.offset_y = (height - c->screen_height * draw.layout.scale_y) / 2;
	AeronDrawList_Begin(list, NULL, width, height, AERON_DRAWLIST2D_LOAD, NULL);
	AeronDrawList_Begin(markers, NULL, width, height, AERON_DRAWLIST2D_LOAD, NULL);
	if (!snapshot->target_box.direct_overlay)
		draw.list = markers;
	XwHudDraw_Target(&draw, view);
	draw.list = list;
	XwSnapRect full = { 0, 0, c->screen_width, c->screen_height };
	if (draw.definition->base &&
		!XwHudDraw_Image(&draw,
						 (XwHudImageKey) { .source = draw.definition->base, .kind = XW_HUD_IMAGE_BASE }, full,
						 full, c->mirrored))
		return false;
	if (!XwHudDraw_Instruments(&draw) || !Panes(&draw))
		return false;
	XwHudDraw_Cross(&draw);
	AeronCommandBuffer* cmd = Aeron_AcquireCommandBuffer();
	if (!cmd)
		return false;
	if (!AeronDrawList_Prepare(list, cmd) || !AeronDrawList_Prepare(markers, cmd)) {
		Aeron_CancelCommandBuffer(cmd);
		return false;
	}
	if (!Aeron_SubmitCommandBuffer(cmd))
		return false;
	prepared = key;
	valid = ready = true;
	return true;
}

bool XwHudRenderer_PrepareWorldMarkers(AeronCommandBuffer* cmd, const XwRenderSnapshot* s,
									   const XwRenderView* source_view, AeronScene3D* scene) {
	if (!ready || !markers)
		return false;
	int w, h;
	AeronScene_RenderDims(scene, &w, &h);
	XwRenderView view = *source_view;
	view.classic_pixel_scale_x *= (float)w / view.camera.viewport.width;
	view.classic_pixel_scale_y *= (float)h / view.camera.viewport.height;
	view.camera.viewport = (AeronRectI) { 0, 0, w, h };
	memcpy(view.view_proj, AeronScene_JitteredViewProj(scene), sizeof view.view_proj);
	XwHudDraw draw = { .list = markers,
					   .snapshot = s,
					   .definition = XwCockpitAssets_Definition(s->cockpit.definition) };
	if (!draw.definition ||
		!XwRenderMath_Layout(s->cockpit.screen_width, s->cockpit.screen_height, w, h, &draw.layout))
		return false;
	AeronDrawList_Begin(markers, NULL, w, h, AERON_DRAWLIST2D_LOAD, NULL);
	if (!s->target_box.direct_overlay)
		XwHudDraw_Target(&draw, &view);
	return AeronDrawList_Prepare(markers, cmd);
}

void XwHudRenderer_DrawWorldMarkers(AeronCommandBuffer* cmd, AeronRenderPass* pass,
									AeronRenderTarget* target) {
	if (ready)
		AeronDrawList_RenderIntoPass(markers, cmd, pass, target);
}

void XwHudRenderer_Draw(AeronCommandBuffer* cmd, AeronRenderPass* pass, AeronRenderTarget* target) {
	if (ready)
		AeronDrawList_RenderIntoPass(list, cmd, pass, target);
}

void XwHudRenderer_Shutdown(void) {
	AeronDrawList_Destroy(markers);
	markers = NULL;
	AeronDrawList_Destroy(list);
	list = NULL;
	XwHudAssets_Shutdown();
	ready = valid = false;
	memset(&prepared, 0, sizeof prepared);
}
