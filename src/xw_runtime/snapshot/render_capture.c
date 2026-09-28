/* Presentation transactions retain one pending draw across cooperative yields. */
#include "xw_runtime/snapshot/render_capture.h"
#include "xw/flight/object/anim.h"
#include "xw_runtime/snapshot/render_assets.h"
#include "xw_runtime/snapshot/render_camera.h"
#include "xw_runtime/snapshot/render_hud.h"
#include "xw_runtime/snapshot/render_objects.h"
#include "xw_runtime/snapshot/render_sky.h"
#include "xw_runtime/snapshot/render_snapshot_internal.h"
#include "xw_runtime/snapshot/render_world.h"
#include "xw_runtime/timing/flight_timing.h"
#include <aeron/aeron.h>
#include <aeron/compat/host.h>
#include <string.h>

static XwRenderSnapshot pending;
static XwRenderSurfaceState surfaces[XW_RENDER_SURFACE_COUNT];
static uint64_t mission, world, view_serial, completion_serial, pending_serial, write_serial;
static uint64_t simulation_ticks, presentation;
static bool rebuilding;
static uint8_t version, content, owner;
static uint32_t dos_palette[256];
static uint64_t palette_revision;
static bool initialized, active, pending_open, pending_sealed, invalidated;

static bool ValidSurface(XwRenderSurface s) {
	return s > XW_RENDER_SURFACE_NONE && s < XW_RENDER_SURFACE_COUNT;
}

void XwRenderCapture_CancelView(void) {
	XwRenderSnapshot_Clear(&pending);
	pending_open = pending_sealed = false;
	for (unsigned i = 0; i < XW_RENDER_SURFACE_COUNT; ++i)
		if (surfaces[i].pending_view) {
			surfaces[i].pending_view = 0;
			surfaces[i].incomplete = true;
		}
}

void XwRenderCapture_ResetSurfaces(void) {
	invalidated = true;
	if (active && (owner == XW_SNAP_OWNER_FLIGHT || owner == XW_SNAP_OWNER_FLIGHT_TRANSITION || rebuilding))
		owner = rebuilding ? XW_SNAP_OWNER_FLIGHT_TRANSITION : XW_SNAP_OWNER_FLIGHT_UI;
	XwRenderCapture_CancelView();
	memset(surfaces, 0, sizeof surfaces);
	XwRenderSnapshot* out = XwRenderSnapshot_Writer();
	if (out) {
		out->world_valid = out->hud_valid = 0;
		out->classic.valid = 0;
	}
}

void XwRenderCapture_Init(void) {
	mission = world = view_serial = completion_serial = pending_serial = write_serial = 0;
	simulation_ticks = presentation = 0;
	rebuilding = false;
	XwRenderObjects_Reset();
	version = content = 0;
	owner = XW_SNAP_OWNER_NONE;
	initialized = true;
	active = false;
	XwHud_Enable(false);
	palette_revision = 0;
	memset(dos_palette, 0, sizeof dos_palette);
	XwRenderSnapshot_Clear(&pending);
	XwRenderCapture_ResetSurfaces();
}

void XwRenderCapture_Shutdown(void) {
	XwRenderCapture_ResetSurfaces();
	initialized = active = false;
	XwHud_Reset();
	XwHud_Enable(false);
	XwCockpitAssets_Reset();
}

void XwRenderCapture_Export(XwRenderSnapshot* out) {
	if (out->key.mission_generation != mission || out->key.world_generation != world ||
		owner == XW_SNAP_OWNER_FRONTEND || !active || invalidated) {
		XwRenderSnapshot_Clear(out);
		out->key.mission_generation = mission;
		out->key.world_generation = world;
	}
	out->presentation_generation = presentation;
	out->owner = owner;
	out->flight_version = active ? version : 0;
	out->mission_classic = active && content;
	XwRenderAssetId loaded = active ? XwCockpitAssets_Loaded() : 0;
	if (out->loaded_cockpit != loaded) {
		XwRenderAssets_Retain(loaded);
		XwRenderAssets_Release(out->loaded_cockpit);
		out->loaded_cockpit = loaded;
	}
	XwHud_ExportDirect(out);
	if (active && owner == XW_SNAP_OWNER_FRONTEND)
		XwRenderAssets_ExportResident(out);
	if (active && !out->world_valid && owner != XW_SNAP_OWNER_FRONTEND)
		XwRenderAssets_Capture(out);
	if (active && version != 98 && owner != XW_SNAP_OWNER_FRONTEND && palette_revision) {
		memcpy(out->appearance.palette_argb, dos_palette, sizeof dos_palette);
		out->appearance.palette_revision = palette_revision;
	}
}

