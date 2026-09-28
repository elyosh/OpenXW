/* Resident cockpit families follow OpenXvT: shared parts, separate view bases,
 * preparation before selection, and retirement driven by original resource bindings. */
#include "xw_remaster/hud_assets.h"
#include "xw_remaster/config.h"
#include "xw_remaster/hud_asset_upload.h"
#include "xw_remaster/hud_decode.h"
#include <aeron/aeron.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { VIEW_CAPACITY = XW_SNAP_COCKPIT_VIEWS + 2 };

typedef struct HudVariant {
	AeronRuntimeAtlas atlas;
	uint32_t palette[256];
	unsigned count;
} HudVariant;

typedef struct HudBase {
	XwHudImage image;
	AeronImageCoverage coverage;
	XwSnapRect aperture;
	bool used[256];
	HudVariant prepared;
} HudBase;

typedef struct HudGroup {
	struct HudGroup* next;
	XwRenderAssetId definition;
	uint64_t mission;
	bool filter, used[256];
	unsigned count, base_count;
	XwHudImage images[XW_HUD_IMAGES];
	HudVariant parts[VIEW_CAPACITY];
	HudBase bases[VIEW_CAPACITY];
} HudGroup;

static HudGroup *groups, *current;
static HudVariant* current_parts;
static HudBase* current_base;
static uint64_t generation;

static bool Same(XwHudImageKey a, XwHudImageKey b) {
	return a.source == b.source && a.frame == b.frame && a.transparent == b.transparent && a.kind == b.kind;
}

static bool Failure(XwHudImageKey key, const char* reason) {
	const XwRenderSource* source = XwRenderAssets_Source(key.source);
	char message[1536];
	snprintf(message, sizeof message, "Unable to prepare cockpit image %s (kind %u, frame %u):\n\n%s",
			 source ? source->path : "<unavailable>", key.kind, key.frame, reason);
	Aeron_RequestFatalError("Renderer Error", message);
	return false;
}

static void ReleaseImage(XwHudImage* image) {
	AeronIndexedFrame_Free(&image->bitmap);
	AeronDecodedFont_Free(&image->font);
	XwRenderAssets_Release(image->key.source);
}

static void Release(HudGroup* group) {
	if (current == group) {
		current = NULL;
		current_parts = NULL;
		current_base = NULL;
	}
	for (unsigned i = 0; i < group->count; ++i)
		ReleaseImage(&group->images[i]);
	for (unsigned i = 0; i < group->base_count; ++i) {
		ReleaseImage(&group->bases[i].image);
		Aeron_ImageFreeCoverage(&group->bases[i].coverage);
		Aeron_RuntimeAtlasRelease(&group->bases[i].prepared.atlas);
	}
	for (unsigned i = 0; i < VIEW_CAPACITY; ++i)
		Aeron_RuntimeAtlasRelease(&group->parts[i].atlas);
	XwRenderAssets_Release(group->definition);
	free(group);
}

/* Selected base/aperture and current display palette are view state, not loaded-resource identity. */
static bool Matches(const HudGroup* group, const XwCockpitDefinition* d, uint64_t mission) {
	const XwCockpitDefinition* old = XwCockpitAssets_Definition(group->definition);
	if (!d || !old || group->mission != mission || old->version != d->version || old->width != d->width ||
		old->height != d->height || old->color_mode != d->color_mode ||
		old->brightness_q8 != d->brightness_q8 || memcmp(old->parts, d->parts, sizeof d->parts) ||
		memcmp(old->fonts, d->fonts, sizeof d->fonts))
		return false;
	for (unsigned i = 0; i < XW_SNAP_COCKPIT_VIEWS; ++i)
		if (old->views[i].image != d->views[i].image ||
			memcmp(&old->views[i].aperture, &d->views[i].aperture, sizeof(XwSnapRect)))
			return false;
	return true;
}

static bool PaletteMatches(const HudVariant* variant, const uint32_t colors[256], const bool used[256],
						   bool filter) {
	if (!variant->count)
		return false;
	/* Undithering's nearest-color search considers the whole palette. */
	for (unsigned i = 0; i < 256; ++i)
		if ((filter || used[i]) && variant->palette[i] != colors[i])
			return false;
	return true;
}

