#include "xw/flight/fview.h"
#include "xw/flight/gate.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw_dos94/flight/object/collision.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/fview.h"

const Dos94MeshView* Dos94World_Bounds(uint16_t model, uint16_t component, XwBounds16* out) {
	uint16_t count;
	const Dos94Lod* lod = Dos94_DRAW_getcomponentptr(model, component, &count);
	if (!lod || !count)
		return NULL;
	*out = (XwBounds16) { lod->mesh.bounds[0] >> 1, lod->mesh.bounds[1] >> 1, lod->mesh.bounds[2] >> 1,
						  lod->mesh.bounds[3] >> 1, lod->mesh.bounds[4] >> 1, lod->mesh.bounds[5] >> 1 };
	return &lod->mesh;
}

uint16_t Dos94World_TrenchKey(int16_t y) { return ((y & 7) << 3) | ((y & 0xf8) << 8) | 0x400; }

/* DOS94 0x742830: forced fractions are signed -1.0, not midpoint hits. */
uint16_t Dos94World_Hit(const Dos94MeshView* mesh, int32_t x, int32_t y, int32_t z, bool forced) {
	int16_t end[3] = { (uint32_t)g_collisionProbeWorldX - (uint32_t)x,
					   (uint32_t)y - (uint32_t)g_collisionProbeWorldY,
					   (uint32_t)g_collisionProbeWorldZ - (uint32_t)z };
	int16_t start[3] = { (uint32_t)g_collisionSegmentStartWorldX - (uint32_t)x,
						 (uint32_t)y - (uint32_t)g_collisionSegmentStartWorldY,
						 (uint32_t)g_collisionSegmentStartWorldZ - (uint32_t)z };
	uint16_t fraction = forced ? 0x8000 : Dos94_COLLIDE_checkhitpolygons(mesh, end, start, 1);
	if (fraction)
		Dos94_collide_hitoffsets(fraction);
	return fraction;
}

/* DOS94 0x7816AC. The input/output fields are shared with the scoring continuation. */
int Dos94_gate_TestGatePlaneCollision(uint16_t type) {
	int16_t a[3] = { g_collisionScratchPoint1X, g_collisionScratchPoint1Y, g_collisionScratchPoint1Z };
	int16_t b[3] = { g_collisionScratchPoint2X, g_collisionScratchPoint2Y, g_collisionScratchPoint2Z };
	if ((int16_t)(a[1] ^ b[1]) >= 0)
		return 0;
	if (!a[1]) {
		g_gateIntersectionX = a[0];
		g_gateIntersectionZ = a[2];
	} else if (!b[1]) {
		g_gateIntersectionX = b[0];
		g_gateIntersectionZ = b[2];
	} else {
		if (a[1] < 0)
			for (int i = 0; i < 3; ++i) {
				int16_t v = a[i];
				a[i] = b[i];
				b[i] = v;
			}
		uint16_t fraction = (uint16_t)(((uint32_t)(uint16_t)a[1] << 16) / (uint16_t)(a[1] - b[1])) >> 1;
		g_gateIntersectionX = (int16_t)(a[0] + (((int16_t)(b[0] - a[0]) * (int32_t)fraction) >> 15));
		g_gateIntersectionZ = (int16_t)(a[2] + (((int16_t)(b[2] - a[2]) * (int32_t)fraction) >> 15));
		g_collisionScratchPoint1X = a[0];
		g_collisionScratchPoint1Y = a[1];
		g_collisionScratchPoint1Z = a[2];
		g_collisionScratchPoint2X = b[0];
		g_collisionScratchPoint2Y = b[1];
		g_collisionScratchPoint2Z = b[2];
	}
	int16_t x = g_gateIntersectionX, z = g_gateIntersectionZ;
	if (x < 0)
		x = (int16_t)-x;
	if (z < 0)
		z = (int16_t)-z;
	if (type == 109)
		return x < 1920 && z < 2259;
	if (x > 2400 || z > 8320)
		return 0;
	return z <= 7520 || x <= (int16_t)(3 * (8320 - g_gateIntersectionZ));
}

/* DOS94 0x780948: normalize both points together, then compare the gate up-plane signs. */
int Dos94_gate_PointsOnSameSide(uint16_t index, const int32_t first[3], const int32_t second[3]) {
	const XwMissionObjectRecord* object = &g_missionObjects[index];
	int32_t world[3] = { object->worldX * 256, object->worldY * 256, object->worldZ * 256 }, p[2][3];
	uint16_t high = 0;
	for (unsigned i = 0; i < 3; ++i) {
		p[0][i] = (int32_t)((uint32_t)first[i] - (uint32_t)world[i]);
		p[1][i] = (int32_t)((uint32_t)second[i] - (uint32_t)world[i]);
		for (unsigned j = 0; j < 2; ++j) {
			int16_t h = p[j][i] >> 14;
			high |= h < 0 ? (uint16_t)-h : h;
		}
	}
	do {
		high >>= 1;
		for (unsigned j = 0; j < 2; ++j)
			for (unsigned i = 0; i < 3; ++i)
				p[j][i] >>= 1;
	} while (high);
	fview_calcrotatemove((uint16_t)object->pitchAngle8 << 8, (uint16_t)object->yawAngle8 << 8, NULL);
	fview_calcrotateorient((uint16_t)object->rollAngle8 << 8, 0, NULL);
	int16_t axis[3] = { g_fviewUpX_Q15, g_fviewUpY_Q15, g_fviewUpZ_Q15 };
	int16_t a[3] = { p[0][0], p[0][1], p[0][2] }, b[3] = { p[1][0], p[1][1], p[1][2] };
	return (int16_t)(Dos94_math2_dot3Q15(axis, a) ^ Dos94_math2_dot3Q15(axis, b)) >= 0;
}

int32_t Dos94World_ClampDot(int32_t value) {
	int16_t high = (int16_t)((uint32_t)value >> 16);
	if (high >= 0x4000)
		high = 0x3fff;
	if (high <= -0x4000)
		high = -0x3fff;
	return (int32_t)(((uint32_t)(uint16_t)high << 16) | ((uint32_t)value & 0xffff));
}
