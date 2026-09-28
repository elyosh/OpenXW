#include "xw_dos94/render/draw.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/dos93_order.h"
#include "xw_dos94/render/fview.h"
#include "xw_dos94/render/view.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/snapshot/render_sky.h"
#include <stdio.h>

static int32_t add32(int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); }

static uint16_t variant(const Dos94Component* component, unsigned state) {
	return state < component->stateCount ? component->stateVariants[state] : 0xFFFF;
}

void Dos94_DRAW_drawcomponent(uint16_t model, uint16_t component, Dos94EyePoint eye) {
	uint16_t count;
	const Dos94Lod* lods = Dos94_DRAW_getcomponentptr(model, component, &count);
	const Dos94MeshView* mesh = Dos94_DRAW_getdetailptr(lods, count, eye.z);
	if (mesh)
		Dos94_drawpol_drawpolyobject(mesh, eye);
}

static int16_t bitmap_angle(void) {
	const int16_t (*matrix)[3] = Dos94_display->draw.object.matrix;
	int16_t a = matrix[2][0], b = matrix[2][1];
	int absA = a < 0 ? -a : a, absB = b < 0 ? -b : b;
	int16_t denominator = absB > absA ? matrix[0][0] : matrix[0][1];
	int16_t numerator = absB > absA ? matrix[1][0] : matrix[1][1];
	return denominator < 0 ? (int16_t)Dos94_trig2_arctan(numerator, (int16_t)-denominator)
						   : (int16_t)-Dos94_trig2_arctan(numerator, denominator);
}

static bool queue_child(uint16_t reference, uint16_t bitmap, const Dos94Component* component, uint16_t factor,
						int16_t angle) {
	Dos94BitmapQueue* queue = &Dos94_display->bitmaps;
	if ((int32_t)component->eyeOffsetX + component->eyeOffsetY + component->eyeOffsetZ) {
		queue->eye.x = add32(queue->eye.x, component->eyeOffsetX);
		queue->eye.y = add32(queue->eye.y, component->eyeOffsetY);
		queue->eye.z = add32(queue->eye.z, component->eyeOffsetZ);
	}
	int32_t x = Dos94_TRANSFM2_getscreenx(queue->eye.x, (uint32_t)queue->eye.z);
	if ((x >> 16) > 0 || (x >> 16) < -1)
		return false;
	int32_t y = Dos94_TRANSFM2_getscreeny(queue->eye.y, (uint32_t)queue->eye.z);
	if ((y >> 16) > 0 || (y >> 16) < -1)
		return false;
	Dos94BitmapDrawEntry entry = {
		reference, bitmap, factor, (int16_t)x, (int16_t)(2 * g_flightVpCenterY - y), queue->eye.z, angle
	};
	Dos94_ANIM_add_bitmap_draw(queue, &entry);
	return true;
}

/* DOS94 0x694694: original component IDs and descriptor-linked bitmap children. */
void Dos94_DRAW_drawcraft(uint16_t index, uint16_t model, const uint16_t* order, unsigned count) {
	Dos94Model* resource = Dos94Assets_Model(model);
	CraftData* craft = g_objectTable[index].instanceData;
	if (!resource || !craft)
		return;
	Dos94Raster* r = &Dos94_display->raster;
	r->parentObject = index + (model == 13 || model == 16 ? 0x7000 : 0);
	bool angleCached = false;
	int16_t angle = 0;
	for (unsigned i = 0; i < count; ++i) {
		unsigned id = order[i];
		if (id >= resource->metadata.descriptorCount || id >= 16)
			continue;
		const Dos94Component* descriptor = &resource->metadata.components[id];
		uint16_t selected = variant(descriptor, craft->componentState[id]);
		int16_t rotation = (int16_t)((uint16_t)craft->meshRotation[id] << 8);
		Dos94Transform* transform = &Dos94_display->draw.object;
		transform->origin = Dos94_display->bitmaps.eye;
		bool rotated = rotation && (model == 1 || model == 15 || model == 118);
		if (rotated) {
			if (model == 1)
				Dos94_FVIEW_sfoilrotation(transform, rotation);
			else if (model == 15)
				Dos94_FVIEW_corvettegunrotation(transform, rotation);
			else
				Dos94_FVIEW_bwingrotation(transform, rotation, id);
		}
		int16_t render = g_drawMarkingsFlag;
		if (model == 9 && id == (Dos94Assets_Version() == XW_GAME_VERSION_93 ? 3u : 6u))
			g_drawMarkingsFlag = g_objectTable[index].iff == 0;
		if (selected < 0x8000) {
			uint16_t detail = g_shipDetailPolyCount;
			if ((model == 13 && !id) || (model == 14 && id == 2))
				g_shipDetailPolyCount = 35;
			Dos94_DRAW_drawcomponent(model, selected, transform->origin);
			g_shipDetailPolyCount = detail;
		}
		g_drawMarkingsFlag = render;
		if (rotated)
			Dos94_FVIEW_restorerotation(transform);
		if (selected >= 0x8000)
			continue;
		unsigned child = descriptor->nextChildIndex;
		for (unsigned remaining = descriptor->childCount; remaining; --remaining) {
			if (child >= resource->metadata.descriptorCount || child >= 16)
				break;
			const Dos94Component* entry = &resource->metadata.components[child];
			uint16_t image = variant(entry, craft->componentState[child]);
			if (image >= 0x8000 && image < 0xFF00) {
				if (!angleCached) {
					angle = bitmap_angle();
					angleCached = true;
				}
				angle = (int16_t)(angle + g_objectTable[index].roll);
				if (!queue_child(r->parentObject, image, entry, entry->bitmapScale, angle))
					continue;
			}
			child = entry->nextChildIndex;
			if (!child)
				break;
		}
	}
}

