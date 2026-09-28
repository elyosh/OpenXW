#include "xw/flight/feinput.h"
#include "xw/flight/fview.h"
#include "xw/flight/gate.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/fview.h"
#include "xw_dos94/render/special_draw.h"

static void pair_order(uint16_t* out, uint16_t checkpoint, uint16_t left, uint16_t right, bool leftFirst,
					   bool gunsFirst) {
	uint16_t first = leftFirst ? left : right, second = leftFirst ? right : left;
	out[0] = gunsFirst ? first : checkpoint;
	out[1] = gunsFirst ? second : first;
	out[2] = gunsFirst ? checkpoint : second;
}

static void side_order(uint16_t* out, uint16_t checkpoint, uint16_t left, uint16_t right, int16_t side,
					   int16_t center, unsigned shift) {
	if (side < ((int16_t)(center - 592) >> shift)) {
		out[0] = left;
		out[1] = checkpoint;
		out[2] = right;
	} else if (side > ((int16_t)(center + 592) >> shift)) {
		out[0] = right;
		out[1] = checkpoint;
		out[2] = left;
	} else {
		out[0] = checkpoint;
		out[1] = right;
		out[2] = left;
	}
}

/* DOS94 0x7803D2: six gun identities and three checkpoint triples, ordered by viewpoint. */
void Dos94_gate_Order(uint16_t type, int16_t side, int16_t forward, int16_t up, unsigned shift,
					  uint16_t* order) {
	static const uint8_t guns[8][6] = { { 4, 5, 6, 7, 8, 9 }, { 9, 8, 7, 6, 5, 4 }, { 6, 7, 4, 5, 8, 9 },
										{ 7, 6, 9, 8, 5, 4 }, { 6, 7, 4, 5, 9, 8 }, { 8, 9, 4, 5, 6, 7 },
										{ 6, 7, 8, 9, 4, 5 }, { 7, 6, 9, 8, 4, 5 } };
	const uint8_t* g = guns[type - 110];
	bool forwardSort = type >= 116;
	int16_t center = type <= 111 ? 200 : type == 114 || type == 115 ? 240 : -112;
	int16_t radius = type <= 111 ? 200 : type == 114 || type == 115 ? 416 : 1120;
	order[0] = up < 0 ? 0 : UINT16_MAX;
	order[10] = up < 0 ? UINT16_MAX : 0;
	if (forwardSort)
		pair_order(order + 1, 1, g[0], g[1], side < 0, forward < (6112 >> shift));
	else
		side_order(order + 1, 1, g[0], g[1], side, 0, shift);
	int16_t offset = 0;
	switch (type) {
		case 111:
			offset = -1216; /* fall through */
		case 110:
			side_order(order + 4, 2, g[2], g[3], side, offset, shift);
			break;
		case 113:
		case 117:
			offset = -1072; /* fall through */
		case 112:
		case 116:
			pair_order(order + 4, 2, g[2], g[3], side<(offset >> shift), up>(416 >> shift));
			break;
		case 115:
			offset = -1216; /* fall through */
		case 114:
			pair_order(order + 4, 2, g[2], g[3], side < (offset >> shift), forward < (-208 >> shift));
			break;
	}
	offset = type & 1 ? 1152 : 0;
	if (forwardSort)
		pair_order(order + 7, 3, g[4], g[5], side < (offset >> shift), forward < (-7040 >> shift));
	else
		side_order(order + 7, 3, g[4], g[5], side, offset, shift);
	unsigned other = forward < ((center - radius) >> shift)   ? 7
					 : forward < ((center + radius) >> shift) ? 4
															  : 0;
	if (other)
		for (unsigned i = 0; i < 3; ++i) {
			uint16_t v = order[1 + i];
			order[1 + i] = order[other + i];
			order[other + i] = v;
		}
}

/* DOS94 0x78012E: render the authored component order, never an OPT root-mesh substitute. */
void Dos94_gate_DrawCourseObject(uint16_t index) {
	const XwMissionObjectRecord* object = &g_missionObjects[index];
	uint16_t type = object->objectType;
	Dos94DrawState* d = &Dos94_display->draw;
	Dos94EyePoint eye = Dos94_display->bitmaps.eye;
	Dos94_display->raster.parentObject = index + 0x4000;
	if ((g_flightGraphicsDetailPreset == 0 && eye.z > 0x20000) ||
		(g_flightGraphicsDetailPreset == 1 && eye.z > 0x30000))
		return;
	if (eye.z > 65536) {
		Dos94World_DrawModel(type, 0, eye);
		return;
	}
	Dos94EyePoint world = Dos94World_Scale(Dos94_display->bitmaps.world, 2);
	int32_t p[3] = { world.x, world.y, world.z };
	uint16_t high[3];
	unsigned shift = 0;
	for (unsigned i = 0; i < 3; ++i) {
		int16_t v = (uint16_t)(p[i] >> 16) * 2;
		high[i] = v < 0 ? (uint16_t)-v : v;
	}
	do {
		++shift;
		for (unsigned i = 0; i < 3; ++i) {
			high[i] >>= 1;
			p[i] >>= 1;
		}
	} while (high[0] | high[1] | high[2]);
	int16_t v[3] = { p[0] >> 1, p[1] >> 1, p[2] >> 1 };
	int16_t sideAxis[3] = { g_fviewSideX_Q15, g_fviewSideY_Q15, g_fviewSideZ_Q15 };
	int16_t forwardAxis[3] = { g_fviewForwardX_Q15, g_fviewForwardY_Q15, g_fviewForwardZ_Q15 };
	int16_t upAxis[3] = { g_fviewUpX_Q15, g_fviewUpY_Q15, g_fviewUpZ_Q15 };
	int16_t side = Dos94_math2_dot3Q15(sideAxis, v), forward = Dos94_math2_dot3Q15(forwardAxis, v),
			up = (int16_t)-Dos94_math2_dot3Q15(upAxis, v);
	if (up < (-4096 >> shift)) {
		Dos94World_DrawModel(type, 0, eye);
		return;
	}
	if (type == 109) {
		d->componentOrder[0] = up < 0 ? 0 : 1;
		d->componentOrder[1] = up < 0 ? 1 : 0;
		for (unsigned i = 0; i < 2; ++i)
			Dos94World_DrawModel(type, d->componentOrder[i], eye);
		return;
	}
	if (type < 110 || type > 117)
		return;
	Dos94_gate_Order(type, side, forward, up, shift, d->componentOrder);
	for (unsigned i = 0; i < 11; ++i) {
		uint16_t component = d->componentOrder[i];
		if (component == UINT16_MAX)
			continue;
		if (type == 111 && (component == 1 || component == 3))
			component = 4 - component;
		g_provingGroundsRenderCheckpointState = 0;
		if (component > 3) {
			if (!(object->stateByte & (1u << (component - 4))))
				continue;
		} else if (component)
			g_provingGroundsRenderCheckpointState = (object->typeSpecificByte >> (2 * (component - 1))) & 3;
		d->gateTint = g_provingGroundsRenderCheckpointState;
		Dos94World_DrawModel(type, component, eye);
	}
}