static bool Decode(XwHudImage* image, XwHudImageKey key, const XwCockpitDefinition* d, bool used[256]) {
	image->key = key;
	XwRenderAssets_Retain(key.source);
	AeronDecodeError error = { 0 };
	if (!key.source || !XwHudDecode_Image(image, d, &error))
		return Failure(key, error.message[0] ? error.message : "Missing registered image source");
	for (size_t i = 0; i < (size_t)image->bitmap.width * image->bitmap.height; ++i)
		if (image->bitmap.coverage[i])
			used[image->bitmap.indices[i]] = true;
	return true;
}

static bool Request(HudGroup* group, XwHudImageKey key, const XwCockpitDefinition* d) {
	for (unsigned i = 0; i < group->count; ++i)
		if (Same(group->images[i].key, key))
			return true;
	if (group->count == XW_HUD_IMAGES)
		return Failure(key, "Resident image capacity exceeded");
	unsigned index = group->count++;
	group->images[index].atlas_frame = index;
	return Decode(&group->images[index], key, d, group->used);
}

static bool Requests(HudGroup* group, const XwCockpitDefinition* d, const XwSnapCockpit* c) {
	if (!group->count) {
		for (unsigned i = 0; i < XW_SNAP_PANEL_SPRITES; ++i) {
			XwCockpitPart p = d->parts[i];
			if (p.source &&
				(!Request(group, (XwHudImageKey) { p.source, p.frame, 0, XW_HUD_IMAGE_PART }, d) ||
				 !Request(group, (XwHudImageKey) { p.source, p.frame, 253, XW_HUD_IMAGE_PART }, d)))
				return false;
		}
		for (unsigned i = 0; i < 2; ++i)
			if (!Request(group, (XwHudImageKey) { .source = d->fonts[i], .kind = XW_HUD_IMAGE_FONT }, d))
				return false;
	}
	if (!c)
		return true;
	for (unsigned i = 0; i < XW_SNAP_WIDGETS; ++i) {
		const XwSnapWidget* w = &c->widgets[i];
		const XwCockpitElement* e = &d->elements[i];
		if (!w->visible || w->kind != XW_SNAP_WIDGET_SPRITE)
			continue;
		unsigned frame = e->sprite + w->value;
		if (frame >= XW_SNAP_PANEL_SPRITES)
			return false;
		XwCockpitPart p = d->parts[frame];
		if (!Request(group, (XwHudImageKey) { p.source, p.frame, e->selector, XW_HUD_IMAGE_PART }, d))
			return false;
	}
	for (unsigned i = 0; i < c->sprite_count; ++i) {
		const XwSnapHudSprite* s = &c->sprites[i];
		if (!Request(group, (XwHudImageKey) { s->image, s->frame, s->transparent_color, XW_HUD_IMAGE_PART },
					 d))
			return false;
	}
	for (unsigned i = 0; i < c->glyph_count; ++i)
		if (!Request(group, (XwHudImageKey) { .source = c->glyphs[i].font, .kind = XW_HUD_IMAGE_FONT }, d))
			return false;
	return true;
}

static bool Upload(AeronCommandBuffer** cmd, HudVariant* variant, const XwHudImage* images, unsigned count,
				   const uint32_t colors[256], bool filter, AeronImageCoverage* coverage) {
	if (!*cmd)
		*cmd = Aeron_AcquireCommandBuffer();
	if (!*cmd)
		return false;
	AeronRuntimeAtlas next = { 0 };
	AeronImageCoverage next_coverage = { 0 };
	if (!XwHudAssetUpload(*cmd, images, count, colors, filter, &next, coverage ? &next_coverage : NULL))
		return false;
	if (coverage) {
		Aeron_ImageFreeCoverage(coverage);
		*coverage = next_coverage;
	}
	Aeron_RuntimeAtlasRelease(&variant->atlas);
	variant->atlas = next;
	memcpy(variant->palette, colors, sizeof variant->palette);
	variant->count = count;
	return true;
}

static HudVariant* FindParts(HudGroup* group, const uint32_t colors[256]) {
	for (unsigned i = 0; i < VIEW_CAPACITY; ++i)
		if (group->parts[i].count == group->count &&
			PaletteMatches(&group->parts[i], colors, group->used, group->filter))
			return &group->parts[i];
	return NULL;
}

