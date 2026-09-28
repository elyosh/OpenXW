#include "xw_dos94/render/ordering.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/xw.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/fview.h"
#include <stdbool.h>

static int32_t subtract(int32_t a, int32_t b) { return (int32_t)((uint32_t)a - (uint32_t)b); }

static uint16_t high_magnitude(int32_t value) {
	int16_t high = (int16_t)(((uint32_t)value << 2) >> 16);
	return high < 0 ? (uint16_t)-high : (uint16_t)high;
}

static void local_position(const ObjectRecord* owner, const int32_t delta[3], int16_t out[3]) {
	int16_t point[3] = { (int16_t)delta[0], (int16_t)delta[1], (int16_t)delta[2] };
	int16_t axes[3][3] = { { owner->cachedSideX, owner->cachedSideY, owner->cachedSideZ },
						   { owner->cachedForwardX, owner->cachedForwardY, owner->cachedForwardZ },
						   { owner->cachedUpX, owner->cachedUpY, owner->cachedUpZ } };
	for (unsigned i = 0; i < 3; ++i)
		out[i] = Dos94_math2_dot3Q15(axes[i], point);
	out[1] = (int16_t)-out[1];
}

/* DOS94 0x694BCA: ordinary-space plane ownership and first-vertex plane test. */
uint16_t Dos94_DRAW_polydepthsort(uint8_t faceA, uint16_t idA, uint16_t parentA, uint16_t componentA,
								  uint8_t faceB, uint16_t idB, uint16_t parentB, uint16_t componentB) {
	unsigned ca = parentA >> 8, cb = parentB >> 8;
	if (ca == 0x38 || ca == 0x78 || ca == 0x10 || ca == 0x20 || ca == 0x30 || (ca == 0x40 && componentA))
		return idB;
	if (cb == 0x38 || cb == 0x78 || cb == 0x10 || cb == 0x20 || cb == 0x30 || (cb == 0x40 && componentB))
		return idA;
	if (ca == 0x40 || cb == 0x40) {
		int32_t positionA[3] = { 0 }, positionB[3] = { 0 };
		uint16_t modelB = 110;
		if (ca != 0x40) {
			ObjectRecord* a = &g_objectTable[(uint8_t)parentA];
			positionA[0] = a->worldX;
			positionA[1] = a->worldY;
			positionA[2] = a->worldZ;
		}
		if (cb != 0x40) {
			ObjectRecord* b = &g_objectTable[(uint8_t)parentB];
			positionB[0] = b->worldX;
			positionB[1] = b->worldY;
			positionB[2] = b->worldZ;
			modelB = b->objectType == 43 ? b->sourceObjectType : b->objectType;
		}
		const int32_t camera[3] = { g_flightCamera.worldPosition.x, g_flightCamera.worldPosition.y,
									g_flightCamera.worldPosition.z };
		if (ca == 0x40)
			return modelB == 112 || !Dos94_gate_PointsOnSameSide((uint8_t)parentA, camera, positionB) ? idA
																									  : idB;
		return Dos94_gate_PointsOnSameSide((uint8_t)parentB, camera, positionA) ? idA : idB;
	}
	ObjectRecord* a = &g_objectTable[(uint8_t)parentA];
	ObjectRecord* b = &g_objectTable[(uint8_t)parentB];
	uint16_t modelA = a->objectType == 43 ? a->sourceObjectType : a->objectType;
	uint16_t modelB = b->objectType == 43 ? b->sourceObjectType : b->objectType;
	const Dos94Model *ma = Dos94Assets_Model(modelA), *mb = Dos94Assets_Model(modelB);
	if (!ma || !mb)
		return idA;
	bool chooseB = ca != 0x70 && (cb == 0x70 || mb->metadata.maxBoundsExtent >= ma->metadata.maxBoundsExtent);
	ObjectRecord *owner = chooseB ? b : a, *other = chooseB ? a : b;
	uint16_t ownerId = chooseB ? idB : idA, otherId = chooseB ? idA : idB;
	uint16_t model = chooseB ? modelB : modelA, component = chooseB ? componentB : componentA;
	uint16_t faceIndex = chooseB ? faceB : faceA;
	g_curCraft = owner->instanceData;
	int32_t delta[3] = { subtract(other->worldX, owner->worldX), subtract(other->worldY, owner->worldY),
						 subtract(other->worldZ, owner->worldZ) };
	int32_t camera[3] = { subtract(g_flightCamera.worldPosition.x, owner->worldX),
						  subtract(g_flightCamera.worldPosition.y, owner->worldY),
						  subtract(g_flightCamera.worldPosition.z, owner->worldZ) };
	uint16_t high[3] = { high_magnitude(delta[0]), high_magnitude(delta[1]), high_magnitude(delta[2]) };
	unsigned shift = 0;
	do {
		++shift;
		for (unsigned i = 0; i < 3; ++i) {
			high[i] >>= 1;
			delta[i] >>= 1;
			camera[i] >>= 1;
		}
	} while (high[0] | high[1] | high[2]);
	int16_t local[3], eye[3];
	local_position(owner, delta, local);
	local_position(owner, camera, eye);
	shift += model == 13 || model == 16 ? -1 : 1;
	uint16_t count;
	const Dos94Lod* lods = Dos94_DRAW_getcomponentptr(model, component, &count);
	const Dos94MeshView* mesh = Dos94_DRAW_getdetailptr(lods, count, g_curCraft ? g_curCraft->viewZ : 0);
	if (!mesh || !mesh->faceCount)
		return ownerId;
	if (faceIndex >= mesh->faceCount)
		faceIndex = mesh->faceCount - 1;
	Dos94FaceView face;
	int16_t vertex[3];
	if (!Dos94Models_Face(mesh, faceIndex, &face))
		return ownerId;
	const uint8_t* stream = Dos94Assets_Data(face.stream);
	if (!stream || !Dos94Models_Vertex(mesh, stream[1], vertex))
		return ownerId;
	for (unsigned i = 0; i < 3; ++i) {
		int16_t coordinate = (int16_t)(vertex[i] >> (shift & 31));
		local[i] = (int16_t)(local[i] - coordinate);
		eye[i] = (int16_t)(eye[i] - coordinate);
	}
	int16_t side1 = Dos94_math2_dot3Q15(face.normal, local), side2 = Dos94_math2_dot3Q15(face.normal, eye);
	return (int16_t)(side1 ^ side2) < 0 ? ownerId : otherId;
}

