#include "xw/flight/fview.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/special_draw.h"
#include <string.h>

Dos94EyePoint Dos94World_Eye(Dos94EyePoint relative) {
	int16_t (*m)[3] = Dos94_display->draw.camera.matrix;
	return (Dos94EyePoint) { Dos94_transfm2_geteye(m[0], &relative), Dos94_transfm2_geteye(m[1], &relative),
							 Dos94_transfm2_geteye(m[2], &relative) };
}

Dos94EyePoint Dos94World_Step(const Dos94SurfaceDraw* state, unsigned axis, unsigned count) {
	if (!count)
		return (Dos94EyePoint) { 0 };
	return (Dos94EyePoint) { state->steps[axis][0][count - 1], state->steps[axis][1][count - 1],
							 state->steps[axis][2][count - 1] };
}

void Dos94World_Position(Dos94EyePoint world) {
	Dos94EyePoint camera = { g_flightCamera.worldPosition.x, g_flightCamera.worldPosition.y,
							 g_flightCamera.worldPosition.z };
	Dos94EyePoint p = Dos94World_Sub(world, camera);
	Dos94_display->bitmaps.world = p;
	g_camRelWorldX = p.x;
	g_camRelWorldY = p.y;
	g_camRelWorldZ = p.z;
}

bool Dos94World_Visible(Dos94EyePoint eye, uint16_t extent) {
	int32_t depth = (int32_t)((uint32_t)eye.z + extent);
	int32_t x = eye.x < 0 ? (int32_t)(0u - (uint32_t)eye.x) : eye.x;
	int32_t y = eye.y < 0 ? (int32_t)(0u - (uint32_t)eye.y) : eye.y;
	return depth >= 0 && x <= depth && y <= depth;
}

void Dos94World_DrawModel(uint16_t model, uint16_t component, Dos94EyePoint eye) {
	uint16_t count = 0;
	const Dos94Lod* lod = Dos94_DRAW_getcomponentptr(model, component, &count);
	const Dos94MeshView* mesh = Dos94_DRAW_getdetailptr(lod, count, eye.z);
	if (mesh)
		Dos94_drawpol_drawpolyobject(mesh, eye);
}

const XwSurfaceCellDamageState* Dos94World_Damage(uint16_t start, uint16_t key) {
	unsigned slot = start;
	do {
		const XwSurfaceCellDamageState* state = &g_surfaceCellDamageStates[slot];
		if (!state->cellKey)
			return NULL;
		if (state->cellKey == key)
			return state;
		slot = (slot + 1) & 63;
	} while (slot != start);
	return NULL;
}

static void coarse_cell(const Dos94SurfaceDraw* s, Dos94EyePoint center, unsigned level, unsigned parity) {
	static const uint8_t colors[28] = { 1, 2, 3, 4, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5,
										5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2 };
	int32_t span = 0x4000 << (level - 1);
	int32_t limit = (int32_t)((uint32_t)g_flightCamera.worldPosition.z * 16u + 3u * (uint32_t)span + 0x40000);
	if (center.z > limit)
		return;
	Dos94EyePoint p[4], inset = { 0 }, first = s->stepX, second = s->stepY;
	p[0] = Dos94World_Sub(Dos94World_Sub(center, s->halfX), s->halfY);
	if (parity == 0) {
		inset = Dos94World_Half(s->halfX, 2);
		first = s->stepY;
		second = s->stepX;
	} else if (parity == 1)
		inset = Dos94World_Half(s->halfY, 2);
	else if (parity == 2)
		inset = Dos94World_Half(s->halfY, 1);
	p[0] = Dos94World_Add(p[0], inset);
	p[1] = Dos94World_Add(p[0], first);
	if (parity < 2)
		inset = Dos94World_Scale(inset, 2);
	else if (parity == 3)
		inset = Dos94World_Half(s->halfY, 1);
	p[2] = Dos94World_Sub(Dos94World_Add(p[1], second), inset);
	p[3] = Dos94World_Sub(p[2], first);
	Dos94_DRAWPOL_drawsurfacepoly(p, colors[4 * level + parity]);
}

