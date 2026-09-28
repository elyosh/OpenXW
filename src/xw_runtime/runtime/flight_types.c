#include "xw_runtime/runtime/flight_types.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw_dos94/assets/tables.h"
#include "xw_dos94/flight/object/effects.h"
#include "xw_dos94/flight/special_world.h"
#include "xw_runtime/storage/dos94_assets.h"
#include <string.h>

static XwModelTypeRecord windowsModels[MODEL_TYPE_RECORD_COUNT];
static XwCraftTypeDef windowsCrafts[XW_CRAFT_TYPE_COUNT];
static XwCraftModelBounds windowsBounds[XW_CRAFT_TYPE_COUNT];
static uint8_t windowsHudIds[MODEL_TYPE_RECORD_COUNT];
static uint16_t dosToWindows[DOS94_MODEL_COUNT];
static uint16_t windowsToDos[MODEL_TYPE_RECORD_COUNT];
static bool initialized;
static XwGameVersion selectedVersion = XW_GAME_VERSION_98;

static void map_type(uint16_t dos, uint16_t windows) {
	dosToWindows[dos] = windows;
	windowsToDos[windows] = dos;
}

void XwFlightTypes_Init(void) {
	if (initialized)
		return;
	memcpy(windowsModels, g_modelTypeTable, sizeof windowsModels);
	memcpy(windowsCrafts, g_craftTypeDefs, sizeof windowsCrafts);
	memcpy(windowsBounds, g_craftModelBounds, sizeof windowsBounds);
	memcpy(windowsHudIds, g_objectTypeHudShipIds, sizeof windowsHudIds);
	memset(dosToWindows, 0xff, sizeof dosToWindows);
	memset(windowsToDos, 0xff, sizeof windowsToDos);
	for (uint16_t i = 0; i < DOS94_MISSION_TYPE_COUNT; ++i)
		map_type(Dos94_missionModelTypes[i], g_craftTypeToObjectType[i]);
	for (uint16_t i = 0; i < 4; ++i)
		map_type(57 + i, 123 + i);
	map_type(61, 131);
	map_type(62, 132);
	for (uint16_t i = 74; i <= 117; ++i)
		map_type(i, i + 93);
	map_type(118, XW_OBJ_B_WING);
	map_type(119, XW_OBJ_INTERDICTOR);
	map_type(43, XW_OBJ_DETACHED_COMPONENT);
	for (uint16_t i = 0; i < 8; ++i)
		map_type(18 + i, XW_OBJ_LASER_143 + i);
	map_type(64, XW_OBJ_EXPLOSION_133);
	map_type(65, XW_OBJ_EXPLOSION_134);
	map_type(66, XW_OBJ_EXPLOSION_135);
	map_type(67, XW_OBJ_EXPLOSION_136);
	map_type(68, XW_OBJ_ASTEROID_IMPACT);
	map_type(69, XW_OBJ_EXPLOSION_138);
	map_type(70, 139);
	map_type(71, 140);
	for (uint16_t i = 0; i < 4; ++i)
		map_type(44 + i, 110 + i);
	initialized = true;
}

bool XwFlightTypes_Dos(void) { return XwGameVersion_IsDos(selectedVersion); }

void XwFlightTypes_RefreshModel(uint16_t objectType) {
	if (!XwFlightTypes_Dos() || objectType >= DOS94_MODEL_COUNT)
		return;
	const Dos94ModelMetadata* source = &Dos94Assets_Model(objectType)->metadata;
	XwModelTypeRecord* target = &g_modelTypeTable[objectType];
	*target = (XwModelTypeRecord) { 0 };
	target->familyId = source->familyId;
	target->genusId = source->genusId;
	target->maxBoundsExtent = source->maxBoundsExtent;
	target->halfMaxBoundsExtent = source->maxBoundsExtent >> 1;
	target->objectFlags = source->radarVisible ? MODEL_TYPE_TARGETABLE : 0;
	target->craftDefinitionIndex = XwFlightTypes_Definition(objectType);
	/* Handles, flags and descriptors remain DOS-owned. In particular, bit 0 is not OPT. */
}

