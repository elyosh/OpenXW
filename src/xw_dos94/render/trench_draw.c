#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/special_draw.h"

static bool any_front(const Dos94EyePoint p[4]) {
	return p[0].z >= 0 || p[1].z >= 0 || p[2].z >= 0 || p[3].z >= 0;
}

/* DOS94 0x743252: left wall, floor, right wall, including coarse occluding covers. */
static void trench_section(const Dos94SurfaceDraw* s, Dos94EyePoint origin, unsigned level, unsigned parity) {
	Dos94EyePoint across = Dos94World_Step(s, 0, 6), down = Dos94World_Step(s, 2, 6);
	Dos94EyePoint along = Dos94World_Scale(s->stepY, 2), p[4];
	origin = Dos94World_Sub(origin, Dos94World_Step(s, 0, 3));
	if (level == 5)
		origin = Dos94World_Sub(origin, s->stepY);
	p[0] = origin;
	p[1] = Dos94World_Add(origin, along);
	p[2] = Dos94World_Sub(p[1], down);
	p[3] = Dos94World_Sub(p[0], down);
	if (any_front(p)) {
		if (g_flightCamera.worldPosition.x > -3072)
			Dos94_DRAWPOL_drawsurfacepoly(p, Dos94_trenchSurfaceColors[3 * level] + parity);
		else if (level == 5) {
			Dos94EyePoint cover[4] = { p[0], p[1], Dos94World_Sub(p[2], down), Dos94World_Sub(p[3], down) };
			uint16_t saved = Dos94_display->raster.flatObjectNumber;
			Dos94_display->raster.flatObjectNumber = 0;
			Dos94_DRAWPOL_drawsurfacepoly(cover, 0);
			Dos94_display->raster.flatObjectNumber = saved;
		}
	}
	origin = p[3];
	p[0] = origin;
	p[1] = Dos94World_Add(origin, along);
	p[2] = Dos94World_Add(p[1], across);
	p[3] = Dos94World_Add(p[0], across);
	if (any_front(p))
		Dos94_DRAWPOL_drawsurfacepoly(p, Dos94_trenchSurfaceColors[3 * level + 1] + parity);
	origin = p[3];
	p[0] = origin;
	p[1] = Dos94World_Add(origin, along);
	p[2] = Dos94World_Add(p[1], down);
	p[3] = Dos94World_Add(p[0], down);
	if (any_front(p)) {
		if (g_flightCamera.worldPosition.x < 3072)
			Dos94_DRAWPOL_drawsurfacepoly(p, Dos94_trenchSurfaceColors[3 * level + 2] + parity);
		else if (level == 5) {
			Dos94EyePoint cover[4] = { Dos94World_Sub(p[0], down), Dos94World_Sub(p[1], down), p[2], p[3] };
			uint16_t saved = Dos94_display->raster.flatObjectNumber;
			Dos94_display->raster.flatObjectNumber = 0;
			Dos94_DRAWPOL_drawsurfacepoly(cover, 0);
			Dos94_display->raster.flatObjectNumber = saved;
		}
	}
}

