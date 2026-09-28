#include "xw/flight/object/move.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/flight_timing.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fview.h"
#include "xw/flight/gate.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/object.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C7890
const uint16_t g_warheadHomingAngularRateByTier[MOVE_HOMING_RATE_COUNT] = {
	0, 1024, 2048, 3072, 5120, 7168, 9216, 0, 512, 1024, 2048, 3072, 4608, 6144, 0, 0
};

// GLOBAL: XW 0x4C78B0
const int16_t g_playerCockpitOffsetForwardByCraft[MOVE_COCKPIT_OFFSET_CRAFT_COUNT] = { 48, 224, 24 };

// GLOBAL: XW 0x4C78B8
const int16_t g_playerCockpitOffsetUpByCraft[MOVE_COCKPIT_OFFSET_CRAFT_COUNT] = { 43, 0, 8 };

// GLOBAL: XW 0x4F4918
int16_t g_playerCockpitOffsetSideByCraft[MOVE_COCKPIT_OFFSET_CRAFT_COUNT] = { 0 };

// GLOBAL: XW 0x637308
ObjectRecord* g_movementCurrentObject = NULL;

// FUNCTION: XW 0x4115B0
void move_moveobjects(void) {
	int16_t playerOffsetSide;
	int16_t playerOffsetForward;
	int16_t playerOffsetUp;
	int16_t stepScale;
	int objectIndex;
	if (g_playerFlightState.craftTypeIndex < MOVE_COCKPIT_OFFSET_CRAFT_COUNT) {
		int playerCraftType = g_playerFlightState.craftTypeIndex;
		playerOffsetSide = g_playerCockpitOffsetSideByCraft[playerCraftType];
		playerOffsetForward = g_playerCockpitOffsetForwardByCraft[playerCraftType];
		playerOffsetUp = g_playerCockpitOffsetUpByCraft[playerCraftType];
	} else {
		playerOffsetSide = 0;
		playerOffsetForward = 0;
		playerOffsetUp = MOVE_DEFAULT_COCKPIT_UP;
	}
	pai_calcrotatedpoint(g_playerFlightState.object, playerOffsetSide, playerOffsetUp, playerOffsetForward);
	g_playerFlightState.previousRotatedCockpitOffset = g_playerFlightState.rotatedCockpitOffset;
	g_playerFlightState.rotatedCockpitOffset.x = g_rotatedX;
	g_playerFlightState.rotatedCockpitOffset.y = g_rotatedY;
	g_playerFlightState.rotatedCockpitOffset.z = g_rotatedZ;
	stepScale = (int16_t)g_simStepScale;
	for (objectIndex = 0; (uint16_t)objectIndex < XW_OBJECT_COUNT; ++objectIndex) {
		ObjectRecord* object = &g_objectTable[objectIndex];
		uint8_t objectType = object->objectType;
		uint16_t objectGenus;
		int frameDistance;
		g_movementCurrentObject = object;
#ifdef XW_MODERN
		XwFlightIntegration_Observe(objectIndex);
#endif
		if (objectType == XW_OBJ_NONE)
			continue;
		objectGenus = object->genusId;
		if (object->lifetimeTicks != 0) {
			int16_t remainingLifetime = (int16_t)(object->lifetimeTicks - g_elapsedTicks);
			if (remainingLifetime < 0)
				remainingLifetime = 0;
			object->lifetimeTicks = remainingLifetime;
			if (remainingLifetime == 0) {
				switch (objectGenus) {
					case XW_GENUS_TRANSPORT:
					case XW_GENUS_UTILITY:
					case XW_GENUS_FREIGHTER:
					case XW_GENUS_STARSHIP:
						if (!g_deathStarSurfaceModeActive &&
							g_modelTypeTable[objectType].maxBoundsExtent > COLLIDE_CRAFT_LARGE_EXTENT)
							starship_createstarshipexplo(objectIndex);
#ifdef XW_MODERN
						collide_makeobjectexplosion(objectIndex,
													(uint8_t)(XwFlightTypes_ObjectType(XW_OBJ_EXPLOSION_133) +
															  (math2_getrandom() & 1)));
#else
						collide_makeobjectexplosion(
							objectIndex, (uint8_t)(XW_OBJ_EXPLOSION_133 + (math2_getrandom() & 1)));
#endif
						break;
					case XW_GENUS_STARFIGHTER:
						create_blowoffcomponent(objectIndex, 1);
#ifdef XW_MODERN
						collide_makeobjectexplosion(objectIndex,
													(uint8_t)(XwFlightTypes_ObjectType(XW_OBJ_EXPLOSION_133) +
															  (math2_getrandom() & 1)));
#else
						collide_makeobjectexplosion(
							objectIndex, (uint8_t)(XW_OBJ_EXPLOSION_133 + (math2_getrandom() & 1)));
#endif
						break;
					case XW_GENUS_DEBRIS:
#ifdef XW_MODERN
						collide_makeobjectexplosion(objectIndex,
													XwFlightTypes_ObjectType(XW_OBJ_EXPLOSION_133));
#else
						collide_makeobjectexplosion(objectIndex, XW_OBJ_EXPLOSION_133);
#endif
						break;
					default:
#ifdef XW_MODERN
						if (objectType != XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD) &&
							objectType != XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)) {
#else
						if (objectType != XW_OBJ_TRACKED_WARHEAD && objectType != XW_OBJ_WARHEAD_149) {
#endif
							object->objectType = XW_OBJ_NONE;
							continue;
						}
#ifdef XW_MODERN
						collide_makeobjectexplosion(objectIndex,
													XwFlightTypes_ObjectType(XW_OBJ_EXPLOSION_133));
#else
						collide_makeobjectexplosion(objectIndex, XW_OBJ_EXPLOSION_133);
#endif
						break;
				}
			}
		}
		if (g_missionRuntimeState.provingGroundsActive && objectIndex == g_playerFlightState.objectIndex)
			gate_savegatelastpos();
		object->prevWorldY = object->worldY;
		object->prevWorldZ = object->worldZ;
		object->prevWorldX = object->worldX;
		if (object->rollImpulseRate != 0) {
			object->orientMatrixDirty = 1;
#ifdef XW_MODERN
			if (XwFlightTiming_IsUnlocked())
				object->roll = (int16_t)(object->roll +
										 MOVE_ROLL_IMPULSE_SCALE *
											 XwFlightIntegration_Rate(objectIndex, XW_INTEGRATE_IMPULSE_ROLL,
																	  object->rollImpulseRate, g_elapsedTicks,
																	  XW_SIMULATION_TICKS_PER_SECOND));
			else
#endif
				object->roll =
					(int16_t)(object->roll + MOVE_ROLL_IMPULSE_SCALE * (object->rollImpulseRate / stepScale));
		}
#ifdef XW_MODERN
		if (XwFlightTiming_IsUnlocked())
			frameDistance = 0;
		else
#endif
			frameDistance = object->speed ? (int16_t)math2_mphconvert(object->speed, g_simStepScale) : 0;
		switch (objectGenus) {
			case XW_GENUS_STARFIGHTER:
			case XW_GENUS_TRANSPORT:
			case XW_GENUS_UTILITY:
			case XW_GENUS_FREIGHTER:
			case XW_GENUS_STARSHIP: {
				CraftData* craft = object->instanceData;
				if (object->moveVectorDirty)
					fview_calcrotatemove(object->pitch, object->yaw, object);
#ifdef XW_MODERN
				if (XwFlightTiming_IsUnlocked())
					XwFlightIntegration_Move(objectIndex);
				else
#endif
				{
					g_moveDeltaX = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveX) >>
													   FVIEW_MATRIX_FRACTION_BITS);
					g_moveDeltaY = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveY) >>
													   FVIEW_MATRIX_FRACTION_BITS);
					g_moveDeltaZ = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveZ) >>
													   FVIEW_MATRIX_FRACTION_BITS);
				}
				if (craft->workingSubsystems != 0) {
					int axisCorrectionLimit =
						craft->aiManeuverId == MOVE_BOARD_MANEUVER
							? MOVE_BOARD_CORRECTION_LIMIT
							: g_craftTypeDefs[craft->craftTypeIndex].aiDisplacementRateLimit;
#ifdef XW_MODERN
					if (XwFlightTiming_IsUnlocked()) {
						XwFlightIntegration_Push(objectIndex, 0, &craft->aiDisplacementX, axisCorrectionLimit,
												 &g_moveDeltaX);
						XwFlightIntegration_Push(objectIndex, 1, &craft->aiDisplacementY, axisCorrectionLimit,
												 &g_moveDeltaY);
						XwFlightIntegration_Push(objectIndex, 2, &craft->aiDisplacementZ, axisCorrectionLimit,
												 &g_moveDeltaZ);
					} else
#endif
					{
						if (craft->aiDisplacementX != 0) {
							int remainingCorrection = craft->aiDisplacementX;
							int16_t clampedCorrection;
							int correctionStep;
							if (remainingCorrection < -axisCorrectionLimit)
								clampedCorrection = -axisCorrectionLimit;
							else if (remainingCorrection > axisCorrectionLimit)
								clampedCorrection = (int16_t)axisCorrectionLimit;
							else
								clampedCorrection = (int16_t)remainingCorrection;
							correctionStep = (int16_t)clampedCorrection / stepScale;
							if (correctionStep == 0)
								correctionStep = remainingCorrection;
#ifdef XW_MODERN
							craft->aiDisplacementX =
								(int32_t)((uint32_t)remainingCorrection - (uint32_t)correctionStep);
							g_moveDeltaX = (int32_t)((uint32_t)g_moveDeltaX + (uint32_t)correctionStep);
#else
							craft->aiDisplacementX = remainingCorrection - correctionStep;
							g_moveDeltaX += correctionStep;
#endif
						}
						if (craft->aiDisplacementY != 0) {
							int remainingCorrection = craft->aiDisplacementY;
							int16_t clampedCorrection;
							int correctionStep;
							if (remainingCorrection < -axisCorrectionLimit)
								clampedCorrection = -axisCorrectionLimit;
							else if (remainingCorrection > axisCorrectionLimit)
								clampedCorrection = (int16_t)axisCorrectionLimit;
							else
								clampedCorrection = (int16_t)remainingCorrection;
							correctionStep = (int16_t)clampedCorrection / stepScale;
							if (correctionStep == 0)
								correctionStep = remainingCorrection;
#ifdef XW_MODERN
							craft->aiDisplacementY =
								(int32_t)((uint32_t)remainingCorrection - (uint32_t)correctionStep);
							g_moveDeltaY = (int32_t)((uint32_t)g_moveDeltaY + (uint32_t)correctionStep);
#else
							craft->aiDisplacementY = remainingCorrection - correctionStep;
							g_moveDeltaY += correctionStep;
#endif
						}
						if (craft->aiDisplacementZ != 0) {
							int remainingCorrection = craft->aiDisplacementZ;
							int16_t clampedCorrection;
							int correctionStep;
							if (remainingCorrection < -axisCorrectionLimit)
								clampedCorrection = -axisCorrectionLimit;
							else if (remainingCorrection > axisCorrectionLimit)
								clampedCorrection = (int16_t)axisCorrectionLimit;
							else
								clampedCorrection = (int16_t)remainingCorrection;
							correctionStep = (int16_t)clampedCorrection / stepScale;
							if (correctionStep == 0)
								correctionStep = remainingCorrection;
#ifdef XW_MODERN
							craft->aiDisplacementZ =
								(int32_t)((uint32_t)remainingCorrection - (uint32_t)correctionStep);
							g_moveDeltaZ = (int32_t)((uint32_t)g_moveDeltaZ + (uint32_t)correctionStep);
#else
							craft->aiDisplacementZ = remainingCorrection - correctionStep;
							g_moveDeltaZ += correctionStep;
#endif
						}
					}
				}
				move_updatexyz(object);
				break;
			}
			case XW_GENUS_PLAYER_PROJECTILE:
			case XW_GENUS_OTHER_PROJECTILE: {
				WarheadGuidanceState* guidance = object->instanceData;
				uint16_t homingRateIndex = guidance->homingTier;
				if (homingRateIndex != 0 && guidance->targetObjIdx != XW_OBJECT_SLOT_UNAVAILABLE) {
					uint16_t targetObjectRef = guidance->targetObjIdx;
					int guidanceAngularStep;
					int16_t angleDelta;
					int16_t absoluteDelta;
					if (targetObjectRef < XW_OBJECT_COUNT &&
						g_objectTable[targetObjectRef].objectType == XW_OBJ_NONE) {
#ifdef XW_MODERN
						collide_makeobjectexplosion(objectIndex,
													XwFlightTypes_ObjectType(XW_OBJ_EXPLOSION_133));
#else
						collide_makeobjectexplosion(objectIndex, XW_OBJ_EXPLOSION_133);
#endif
						break;
					}
#ifdef XW_MODERN
					if (object->objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149))
#else
					if (object->objectType == XW_OBJ_WARHEAD_149)
#endif
						homingRateIndex += MOVE_TORPEDO_HOMING_RATE_OFFSET;
					pai_distancebetween(objectIndex, targetObjectRef);
#ifdef XW_MODERN
					if (XwFlightTiming_IsUnlocked()) {
						object->yaw = XwFlightIntegration_Home(
							objectIndex, XW_INTEGRATE_HOME_YAW, object->yaw, g_trig2Yaw,
							g_warheadHomingAngularRateByTier[homingRateIndex]);
						object->pitch = XwFlightIntegration_Home(
							objectIndex, XW_INTEGRATE_HOME_PITCH, object->pitch, g_trig2Pitch,
							g_warheadHomingAngularRateByTier[homingRateIndex]);
					} else
#endif
					{
						guidanceAngularStep =
							g_warheadHomingAngularRateByTier[homingRateIndex] / (int)g_simStepScale;
						angleDelta = (int16_t)(g_trig2Yaw - object->yaw);
						absoluteDelta = angleDelta < 0 ? (int16_t)-angleDelta : angleDelta;
						if (absoluteDelta > (int)(uint16_t)guidanceAngularStep)
							object->yaw = (int16_t)(object->yaw + (angleDelta < 0 ? -guidanceAngularStep
																				  : guidanceAngularStep));
						else
							object->yaw = g_trig2Yaw;
						angleDelta = (int16_t)(g_trig2Pitch - object->pitch);
						absoluteDelta = angleDelta < 0 ? (int16_t)-angleDelta : angleDelta;
						if (absoluteDelta > (int)(uint16_t)guidanceAngularStep)
							object->pitch = (int16_t)(object->pitch + (angleDelta < 0 ? -guidanceAngularStep
																					  : guidanceAngularStep));
						else
							object->pitch = g_trig2Pitch;
					}
					object->orientMatrixDirty = 1;
					object->moveVectorDirty = 1;
					fview_calcrotatemove(object->pitch, object->yaw, object);
					object->moveX = (int16_t)g_craftMoveX;
					object->moveY = (int16_t)g_craftMoveZ;
					object->moveZ = (int16_t)g_craftMoveY;
				}
				if (object->moveVectorDirty)
					fview_calcrotatemove(object->pitch, object->yaw, object);
