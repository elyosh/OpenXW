#include "xw/flight/object/dynamix.h"

#ifdef XW_MODERN
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/flight_timing.h"
#endif

#include "xw/flight/death_star.h"
#include "xw/flight/fview.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C5368
const uint16_t g_engineAverageFactorQ16ByCount[DYNAMIX_ENGINE_WEIGHT_COUNT] = { 0xFFFF, 0xFFFF, 0x8000,
																				0x5555, 0x4000 };

// GLOBAL: XW 0x63BBC6
uint16_t g_dynamicsCraftTypeIndex = 0;

// FUNCTION: XW 0x408FF0
void dynamix_planedynamics(void) {
	unsigned int objectIndex;
	for (objectIndex = 0; (uint16_t)objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		ObjectRecord* object = &g_objectTable[objectIndex];
		CraftData* craft;
		int16_t previousPitch, previousYaw, previousRoll;
		uint16_t combinedThrottle;
		int throttleAccumulator;
		uint16_t engineCount;
#ifdef XW_MODERN
		XwFlightIntegration_ObserveCraft(objectIndex);
#endif
		if (object->objectType == XW_OBJ_NONE || object->familyId != XW_OBJECT_FAMILY_CRAFT)
			continue;
		previousPitch = object->pitch;
		craft = object->instanceData;
		previousYaw = object->yaw;
		previousRoll = object->roll;
		g_curCraft = craft;
		g_dynamicsCraftTypeIndex = craft->craftTypeIndex;
		combinedThrottle = 0;
		throttleAccumulator = 0;
		engineCount = g_craftTypeDefs[g_dynamicsCraftTypeIndex].engineCount;
		if ((craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ENGINE) != 0 && engineCount != 0) {
			unsigned int engineIndex;
			unsigned int enginesRemaining = engineCount;
			for (engineIndex = 0; enginesRemaining != 0; ++engineIndex, --enginesRemaining) {
				uint16_t output = math2_fraction(craft->engineThrottle[engineIndex],
												 craft->enginePowerScaleQ16[engineIndex]);
				g_curCraft->engineOutputQ16[engineIndex] = output;
				throttleAccumulator += math2_fraction(output, g_engineAverageFactorQ16ByCount[engineCount]);
				if ((uint16_t)throttleAccumulator < combinedThrottle)
					throttleAccumulator = UINT16_MAX;
				craft = g_curCraft;
				combinedThrottle = throttleAccumulator;
			}
		}
		if ((uint16_t)objectIndex != g_playerFlightState.objectIndex &&
			(craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) != 0) {
			if (craft->aiRollState >= XW_AI_ROLL_STATE_1 && craft->aiRollState <= XW_AI_ROLL_STATE_3) {
				uint16_t delta = craft->aiTargetRoll - object->roll;
				uint16_t step;
#ifdef XW_MODERN
				if (XwFlightTiming_IsUnlocked())
					step = XwFlightIntegration_Steer(
						objectIndex, XW_INTEGRATE_ROLL, craft->rollRateLimit, craft->aiRollRateScaleQ16,
						craft->aiRollStepQ16,
						(craft->aiRollState == XW_AI_ROLL_STATE_3 ? craft->aiTargetRoll : delta) <
								TRIG2_ANGLE_SIGN_BIT
							? 1
							: -1);
				else
#endif
				{
					step = craft->rollRateLimit / (int)g_simStepScale;
					step = math2_fraction(step, craft->aiRollRateScaleQ16);
					step = math2_fraction(step, g_curCraft->aiRollStepQ16);
				}
				craft = g_curCraft;
				if (craft->aiRollState != XW_AI_ROLL_STATE_3) {
					if (delta < TRIG2_ANGLE_SIGN_BIT) {
						if (delta <= step) {
							object->roll = craft->aiTargetRoll;
							craft->aiRollState = XW_AI_ROLL_TARGET_REACHED;
							craft = g_curCraft;
						} else
							object->roll += step;
					} else {
						delta = -delta;
						if (delta <= step) {
							object->roll = craft->aiTargetRoll;
							craft->aiRollState = XW_AI_ROLL_TARGET_REACHED;
							craft = g_curCraft;
						} else
							object->roll -= step;
					}
				} else if (craft->aiTargetRoll < TRIG2_ANGLE_SIGN_BIT)
					object->roll += step;
				else
					object->roll -= step;
			}
			if (craft->aiPitchState > XW_AI_PITCH_INACTIVE) {
				uint16_t delta = craft->aiTargetPitch - craft->pitch;
				uint16_t step;
				if (delta >= TRIG2_ANGLE_SIGN_BIT)
					delta = -delta;
#ifdef XW_MODERN
				if (XwFlightTiming_IsUnlocked())
					step = XwFlightIntegration_Steer(objectIndex, XW_INTEGRATE_PITCH, craft->pitchRateLimit,
													 craft->aiPitchRateScaleQ16, craft->aiPitchStepQ16,
													 craft->aiPitchState == XW_AI_PITCH_DECREMENT ? -1 : 1);
				else
#endif
				{
					step = craft->pitchRateLimit / (int)g_simStepScale;
					step = math2_fraction(step, craft->aiPitchRateScaleQ16);
					step = math2_fraction(step, g_curCraft->aiPitchStepQ16);
				}
				craft = g_curCraft;
				if (craft->aiPitchState == XW_AI_PITCH_DECREMENT) {
					if (delta <= step && craft->aiPitchForce == 0) {
						craft->pitch = craft->aiTargetPitch;
						g_curCraft->aiPitchState = XW_AI_PITCH_TARGET_REACHED;
						craft = g_curCraft;
					} else {
						craft->pitch -= step;
						craft = g_curCraft;
						if (craft->pitch >= DYNAMIX_PITCH_NEGATIVE_WRAP) {
							craft->pitch = -craft->pitch;
							craft = g_curCraft;
							object->yaw += TRIG2_ANGLE_SIGN_BIT;
							object->roll += TRIG2_ANGLE_SIGN_BIT;
							craft->aiPitchForce = 0;
							g_curCraft->aiPitchState = XW_AI_PITCH_INCREMENT;
							craft = g_curCraft;
						}
					}
				} else if (craft->aiPitchState == XW_AI_PITCH_INCREMENT) {
					if (delta <= step && craft->aiPitchForce == 0) {
						craft->pitch = craft->aiTargetPitch;
						g_curCraft->aiPitchState = XW_AI_PITCH_TARGET_REACHED;
						craft = g_curCraft;
					} else {
						craft->pitch += step;
						craft = g_curCraft;
						if (craft->pitch >= TRIG2_ANGLE_SIGN_BIT) {
							craft->pitch = -craft->pitch;
							craft = g_curCraft;
							object->yaw += TRIG2_ANGLE_SIGN_BIT;
							object->roll += TRIG2_ANGLE_SIGN_BIT;
							craft->aiPitchForce = 0;
							g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
							craft = g_curCraft;
						}
					}
				}
			}
			if (craft->objectKind != XW_CRAFT_OBJECT_KIND_2 && craft->aiYawState >= XW_AI_YAW_STATE_1) {
				uint16_t delta = craft->aiTargetYaw - object->yaw;
				if (delta != 0) {
					uint16_t step;
#ifdef XW_MODERN
					if (XwFlightTiming_IsUnlocked())
						step = XwFlightIntegration_Steer(objectIndex, XW_INTEGRATE_YAW, craft->yawRateLimit,
														 craft->aiYawRateScaleQ16, craft->aiYawStepQ16,
														 delta < TRIG2_ANGLE_SIGN_BIT ? 1 : -1);
					else
#endif
					{
						step = craft->yawRateLimit / (int)g_simStepScale;
						step = math2_fraction(step, craft->aiYawRateScaleQ16);
						step = math2_fraction(step, g_curCraft->aiYawStepQ16);
					}
					if (delta < TRIG2_ANGLE_SIGN_BIT && delta > step)
						object->yaw += step;
					else if (delta >= TRIG2_ANGLE_SIGN_BIT && (uint16_t)-delta > step)
						object->yaw -= step;
					else {
						craft = g_curCraft;
						object->yaw = craft->aiTargetYaw;
						craft->aiYawState = XW_AI_YAW_TARGET_REACHED;
						step = 0;
					}
					craft = g_curCraft;
					if (craft->aiRollState == 0 || craft->aiRollState == XW_AI_ROLL_TARGET_REACHED) {
						int16_t bankStep;
#ifdef XW_MODERN
						if (XwFlightTiming_IsUnlocked())
							bankStep = (int16_t)XwFlightIntegration_Bank(
								objectIndex, step, g_craftTypeDefs[g_dynamicsCraftTypeIndex].rollPerYawQ16,
								delta < TRIG2_ANGLE_SIGN_BIT ? 1 : -1);
						else
#endif
							bankStep =
								math2_fraction(step, g_craftTypeDefs[g_dynamicsCraftTypeIndex].rollPerYawQ16);
						if (delta < TRIG2_ANGLE_SIGN_BIT)
							object->roll -= bankStep;
						else
							object->roll += bankStep;
						craft = g_curCraft;
					}
				}
			}
		}
		if ((uint16_t)objectIndex != g_playerFlightState.objectIndex
#ifdef XW_MODERN
			&& XwFlightTiming_ReferenceDue()
#endif
		) {
#ifdef XW_MODERN
			XwFlightClock clock = XwFlightTiming_EnterReference();
#endif
			if (craft->aiClimbState == DYNAMIX_CLIMB_DIVE_ACTIVE &&
				(craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) != 0 &&
				object->worldZ >= craft->aiAimPointZ) {
				craft->aiClimbState = 0;
				g_curCraft->pitch = TRIG2_QUARTER_TURN;
				craft = g_curCraft;
			}
			if ((uint16_t)objectIndex != g_playerFlightState.objectIndex &&
				craft->aiDiveState == DYNAMIX_CLIMB_DIVE_ACTIVE &&
				(craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) != 0) {
				dynamix_pulloutdive(objectIndex);
				craft = g_curCraft;
			}
#ifdef XW_MODERN
			XwFlightTiming_RestoreClock(clock);
#endif
		}
		switch (craft->objectKind) {
			case XW_CRAFT_OBJECT_KIND_0: {
				uint16_t baseSpeed;
				int16_t powerAdjustment;
				uint16_t speedStep, targetSpeed;
				object->pitch = craft->pitch;
				baseSpeed = craft->maxSpeed;
				if (g_deathStarSurfaceModeActive != 0 &&
					g_objectTable[g_playerFlightState.objectIndex].worldZ < 0)
					baseSpeed *= DYNAMIX_SURFACE_ACCELERATION_MULTIPLIER;
				powerAdjustment = DYNAMIX_POWER_BALANCED_TOTAL - craft->shieldRedirect - craft->laserRedirect;
				if (object->objectType == XW_OBJ_Y_WING)
					speedStep = math2_fraction(baseSpeed, DYNAMIX_Y_WING_POWER_SPEED_Q16);
				else
					speedStep = math2_fraction(baseSpeed, DYNAMIX_POWER_SPEED_Q16);
				baseSpeed += powerAdjustment * speedStep;
				targetSpeed = math2_fraction(baseSpeed, combinedThrottle);
				if (targetSpeed < object->speed) {
					if (g_deathStarSurfaceModeActive != 0 &&
						g_objectTable[g_playerFlightState.objectIndex].worldZ < 0)
						dynamix_subvelocity(objectIndex, DYNAMIX_SURFACE_DECELERATION);
					else
						dynamix_subvelocity(objectIndex, DYNAMIX_DECELERATION);
				} else
					dynamix_adjustvelocity(objectIndex, targetSpeed, 1, combinedThrottle);
#ifdef XW_MODERN
				if (object->speed == targetSpeed)
					XwFlightIntegration_Clear(objectIndex, XW_INTEGRATE_VELOCITY);
#endif
				craft = g_curCraft;
				break;
			}
			case XW_CRAFT_OBJECT_KIND_2:
				if (object->speed > 0) {
					dynamix_subvelocity(objectIndex, DYNAMIX_DECELERATION);
					craft = g_curCraft;
				}
#ifdef XW_MODERN
				if (!object->speed)
					XwFlightIntegration_Clear(objectIndex, XW_INTEGRATE_VELOCITY);
#endif
				break;
			case XW_CRAFT_OBJECT_KIND_1:
			case XW_CRAFT_OBJECT_KIND_3:
			case XW_CRAFT_OBJECT_KIND_4:
			case XW_CRAFT_OBJECT_KIND_6:
#ifdef XW_MODERN
				XwFlightIntegration_Clear(objectIndex, XW_INTEGRATE_VELOCITY);
#endif
				craft->aiClimbState = 0;
				g_curCraft->aiDiveState = 0;
				g_curCraft->aiRollState = 0;
				g_curCraft->aiPitchState = 0;
				craft = g_curCraft;
				break;
			case XW_CRAFT_OBJECT_KIND_5:
				if (craft->aiManeuverAuxTimerTicks != 0)
					dynamix_addvelocity(objectIndex, DYNAMIX_HYPERSPACE_INITIAL_ACCELERATION);
				else if (craft->aiManeuverTimerTicks != 0)
					dynamix_addvelocity(objectIndex, DYNAMIX_HYPERSPACE_MIDDLE_ACCELERATION);
				else
					dynamix_addvelocity(objectIndex, DYNAMIX_HYPERSPACE_FINAL_ACCELERATION);
				craft = g_curCraft;
				break;
		}
#ifdef XW_MODERN
		XwFlightIntegration_ObserveCraft(objectIndex);
#endif
		craft->yaw = object->yaw;
		if (previousPitch != object->pitch || previousYaw != object->yaw || previousRoll != object->roll) {
			object->moveVectorDirty = 1;
			object->orientMatrixDirty = 1;
		}
	}
}

