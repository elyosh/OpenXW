#ifndef XW_DOS94_ASSETS_TABLES_H
#define XW_DOS94_ASSETS_TABLES_H

#include "xw/flight/object/craft.h"

enum { DOS94_MODEL_COUNT = 120, DOS94_CRAFT_DEFINITION_COUNT = 19, DOS94_MISSION_TYPE_COUNT = 109 };

/* Native references to the resident DOS tables; these are not disk layouts. */
typedef struct Dos94Component {
	const uint16_t* stateVariants;
	uint16_t stateCount;
	uint8_t nextChildIndex, childCount;
	uint16_t bitmapScale;
	int16_t eyeOffsetX, eyeOffsetY, eyeOffsetZ;
} Dos94Component;

typedef struct Dos94ModelMetadata {
	uint8_t isAlias, flags, familyId, genusId;
	uint16_t maxBoundsExtent;
	const Dos94Component* components;
	uint8_t descriptorCount, componentCount;
	const uint8_t* bitmapPalette;
	uint8_t radarVisible;
} Dos94ModelMetadata;

extern const Dos94ModelMetadata Dos94_modelTemplates[DOS94_MODEL_COUNT];
extern const Dos94ModelMetadata Dos93_modelTemplates[DOS94_MODEL_COUNT];
extern const XwCraftTypeDef Dos94_craftDefinitions[DOS94_CRAFT_DEFINITION_COUNT];
extern const uint8_t Dos94_missionModelTypes[DOS94_MISSION_TYPE_COUNT];
extern const uint8_t Dos94_scoreWeights[24];

#endif