#ifdef XW_MODERN
				if (XwFlightTiming_IsUnlocked())
					XwFlightIntegration_Move(objectIndex);
				else
#endif
				{
					g_moveDeltaX = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveX) >>
													   FVIEW_MATRIX_FRACTION_BITS);
					g_moveDeltaY = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveY) >>
													   FVIEW_MATRIX_FRACTION_BITS);
					g_moveDeltaZ = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveZ) >>
													   FVIEW_MATRIX_FRACTION_BITS);
				}
				move_updatexyz(object);
				break;
			}
			case XW_GENUS_DEBRIS:
			case XW_GENUS_EXPLOSION_EFFECT:
				if (object->moveVectorDirty)
					fview_calcrotatemove(object->pitch, object->yaw, object);
#ifdef XW_MODERN
				if (XwFlightTiming_IsUnlocked())
					XwFlightIntegration_Move(objectIndex);
				else
#endif
				{
					g_moveDeltaX = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveX) >>
													   FVIEW_MATRIX_FRACTION_BITS);
					g_moveDeltaY = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveY) >>
													   FVIEW_MATRIX_FRACTION_BITS);
					g_moveDeltaZ = (int32_t)(uint32_t)((uint64_t)((int64_t)frameDistance * object->moveZ) >>
													   FVIEW_MATRIX_FRACTION_BITS);
				}
				move_updatexyz(object);
				if (object->sourceObjectRef == MOVE_FALLING_EFFECT_SOURCE) {
					object->pitch = (int16_t)(object->pitch + MOVE_FALLING_PITCH_RATE * g_elapsedTicks);
					if ((uint16_t)object->pitch > MOVE_FALLING_PITCH_LIMIT)
						object->pitch = MOVE_FALLING_PITCH_LIMIT;
					object->moveVectorDirty = 1;
					object->orientMatrixDirty = 1;
				}
				break;
			default:
				break;
		}
	}
}