/* DOS94 0x74069A: coarse cells and first-LOD/detail placement identities share the health cache. */
static void surface_cell(const Dos94SurfaceDraw* s, Dos94EyePoint center, unsigned level, unsigned parity) {
	uint32_t margin = 3u * (0x4000u << (level - 1));
	int32_t expanded = (int32_t)((uint32_t)center.z + margin);
	if (expanded < 0 || (center.x < 0 ? (int32_t)(0u - (uint32_t)center.x) : center.x) > expanded ||
		(center.y < 0 ? (int32_t)(0u - (uint32_t)center.y) : center.y) > expanded)
		return;
	++Dos94_display->raster.parentObject;
	if (level > 1) {
		coarse_cell(s, center, level, parity);
		return;
	}
	int16_t x = g_deathStarSurfaceCellX, y = g_deathStarSurfaceCellY;
	uint16_t start = (x & 7) | ((y & 7) << 3), key = start | ((x & 0xf8) << 3) | ((y & 0xf8) << 8);
	unsigned layout = (x & 1) | ((y & 1) << 1);
	if (x == 0 || x == -1)
		layout = 4;
	bool raised = layout != 1 && layout != 4;
	if (raised) {
		Dos94World_Position((Dos94EyePoint) { (int32_t)(((uint32_t)(uint16_t)x << 16) | 0x8000),
											  (int32_t)(((uint32_t)(uint16_t)y << 16) | 0x8000), 0 });
		Dos94_display->raster.parentObject += 0x5800;
		Dos94World_DrawModel(89, 0, center);
		Dos94_display->raster.parentObject -= 0x5800;
		center = Dos94World_Add(center, Dos94World_Step(s, 2, 4));
	}
	center = Dos94World_Sub(Dos94World_Sub(center, s->halfX), s->halfY);
	if (g_flightCamera.worldPosition.z > 65536)
		return;
	const XwSurfaceCellDamageState* damage = Dos94World_Damage(start, key);
	++Dos94_display->raster.parentObject;
	const XwSurfacePlacementList* list = g_surfacePlacementLists[layout];
	unsigned count = list->count < g_surfaceObjectDetailLimit ? list->count : g_surfaceObjectDetailLimit;
	if (count < 8)
		count = 8;
	for (unsigned i = 0; i < count; ++i) {
		if (damage && !damage->objectHealthOrEffectState[i])
			continue;
		XwSurfaceObjectPlacement placement = list->placements[i];
		unsigned px = placement.packedPosition & 15, py = placement.packedPosition >> 4;
		Dos94World_Position((Dos94EyePoint) { (int32_t)(((uint32_t)(uint16_t)x << 16) | (px << 12)),
											  (int32_t)(((uint32_t)(uint16_t)y << 16) | (py << 12)),
											  raised ? 4096 : 0 });
		Dos94EyePoint eye = Dos94World_Add(
			center,
			Dos94World_Scale(Dos94World_Add(Dos94World_Step(s, 0, px), Dos94World_Step(s, 1, py)), 4));
		const Dos94Model* model = Dos94Assets_Model(placement.objectType);
		if (eye.z > 65536 || !model || !Dos94World_Visible(eye, model->metadata.maxBoundsExtent))
			continue;
		Dos94World_DrawModel(placement.objectType, 0, eye);
		++Dos94_display->raster.parentObject;
	}
}