// FUNCTION: XW 0x4095C0
void dynamix_adjustvelocity(uint16_t objectIndex, int16_t targetSpeed, int16_t allowDeceleration,
							uint16_t throttleFraction) {
	targetSpeed -= g_objectTable[objectIndex].speed;
#ifdef XW_MODERN
	if (!targetSpeed || ((uint16_t)targetSpeed >= DYNAMIX_NEGATIVE_SPEED_DELTA && allowDeceleration != 1))
		XwFlightIntegration_Clear(objectIndex, XW_INTEGRATE_VELOCITY);
#endif
	if ((uint16_t)targetSpeed != 0) {
		if ((uint16_t)targetSpeed < DYNAMIX_NEGATIVE_SPEED_DELTA) {
			uint16_t acceleration = math2_fraction(g_craftTypeDefs[g_dynamicsCraftTypeIndex].maxAcceleration,
												   DYNAMIX_BASE_ACCELERATION_Q16);
			if (acceleration == 0)
				acceleration = 1;
			acceleration += math2_fraction(
				g_craftTypeDefs[g_dynamicsCraftTypeIndex].maxAcceleration - acceleration, throttleFraction);
			if (g_deathStarSurfaceModeActive != 0 &&
				g_objectTable[g_playerFlightState.objectIndex].worldZ < 0)
				acceleration *= DYNAMIX_SURFACE_ACCELERATION_MULTIPLIER;
			if ((uint16_t)targetSpeed < acceleration)
				acceleration = (uint16_t)targetSpeed;
			dynamix_addvelocity(objectIndex, acceleration);
		} else if (allowDeceleration == 1) {
			targetSpeed =
				math2_fraction(-targetSpeed, g_craftTypeDefs[g_dynamicsCraftTypeIndex].decelerationGain);
			if (targetSpeed == 0)
				targetSpeed = 1;
			dynamix_subvelocity(objectIndex, targetSpeed);
		}
	}
}

