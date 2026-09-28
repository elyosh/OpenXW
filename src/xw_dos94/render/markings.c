#include "xw_dos94/render/display.h"
#include "xw_dos94/render/drawpol.h"
#include "xw_dos94/render/trace2.h"

/* DOS94 0x69B41E–0x69BCFB: separately rounded arithmetic shifts. */
static int32_t weighted_delta(int32_t delta, uint8_t operand) {
	unsigned weight = operand / 2;
	uint32_t value = 0;
	if (weight == 7)
		return (int32_t)((uint32_t)(delta >> 1) - (uint32_t)(delta >> 4));
	if (weight == 14)
		return (int32_t)((uint32_t)delta - (uint32_t)(delta >> 3));
	if (weight == 15)
		return (int32_t)((uint32_t)delta - (uint32_t)(delta >> 4));
	if (weight & 1)
		value += (uint32_t)(delta >> 4);
	if (weight & 2)
		value += (uint32_t)(delta >> 3);
	if (weight & 4)
		value += (uint32_t)(delta >> 2);
	if (weight & 8)
		value += (uint32_t)(delta >> 1);
	return (int32_t)value;
}

static int32_t interpolate(int32_t base, int32_t previous, int32_t next, uint8_t a, uint8_t b) {
	return (int32_t)((uint32_t)base +
					 (uint32_t)weighted_delta((int32_t)((uint32_t)previous - (uint32_t)base), a) +
					 (uint32_t)weighted_delta((int32_t)((uint32_t)next - (uint32_t)base), b));
}

static bool marking_vertices(const Dos94FaceView* face, const uint8_t* source, unsigned count, bool eyeSpace,
							 Dos94EyePoint* eye, Dos94ScreenPoint* screen, Dos94ScreenBounds* bounds) {
	const uint8_t* ring = Dos94Assets_Data(face->stream);
	Dos94MeshProjection* p = &Dos94_display->projection;
	for (unsigned i = 0; i < count; ++i) {
		unsigned offset = source[3 * i] + 1;
		if (offset < 2 || offset + 2 >= face->stream.size)
			return false;
		uint8_t previous = ring[offset - 2], base = ring[offset], next = ring[offset + 2];
		if (previous >= p->mesh.vertexCount || base >= p->mesh.vertexCount || next >= p->mesh.vertexCount)
			return false;
		uint8_t a = source[3 * i + 1], b = source[3 * i + 2];
		if ((a | b) & 1 || a > 30 || b > 30)
			return false;
		if (eyeSpace) {
			Dos94EyePoint first = p->eyeVertices[base], before = p->eyeVertices[previous],
						  after = p->eyeVertices[next];
			eye[i] = (Dos94EyePoint) { interpolate(first.x, before.x, after.x, a, b),
									   interpolate(first.y, before.y, after.y, a, b),
									   interpolate(first.z, before.z, after.z, a, b) };
		} else {
			if (!p->vertexScreen[base] || !p->vertexScreen[previous] || !p->vertexScreen[next])
				return false;
			Dos94ScreenPoint first = p->vertexScreen[base]->screen,
							 before = p->vertexScreen[previous]->screen,
							 after = p->vertexScreen[next]->screen;
			screen[i] = (Dos94ScreenPoint) { interpolate(first.x, before.x, after.x, a, b),
											 interpolate(first.y, before.y, after.y, a, b) };
			Dos94_TRANSFM2_screenbounds(bounds, screen[i], i, false);
		}
	}
	return true;
}

static unsigned project_marking_line(Dos94EyePoint eye[2], Dos94ScreenPoint screen[2]) {
	if (eye[0].z < 0 && eye[1].z < 0)
		return 0;
	for (unsigned i = 0; i < 2; ++i)
		screen[i] = eye[i].z < 0
						? Dos94_TRANSFM2_calczintersect(&eye[i], &eye[1 - i])
						: (Dos94ScreenPoint) { Dos94_TRANSFM2_getscreenx(eye[i].x, (uint32_t)eye[i].z),
											   Dos94_TRANSFM2_getscreeny(eye[i].y, (uint32_t)eye[i].z) };
	return 2;
}