static uint16_t flat_mesh(uint16_t flatId, uint16_t meshId, bool trenchCover) {
	Dos94Raster* r = &Dos94_display->raster;
	Dos94RasterFlat* flat = &r->flat[flatId - 128];
	Dos94RasterObject* mesh = r->objectById[meshId];
	if (!mesh)
		return flatId;
	if (flat->z == (int16_t)0x8000)
		return meshId;
	if (trenchCover && (mesh->parent >> 8) == 0x10)
		return flatId;
	if (flat->z == (int16_t)0x8001)
		return meshId;
	if (mesh->parent == flat->parent) {
		return meshId < flat->component ? meshId : flatId;
	}
	int16_t coordinate[3] = { flat->x, flat->y, flat->z };
	for (unsigned axis = 0; axis < 3; ++axis) {
		int16_t low = mesh->bounds[axis], high = mesh->bounds[axis + 3];
		if (low >= coordinate[axis])
			return low < 0 ? meshId : flatId;
		if (high <= coordinate[axis])
			return high >= 0 ? meshId : flatId;
	}
	return flatId;
}

static void cache_behind(Dos94RasterObject* object, uint8_t id) {
	unsigned slot = 0;
	while (slot < 4 && object->behind[slot])
		++slot;
	object->behind[slot] = id;
}

/* DOS94 0x6A31A0. */
uint16_t Dos94_xtrans2_getinfront(uint16_t a, uint16_t b, uint8_t faceA, uint8_t faceB, bool trenchCover) {
	if (a == 128)
		return b;
	if (b == 128)
		return a;
	Dos94Raster* r = &Dos94_display->raster;
	if (a > 128 && b > 128) {
		if (trenchCover) {
			if (r->flat[b - 128].parent == 0x1000)
				return a;
			if (r->flat[a - 128].parent == 0x1000)
				return b;
		}
		return a >= b ? a : b;
	}
	if (a > 128)
		return flat_mesh(a, b, trenchCover);
	if (b > 128)
		return flat_mesh(b, a, trenchCover);
	Dos94RasterObject *first = r->objectById[a], *second = r->objectById[b];
	if (!first || !second)
		return first ? a : b;
	if (first->parent == second->parent)
		return a < b ? a : b;
	for (unsigned i = 0; i < 5; ++i)
		if (second->behind[i] == a)
			return b;
	for (unsigned i = 0; i < 5; ++i)
		if (first->behind[i] == b)
			return a;
	for (unsigned axis = 0; axis < 3; ++axis) {
		int16_t high = second->bounds[axis + 3], low = first->bounds[axis];
		if (high <= low) {
			if (low < 0) {
				cache_behind(first, b);
				return a;
			}
			if (high >= 0) {
				cache_behind(second, a);
				return b;
			}
		}
		low = second->bounds[axis];
		high = first->bounds[axis + 3];
		if (low >= high) {
			if (low < 0) {
				cache_behind(second, a);
				return b;
			}
			if (high >= 0) {
				cache_behind(first, b);
				return a;
			}
		}
	}
	return Dos94_DRAW_polydepthsort(first->modelFace[faceA], a, first->parent, first->component,
									second->modelFace[faceB], b, second->parent, second->component);
}
