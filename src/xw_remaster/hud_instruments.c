#include "xw_remaster/hud_draw.h"
#include <math.h>

static bool Part(const XwHudDraw* d, unsigned index, int x, int y, unsigned key, bool mirror) {
	if (index >= XW_SNAP_PANEL_SPRITES)
		return false;
	XwCockpitPart part = d->definition->parts[index];
	XwHudImageKey image_key = { part.source, part.frame, key, XW_HUD_IMAGE_PART };
	const XwHudImage* image = XwHudAssets_Image(image_key);
	if (!image)
		return false;
	XwSnapRect rect = { mirror ? x - image->bitmap.width + 1 : x, y, image->bitmap.width,
						image->bitmap.height };
	return XwHudDraw_Image(d, image_key, rect,
						   (XwSnapRect) { 0, 0, d->definition->width, d->definition->height }, mirror);
}

static void Throttle(const XwHudDraw* d, const XwCockpitElement* e, const XwSnapWidget* w) {
	bool low = d->definition->width == 320;
	unsigned columns = low ? 38 : 74;
	for (unsigned i = 0; i < columns; ++i) {
		unsigned color = i >= w->value ? 0
									   : (i >= columns - (low ? 4 : 7)    ? 54
										  : i >= columns - (low ? 8 : 14) ? 58
																		  : 62) -
											 (i & 1 ? 2 : 0);
		XwSnapRect rect = { e->x + (int)i, e->y, 1, low ? 1 : 2 };
		XwHudDraw_Fill(d, rect, rect, color, true);
	}
}

static void Marker(const XwHudDraw* d, int x, int y, bool cross, unsigned color) {
	static const int8_t bracket[10][2] = { { -1, 1 }, { -2, 1 }, { -2, 0 }, { -2, -1 }, { -1, -1 },
										   { 1, -1 }, { 2, -1 }, { 2, 0 },  { 2, 1 },   { 1, 1 } };
	static const int8_t cross_points[7][2] = { { -2, 0 }, { -1, 0 }, { 0, 0 }, { 1, 0 },
											   { 2, 0 },  { 0, 1 },  { 0, -1 } };
	static const int8_t tall[12][2] = { { -1, 2 }, { -2, 2 }, { -2, 1 }, { -2, 0 }, { -2, -1 }, { -1, -1 },
										{ 1, -1 }, { 2, -1 }, { 2, 0 },  { 2, 1 },  { 2, 2 },   { 1, 2 } };
	bool taller = !cross && d->snapshot->cockpit.radar.tall_bracket;
	const int8_t (*points)[2] = cross ? cross_points : taller ? tall : bracket;
	for (unsigned i = 0; i < (cross ? 7u : taller ? 12u : 10u); ++i) {
		XwSnapRect rect = { x + points[i][0], y + points[i][1], 1, 1 };
		XwHudDraw_Fill(d, rect, rect, color, true);
	}
}

static void Blips(const XwHudDraw* d, const XwSnapRadarBlip* blips, unsigned count) {
	for (unsigned i = 0; i < count; ++i)
		for (unsigned row = 0; row < 2; ++row)
			if (blips[i].coverage & (1u << row)) {
				XwSnapRect rect = { blips[i].x, blips[i].y + (int)row, 1, 1 };
				XwHudDraw_Fill(d, rect, rect, blips[i].color_index, true);
			}
}

