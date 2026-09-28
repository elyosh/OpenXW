#include "xw_runtime/snapshot/render_world.h"
#include "xw/flight/death_star.h"
#include "xw/flight/gate.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_runtime/snapshot/render_capture.h"
#include <string.h>

static bool Placements(XwRenderPlacementList* out, const XwSurfacePlacementList* list,
					   const XwSurfaceHealthList* health) {
	if (!list || !health || list->count > XW_WORLD_PLACEMENTS || health->count > XW_WORLD_PLACEMENTS)
		return false;
	out->count = list->count;
	for (unsigned i = 0; i < list->count; ++i)
		out->entries[i] =
			(XwRenderPlacement) { list->placements[i].packedPosition, list->placements[i].objectType,
								  i < health->count ? health->health[i] : 0 };
	return true;
}

static bool Register(uint8_t version) {
	if (XwRenderAssets_Find(XW_SOURCE_SPECIAL_LAYOUT, "layout"))
		return true;
	XwRenderWorldLayout layout = { 0 };
	for (unsigned i = 0; i < 5; ++i)
		if (!Placements(&layout.surface[i], g_surfacePlacementLists[i], g_surfaceHealthLists[i]))
			return false;
	for (unsigned i = 0; i < 9; ++i)
		if (!Placements(&layout.trench[i], g_trenchPlacementLists[i], g_trenchHealthLists[i]))
			return false;
	static const uint8_t surface_colors[28] = { 1, 2, 3, 4, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5,
												5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2 };
	memcpy(layout.surface_colors, surface_colors, sizeof surface_colors);
	memcpy(layout.trench_colors, Dos94_trenchSurfaceColors, sizeof layout.trench_colors);
	memcpy(layout.detail_scale, g_deathStarDetailScreenSizeScale, sizeof layout.detail_scale);
	layout.minimum_size = g_surfaceDetailMinProjectedSize;
	layout.minimum_large_size = g_surfaceSpecialDetailMinProjectedSize;
	/* Coordinates are immutable authored data. Negating Y removes the original
	 * procedural object's quarter-turn pose; these quads are already world aligned. */
	for (unsigned face = 0; face < 4; ++face) {
		const OptVector* vertices = face ? g_deathStarTrenchVertices : g_deathStarSurfaceVertices;
		const OptTexCoord* uv = face ? g_deathStarTrenchTexCoords : g_deathStarSurfaceTexCoords;
		const OptPackedFaceRecord* record =
			face ? &g_deathStarTrenchFaceData.records[face - 1] : &g_deathStarSurfaceFaceData.records[0];
		for (unsigned v = 0; v < 4; ++v) {
			const OptVector* p = &vertices[record->vertexIndices[v]];
			layout.quads[face].positions[v][0] = p->x;
			layout.quads[face].positions[v][1] = -p->y;
			layout.quads[face].positions[v][2] = p->z;
			layout.quads[face].uv[v][0] = uv[record->texCoordIndices[v]].u;
			layout.quads[face].uv[v][1] = uv[record->texCoordIndices[v]].v;
		}
		layout.quads[face].normal[face == 1 || face == 3 ? 0 : 2] = face == 1 ? -1 : 1;
	}
	return XwRenderAssets_RegisterBytes(XW_SOURCE_SPECIAL_LAYOUT, 0, "world/layout", version, &layout,
										sizeof layout) != 0;
}

bool XwRenderWorld_Capture(XwRenderSnapshot* s) {
	for (unsigned i = 0; i < XW_SNAP_DAMAGE_CELLS; ++i) {
		s->special.damage[i].key = g_surfaceCellDamageStates[i].cellKey;
		memcpy(s->special.damage[i].state, g_surfaceCellDamageStates[i].objectHealthOrEffectState,
			   sizeof s->special.damage[i].state);
	}
	for (unsigned i = 0; i < s->object_count; ++i)
		s->objects[i].checkpoint_lit = g_provingGroundsCheckpointBlinkTicks < GATE_BLINK_ON_TICKS ? 7 : 0;
	if (s->special.surface_active && !Register(s->flight_version)) {
		XwRenderAssets_Fail("special world", "invalid immutable placement layout");
		return false;
	}
	return true;
}

void XwRenderWorld_Checkpoint(unsigned slot, unsigned component, bool lit) {
	XwRenderSnapshot* s = XwRenderCapture_Pending();
	if (!s || s->flight_version != 98 || component < 1 || component > 3)
		return;
	for (unsigned i = 0; i < s->object_count; ++i) {
		XwSnapObject* object = &s->objects[i];
		if (object->id.kind == XW_SNAP_OBJECT_MISSION && object->id.slot == slot) {
			unsigned bit = 1u << (component - 1);
			object->checkpoint_lit = (object->checkpoint_lit & ~bit) | (lit ? bit : 0);
			return;
		}
	}
}