static void ReplaceWorld(bool continuous) {
	if (!initialized)
		return;
	++world;
	rebuilding = continuous && active && owner != XW_SNAP_OWNER_FRONTEND && owner != XW_SNAP_OWNER_NONE;
	if (!rebuilding)
		++presentation;
	simulation_ticks = 0;
	XwRenderObjects_Reset();
	XwRenderCapture_ResetSurfaces();
	XwRenderSnapshot* out = XwRenderSnapshot_Writer();
	if (out)
		XwRenderCapture_Export(out);
}

void XwRenderCapture_WorldChanged(void) { ReplaceWorld(false); }

void XwRenderCapture_ContinueFlight(void) { ReplaceWorld(true); }

void XwRenderCapture_BeginMission(uint8_t flight_version, bool classic_content) {
	if (!initialized)
		return;
	++mission;
	version = flight_version;
	content = classic_content;
	XwHud_Reset();
	XwCockpitAssets_Reset();
	XwRenderAssets_BeginMission(flight_version, classic_content);
	XwHud_Enable(true);
	active = true;
	owner = XW_SNAP_OWNER_FLIGHT_UI;
	palette_revision = 0;
	XwRenderCapture_WorldChanged();
}

void XwRenderCapture_EndMission(void) {
	if (!active)
		return;
	active = false;
	XwHud_Reset();
	XwHud_Enable(false);
	XwCockpitAssets_Reset();
	XwRenderAssets_EndMission();
	XwRenderCapture_WorldChanged();
}

void XwRenderCapture_SetOwner(uint8_t next) {
	if (!initialized || (next == owner && next != XW_SNAP_OWNER_FLIGHT_UI))
		return;
	if (next == XW_SNAP_OWNER_FRONTEND || next == XW_SNAP_OWNER_NONE || next == XW_SNAP_OWNER_FLIGHT_UI) {
		rebuilding = false;
		++presentation;
		XwRenderCapture_ResetSurfaces();
	}
	owner = next;
	XwHud_Enable(active && next != XW_SNAP_OWNER_FRONTEND && next != XW_SNAP_OWNER_NONE);
	XwRenderSnapshot* out = XwRenderSnapshot_Writer();
	if (out)
		XwRenderCapture_Export(out);
}

void XwRenderCapture_Simulate(uint16_t ticks) {
	if (initialized && active)
		simulation_ticks += ticks;
}

void XwRenderCapture_CaptureWorld(void) {
	XwRenderSnapshot* out = XwRenderCapture_Pending();
	if (!out)
		return;
	out->simulation_ticks = simulation_ticks;
	out->flight_unlocked = XwFlightTiming_IsUnlocked();
	out->component_view_time_ticks = XwFlightTiming_CompletedMovementTicks();
	out->component_event_serial = XwFlightTiming_AnimationSerial();
	out->component_event_time_ticks = XwFlightTiming_AnimationTime();
	unsigned reference = XwFlightTiming_ReferenceTicks();
	out->component_event_interval_ticks = ((ANIM_UPDATE_INTERVAL + reference - 1) / reference) * reference;
	bool camera_valid = XwRenderCamera_Capture(&out->camera);
	XwRenderCamera_Appearance(&out->appearance);
	out->world_valid = XwRenderObjects_Capture(out) && camera_valid;
	if (!XwRenderWorld_Capture(out) || !XwRenderSky_Capture(out))
		out->world_valid = 0;
}