void XwFlightTypes_Select(XwGameVersion version) {
	XwFlightTypes_Init();
	Dos94Assets_Select(version);
	Dos94_ionExhaustedHealth = 0;
	Dos94_ionExhaustedRepairTimer = 0;
	memset(g_objectTypeMeshCache, 0, sizeof g_objectTypeMeshCache);
	memset(g_modelBoundsCached, 0, sizeof g_modelBoundsCached);
	memset(g_modelBoundsMin, 0, sizeof g_modelBoundsMin);
	memset(g_modelBoundsMax, 0, sizeof g_modelBoundsMax);
	selectedVersion = version;
	Dos94World_Select(XwGameVersion_IsDos(version));
	memset(g_objectTypeHudShipIds, 0, sizeof windowsHudIds);
	if (version == XW_GAME_VERSION_98) {
		memcpy(g_modelTypeTable, windowsModels, sizeof windowsModels);
		memcpy(g_craftTypeDefs, windowsCrafts, sizeof windowsCrafts);
		memcpy(g_craftModelBounds, windowsBounds, sizeof windowsBounds);
		memcpy(g_objectTypeHudShipIds, windowsHudIds, sizeof windowsHudIds);
		return;
	}
	memset(g_modelTypeTable, 0, sizeof g_modelTypeTable);
	memset(g_craftTypeDefs, 0, sizeof g_craftTypeDefs);
	memset(g_craftModelBounds, 0, sizeof g_craftModelBounds);
	memcpy(g_craftTypeDefs, Dos94_craftDefinitions, sizeof Dos94_craftDefinitions);
	for (uint16_t i = 0; i < DOS94_MODEL_COUNT; ++i)
		XwFlightTypes_RefreshModel(i);
	for (uint16_t i = 1; i <= 17; ++i)
		g_objectTypeHudShipIds[i] = i;
	g_objectTypeHudShipIds[118] = 18;
	g_objectTypeHudShipIds[119] = 19;
}

uint16_t XwFlightTypes_ObjectType(uint16_t windowsType) {
	if (!XwFlightTypes_Dos())
		return windowsType;
	return windowsType < MODEL_TYPE_RECORD_COUNT ? windowsToDos[windowsType] : UINT16_MAX;
}

uint16_t XwFlightTypes_CanonicalType(uint16_t objectType) {
	if (!XwFlightTypes_Dos())
		return objectType;
	return objectType < DOS94_MODEL_COUNT ? dosToWindows[objectType] : UINT16_MAX;
}

uint8_t XwFlightTypes_MissionType(uint16_t missionType) {
	if (XwFlightTypes_Dos())
		return missionType < DOS94_MISSION_TYPE_COUNT ? Dos94_missionModelTypes[missionType] : 0;
	return missionType < MISSION_CRAFT_TYPE_COUNT ? g_craftTypeToObjectType[missionType] : 0;
}

uint16_t XwFlightTypes_Definition(uint16_t objectType) {
	if (!XwFlightTypes_Dos())
		return objectType < MODEL_TYPE_RECORD_COUNT ? g_modelTypeTable[objectType].craftDefinitionIndex : 255;
	/* DOS94 0x696586: spec_getspecnum. Only craft callers may index definitions. */
	return objectType == 118 ? 17 : objectType == 119 ? 18 : (uint16_t)(objectType - 1);
}

uint16_t XwFlightTypes_StatisticsCategory(uint16_t objectType) {
	if (XwFlightTypes_Dos())
		return XwFlightTypes_Definition(objectType);
	if (objectType == XW_OBJ_B_WING)
		return 17;
	if (objectType == XW_OBJ_INTERDICTOR)
		return 18;
	return objectType < MODEL_TYPE_RECORD_COUNT && g_objectTypeHudShipIds[objectType]
			   ? g_objectTypeHudShipIds[objectType] - 1
			   : 0;
}

uint16_t XwFlightTypes_ScoreWeight(uint16_t category) {
	if (category >= 24)
		return 0;
	if (XwFlightTypes_Dos())
		return Dos94_scoreWeights[category];
	uint16_t definition = category == 17 ? 4 : category == 18 ? 51 : g_craftTypeToObjectType[category];
	/* Windows categories 19..23 map beyond the craft definition table. */
	if (definition >= XW_CRAFT_TYPE_COUNT)
		return 0;
	return g_craftTypeDefs[definition].killValue;
}

