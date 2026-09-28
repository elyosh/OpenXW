#include "xw_runtime/runtime/flight_math.h"

#include "xw/flight/ai/pai.h"
#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/trig2.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"

/* DOS clears the multiplication high word before adding/subtracting world Y. */
void XwFlightMath_MoveHyperspace(int rate) {
	int delta = rate * g_elapsedTicks;
	if (XwFlightTypes_Dos())
		delta = rate < 0 ? -(int)(uint16_t)-delta : (uint16_t)delta;
	g_playerFlightState.object->worldY =
		(int32_t)((uint32_t)g_playerFlightState.object->worldY + (uint32_t)delta);
}

/* DOS narrows each Q15 component before multiplying by the prediction horizon. */
int32_t XwFlightMath_PredictionStep(int16_t distance, int32_t direction) {
	int32_t step = (int32_t)(((int64_t)distance * direction) >> FVIEW_MATRIX_FRACTION_BITS);
	if (XwFlightTypes_Dos())
		return (int16_t)step;
	return step;
}

bool XwFlightMath_ConvergeLaser(uint16_t sourceIndex, int32_t muzzleX, int32_t muzzleY, int32_t muzzleZ,
								XwLaserAim* aim) {
	if (!XwProfile_HasActiveFlight() || !XwProfile_MissionLaserConvergence() ||
		sourceIndex != g_playerFlightState.objectIndex || sourceIndex >= XW_CRAFT_OBJECT_COUNT)
		return false;
	uint16_t target = g_playerFlightState.currentTargetObjectIdx;
	const XwFlightProfile* profile = XwProfile_ActiveFlight();
	if (target < XW_MISSION_OBJECT_REF_BASE) {
		if (target >= profile->object_count || target == sourceIndex ||
			g_objectTable[target].objectType == XW_OBJ_NONE)
			return false;
	} else if (target - XW_MISSION_OBJECT_REF_BASE >= profile->static_object_count ||
			   g_missionObjects[target - XW_MISSION_OBJECT_REF_BASE].objectType == XW_OBJ_NONE)
		return false;

	/* Converge straight ahead at the selected target's distance. */
	pai_distancebetween(sourceIndex, target);
	int distance = g_trig2PolarDistance;
	if (distance < 4096)
		distance = 4096;
	if (distance > 65536)
		distance = 65536;
	const ObjectRecord* source = &g_objectTable[sourceIndex];
	int32_t dx =
		(int32_t)((uint32_t)source->worldX - (uint32_t)muzzleX +
				  (uint32_t)(((int64_t)distance * source->cachedForwardX) >> FVIEW_MATRIX_FRACTION_BITS));
	int32_t dy =
		(int32_t)((uint32_t)source->worldY - (uint32_t)muzzleY +
				  (uint32_t)(((int64_t)distance * source->cachedForwardY) >> FVIEW_MATRIX_FRACTION_BITS));
	int32_t dz =
		(int32_t)((uint32_t)source->worldZ - (uint32_t)muzzleZ +
				  (uint32_t)(((int64_t)distance * source->cachedForwardZ) >> FVIEW_MATRIX_FRACTION_BITS));
	if (dx == 0 && dy == 0 && dz == 0)
		return false;

	/* Use the flight version's angle quantization for both real shots and HUD prediction. */
	trig2_ctop(dx, dy, dz);
	aim->pitch = g_trig2Pitch;
	aim->yaw = g_trig2Yaw;
	fview_calcrotatemove(aim->pitch, aim->yaw, NULL);
	aim->moveX = (int16_t)g_craftMoveX;
	aim->moveY = (int16_t)g_craftMoveZ;
	aim->moveZ = (int16_t)g_craftMoveY;
	return true;
}
