/* DOS course ordering is the recovered pure gate program, applied to snapshots. */
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/fview.h"
#include "xw_remaster/dos_draw.h"
#include "xw_remaster/special_world_internal.h"
#include <math.h>

static bool Part(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
				 const XwRenderView* view, unsigned component) {
	XwDosPart part;
	if (!XwDosShip_Component(s, set, object, view, component, 0x4000 + object->id.slot, &part))
		return false;
	if (component >= 1 && component <= 3)
		part.gate_tint = (object->type_specific >> (2 * (component - 1))) & 3;
	return XwDosDraw_Add(&part);
}

bool XwWorld_DosCourse(const XwRenderSnapshot* s, const XwRenderAssetSet* set, const XwSnapObject* object,
					   const XwRenderView* view) {
	float local[3], pose[16];
	XwRenderMath_Local(s->camera.world_pos, object->world_pos, local);
	float depth = 0;
	for (unsigned a = 0; a < 3; ++a)
		depth += local[a] * s->camera.rows[6 + a];
	unsigned detail = s->appearance.graphics_detail;
	if ((detail == 0 && depth > 0x20000) || (detail == 1 && depth > 0x30000))
		return true;
	XwRenderMath_ObjectMatrix(object, s->flight_version, view->origin_world, pose);
	int32_t p[3];
	uint16_t high[3];
	unsigned shift = 0;
	for (unsigned a = 0; a < 3; ++a) {
		p[a] = (int32_t)(((uint32_t)object->world_pos[a] - (uint32_t)s->camera.world_pos[a]) * 2u);
		int16_t v = (uint16_t)(p[a] >> 16) * 2;
		high[a] = v < 0 ? (uint16_t)-v : v;
	}
	do {
		++shift;
		for (unsigned a = 0; a < 3; ++a) {
			high[a] >>= 1;
			p[a] >>= 1;
		}
	} while (high[0] | high[1] | high[2]);
	int16_t v[3] = { p[0] >> 1, p[1] >> 1, p[2] >> 1 };
	int16_t axes[3][3];
	for (unsigned col = 0; col < 3; ++col)
		for (unsigned row = 0; row < 3; ++row)
			axes[col][row] =
				(int16_t)fmaxf(-32767, fminf(32767, pose[row * 4 + col] * (col == 1 ? -32768 : 32768)));
	int16_t side = Dos94_math2_dot3Q15(axes[0], v), forward = Dos94_math2_dot3Q15(axes[1], v);
	int16_t up = (int16_t)-Dos94_math2_dot3Q15(axes[2], v);
	if (up < (-4096 >> shift))
		return Part(s, set, object, view, 0);
	if (object->type == 109)
		return Part(s, set, object, view, up < 0 ? 0 : 1) && Part(s, set, object, view, up < 0 ? 1 : 0);
	if (object->type < 110 || object->type > 117)
		return true;
	uint16_t order[11];
	Dos94_gate_Order(object->type, side, forward, up, shift, order);
	for (unsigned i = 0; i < 11; ++i) {
		unsigned component = order[i];
		if (component == UINT16_MAX)
			continue;
		if (object->type == 111 && (component == 1 || component == 3))
			component = 4 - component;
		if (component > 3 && !(object->state & (1u << (component - 4))))
			continue;
		if (!Part(s, set, object, view, component))
			return false;
	}
	return true;
}