/* DOS94 0x694560. Child offsets are relative to the current component-tree node. */
static unsigned Dos94_draw_gettreeorder(uint16_t model, const int16_t relative[3], int shift,
										uint16_t out[16]) {
	uint16_t stack[256];
	unsigned size = 1, count = 0, visits = 0;
	stack[0] = 0;
	while (size && count < 16 && visits++ < 512) {
		uint16_t index = stack[--size];
		Dos94ComponentNode node;
		if (!Dos94Models_ComponentNode(model, index, &node))
			break;
		if (!node.firstChildOffset) {
			out[count++] = node.secondChildOffsetOrComponent;
			continue;
		}
		int16_t delta[3];
		for (unsigned i = 0; i < 3; ++i)
			delta[i] = shift < 0 ? (int16_t)((relative[i] >> (-shift)) - node.point[i])
								 : (int16_t)(relative[i] - (node.point[i] >> shift));
		bool forward = Dos94_math2_dot3Q15(node.normal, delta) >= 0;
		uint16_t a = (uint16_t)(16u * index + node.firstChildOffset) / 16;
		uint16_t b = (uint16_t)(16u * index + node.secondChildOffsetOrComponent) / 16;
		if (size + 2 > 256)
			break;
		stack[size++] = forward ? b : a;
		stack[size++] = forward ? a : b;
	}
	return count;
}

void Dos94_DRAW_drawcomplexobject(uint16_t index) {
	if (Dos94Assets_Version() == XW_GAME_VERSION_93) {
		Dos93_DRAW_drawcomplexobject(index);
		return;
	}
	ObjectRecord* object = &g_objectTable[index];
	Dos94_DRAWPOL_setmarkingcolors(object->markings);
	int32_t delta[3] = { (int32_t)((uint32_t)g_flightCamera.worldPosition.x - (uint32_t)object->worldX),
						 (int32_t)((uint32_t)g_flightCamera.worldPosition.y - (uint32_t)object->worldY),
						 (int32_t)((uint32_t)g_flightCamera.worldPosition.z - (uint32_t)object->worldZ) };
	uint16_t high[3];
	for (unsigned i = 0; i < 3; ++i) {
		delta[i] = (int32_t)((uint32_t)delta[i] * 2);
		int16_t h = (int16_t)(delta[i] >> 16);
		high[i] = (uint16_t)((h < 0 ? -h : h) * 2);
	}
	int shift = -1;
	do {
		++shift;
		for (unsigned i = 0; i < 3; ++i) {
			high[i] >>= 1;
			delta[i] >>= 1;
		}
	} while (high[0] | high[1] | high[2]);
	int16_t relative[3], point[3] = { (int16_t)delta[0], (int16_t)delta[1], (int16_t)delta[2] };
	for (unsigned axis = 0; axis < 3; ++axis) {
		int16_t basis[3] = { Dos94_display->draw.craftBasis[0][axis], Dos94_display->draw.craftBasis[1][axis],
							 Dos94_display->draw.craftBasis[2][axis] };
		relative[axis] = Dos94_math2_dot3Q15(basis, point);
	}
	relative[1] = (int16_t)-relative[1];
	if (object->objectType == 13 || object->objectType == 16)
		shift -= 2;
	uint16_t* order = Dos94_display->draw.componentOrder;
	Dos94_draw_gettreeorder(object->objectType, relative, shift, order);
	Dos94Model* model = Dos94Assets_Model(object->objectType);
	if (model)
		Dos94_DRAW_drawcraft(index, object->objectType, order, model->metadata.componentCount);
	Dos94_DRAWPOL_setmarkingcolors(0);
}