// FUNCTION: XW 0x4096F0
void dynamix_addvelocity(uint16_t objectIndex, uint16_t acceleration) {
	uint16_t speedStep;
	uint16_t previousFraction;

#ifdef XW_MODERN
	if (XwFlightTiming_IsUnlocked()) {
		XwFlightIntegration_ChangeVelocity(objectIndex, acceleration, 1);
		return;
	}
#endif

	speedStep = math2_divide(acceleration, g_simStepScale);
	previousFraction = g_objectTable[objectIndex].speedFractionQ16;
	g_objectTable[objectIndex].speedFractionQ16 += g_mathDivideFractionQ16;
	if (g_objectTable[objectIndex].speedFractionQ16 < previousFraction) {
		++g_objectTable[objectIndex].speed;
	}
	g_objectTable[objectIndex].speed += speedStep;
	if (g_objectTable[objectIndex].speed > 3600) {
		g_objectTable[objectIndex].speed = 3600;
	}
}

// FUNCTION: XW 0x409760
void dynamix_subvelocity(uint16_t objectIndex, uint16_t deceleration) {
	uint16_t speedStep;
	uint16_t previousFraction;

#ifdef XW_MODERN
	if (XwFlightTiming_IsUnlocked()) {
		XwFlightIntegration_ChangeVelocity(objectIndex, deceleration, -1);
		return;
	}
#endif

	speedStep = math2_divide(deceleration, g_simStepScale);
	previousFraction = g_objectTable[objectIndex].speedFractionQ16;
	g_objectTable[objectIndex].speedFractionQ16 -= g_mathDivideFractionQ16;
	if (g_objectTable[objectIndex].speedFractionQ16 > previousFraction) {
		--g_objectTable[objectIndex].speed;
	}
	g_objectTable[objectIndex].speed -= speedStep;
	if (g_objectTable[objectIndex].speed > 0x8000) {
		g_objectTable[objectIndex].speed = 0;
	}
}