int XwFlightTypes_ProjectileIndex(uint16_t objectType) {
	uint16_t canonical = XwFlightTypes_CanonicalType(objectType);
	return canonical >= XW_OBJ_LASER_143 && canonical <= XW_OBJ_TRACKED_WARHEAD ? canonical - XW_OBJ_LASER_143
																				: -1;
}

int XwFlightTypes_ProjectileSound(uint16_t objectType) {
	int index = XwFlightTypes_ProjectileIndex(objectType);
	return index < 0 ? -1 : index + 4;
}

bool XwFlightTypes_IsWarhead(uint16_t objectType) {
	int index = XwFlightTypes_ProjectileIndex(objectType);
	return index == 6 || index == 7;
}

bool XwFlightTypes_IsMine(uint16_t objectType) {
	if (XwFlightTypes_Dos())
		return objectType == 26 || objectType == 27 || objectType == 28 || objectType == 29;
	return objectType == 75 || objectType == 76 || objectType == 77 || objectType == 78;
}

bool XwFlightTypes_IsFlybyCraft(uint16_t objectType) {
	uint16_t type = XwFlightTypes_CanonicalType(objectType);
	return type == XW_OBJ_TIE_FIGHTER || type == XW_OBJ_TIE_INTERCEPTOR || type == XW_OBJ_TIE_BOMBER ||
		   type == XW_OBJ_TIE_ADVANCED;
}

bool XwFlightTypes_Targetable(uint16_t objectType) {
	if (XwFlightTypes_Dos())
		return objectType < DOS94_MODEL_COUNT && Dos94Assets_Model(objectType)->metadata.radarVisible != 0;
	return objectType < MODEL_TYPE_RECORD_COUNT &&
		   (g_modelTypeTable[objectType].objectFlags & MODEL_TYPE_TARGETABLE) != 0;
}

uint8_t XwFlightTypes_ModelFlags(uint16_t objectType) {
	if (XwFlightTypes_Dos())
		return objectType < DOS94_MODEL_COUNT ? Dos94Assets_Model(objectType)->metadata.flags : 0;
	return objectType < MODEL_TYPE_RECORD_COUNT ? g_modelTypeTable[objectType].flags : 0;
}

void XwFlightTypes_SetModelFlags(uint16_t objectType, uint8_t flags) {
	if (XwFlightTypes_Dos()) {
		if (objectType < DOS94_MODEL_COUNT)
			Dos94Assets_Model(objectType)->metadata.flags = flags;
	} else if (objectType < MODEL_TYPE_RECORD_COUNT)
		g_modelTypeTable[objectType].flags = flags;
}

void XwFlightTypes_ClearRequiredModels(void) {
	if (XwFlightTypes_Dos()) {
		for (uint16_t i = 0; i < DOS94_MODEL_COUNT; ++i) {
			Dos94ModelMetadata* model = &Dos94Assets_Model(i)->metadata;
			if (!model->isAlias)
				model->flags &= ~0x10;
		}
	} else {
		for (uint16_t i = 0; i < MODEL_TYPE_RECORD_COUNT; ++i)
			if (g_modelTypeTable[i].resourceFlags)
				g_modelTypeTable[i].flags &= ~MODEL_TYPE_FLAG_MISSION_REQUIRED;
	}
}

void XwFlightTypes_RequireModel(uint16_t objectType) {
	if (XwFlightTypes_Dos()) {
		if (objectType >= DOS94_MODEL_COUNT)
			return;
		Dos94ModelMetadata* model = &Dos94Assets_Model(objectType)->metadata;
		/* Alias flag bytes are source IDs, not load masks. */
		if (model->isAlias)
			objectType = model->flags;
	}
	XwFlightTypes_SetModelFlags(objectType, XwFlightTypes_ModelFlags(objectType) | 0x10);
}

uint16_t XwFlightTypes_ComponentCount(uint16_t objectType) {
	if (XwFlightTypes_Dos())
		return objectType < DOS94_MODEL_COUNT ? Dos94Assets_Model(objectType)->metadata.componentCount : 0;
	return ModelMesh_GetCachedObjectTypeMeshCount(objectType);
}