/* DOS94 0x74111C: center, then clockwise around the eight neighbouring cells. */
static void surface_block(const Dos94SurfaceDraw* s, Dos94EyePoint origin, unsigned level, unsigned parity) {
	static const int8_t offsets[9][2] = { { 0, 0 },  { 1, 0 },   { 1, 1 },  { 0, 1 }, { -1, 1 },
										  { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
	int16_t x = g_deathStarSurfaceCellX, y = g_deathStarSurfaceCellY;
	Dos94EyePoint center = Dos94World_Add(origin, Dos94World_Add(s->halfX, s->halfY));
	for (unsigned i = 0; i < 9; ++i) {
		int dx = offsets[i][0], dy = offsets[i][1];
		g_deathStarSurfaceCellX = (int16_t)(x + dx);
		g_deathStarSurfaceCellY = (int16_t)(y + dy);
		Dos94EyePoint eye = Dos94World_Add(center, Dos94World_Add(Dos94World_Scale(s->stepX, (uint32_t)dx),
																  Dos94World_Scale(s->stepY, (uint32_t)dy)));
		surface_cell(s, eye, level, parity ^ (dx & 1) ^ ((dy & 1) << 1));
	}
}

static void horizon(void) {
	int16_t (*m)[3] = Dos94_display->draw.camera.matrix;
	if (m[2][2] <= -0x6000 || m[2][2] >= 0x6000)
		return;
	int32_t fx = m[2][0] * 8, fy = m[2][1] * 8;
	Dos94EyePoint world = { fy - m[2][0] * 4, -fx - m[2][1] * 4,
							(int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z) },
				  points[4];
	points[0] = Dos94World_Eye(world);
	fx *= 2;
	fy *= 2;
	world.x -= fy;
	world.y += fx;
	points[1] = Dos94World_Eye(world);
	fx *= 16;
	fy *= 16;
	world.x += fx - fy;
	world.y += fx + fy;
	points[2] = Dos94World_Eye(world);
	fx *= 2;
	fy *= 2;
	world.x += fy;
	world.y -= fx;
	points[3] = Dos94World_Eye(world);
	Dos94_DRAWPOL_drawsurfacepoly(points, 1);
}

/* DOS94 0x740000: native traversal state, never the original renderer arena. */
void Dos94_DeathStar_DrawSurfaceAndTrench(void) {
	Dos94SurfaceDraw s = { 0 };
	Dos94DrawState* d = &Dos94_display->draw;
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned axis = 0; axis < 3; ++axis) {
			d->object.matrix[row][axis] =
				axis == 1 ? (int16_t)-d->camera.matrix[row][axis] : d->camera.matrix[row][axis];
			d->craftBasis[row][axis] = row == axis ? 32767 : 0;
			for (unsigned n = 0; n < 16; ++n)
				s.steps[axis][row][n] = (int16_t)(((int32_t)d->camera.matrix[row][axis] * (n + 1)) >> 5);
		}
	d->worldLight[0] = (int16_t)g_modelLightDirectionX;
	d->worldLight[1] = (int16_t)g_modelLightDirectionY;
	d->worldLight[2] = (int16_t)g_modelLightDirectionZ;
	memcpy(d->objectLight, d->worldLight, sizeof d->objectLight);
	g_objectLightDirectionX = d->objectLight[0];
	g_objectLightDirectionY = d->objectLight[1];
	g_objectLightDirectionZ = d->objectLight[2];
	g_objViewMat_R0_X = d->object.matrix[0][0];
	g_objViewMat_R0_Y = d->object.matrix[1][0];
	g_objViewMat_R0_Z = d->object.matrix[2][0];
	g_objViewMat_R1_X = d->object.matrix[0][1];
	g_objViewMat_R1_Y = d->object.matrix[1][1];
	g_objViewMat_R1_Z = d->object.matrix[2][1];
	g_objViewMat_R2_X = d->object.matrix[0][2];
	g_objViewMat_R2_Y = d->object.matrix[1][2];
	g_objViewMat_R2_Z = d->object.matrix[2][2];
	g_fviewSideX_Q15 = g_fviewForwardY_Q15 = g_fviewUpZ_Q15 = 32767;
	g_fviewSideY_Q15 = g_fviewSideZ_Q15 = g_fviewForwardX_Q15 = g_fviewForwardZ_Q15 = g_fviewUpX_Q15 =
		g_fviewUpY_Q15 = 0;
	Dos94_display->raster.parentObject = 0x2000;
	if (g_flightCamera.worldPosition.z >= 0) {
		horizon();
		s.stepX = (Dos94EyePoint) { d->camera.matrix[0][0] * 64, d->camera.matrix[1][0] * 64,
									d->camera.matrix[2][0] * 64 };
		s.stepY = (Dos94EyePoint) { d->camera.matrix[0][1] * 64, d->camera.matrix[1][1] * 64,
									d->camera.matrix[2][1] * 64 };
		s.halfX = Dos94World_Half(s.stepX, 1);
		s.halfY = Dos94World_Half(s.stepY, 1);
		int16_t mask = -32;
		for (unsigned level = 6; level; --level) {
			int16_t x = (g_flightCamera.worldPosition.x >> 16) & mask,
					y = (g_flightCamera.worldPosition.y >> 16) & mask;
			unsigned parity = ((x >> (level - 1)) & 1) | (((y >> (level - 1)) & 1) << 1);
			mask >>= 1;
			if (level == 1) {
				g_deathStarSurfaceCellX = x;
				g_deathStarSurfaceCellY = y;
			}
			int32_t z = g_flightCamera.worldPosition.z;
			bool draw = level == 1 || (6 - level <= g_deathStarDetailLevel && !(level == 6 && z < 32768) &&
									   !(level == 5 && z < 24576) && !(level == 4 && z < 20480) &&
									   !(level == 3 && z < 16384));
			if (draw) {
				Dos94EyePoint world = { (int32_t)((uint32_t)(uint16_t)x << 16),
										(int32_t)((uint32_t)(uint16_t)y << 16), 0 };
				Dos94EyePoint camera = { g_flightCamera.worldPosition.x, g_flightCamera.worldPosition.y, z };
				surface_block(&s, Dos94World_Eye(Dos94World_Sub(world, camera)), level, parity);
			}
			s.stepX = Dos94World_Half(s.stepX, 1);
			s.stepY = Dos94World_Half(s.stepY, 1);
			s.halfX = Dos94World_Half(s.halfX, 1);
			s.halfY = Dos94World_Half(s.halfY, 1);
		}
	}
	Dos94_DeathStar_DrawTrench(&s);
}