// FUNCTION: XW 0x411C70
void move_updatexyz(struct ObjectRecord* object) {
	object->worldX = (int32_t)((uint32_t)object->worldX + (uint32_t)g_moveDeltaX);
	if (object->worldX < -MOVE_WORLD_COORDINATE_LIMIT) {
		object->worldX = -MOVE_WORLD_COORDINATE_LIMIT;
	}
	if (object->worldX > MOVE_WORLD_COORDINATE_LIMIT) {
		object->worldX = MOVE_WORLD_COORDINATE_LIMIT;
	}
	object->worldY = (int32_t)((uint32_t)object->worldY + (uint32_t)g_moveDeltaY);
	if (object->worldY < -MOVE_WORLD_COORDINATE_LIMIT) {
		object->worldY = -MOVE_WORLD_COORDINATE_LIMIT;
	}
	if (object->worldY > MOVE_WORLD_COORDINATE_LIMIT) {
		object->worldY = MOVE_WORLD_COORDINATE_LIMIT;
	}
	object->worldZ = (int32_t)((uint32_t)object->worldZ + (uint32_t)g_moveDeltaZ);
	if (object->worldZ < -MOVE_WORLD_COORDINATE_LIMIT) {
		object->worldZ = -MOVE_WORLD_COORDINATE_LIMIT;
	}
	if (object->worldZ > MOVE_WORLD_COORDINATE_LIMIT) {
		object->worldZ = MOVE_WORLD_COORDINATE_LIMIT;
	}
#ifdef XW_MODERN
	if (XwFlightTiming_IsUnlocked())
		XwFlightIntegration_ClampPosition((unsigned)(object - g_objectTable));
#endif
}