void Dos94_ANIM_drawverysimpleobject(uint16_t index) {
	ObjectRecord* object = &g_objectTable[index];
	uint16_t model = object->objectType;
	Dos94Model* resource = Dos94Assets_Model(model);
	if (!resource)
		return;
	Dos94_display->raster.parentObject = index;
	bool cached = false;
	int16_t angle = 0;
	unsigned count = resource->metadata.descriptorCount;
	const Dos94Component* components = resource->metadata.components;
	for (unsigned i = 0; i < count; ++i) {
		unsigned state = i ? object->secondaryAnimationState : object->animationState;
		const Dos94Component* component = &components[i];
		uint16_t image = variant(component, state);
		if (image >= 0xFF00)
			continue;
		if (image < 0x8000) {
			if (model == 43)
				model = object->sourceObjectType;
			Dos94_DRAW_drawcomponent(model, image, Dos94_display->bitmaps.eye);
			continue;
		}
		if (Dos94_display->bitmaps.eye.z < 0)
			break;
		if (!cached) {
			angle = bitmap_angle();
			cached = true;
		}
		angle = (int16_t)(angle + object->roll);
		unsigned scale = object->billboardScaleCode;
		if (scale) {
			scale = (uint16_t)(scale << 6);
			if (scale >= 256)
				scale = (uint16_t)(scale + component->bitmapScale);
		} else
			scale = component->bitmapScale;
		queue_child(index, image, component, (uint16_t)scale, angle);
	}
}

void Dos94_DRAW_drawlaser(uint16_t index) {
	uint16_t count;
	Dos94_display->raster.parentObject = index;
	const Dos94Lod* lods = Dos94Models_ProjectileLods(g_objectTable[index].objectType, &count);
	const Dos94MeshView* mesh = Dos94_DRAW_getdetailptr(lods, count, Dos94_display->bitmaps.eye.z);
	if (mesh)
		Dos94_drawpol_drawpolyobject(mesh, Dos94_display->bitmaps.eye);
}

void Dos94_DRAW_drawhyperstar(uint16_t index) {
	Dos94EyePoint position = Dos94_display->bitmaps.world;
	XwRenderSky_DosHyperstar(index, (const int32_t[3]) { position.x, position.y, position.z });
	Dos94MeshView mesh;
	Dos94Raster* r = &Dos94_display->raster;
	uint16_t saved = r->flatObjectNumber;
	r->parentObject = 0x3800 + index;
	if (Dos94Models_Hyperstar(&mesh)) {
		uint8_t* bytes = Dos94Assets_Data(mesh.payload);
		bytes[mesh.streams + 4] = (uint8_t)((index & 3) - 4);
		Dos94_drawpol_drawpolyobject(&mesh, Dos94_display->bitmaps.eye);
	}
	r->flatObjectNumber = saved;
}

static bool alternate_bitmap(Dos94Model* model, unsigned frame, Dos94ByteView* out) {
	Dos94ByteView payload = model->alternateBitmap;
	uint16_t count, offset;
	if (!Dos94Models_ReadWord(payload, 0, &count) || frame >= count ||
		!Dos94Models_ReadWord(payload, 2 + 2 * frame, &offset))
		return false;
	return Dos94Assets_Subview(payload, offset, payload.size - offset, out);
}

static void bitmap_error(uint16_t model, unsigned frame, const char* reason) {
	const Dos94Model* resource = Dos94Assets_Model(model);
	char error[160];
	snprintf(error, sizeof error, "X-Wing %d bitmap model %u (%.8s), frame %u: %s",
			 XwGameVersion_Year(Dos94Assets_Version()), model, resource ? resource->resourceName : "", frame,
			 reason);
	XwPort_Fail(error);
}

