#include "xw_dos94/flight/object/effects.h"
#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/fview.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw_dos94/assets/models.h"
#include "xw_dos94/render/detail.h"

uint16_t Dos94_ionExhaustedHealth, Dos94_ionExhaustedRepairTimer;

static const Dos94MeshView* first_mesh(uint16_t model, uint16_t component) {
	uint16_t count;
	const Dos94Lod* lod = Dos94Models_ComponentLods(model, component, &count);
	if (!lod || !count)
		return NULL;
	Dos94_solidindex = component;
	return &lod->mesh;
}

static void initialize_effect(ObjectRecord* effect, const ObjectRecord* source) {
	effect->worldX = (int32_t)((uint32_t)source->worldX + (uint32_t)g_rotatedX);
	effect->worldY = (int32_t)((uint32_t)source->worldY + (uint32_t)g_rotatedY);
	effect->worldZ = (int32_t)((uint32_t)source->worldZ + (uint32_t)g_rotatedZ);
	effect->instanceData = NULL;
	effect->objectType = 64 + (math2_getrandom() & 1);
	effect->genusId = XW_GENUS_EXPLOSION_EFFECT;
	effect->familyId = XW_OBJECT_FAMILY_5;
	effect->animationState = 2;
	effect->ageSeconds = effect->lifetimeTicks = effect->speed = 0;
	effect->roll = effect->pitch = effect->yaw = 0;
	effect->orientMatrixDirty = effect->moveVectorDirty = 1;
}

/* DOS94 0x7C2628. Resolve geometry before allocation or random-number consumption. */
uint16_t Dos94_starship_makestarshipcompexplo(ObjectRecord* object, uint16_t component, uint16_t scale,
											  uint16_t vertex) {
	const Dos94MeshView* mesh = first_mesh(object->objectType, component);
	if (!mesh)
		return UINT16_MAX;
	int16_t point[3];
	if (vertex == UINT16_MAX) {
		for (int i = 0; i < 3; ++i)
			point[i] = (int16_t)(mesh->bounds[i] + mesh->bounds[i + 3]) >> 1;
	} else if (!mesh->vertexCount || !Dos94Models_Vertex(mesh, vertex % mesh->vertexCount, point))
		return UINT16_MAX;
	pai_calcrotatedpoint(object, point[0], point[2], (int16_t)-point[1]);
	uint16_t slot = create_findslot(XW_GENUS_EXPLOSION_EFFECT);
	if (slot == UINT16_MAX)
		return slot;
	if (object->objectType == 13 || object->objectType == 16) {
		g_rotatedX = (int32_t)((uint32_t)g_rotatedX * 2);
		g_rotatedY = (int32_t)((uint32_t)g_rotatedY * 2);
		g_rotatedZ = (int32_t)((uint32_t)g_rotatedZ * 2);
	} else {
		g_rotatedX >>= 1;
		g_rotatedY >>= 1;
		g_rotatedZ >>= 1;
	}
	initialize_effect(&g_objectTable[slot], object);
	g_objectTable[slot].billboardScaleCode = scale >> 6;
	return slot;
}

/* DOS94 0x7C12FC: HP absorption and subsystem effects use DOS component identity. */
int Dos94_starship_damagecomponent(uint16_t victim, int16_t component, uint16_t damage) {
	if (component <= 0 || component > 16)
		return damage;
	unsigned index = component - 1;
	uint8_t hp = g_curCraft->componentHp[index];
	if (!hp || hp == 255)
		return damage;
	uint16_t units = damage >> 4;
	if (hp > units) {
		g_curCraft->componentHp[index] = hp - units;
		return 0;
	}
	damage = (units - hp) << 4;
	g_curCraft->componentHp[index] = 0;
	g_curCraft->componentState[index] = 2;
	ObjectRecord* object = &g_objectTable[victim];
	if (object->objectType == 15 &&
		(index == Dos94_corvetteguncomponent(false) || index == Dos94_corvetteguncomponent(true))) {
		unsigned first = index == Dos94_corvetteguncomponent(true) ? 0 : 2;
		for (unsigned i = first; i < first + 2; ++i) {
			g_curCraft->weaponSlots[i].firingGate = 0;
			g_curCraft->weaponSlots[i].laserCharge = 0;
		}
	} else if (object->objectType == 16 && !g_curCraft->componentHp[1] && !g_curCraft->componentHp[2]) {
		g_curCraft->shieldEnergy[0] = 0;
		g_curCraft->workingSubsystems &= ~1;
	}
	uint16_t slot = create_findslot(XW_GENUS_EXPLOSION_EFFECT);
	if (slot == UINT16_MAX)
		return damage;
	const Dos94MeshView* mesh = first_mesh(object->objectType, index);
	if (!mesh)
		return damage;
	int16_t point[3], extent = INT16_MIN;
	bool large = object->objectType == 13 || object->objectType == 16;
	for (int i = 0; i < 3; ++i) {
		point[i] = (int16_t)(mesh->bounds[i] + mesh->bounds[i + 3]);
		if (!large)
			point[i] /= 4;
		int16_t span = (int16_t)(mesh->bounds[i + 3] - mesh->bounds[i]);
		if (span > extent)
			extent = span;
	}
	pai_calcrotatedpoint(object, point[0], point[2], (int16_t)-point[1]);
	ObjectRecord* effect = &g_objectTable[slot];
	initialize_effect(effect, object);
	effect->speed = object->speed;
	effect->pitch = object->pitch;
	effect->yaw = object->yaw;
	fsfx_triggersfx(15, slot);
	if (!large)
		extent /= 4;
	effect->billboardScaleCode = (uint16_t)extent >> 8;
	return damage;
}

