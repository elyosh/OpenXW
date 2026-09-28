#include "xw_dos94/render/lighting.h"
#include "xw/flight/player/user.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/dos93_projection.h"
#include <string.h>

static int32_t dot(const int16_t a[3], const int16_t b[3]) {
	return (int32_t)((uint32_t)((int32_t)a[0] * b[0]) + (uint32_t)((int32_t)a[1] * b[1]) +
					 (uint32_t)((int32_t)a[2] * b[2]));
}

void Dos94Lighting_BeginMesh(Dos94Lighting* lighting) {
	memset(lighting->vertexLight, 0xFF, sizeof lighting->vertexLight);
}

/* DOS94 0x69ACB9: no saturated Q15 helper in this face-orientation test. */
unsigned Dos94_DRAWPOL_checknormal(const Dos94Transform* transform, const Dos94FaceView* face,
								   const Dos94EyePoint* eyeVertices, uint16_t vertexCount) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93)
		return Dos93_DRAWPOL_checknormal(transform, face, eyeVertices, vertexCount);
	const uint8_t* stream = Dos94Assets_Data(face->stream);
	if (!stream || face->stream.size < 4 || stream[3] >= vertexCount)
		return 2;
	const Dos94EyePoint* point = &eyeVertices[stream[3]];
	int32_t coordinate[3] = { point->x, point->y, point->z };
	uint32_t sum = 0;
	for (unsigned axis = 0; axis < 3; ++axis) {
		int32_t normal = (int32_t)((uint32_t)dot(face->normal, transform->matrix[axis]) * 2u);
		sum += (uint32_t)(((int64_t)normal * coordinate[axis]) >> 32);
	}
	return (int32_t)sum >= 0 ? 2 : 0;
}

/* DOS94 0x69AE40 and 0x6A1236: two-sided C2h lines retain negative light. */
bool Dos94_DRAWPOL_vertexlight(Dos94Lighting* l, const Dos94MeshView* mesh, uint8_t vertex,
							   bool retainNegative, bool recalculate) {
	if (mesh->layout != DOS94_MESH_CPLX || vertex >= mesh->vertexCount || vertex >= 128)
		return false;
	if (!recalculate && l->vertexLight[vertex] != -1)
		return true;
	int16_t normal[3];
	for (unsigned axis = 0; axis < 3; ++axis) {
		uint16_t value;
		if (!Dos94Models_ReadWord(mesh->payload, mesh->normals + 6u * vertex + 2u * axis, &value))
			return false;
		normal[axis] = (int16_t)value;
	}
	int32_t value = dot(l->direction, normal);
	if (value < 0 && !retainNegative)
		value = 0;
	l->vertexLight[vertex] = (int16_t)(value >> 15);
	return true;
}

/* DOS94 0x69ADA8: flat and Gouraud material interpretation share this entry. */
bool Dos94_DRAWPOL_getlightvalue(Dos94Lighting* l, const Dos94MeshView* mesh, uint16_t faceIndex,
								 uint8_t color, uint8_t* output) {
	/* DOS94 0x699B07: two-bit checkpoint state selects a material offset. */
	Dos94Raster* r = &Dos94_display->raster;
	l->gouraud = false;
	if (!r->dos93)
		color = (uint8_t)(color + r->markColorOffset[color & 63]);
	if (!l->threeD || (r->dos93 && color == 255)) {
		*output = color;
		return true;
	}
	if ((l->parent >> 8) == 0x40)
		color = (uint8_t)(color + Dos94_trainingGateColors[l->gateTint]);
	if (l->parent == l->target) {
		if (!color || color > (r->dos93 ? 27 : 39))
			return false;
		color = r->targetMapping[color - 1];
	}
	Dos94FaceView face;
	if (!Dos94Models_Face(mesh, faceIndex, &face))
		return false;
	uint8_t* stream = Dos94Assets_Data(face.stream);
	l->gouraud = face.gouraud && (g_gouraudEnableMask & 0x40) != 0;
	if (l->gouraud) {
		unsigned count = face.vertexCount, cursor = 0;
		if (count == 2) {
			/* The closure lives in the owned source payload; aliases observe the write. */
			if (face.stream.size < 7)
				return false;
			stream[6] = stream[3];
			cursor = 3;
		}
		for (unsigned i = 0; i < count; ++i, cursor += 2) {
			if (cursor + 1 >= face.stream.size ||
				!Dos94_DRAWPOL_vertexlight(l, mesh, stream[cursor + 1], face.twoSided && count == 2,
										   face.twoSided))
				return false;
		}
		if (!(color & 0x80)) {
			*output = color;
			return true;
		}
	}
	if (!r->dos93)
		color &= 0x7F;
	if (!color || color > (r->dos93 ? 27 : 39))
		return false;
	int32_t value = dot(l->direction, face.normal);
	unsigned shade = value < 0 ? 0 : (uint8_t)((uint32_t)value >> 24) >> 2;
	l->markLightValue = (uint8_t)shade;
	unsigned index = (unsigned)color * 16u - 1u - shade;
	if (index >= sizeof r->materialColors)
		return false;
	*output = r->materialColors[index];
	return true;
}

static int32_t face_axis(int32_t negative, int32_t positive, uint32_t ratio, bool vertical) {
	int32_t difference = (int32_t)((uint32_t)negative - (uint32_t)positive);
	int64_t intersection = ((int64_t)difference * (int32_t)ratio >> 16) + positive;
	int64_t projected;
	if (vertical) {
		/* The original second IMUL consumes only EAX, discarding its high extension. */
		projected = ((int64_t)(int32_t)intersection * 0xE8BA) >> 8;
	} else
		projected = intersection * 256;
	int32_t high = (int32_t)(projected >> 32);
	int32_t result = (int32_t)projected;
	if (high != 0 && high != -1)
		result = high < 0 ? -0x7FFFF000 : 0x7FFFF000;
	uint32_t center = vertical ? (uint32_t)(int32_t)(int16_t)g_flightVpCenterY + (uint32_t)g_projOffsetY
							   : (uint32_t)(int32_t)(int16_t)g_flightVpCenterX;
	return (int32_t)((uint32_t)result + center);
}

/* DOS94 0x6A11E3: face intersections use a full 32-bit divisor, unlike the
 * normalized word division and rounded Y used by the flat-polygon helper. */
Dos94ScreenPoint Dos94_TRANSFM2_facezintersect(const Dos94EyePoint* negative, const Dos94EyePoint* positive,
											   int16_t negativeLight, int16_t positiveLight, int16_t* light) {
	uint32_t denominator = (uint32_t)positive->z - (uint32_t)negative->z;
	uint32_t ratio = (uint32_t)(((uint64_t)(uint32_t)positive->z << 16) / denominator);
	int16_t delta = (int16_t)(negativeLight - positiveLight);
	/* ROR EBX,1 feeds a signed word multiply; 0x10000 becomes -32768. */
	int16_t half = (int16_t)(ratio >> 1);
	*light = (int16_t)(positiveLight + (((int32_t)delta * half) >> 15));
	return (Dos94ScreenPoint) { face_axis(negative->x, positive->x, ratio, false),
								face_axis(negative->y, positive->y, ratio, true) };
}
