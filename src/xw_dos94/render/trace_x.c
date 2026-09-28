#include "xw/render/flight_view.h"
#include "xw_dos94/render/raster.h"
#include "xw_dos94/render/trace2.h"

static void clipped_x(Dos94TraceXEdge edge, uint32_t packed, uint32_t step, int row, uint8_t count, bool up,
					  bool right) {
	packed &= 0xFFFF3FFFu;
	uint32_t position;
	uint16_t light;
	if (up) {
		light = (uint16_t)((uint16_t)packed * 2u - (uint16_t)((int16_t)count * edge.lightIncY));
		packed = (packed & 0xFFFF0000u) | light;
		position = right ? packed : 0u - packed;
		position = (uint32_t)count * edge.slopeQ7 + (position >> 16);
		row = row + 1 - count;
	} else {
		light = (uint16_t)(packed * 2u);
		position = (right ? packed : 0u - packed) >> 16;
	}
	Dos94ClippedState state = { .kind = DOS_CLIP_X_Q7,
								.right = right,
								.increase = !up,
								.step = (uint16_t)(step >> 16),
								.rows = count,
								.position = position & 0xFFFFFFu };
	Dos94_TRACE2_enterclipped((uint16_t)row, edge.tag, light, state);
}

/* DOS94 0x69D5A2–0x69DAEE: operate on the packed X/light accumulator.
 * Its light guard bits deliberately prevent a borrow from changing X. */
static void trace_x(Dos94TraceXEdge edge, bool up, bool right) {
	int row = (int)edge.startY - (up ? 1 : 0);
	if (row < 0)
		return;
	uint8_t count = edge.rowCount;
	uint32_t packed = ((uint32_t)edge.xQ7 << 16) | (edge.light >> 1);
	uint16_t lightStep;
	bool reverseLight = up == right;
	if (reverseLight)
		lightStep = (uint16_t)(-(edge.lightIncY >> 1)) & 0x7FFF;
	else
		lightStep = (uint16_t)edge.lightIncY >> 1;
	uint32_t step = ((uint32_t)edge.slopeQ7 << 16) | lightStep;
	uint32_t limit = (uint32_t)(uint16_t)(g_flightVpWidth << 7) << 16;
	if (!edge.skipHalfStep) {
		uint16_t halfLight;
		if (reverseLight)
			halfLight = (uint16_t)(-(edge.lightIncY >> 2)) & 0x7FFF;
		else
			halfLight = (uint16_t)(edge.lightIncY >> 1) >> 1;
		uint32_t half = ((step >> 1) & 0xFFFF0000u) | halfLight;
		bool outside;
		if (right) {
			uint32_t before = packed;
			packed += half;
			outside = packed < before || packed >= limit;
		} else {
			packed |= 0xC000u;
			outside = packed < half;
			packed -= half;
		}
		if (outside) {
			clipped_x(edge, packed, step, row, count, up, right);
			return;
		}
	}
	for (;;) {
		packed &= 0xFFFF3FFFu;
		Dos94_TRACE2_enterevent((uint16_t)row, edge.tag, (uint16_t)(packed >> 23), (packed >> 8) & 63);
		row += up ? -1 : 1;
		if (row < 0 || row >= g_flightVpHeight)
			return;
		if (!--count)
			return;
		bool outside;
		if (right) {
			uint32_t before = packed;
			packed += step;
			outside = packed < before;
			/* Down/right masks before the boundary comparison; up/right does it later. */
			if (!up && !outside)
				packed &= 0xFFFF3FFFu;
			outside = outside || packed >= limit;
		} else {
			packed |= 0xC000u;
			outside = packed < step;
			packed -= step;
		}
		if (outside) {
			clipped_x(edge, packed, step, row, count, up, right);
			return;
		}
	}
}

void Dos94_TRACE2_xdownleft(Dos94TraceXEdge edge) { trace_x(edge, false, false); }

void Dos94_TRACE2_xdownright(Dos94TraceXEdge edge) { trace_x(edge, false, true); }

void Dos94_TRACE2_xupleft(Dos94TraceXEdge edge) { trace_x(edge, true, false); }

void Dos94_TRACE2_xupright(Dos94TraceXEdge edge) { trace_x(edge, true, true); }