bool XwHudDraw_Instruments(const XwHudDraw* d) {
	const XwSnapCockpit* c = &d->snapshot->cockpit;
	for (unsigned i = 0; i < XW_SNAP_WIDGETS; ++i) {
		const XwSnapWidget* w = &c->widgets[i];
		const XwCockpitElement* e = &d->definition->elements[i];
		if (!w->visible)
			continue;
		if (w->kind == XW_SNAP_WIDGET_SPRITE) {
			if (!Part(d, e->sprite + w->value, e->x, e->y, e->selector, false))
				return false;
		} else if (w->kind == XW_SNAP_WIDGET_GAUGE || w->kind == XW_SNAP_WIDGET_LASER) {
			for (unsigned j = 0; j < w->segments; ++j) {
				unsigned state = j < w->value ? w->filled_state : w->empty_state;
				if (!Part(d, e->sprite + state, e->x + j * w->step_x, e->y - j * w->step_y, 253,
						  w->kind == XW_SNAP_WIDGET_LASER && e->selector != 0))
					return false;
			}
		} else if (w->kind == XW_SNAP_WIDGET_THROTTLE)
			Throttle(d, e, w);
	}
	const XwSnapRadar* r = &c->radar;
	if (r->front_visible)
		Blips(d, r->front, r->front_count);
	if (r->rear_visible)
		Blips(d, r->rear, r->rear_count);
	if (r->target_visible)
		Marker(d, r->target_x, r->target_y, false, 7);
	return true;
}

/* The target computer cross overlays its pane art, unlike the radar bracket. */
void XwHudDraw_Cross(const XwHudDraw* d) {
	const XwSnapRadar* r = &d->snapshot->cockpit.radar;
	if (r->cross_visible)
		Marker(d, r->cross_x, r->cross_y, true, r->cross_color);
}

static void TargetStrip(const XwHudDraw* d, const XwRenderView* view, float x, float y, float width,
						float height, float depth, const float rgba[4]) {
	AeronRectI clip = { 0, 0, (int)d->layout.target_width, (int)d->layout.target_height };
	if (d->snapshot->target_box.direct_overlay) {
		AeronDrawList_AddFill(d->list, x, y, width, height, rgba, AERON_BLIT2D_BLEND_PMA, &clip);
		return;
	}
	/* Inset endpoints by the cap radius so the projected line fills exactly this strip. */
	float thickness = fminf(width, height), inset = thickness * .5f;
	AeronDrawList_AddProjectedLine(d->list, x + inset, y + inset, depth, x + width - inset,
								   y + height - inset, depth, view->camera.near_z, thickness, rgba,
								   AERON_BLIT2D_BLEND_PMA, &clip);
}

void XwHudDraw_Target(const XwHudDraw* d, const XwRenderView* view) {
	const XwSnapTargetBox* box = &d->snapshot->target_box;
	float x, y, depth;
	if (!view || !box->visible || !XwRenderMath_ProjectWorld(view, box->world_pos, &x, &y, &depth))
		return;
	const XwSnapCamera* camera = &d->snapshot->camera;
	float projected = camera->focal_x * (float)box->extent / depth;
	/* Targeting_DrawObjectBox truncates before padding and centering in classic pixels. */
	int size =
		(int)fminf(fmaxf(projected, camera->screen_width == 320 ? 4 : 8), camera->screen_width * .75f) + 4;
	int corner = size >> 3;
	if (corner < 3)
		corner = 3;
	float rgba[4];
	XwHudDraw_Color(d, box->color_index, true, rgba);
	float scale_x = view->classic_pixel_scale_x, scale_y = view->classic_pixel_scale_y;
	x -= (size / 2) * scale_x;
	y -= (size / 2) * scale_y;
	bool hardware =
		!box->direct_overlay && camera->legacy_render_convention == XW_SNAP_CLASSIC_WINDOWS_HARDWARE;
	/* Hardware strips start at size; software's last pixel is size - 1. */
	int edge = size - !hardware;
	for (unsigned i = 0; i < 4; ++i) {
		bool right = (i & 1) != 0, bottom = (i & 2) != 0;
		/* Hud_DrawBoxOverlayHW extends only the bottom-right horizontal strip by one pixel. */
		TargetStrip(d, view, x + (right ? size - corner : 0) * scale_x, y + (bottom ? edge : 0) * scale_y,
					(corner + (hardware && right && bottom)) * scale_x, scale_y, depth, rgba);
		TargetStrip(d, view, x + (right ? edge : 0) * scale_x,
					y + (bottom ? size - corner : !hardware) * scale_y, scale_x,
					(corner - !hardware) * scale_y, depth, rgba);
	}
}