void XwRenderCapture_BeginView(XwRenderSurface target) {
	XwRenderCapture_CancelView();
	if (!initialized || !active || !ValidSurface(target))
		return;
	XwRenderSnapshot_Clear(&pending);
	pending.key.mission_generation = mission;
	pending.key.world_generation = world;
	pending.presentation_generation = presentation;
	pending.flight_version = version;
	pending.mission_classic = content;
	pending.capture_host_us = Aeron_NowUs();
	surfaces[target].pending_view = ++pending_serial;
	surfaces[target].incomplete = true;
	surfaces[target].classic_complete = version != 98 || !AeronDx5_IsClassicFlightRenderingSuppressed();
	pending_open = true;
}

XwRenderSnapshot* XwRenderCapture_Pending(void) { return pending_open && !pending_sealed ? &pending : NULL; }

bool XwRenderCapture_HasPendingView(void) { return pending_open; }

void XwRenderCapture_SealView(void) {
	if (!pending_open)
		return;
	if (pending.object_count > XW_SNAP_OBJECTS || pending.craft_count > XW_SNAP_CRAFTS ||
		pending.hyperspace.count > XW_SNAP_HYPERSTARS || pending.cockpit.glyph_count > XW_SNAP_HUD_GLYPHS ||
		pending.cockpit.paint_count > XW_SNAP_HUD_PAINT ||
		pending.cockpit.sprite_count > XW_SNAP_HUD_SPRITES) {
		Aeron_RequestFatalError("Renderer Error", "Flight snapshot channel counts exceed their capacities.");
		XwRenderCapture_CancelView();
		return;
	}
	XwRenderAssets_Capture(&pending);
	pending_sealed = true;
	for (unsigned i = 0; i < XW_RENDER_SURFACE_COUNT; ++i)
		if (surfaces[i].pending_view == pending_serial)
			surfaces[i].incomplete = false;
}

const XwRenderSurfaceState* XwRenderCapture_Surface(XwRenderSurface s) {
	return ValidSurface(s) ? &surfaces[s] : NULL;
}

void XwRenderCapture_ClassicSuppressed(void) {
	if (version == 98 && pending_open)
		for (unsigned i = 0; i < XW_RENDER_SURFACE_COUNT; ++i)
			if (surfaces[i].pending_view == pending_serial)
				surfaces[i].classic_complete = false;
}

void XwRenderCapture_SurfaceWrite(XwRenderSurface target) {
	if (initialized && ValidSurface(target))
		surfaces[target].write_revision = ++write_serial;
}

void XwRenderCapture_ClearSurface(XwRenderSurface target) {
	if (!ValidSurface(target))
		return;
	XwHud_ClearSurface(target);
	surfaces[target] = (XwRenderSurfaceState) { .write_revision = ++write_serial };
}

void XwRenderCapture_CopySurface(XwRenderSurface target, XwRenderSurface source, bool complete) {
	if (!ValidSurface(target))
		return;
	if (complete && ValidSurface(source)) {
		XwHud_CopySurface(target, source, true);
		surfaces[target] = surfaces[source];
	} else
		XwRenderCapture_ClearSurface(target);
}

void XwRenderCapture_ComposeHud(XwRenderSurface target, XwRenderSurface source) {
	if (!ValidSurface(target) || !ValidSurface(source))
		return;
	XwHud_CopySurface(target, source, true);
	surfaces[target].hud_source_revision = surfaces[source].write_revision;
	XwRenderCapture_SurfaceWrite(target);
}

void XwRenderCapture_CompleteHudComposite(bool succeeded) {
	if (succeeded)
		XwRenderCapture_ComposeHud(XW_RENDER_SURFACE_BACK, XW_RENDER_SURFACE_OFFSCREEN);
	else
		XwRenderCapture_CancelView();
}

void XwRenderCapture_BeginReplayOverlay(void) {
	XwRenderSnapshot* out = XwRenderSnapshot_Writer();
	if (!out || !out->world_valid || !out->camera.replay_mode || !XwRenderCapture_IsCurrentWorld(&out->key) ||
		pending_open || version != 98)
		return;
	/* The dialog's first flip reuses this world and the latest working replay controls. */
	surfaces[XW_RENDER_SURFACE_BACK] = (XwRenderSurfaceState) { .key = out->key };
	XwRenderCapture_ComposeHud(XW_RENDER_SURFACE_BACK, XW_RENDER_SURFACE_OFFSCREEN);
	/* HUD-only flips cannot certify that suppressed classic world drawing has completed. */
	out->classic.valid = 0;
}