static bool PrepareParts(AeronCommandBuffer** cmd, HudGroup* group, const uint32_t colors[256]) {
	if (FindParts(group, colors))
		return true;
	unsigned index = 0;
	while (index + 1 < VIEW_CAPACITY && group->parts[index].count &&
		   !PaletteMatches(&group->parts[index], colors, group->used, group->filter))
		++index;
	/* The final slot handles live DAC/palette changes without unbounded color variants. */
	return Upload(cmd, &group->parts[index], group->images, group->count, colors, group->filter, NULL);
}

static HudBase* FindBase(HudGroup* group, XwRenderAssetId source, XwSnapRect aperture) {
	for (unsigned i = 0; i < group->base_count; ++i)
		if (group->bases[i].image.key.source == source &&
			!memcmp(&group->bases[i].aperture, &aperture, sizeof aperture))
			return &group->bases[i];
	return NULL;
}

static bool PrepareBase(AeronCommandBuffer** cmd, HudGroup* group, const XwCockpitDefinition* d,
						const uint32_t colors[256]) {
	if (!d->base)
		return true;
	HudBase* base = FindBase(group, d->base, d->aperture);
	if (!base) {
		if (group->base_count == VIEW_CAPACITY)
			return false;
		base = &group->bases[group->base_count++];
		base->aperture = d->aperture;
		if (!Decode(&base->image, (XwHudImageKey) { .source = d->base, .kind = XW_HUD_IMAGE_BASE }, d,
					base->used))
			return false;
	}
	return PaletteMatches(&base->prepared, colors, base->used, group->filter) ||
		   Upload(cmd, &base->prepared, &base->image, 1, colors, group->filter, &base->coverage);
}

static bool PrepareFamily(AeronCommandBuffer** cmd, HudGroup* group, const XwCockpitDefinition* d) {
	if (!PrepareParts(cmd, group, d->palette_argb))
		return false;
	XwCockpitDefinition view = *d;
	for (unsigned i = 0; i < XW_SNAP_COCKPIT_VIEWS; ++i) {
		const XwCockpitView* v = &d->views[i];
		if (!v->image)
			continue;
		if (v->resource >= XW_SNAP_COCKPIT_VIEWS)
			return false;
		view.base = v->image;
		view.aperture = d->views[v->resource].aperture;
		uint32_t colors[256];
		memcpy(colors, d->palette_argb, sizeof colors);
		memcpy(colors, v->palette_argb, sizeof v->palette_argb);
		if (!PrepareBase(cmd, group, &view, colors) || !PrepareParts(cmd, group, colors))
			return false;
	}
	return true;
}

static bool PrepareDefinition(AeronCommandBuffer** cmd, const XwRenderSnapshot* s, XwRenderAssetId id,
							  const XwSnapCockpit* cockpit) {
	if (!id)
		return true;
	const XwCockpitDefinition* d = XwCockpitAssets_Definition(id);
	if (!d)
		return false;
	HudGroup* group = groups;
	while (group && !Matches(group, d, s->key.mission_generation))
		group = group->next;
	bool created = !group;
	if (created) {
		group = calloc(1, sizeof *group);
		if (!group)
			return false;
		group->definition = id;
		group->mission = s->key.mission_generation;
		/* Like OpenXvT, artwork filtering is fixed for the original resource lifetime. */
		group->filter = d->version == 98 && XwRemasterConfig_Effective()->cockpit_undither;
		XwRenderAssets_Retain(id);
		group->next = groups;
		groups = group;
	}
	if (!created && !cockpit)
		return true;
	unsigned previous_count = group->count;
	if (!Requests(group, d, cockpit))
		return false;
	if ((created || previous_count != group->count) && !PrepareFamily(cmd, group, d))
		return false;
	if (cockpit) {
		const uint32_t* colors = s->flight_version == 98 ? cockpit->palette_argb : s->appearance.palette_argb;
		if (!PrepareParts(cmd, group, colors) || !PrepareBase(cmd, group, d, colors))
			return false;
	}
	return true;
}