static ObjectRecord* prepare(uint16_t index) {
	ObjectRecord* object = &g_objectTable[index];
	g_curCraft = object->instanceData;
	if (object->orientMatrixDirty)
		fview_newcalcrotate(object->roll, object->pitch, object->yaw, 0, object);
	return object;
}

/* DOS94 0x7C2544: recurring effects, separate from final destruction. */
void Dos94_starship_createstarshipexplo__partial(uint16_t index) {
	if ((uint16_t)math2_getrandom() >= g_craftExplosionSpawnThreshold)
		return;
	ObjectRecord* object = prepare(index);
	uint16_t roll = math2_getrandom(), component;
	if (object->objectType == 16)
		component = roll < 0x4000 ? 0 : roll < 0x9000 ? 3 : 10;
	else {
		const Dos94Model* model = Dos94Assets_Model(object->objectType);
		if (!model || !model->metadata.componentCount)
			return;
		component = roll % model->metadata.componentCount;
	}
	if (g_curCraft->componentHp[component]) {
		uint16_t scale = g_modelTypeTable[object->objectType].maxBoundsExtent >> 6;
		uint16_t slot =
			Dos94_starship_makestarshipcompexplo(object, component, scale, math2_getrandom() & 0x7fff);
		if (slot != UINT16_MAX)
			fsfx_triggersfx(15, slot);
	}
}

/* DOS93 0x7C2870 / DOS94 0x7C285A: full destruction. */
void Dos94_starship_createstarshipexplo(uint16_t index) {
	ObjectRecord* object = prepare(index);
	uint16_t first = g_objectSlotRangeByGenus[XW_GENUS_EXPLOSION_EFFECT].start;
	for (unsigned i = 0; i < 5; ++i)
		g_objectTable[first + i].objectType = 0;
	switch (object->objectType) {
		case 15: {
			/* DOS93 clamps the original invalid argument to the final CRFT component.
			 * DOS94's bounded native policy rejects it before allocating an effect. */
			uint16_t component = UINT16_MAX;
			const Dos94Model* model = Dos94Assets_Model(object->objectType);
			if (Dos94Assets_Version() == XW_GAME_VERSION_93 && model && model->decoded &&
				model->decoded->componentCount)
				component = model->decoded->componentCount - 1;
			for (unsigned i = 0; i < 5; ++i)
				Dos94_starship_makestarshipcompexplo(object, component, 0xa00, UINT16_MAX);
			break;
		}
		case 14:
			Dos94_starship_makestarshipcompexplo(object, 0, 0xa00, UINT16_MAX);
			Dos94_starship_makestarshipcompexplo(object, 2, 0xc00, UINT16_MAX);
			Dos94_starship_makestarshipcompexplo(object, 3, 0x1800, UINT16_MAX);
			break;
		case 16:
			Dos94_starship_makestarshipcompexplo(object, 3, 0x3f00, UINT16_MAX);
			Dos94_starship_makestarshipcompexplo(object, 0, 0x3f00, UINT16_MAX);
			Dos94_starship_makestarshipcompexplo(object, 11, 0x3f00, UINT16_MAX);
			Dos94_starship_makestarshipcompexplo(object, 10, 0x3f00, 1);
			Dos94_starship_makestarshipcompexplo(object, 10, 0x3f00, 2);
			break;
		case 13:
			Dos94_starship_makestarshipcompexplo(object, 0, 0x3200, UINT16_MAX);
			Dos94_starship_makestarshipcompexplo(object, 1, 0x3200, UINT16_MAX);
			break;
		default:
			Dos94_starship_makestarshipcompexplo(object, 0, 0x2200, UINT16_MAX);
			break;
	}
	fsfx_triggersfx(12, index);
}
