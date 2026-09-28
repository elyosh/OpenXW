#include "xw_runtime/snapshot/render_sky.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/render/rtsvga2.h"
#include "xw_dos94/assets/models.h"
#include "xw_runtime/snapshot/render_assets.h"
#include "xw_runtime/snapshot/render_capture.h"
#include <string.h>

static void Assign(XwRenderSnapshot* s, XwRenderAssetId id) {
	XwRenderAssets_Retain(id);
	XwRenderAssets_Release(s->sky.stars);
	s->sky.stars = id;
}

bool XwRenderSky_Capture(XwRenderSnapshot* s) {
	s->sky.density = g_starDensity;
	unsigned counts[6] = { g_backdropPositiveYCount, g_backdropNegativeYCount, g_backdropPositiveXCount,
						   g_backdropNegativeXCount, g_backdropPositiveZCount, g_backdropNegativeZCount };
	unsigned total = 0;
	for (unsigned i = 0; i < 6; ++i) {
		if (counts[i] > XW_SNAP_BACKDROPS - total) {
			XwRenderAssets_Fail("sky", "backdrop count exceeds snapshot capacity");
			return false;
		}
		total += counts[i];
		s->sky.face_counts[i] = counts[i];
	}
	memcpy(s->sky.backdrop_type, g_backdropModelTypes, total);
	memcpy(s->sky.backdrop_direction, g_backdropPackedDirections, total);
	s->hyperspace.elapsed_ticks = g_playerHyperspaceElapsedTicks;
	s->hyperspace.windows_streak_length = g_hyperspaceStreakLength;
	s->hyperspace.count = 0;
	if (s->flight_version == 98)
		Assign(s, XwRenderAssets_Find(XW_SOURCE_STARS, "windows-stars"));
	else {
		XwRenderDosStars stars;
		for (unsigned i = 0; i < 256; ++i) {
			stars.jitter[i] = g_flightRandomBytePairs[i].value0To63;
			stars.brightness[i] = g_flightRandomBytePairs[i].value0To7;
		}
		Assign(s, XwRenderAssets_RegisterBytes(XW_SOURCE_STARS, 0, "sky/dos-stars", s->flight_version, &stars,
											   sizeof stars));
		s->sky.oscillator = g_legacyOscillatorValue;
		Dos94MeshView mesh;
		uint16_t first, second;
		if (!Dos94Models_Hyperstar(&mesh) || !Dos94Models_ReadWord(mesh.payload, mesh.vertices + 2, &first) ||
			!Dos94Models_ReadWord(mesh.payload, mesh.vertices + 8, &second))
			return false;
		s->hyperspace.dos_endpoints[0] = (int16_t)first;
		s->hyperspace.dos_endpoints[1] = (int16_t)second;
	}
	return true;
}

void XwRenderSky_Stars(const uint8_t* indices, const void* colors) {
	XwRenderStars data;
	memcpy(data.indices, indices, sizeof data.indices);
	bool direct = g_flightBytesPerPixel == 2;
	bool rgb555 = direct && FlightDisplay_IsPixelFormat555();
	for (unsigned i = 0; i < XW_SKY_STAR_COUNT; ++i) {
		unsigned r, g, b;
		if (direct) {
			unsigned c = ((const uint16_t*)colors)[i];
			r = (c >> (rgb555 ? 10 : 11)) & 31;
			g = (c >> 5) & (rgb555 ? 31 : 63);
			b = c & 31;
			r = (r << 3) | (r >> 2);
			g = rgb555 ? (g << 3) | (g >> 2) : (g << 2) | (g >> 4);
			b = (b << 3) | (b >> 2);
		} else {
			const RgbTriplet* c = &g_swPalette[((const uint8_t*)colors)[i]];
			r = c->r * 4;
			g = c->g * 4;
			b = c->b * 4;
		}
		data.colors[i] = 0xff000000u | r << 16 | g << 8 | b;
	}
	XwRenderAssetId id =
		XwRenderAssets_RegisterBytes(XW_SOURCE_STARS, 0, "sky/windows-stars", 98, &data, sizeof data);
	XwRenderSnapshot* s = XwRenderCapture_Pending();
	if (s)
		Assign(s, id);
}

void XwRenderSky_Hyperstar(unsigned slot, const int32_t position[3], uint16_t roll) {
	XwRenderSnapshot* s = XwRenderCapture_Pending();
	if (!s || s->flight_version != 98)
		return;
	if (slot >= 64 || s->hyperspace.count == XW_SNAP_HYPERSTARS) {
		XwRenderAssets_Fail("hyperspace", "instance count exceeds snapshot capacity");
		s->world_valid = 0;
		return;
	}
	unsigned copy = 0;
	for (unsigned i = 0; i < s->hyperspace.count; ++i)
		if (s->hyperspace.stars[i].source_slot == slot)
			++copy;
	XwSnapHyperstar* star = &s->hyperspace.stars[s->hyperspace.count++];
	*star = (XwSnapHyperstar) { .source_slot = slot, .copy_index = copy, .roll = roll };
	memcpy(star->world_pos, position, sizeof star->world_pos);
}

void XwRenderSky_DosHyperstar(unsigned slot, const int32_t relative[3]) {
	XwRenderSnapshot* s = XwRenderCapture_Pending();
	if (!s || s->flight_version == 98)
		return;
	if (slot >= 64 || s->hyperspace.count == XW_SNAP_HYPERSTARS) {
		XwRenderAssets_Fail("DOS hyperspace", "instance count exceeds snapshot capacity");
		s->world_valid = 0;
		return;
	}
	unsigned copy = 0;
	for (unsigned i = 0; i < s->hyperspace.count; ++i)
		if (s->hyperspace.stars[i].source_slot == slot)
			++copy;
	XwSnapHyperstar* star = &s->hyperspace.stars[s->hyperspace.count++];
	*star = (XwSnapHyperstar) { .source_slot = slot,
								.copy_index = copy,
								.color_index = (uint8_t)((slot & 3) - 4) };
	for (unsigned a = 0; a < 3; ++a)
		star->world_pos[a] = (int32_t)((uint32_t)s->camera.world_pos[a] + (uint32_t)relative[a]);
}