bool XwHudAssets_Prepare(const XwRenderSnapshot* s) {
	const XwCockpitDefinition* loaded = s ? XwCockpitAssets_Definition(s->loaded_cockpit) : NULL;
	const XwCockpitDefinition* shown = s ? XwCockpitAssets_Definition(s->cockpit.definition) : NULL;
	HudGroup** link = &groups;
	while (*link) {
		HudGroup* g = *link;
		bool resident = s && s->flight_version && g->mission == s->key.mission_generation &&
						((!loaded && !shown) || Matches(g, loaded, s->key.mission_generation) ||
						 Matches(g, shown, s->key.mission_generation));
		if (resident)
			link = &g->next;
		else {
			*link = g->next;
			Release(g);
			++generation;
		}
	}
	if (!s || !s->flight_version)
		return true;
	AeronCommandBuffer* cmd = NULL;
	uint64_t start = Aeron_NowUs();
	bool ok = PrepareDefinition(&cmd, s, s->loaded_cockpit, NULL) &&
			  PrepareDefinition(&cmd, s, s->cockpit.definition, s->hud_valid ? &s->cockpit : NULL);
	if (ok && cmd) {
		AeronCommandBufferUploadUsage usage;
		ok = Aeron_CommandBufferGetUploadUsage(cmd, &usage) && usage.staged_bytes <= 64u * 1024u * 1024u &&
			 usage.copy_count <= 4096;
	}
	if (!ok && cmd)
		Aeron_CancelCommandBuffer(cmd);
	else if (cmd)
		ok = Aeron_SubmitCommandBuffer(cmd);
	if (!ok) {
		Aeron_RequestFatalRendererError("resident cockpit resource preparation");
		return false;
	}
	if (cmd) {
		++generation;
		Aeron_LogDebug("xw.hud", "Resident cockpit resources prepared in %.1f ms",
					   (Aeron_NowUs() - start) / 1000.0);
	}
	return true;
}

bool XwHudAssets_Select(const XwRenderSnapshot* s) {
	current = NULL;
	current_parts = NULL;
	current_base = NULL;
	const XwCockpitDefinition* d = XwCockpitAssets_Definition(s->cockpit.definition);
	for (HudGroup* g = groups; g; g = g->next) {
		if (!Matches(g, d, s->key.mission_generation))
			continue;
		const uint32_t* colors =
			s->flight_version == 98 ? s->cockpit.palette_argb : s->appearance.palette_argb;
		current_parts = FindParts(g, colors);
		current_base = d->base ? FindBase(g, d->base, d->aperture) : NULL;
		if (!current_parts || (d->base && (!current_base || !PaletteMatches(&current_base->prepared, colors,
																			current_base->used, g->filter))))
			break;
		current = g;
		return true;
	}
	Aeron_RequestFatalError("Renderer Error", "The selected cockpit view has no prepared resident artwork.");
	return false;
}

const XwHudImage* XwHudAssets_Image(XwHudImageKey key) {
	if (!current)
		return NULL;
	if (key.kind == XW_HUD_IMAGE_BASE)
		return current_base && Same(current_base->image.key, key) ? &current_base->image : NULL;
	for (unsigned i = 0; i < current->count; ++i)
		if (Same(current->images[i].key, key))
			return &current->images[i];
	return NULL;
}

const AeronRuntimeAtlas* XwHudAssets_Atlas(const XwHudImage* image) {
	return image && image->key.kind == XW_HUD_IMAGE_BASE
			   ? (current_base ? &current_base->prepared.atlas : NULL)
		   : current_parts ? &current_parts->atlas
						   : NULL;
}

const AeronImageCoverage* XwHudAssets_BaseCoverage(void) {
	return current_base ? &current_base->coverage : NULL;
}

uint64_t XwHudAssets_Generation(void) { return generation; }

bool XwHudAssets_UnditherPending(int requested) {
	for (const HudGroup* g = groups; g; g = g->next) {
		const XwCockpitDefinition* d = XwCockpitAssets_Definition(g->definition);
		if (d && d->version == 98 && g->filter != (requested != 0))
			return true;
	}
	return false;
}

void XwHudAssets_Shutdown(void) {
	while (groups) {
		HudGroup* next = groups->next;
		Release(groups);
		groups = next;
	}
	current = NULL;
	current_parts = NULL;
	current_base = NULL;
	generation = 0;
}