/* DOS94 0x743870. World ordering coordinates and projected placement coordinates differ. */
static void trench_details(const Dos94SurfaceDraw* s, Dos94EyePoint origin, int16_t cellY) {
	if (g_flightCamera.worldPosition.z > 32768)
		return;
	origin = Dos94World_Sub(origin, Dos94World_Step(s, 2, 6));
	const XwSurfaceCellDamageState* damage = Dos94World_Damage(cellY & 63, Dos94World_TrenchKey(cellY));
	unsigned layout = cellY ? cellY & 7 : 8;
	const XwSurfacePlacementList* list = g_trenchPlacementLists[layout];
	unsigned count = list->count < g_surfaceObjectDetailLimit ? list->count : g_surfaceObjectDetailLimit;
	if (count < 8)
		count = 8;
	bool specialEntryPending = false;
	if (layout == 8) {
		count = 13;
		specialEntryPending = true;
	}
	++Dos94_display->raster.parentObject;
	for (unsigned i = 0; i < count; ++i) {
		if (damage && i < 14 && !damage->objectHealthOrEffectState[i])
			continue;
		XwSurfaceObjectPlacement p = list->placements[i];
		unsigned side = p.packedPosition & 3, along = (p.packedPosition >> 2) & 15;
		uint16_t height = (p.packedPosition >> 5) & 14;
		int32_t x = side == 0 ? 3072 : side == 1 ? -3072 : 0;
		Dos94World_Position((Dos94EyePoint) { x, (int32_t)(((uint32_t)(uint16_t)cellY << 16) | (along << 12)),
											  (int16_t)((height << 10) - 6144) });
		Dos94EyePoint eye = Dos94World_Add(origin, Dos94World_Scale(Dos94World_Step(s, 1, along), 4));
		eye = Dos94World_Add(eye, Dos94World_Half(Dos94World_Step(s, 2, height), 1));
		if (side == 0)
			eye = Dos94World_Add(eye, Dos94World_Step(s, 0, 3));
		else if (side == 1)
			eye = Dos94World_Sub(eye, Dos94World_Step(s, 0, 3));
		const Dos94Model* model = Dos94Assets_Model(p.objectType);
		if (eye.z > (specialEntryPending ? 98304 : 49152) || !model ||
			!Dos94World_Visible(eye, model->metadata.maxBoundsExtent))
			continue;
		unsigned component = side < 2 ? (1 - side + (height ? 2 : 0)) : 0;
		uint16_t detail = g_shipDetailPolyCount;
		if (specialEntryPending)
			g_shipDetailPolyCount = 50;
		Dos94World_DrawModel(p.objectType, component, eye);
		++Dos94_display->raster.parentObject;
		if (specialEntryPending) {
			g_shipDetailPolyCount = detail;
			specialEntryPending = false;
		}
	}
}

/* DOS94 0x742FBA: three rows per level; the coarse cover is always submitted first. */
void Dos94_DeathStar_DrawTrench(Dos94SurfaceDraw* s) {
	Dos94DrawState* d = &Dos94_display->draw;
	Dos94_display->raster.parentObject = 0x1000;
	s->stepY = (Dos94EyePoint) { d->camera.matrix[0][1] * 256, d->camera.matrix[1][1] * 256,
								 d->camera.matrix[2][1] * 256 };
	Dos94EyePoint relative = { (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.x), 0,
							   (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z) };
	trench_section(s, Dos94World_Eye(relative), 5, 0);
	s->stepY = Dos94World_Half(s->stepY, 5);
	int32_t x = g_flightCamera.worldPosition.x, z = g_flightCamera.worldPosition.z;
	if (x < -32768 || x > 32768 || z > 131072)
		return;
	int16_t mask = -8;
	for (unsigned level = 4; level; --level) {
		int16_t step = d->camera.matrix[2][1] < 0 ? mask : (int16_t)-mask;
		int16_t y = (g_flightCamera.worldPosition.y >> 16) & mask;
		unsigned parity =
			(((uint16_t)(g_flightCamera.worldPosition.y >> 16) & (uint16_t)~(mask * 2)) << 1) >> level;
		mask >>= 1;
		for (unsigned row = 0; row < 3; ++row) {
			relative.y = (int32_t)(((uint32_t)(uint16_t)y << 16) - (uint32_t)g_flightCamera.worldPosition.y);
			Dos94EyePoint eye = Dos94World_Eye(relative);
			if (level == 1)
				trench_details(s, eye, y);
			else if (4 - level < g_deathStarDetailLevel)
				trench_section(s, eye, level, parity);
			y = (int16_t)(y + step);
			parity ^= 1;
		}
		s->stepY = Dos94World_Half(s->stepY, 1);
	}
}