// FUNCTION: XW 0x4097D0
void dynamix_pulloutdive(uint16_t objectIndex) {
	ObjectRecord* object = &g_objectTable[objectIndex];
	int altitude = (int32_t)((uint32_t)object->worldZ - (uint32_t)g_curCraft->aiAimPointZ);
	if (altitude < 0 || altitude <= DYNAMIX_DIVE_LEVEL_ALTITUDE) {
		g_curCraft->pitch = TRIG2_QUARTER_TURN;
		g_curCraft->aiPitchState = XW_AI_PITCH_INACTIVE;
		g_curCraft->aiDiveState = DYNAMIX_DIVE_LEVELLED;
	} else {
		int descentHorizon;
		if (object->moveVectorDirty != 0)
			fview_calcrotatemove(object->pitch, object->yaw, object);
		if (object->genusId == XW_GENUS_STARFIGHTER)
			descentHorizon = (int32_t)(0u - DYNAMIX_FIGHTER_DESCENT_SCALE * (uint32_t)g_simStepScale *
												(uint32_t)(int32_t)object->moveZ);
		else
			descentHorizon = (int32_t)(0u - DYNAMIX_OTHER_DESCENT_SCALE * (uint32_t)g_simStepScale *
												(uint32_t)(int32_t)object->moveZ);
		if (altitude <= descentHorizon && g_curCraft->pitch > TRIG2_QUARTER_TURN) {
			g_curCraft->aiTargetPitch = ((g_curCraft->pitch - TRIG2_QUARTER_TURN) >> 1) + TRIG2_QUARTER_TURN;
			g_curCraft->aiPitchState = XW_AI_PITCH_DECREMENT;
		}
	}
}