void Dos94_ANIM_sort_and_draw_bitmaps(void) {
	Dos94BitmapDrawEntry entry;
	while (Dos94BitmapQueue_Next(&Dos94_display->bitmaps, &entry)) {
		uint16_t id = (entry.packedBitmapId & 0x7FFF) >> 8, frame = (uint8_t)entry.packedBitmapId;
		Dos94Model* model = Dos94Assets_Model(id);
		if (!model)
			continue;
		Dos94EyePoint position;
		if ((entry.objectRef >> 8) == 0x38) {
			XwMissionObjectRecord* object = &g_missionObjects[(uint8_t)entry.objectRef];
			position = (Dos94EyePoint) { object->worldX * 256, object->worldY * 256, object->worldZ * 256 };
		} else {
			ObjectRecord* object = &g_objectTable[(uint8_t)entry.objectRef];
			position = (Dos94EyePoint) { object->worldX, object->worldY, object->worldZ };
		}
		Dos94_display->bitmaps.world =
			(Dos94EyePoint) { add32(position.x, (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.x)),
							  add32(position.y, (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.y)),
							  add32(position.z, (int32_t)(0u - (uint32_t)g_flightCamera.worldPosition.z)) };
		Dos94_display->raster.parentObject = entry.objectRef;
		uint16_t scale =
			Dos94_ROTSCALE_calcscale(entry.eyeZ, model->metadata.maxBoundsExtent, entry.scaleFactor);
		bool alternate =
			(scale < 128 || !Dos94Assets_Data(model->payload)) && Dos94Assets_Data(model->alternateBitmap);
		Dos94ByteView image;
		bool found =
			alternate ? alternate_bitmap(model, frame, &image) : Dos94Models_Bitmap(id, frame, &image);
		if (!found)
			continue;
		if (alternate)
			scale = (uint16_t)(scale * 2u);
		Dos94_display->rotation.reverse = 1;
		Dos94_ROTSCALE_preparefastdraw(entry.angle, 2);
		Dos94_ROTSCALE_preparecolor(model->metadata.bitmapPalette);
		if (!Dos94_ROTSCALE_rotatescaleimage(entry.screenX, entry.screenY, scale, image))
			bitmap_error(id, frame, "invalid bitmap data");
	}
}

void Dos94_DRAW_drawbackdropimage(uint16_t model, int16_t x, int16_t y, int16_t angle) {
	Dos94Model* resource = Dos94Assets_Model(model);
	Dos94ByteView image;
	if (!resource || !Dos94Models_Bitmap(model, 0, &image))
		return;
	Dos94_display->rotation.reverse = 1;
	Dos94_display->bitmaps.world.z = 0x100000;
	Dos94_ROTSCALE_preparefastdraw(angle, 2);
	Dos94_ROTSCALE_preparecolor(resource->metadata.bitmapPalette);
	if (!Dos94_ROTSCALE_rotatescaleimage(x, y, 256, image))
		bitmap_error(model, 0, "invalid backdrop image");
}

/* DOS94 0x7C0000: static descriptors share geometry but retain cumulative eye offsets. */
void Dos94_static_drawstaticobject(uint16_t index) {
	XwMissionObjectRecord* object = &g_missionObjects[index];
	Dos94Model* resource = Dos94Assets_Model(object->objectType);
	if (!resource)
		return;
	uint16_t reference = 0x3800 + index;
	Dos94_display->raster.parentObject = reference;
	bool cached = false;
	int16_t angle = 0;
	for (unsigned i = 0; i < resource->metadata.descriptorCount; ++i) {
		const Dos94Component* component = &resource->metadata.components[i];
		uint16_t image = variant(component, object->stateByte);
		if (image >= 0xFF00)
			continue;
		if (image < 0x8000) {
			Dos94_DRAW_drawcomponent(object->objectType, image, Dos94_display->bitmaps.eye);
			continue;
		}
		if (Dos94_display->bitmaps.eye.z < 0)
			break;
		if (!cached) {
			angle = bitmap_angle();
			cached = true;
		}
		queue_child(reference, image, component, component->bitmapScale, angle);
	}
}
