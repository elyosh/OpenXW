#include "xw/flight/mission/spec.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#endif
#include "xw/assets/model_mesh.h"

// FUNCTION: XW 0x4239F0
uint16_t spec_getspecnum(uint16_t objectType) {
#ifdef XW_MODERN
	return XwFlightTypes_Definition(objectType);
#else
	return g_modelTypeTable[objectType].craftDefinitionIndex;
#endif
}

// FUNCTION: XW 0x423A10
uint16_t spec_getstatisticscategory(uint16_t objectType) {
#ifdef XW_MODERN
	return XwFlightTypes_StatisticsCategory(objectType);
#else
	if (objectType == XW_OBJ_B_WING) {
		return SPEC_STATISTICS_B_WING;
	}
	if (objectType == XW_OBJ_INTERDICTOR) {
		return SPEC_STATISTICS_INTERDICTOR;
	}
	if (g_objectTypeHudShipIds[objectType] == 0) {
		return 0;
	}
	return g_objectTypeHudShipIds[objectType] - 1;
#endif
}