/* DOS94 0x69AF2C: marking polygons enter the same visibility stream as faces. */
void Dos94_DRAWPOL_drawmarkings(const Dos94MeshView* mesh, const Dos94FaceView* face, uint8_t markId,
								uint16_t source, uint8_t faceId) {
	const uint8_t* bytes = Dos94Assets_Data(mesh->payload);
	if (!bytes || source >= mesh->payload.size)
		return;
	unsigned total = bytes[source];
	Dos94Raster* r = &Dos94_display->raster;
	if (!total || total > (r->dos93 ? 7u : 16u) || source + 1 + total > mesh->payload.size ||
		(!r->dos93 && r->nextObject + 32u * total >= r->objectLimit))
		return;
	Dos94DrawState* draw = &Dos94_display->draw;
	uint16_t savedFlat = r->flatObjectNumber, savedLayer = r->layer;
	r->layer = markId;
	unsigned cursor = source + 1 + total, processed = 0;
	for (; processed < total; ++processed) {
		r->flatObjectNumber = 127 - processed;
		if (cursor >= mesh->payload.size)
			break;
		if (bytes[cursor] == 255) {
			uint32_t depth;
			if (!Dos94Models_ReadDword(mesh->payload, cursor + 1, &depth))
				break;
			cursor += 5;
			const uint8_t* ring = Dos94Assets_Data(face->stream);
			if (!ring || ring[1] >= Dos94_display->projection.mesh.vertexCount)
				break;
			if ((int32_t)depth < Dos94_display->projection.eyeVertices[ring[1]].z)
				break;
		}
		if (cursor >= mesh->payload.size)
			break;
		unsigned count = bytes[cursor++], width = 0;
		if (count > 16) {
			width = count - 16;
			count = 2;
		}
		if (!count || cursor + 3 * count > mesh->payload.size)
			break;
		Dos94EyePoint eye[16];
		Dos94ScreenPoint screen[32];
		Dos94ScreenBounds bounds;
		Dos94_TRANSFM2_resetbounds(&bounds);
		bool eyeSpace = draw->closeToObject || count == 2;
		if (!marking_vertices(face, bytes + cursor, count, eyeSpace, eye, screen, &bounds))
			break;
		cursor += 3 * count;
		unsigned projected = count;
		if (eyeSpace)
			projected = count == 2 ? project_marking_line(eye, screen)
								   : Dos94_TRANSFM2_getscreencoords(eye, count, screen, &bounds);
		if (projected == 2) {
			int32_t depth = (int32_t)((uint32_t)eye[0].z + (uint32_t)eye[1].z) >> 1;
			uint16_t denominator = depth < 0 ? 0 : (uint16_t)((uint32_t)depth >> 8);
			uint16_t thickness = (uint16_t)((denominator ? width / denominator : width) + 1);
			Dos94_DRAWLN2_tracelineedges(&screen[0], &screen[1], thickness, r->lineLight1, r->lineLight2,
										 (uint16_t)(((255 - processed) << 8) | markId));
		} else if (projected > 2)
			Dos94_TRACE2_drawscreencoords(screen, projected, &bounds, 0);
	}
	if (processed) {
		Dos94RasterMark* mark = &r->marks[markId];
		mark->object = (uint8_t)r->objectNumber;
		mark->face = faceId;
		mark->count = 0;
		for (unsigned i = 0; i < total; ++i) {
			uint8_t color = bytes[source + 1 + i], material = color & (r->dos93 ? 31 : 63);
			mark->gouraud[i] = false;
			if (draw->target == r->parentObject && material && material <= (r->dos93 ? 27 : 39))
				material = r->targetMapping[material - 1];
			else if (draw->lighting.gouraud) {
				mark->materials[i] = (uint8_t)(color + r->markColorOffset[material]);
				mark->gouraud[i] = mark->materials[i] < 64;
				continue;
			}
			unsigned index = material * 16u - 1u - draw->lighting.markLightValue;
			if (index < sizeof r->materialColors)
				mark->materials[i] = (uint8_t)(r->materialColors[index] - (color >> (r->dos93 ? 5 : 6)));
		}
		r->nextObject += r->dos93 ? 18 : 34;
	}
	r->flatObjectNumber = savedFlat;
	r->layer = savedLayer;
}