void XwRenderCapture_Presented(XwRenderSurface surface, uint8_t classic_source, bool fresh, bool flip) {
	XwRenderSnapshot* out = XwRenderSnapshot_Writer();
	if (!out || !active || owner == XW_SNAP_OWNER_FRONTEND || !ValidSurface(surface))
		return;
	if ((version == 98) == (surface == XW_RENDER_SURFACE_DOS))
		return;
	XwRenderSurfaceState* source = &surfaces[surface];
	if (source->incomplete)
		return;
	if (source->pending_view) {
		if (!pending_sealed || source->pending_view != pending_serial)
			return;
		XwSnapClassicFrame classic = out->classic;
		XwRenderSnapshot_Copy(out, &pending);
		out->classic = classic;
		XwRenderSnapshot_Clear(&pending);
		out->key.view_serial = ++view_serial;
		source->key = out->key;
		owner = XW_SNAP_OWNER_FLIGHT;
		rebuilding = false;
		pending_open = pending_sealed = false;
		for (unsigned i = 0; i < XW_RENDER_SURFACE_COUNT; ++i)
			surfaces[i].pending_view = 0;
	} else {
		bool same_view = out->world_valid && source->key.mission_generation == mission &&
						 source->key.world_generation == world && source->key.view_serial &&
						 source->key.view_serial == out->key.view_serial;
		owner = same_view    ? XW_SNAP_OWNER_FLIGHT
				: rebuilding ? XW_SNAP_OWNER_FLIGHT_TRANSITION
							 : XW_SNAP_OWNER_FLIGHT_UI;
		if (!same_view) {
			XwRenderSnapshot_Clear(out);
			out->key.mission_generation = mission;
			out->key.world_generation = world;
		}
	}
	XwHud_Present(out, surface, surface == XW_RENDER_SURFACE_BACK && fresh && flip);
	source->key.hud_revision = out->key.hud_revision;
	if (!out->world_valid)
		XwRenderAssets_Capture(out);
	invalidated = false;
	out->presentation_generation = presentation;
	out->owner = owner;
	out->flight_version = version;
	out->mission_classic = content;
	if (fresh && source->classic_complete) {
		out->classic = (XwSnapClassicFrame) {
			.key = out->key, .completion_serial = ++completion_serial, .valid = 1, .source = classic_source
		};
	}
	if (surface == XW_RENDER_SURFACE_BACK && fresh) {
		XwRenderSurfaceState old_front = surfaces[XW_RENDER_SURFACE_FRONT];
		surfaces[XW_RENDER_SURFACE_FRONT] = *source;
		if (flip)
			*source = old_front;
	}
}

bool XwRenderCapture_IsCurrentWorld(const XwSnapViewKey* key) {
	return initialized && active && !invalidated && owner == XW_SNAP_OWNER_FLIGHT && key &&
		   key->mission_generation == mission && key->world_generation == world;
}

bool XwRenderCapture_IsCurrentPresentation(const XwRenderSnapshot* s) {
	return initialized && active && s && s->key.mission_generation == mission &&
		   s->presentation_generation == presentation &&
		   (owner == XW_SNAP_OWNER_FLIGHT || owner == XW_SNAP_OWNER_FLIGHT_TRANSITION);
}

void XwRenderCapture_DosPalette(const uint32_t colors[256]) {
	XwRenderSnapshot* out = XwRenderSnapshot_Writer();
	if (!out || !active || version == 98 || owner == XW_SNAP_OWNER_FRONTEND)
		return;
	memcpy(out->appearance.palette_argb, colors, sizeof out->appearance.palette_argb);
	if (!palette_revision || memcmp(dos_palette, colors, sizeof dos_palette)) {
		memcpy(dos_palette, colors, sizeof dos_palette);
		++palette_revision;
	}
	out->appearance.palette_revision = palette_revision;
}
