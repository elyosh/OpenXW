#include "xw/flight/ai/pai.h"

#include "xw/flight/ai/paifight.h"
#include "xw/flight/ai/paiman.h"
#include "xw/flight/ai/paiorder.h"
#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C51C8
uint16_t g_aiSkillValueQ16ByLevel[PAI_SKILL_LEVEL_COUNT] = { 0, 0x4000, 0x8000, 0xC000, 0xFFFF, 0, 0, 0 };

// GLOBAL: XW 0x4C51D8
uint16_t g_aiThinkIntervalBySkill[PAI_SKILL_LEVEL_COUNT] = { 708, 472, 236, 118, 59, 0, 0, 0 };

// GLOBAL: XW 0x4C51E8
uint8_t g_orderLeaderPlanId[PAI_ORDER_PLAN_COUNT] = { 0,  53, 5,  1,  7,  3,  30, 31, 11, 10, 9,  47, 50, 40,
													  41, 42, 43, 44, 25, 26, 12, 13, 14, 27, 28, 29, 60, 61,
													  62, 63, 64, 65, 69, 0,  0,  0,  0,  0,  0,  0 };

// GLOBAL: XW 0x4C5210
uint8_t g_orderFollowerPlanId[PAI_ORDER_PLAN_COUNT] = { 0,  54, 6,  2,  8,  4,  35, 36, 20, 20,
														20, 49, 50, 40, 41, 42, 43, 44, 20, 20,
														20, 20, 20, 20, 20, 20, 60, 68, 68, 63,
														64, 65, 69, 0,  0,  0,  0,  0,  0,  0 };

// GLOBAL: XW 0x4C8EE8
XwManeuverFunction g_orderTable[PAI_ORDER_PREDICATE_COUNT] = {
	Shared_ReturnZero,
	paiorder_updatecourseorder,
	paiorder_underattackorder,
	paiorder_stillattackorder,
	paiorder_IsAimDistanceAbove131072,
	paiorder_flyhomeorder,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	paiorder_CheckDamageOrTargetSentinel,
	Shared_ReturnZero,
	Shared_ReturnZero,
	paifight_SelectTurretTargets,
	paifight_scanfortargetorder,
	paiorder_waitrunorder,
	paiorder_breakofforder,
	paifight_fightershootorder,
	paiorder_leaderdeadorder,
	paifight_coverleaderorder,
	paifight_followleadatkorder,
	paiorder_abortatkorder,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	paifight_checkescortorder,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	paiorder_ontailorder,
	Shared_ReturnZero,
	Shared_ReturnOne,
	paifight_SelectLowAltitudeTarget,
	paiorder_IsObjectKind4,
	paiorder_leadergohomeorder,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	Shared_ReturnZero,
	paifight_AdvanceWaypointIfProgressComplete,
	Shared_ReturnZero,
	Shared_ReturnZero,
	paifight_AdvanceWaypoint,
	Shared_ReturnZero,
	paiorder_hyperspaceorder,
	paiorder_enterhangarorder,
	paiorder_mothershiporder,
	paifight_escorttargetorder,
	paiorder_lookforcrafttoboardorder,
	paiorder_abortboardorder,
	paiorder_returnboardorder,
	paiorder_awaitboardorder,
	paiorder_makedisabledorder,
	paiorder_returnboardorder_2,
	paiorder_rocketsonboardorder,
	Shared_ReturnZero,
	paiorder_avoidhitorder,
	paiorder_waitforallreturnorder,
	paiorder_waitforallcreateorder,
	paiorder_TargetGroupsBoardingAndArrivalComplete,
	paiorder_TargetGroupsAreExhausted,
	paifight_OrderHasNoRemainingTargets,
	paiorder_evasiveorder,
	paiorder_targetfromplayerorder,
	paiorder_avoidstarshiporder,
	paiorder_IsFinalWaypointTarget,
	paiorder_checkhyperorder,
	paiorder_TargetGroupsHaveArrived
};

// GLOBAL: XW 0x4C9318
uint8_t g_paiPlan0[3] = { 128, 0, 0 };

// GLOBAL: XW 0x4C9320
uint8_t g_paiPlan1[9] = { 252, 6, 1, 5, 20, 51, 48, 51, 0 };

// GLOBAL: XW 0x4C9330
uint8_t g_paiPlan2[11] = { 252, 10, 1, 6, 17, 5, 20, 51, 35, 52, 0 };

// GLOBAL: XW 0x4C9340
uint8_t g_paiPlan3[15] = { 252, 6, 66, 5, 1, 5, 2, 0, 3, 5, 20, 53, 48, 53, 0 };

// GLOBAL: XW 0x4C9350
uint8_t g_paiPlan4[17] = { 252, 10, 66, 6, 1, 6, 17, 5, 2, 0, 3, 6, 20, 53, 35, 54, 0 };

// GLOBAL: XW 0x4C9368
uint8_t g_paiPlan9[19] = { 252, 6, 66, 9, 1, 9, 13, 16, 63, 53, 2, 0, 3, 9, 20, 53, 64, 19, 0 };

// GLOBAL: XW 0x4C9380
uint8_t g_paiPlan16[21] = { 255, 11, 56, 18, 1, 16, 30, 0, 15, 9, 14, 17, 16, 0, 20, 53, 64, 19, 65, 9, 0 };

// GLOBAL: XW 0x4C9398
uint8_t g_paiPlan17[19] = { 255, 12, 66, 17, 16, 0, 1, 17, 58, 0, 15, 9, 20, 53, 64, 19, 65, 9, 0 };

// GLOBAL: XW 0x4C93B0
uint8_t g_paiPlan18[19] = { 255, 23, 66, 18, 16, 0, 1, 18, 58, 0, 15, 9, 20, 53, 64, 19, 65, 9, 0 };

// GLOBAL: XW 0x4C93C8
uint8_t g_paiPlan19[5] = { 255, 27, 1, 9, 0 };

// GLOBAL: XW 0x4C93D0
uint8_t g_paiPlan20[23] = { 252, 10, 66, 20, 1,  20, 17, 9,  18, 21, 19, 21,
							2,   0,  3,  20, 20, 53, 35, 54, 64, 24, 0 };

// GLOBAL: XW 0x4C93E8
uint8_t g_paiPlan21[21] = { 255, 11, 56, 18, 1, 21, 15, 20, 14, 22, 2, 0, 3, 20, 20, 53, 64, 24, 65, 20, 0 };

// GLOBAL: XW 0x4C9400
uint8_t g_paiPlan22[17] = { 255, 12, 66, 22, 1, 20, 15, 20, 16, 0, 20, 53, 64, 24, 65, 20, 0 };

// GLOBAL: XW 0x4C9418
uint8_t g_paiPlan23[17] = { 255, 23, 66, 23, 1, 20, 30, 0, 15, 20, 20, 53, 64, 24, 65, 20, 0 };

// GLOBAL: XW 0x4C9430
uint8_t g_paiPlan24[5] = { 255, 27, 1, 20, 0 };

// GLOBAL: XW 0x4C9438
uint8_t g_paiPlan30[19] = { 255, 17, 66, 30, 24, 53, 1, 30, 49, 32, 2, 0, 3, 30, 20, 53, 64, 34, 0 };

// GLOBAL: XW 0x4C9450
uint8_t g_paiPlan32[19] = { 255, 11, 1, 32, 30, 0, 15, 30, 14, 33, 16, 0, 20, 53, 64, 34, 65, 30, 0 };

// GLOBAL: XW 0x4C9468
uint8_t g_paiPlan33[17] = { 255, 12, 66, 33, 16, 0, 1, 33, 15, 30, 20, 53, 64, 34, 65, 30, 0 };

// GLOBAL: XW 0x4C9480
uint8_t g_paiPlan34[5] = { 255, 27, 1, 30, 0 };

// GLOBAL: XW 0x4C9488
uint8_t g_paiPlan35[23] = { 252, 10, 66, 35, 1,  35, 17, 30, 18, 37, 19, 37,
							2,   0,  3,  35, 20, 53, 35, 54, 64, 39, 0 };

// GLOBAL: XW 0x4C94A0
uint8_t g_paiPlan37[19] = { 255, 11, 1, 37, 15, 35, 14, 38, 2, 0, 3, 35, 20, 53, 64, 39, 65, 35, 0 };

// GLOBAL: XW 0x4C94B8
uint8_t g_paiPlan38[17] = { 255, 12, 66, 38, 1, 35, 15, 35, 16, 0, 20, 53, 64, 39, 65, 35, 0 };

// GLOBAL: XW 0x4C94D0
uint8_t g_paiPlan39[5] = { 255, 27, 1, 35, 0 };

// GLOBAL: XW 0x4C94D8
uint8_t g_paiPlan40[9] = { 255, 25, 50, 45, 62, 53, 58, 0, 0 };

// GLOBAL: XW 0x4C94E8
uint8_t g_paiPlan45[7] = { 255, 18, 1, 53, 51, 46, 0 };

// GLOBAL: XW 0x4C94F0
uint8_t g_paiPlan46[11] = { 255, 7, 1, 0, 52, 40, 50, 45, 58, 0, 0 };

// GLOBAL: XW 0x4C9500
uint8_t g_paiPlan47[11] = { 252, 6, 66, 47, 55, 48, 1, 0, 58, 0, 0 };

// GLOBAL: XW 0x4C9510
uint8_t g_paiPlan48[7] = { 255, 19, 1, 0, 53, 53, 0 };

// GLOBAL: XW 0x4C9518
uint8_t g_paiPlan49[9] = { 252, 10, 55, 48, 1, 0, 58, 0, 0 };

// GLOBAL: XW 0x4C9528
uint8_t g_paiPlan50[7] = { 255, 19, 54, 0, 53, 53, 0 };

// GLOBAL: XW 0x4C9530
uint8_t g_paiPlan53[13] = { 255, 5, 46, 57, 1, 53, 5, 55, 58, 0, 3, 53, 0 };

// GLOBAL: XW 0x4C9540
uint8_t g_paiPlan54[13] = { 255, 10, 46, 57, 1, 54, 17, 53, 58, 0, 3, 54, 0 };

// GLOBAL: XW 0x4C9550
uint8_t g_paiFlyHomePlan[9] = { 255, 5, 46, 57, 1, 51, 5, 55, 0 };

// GLOBAL: XW 0x4C9560
uint8_t g_paiPlan52[9] = { 255, 10, 46, 57, 1, 54, 17, 51, 0 };

// GLOBAL: XW 0x4C9570
uint8_t g_paiEnterHangarPlan[7] = { 255, 20, 47, 0, 1, 55, 0 };

// GLOBAL: XW 0x4C9578
XwAiSingleTransitionPlan g_paiOutOfHangarPlan = { 255, 26, { 1, 0 }, 0 };

// GLOBAL: XW 0x4C9580
uint8_t g_paiPlan57[5] = { 254, 21, 1, 0, 0 };

// GLOBAL: XW 0x4C9588
XwAiSingleTransitionPlan g_paiOutOfHyperspacePlan = { 255, 22, { 1, 0 }, 0 };

// GLOBAL: XW 0x4C9590
uint8_t g_paiPlan59[9] = { 254, 21, 68, 60, 1, 0, 12, 0, 0 };

// GLOBAL: XW 0x4C95A0
uint8_t g_paiPlan60[5] = { 255, 25, 12, 0, 0 };

// GLOBAL: XW 0x4C95A8
uint8_t g_paiPlan61[9] = { 252, 6, 1, 0, 12, 0, 48, 59, 0 };

// GLOBAL: XW 0x4C95B8
uint8_t g_paiPlan68[11] = { 252, 10, 1, 0, 12, 0, 48, 59, 17, 61, 0 };

// GLOBAL: XW 0x4C95C8
uint8_t g_paiPlan63[7] = { 255, 25, 12, 0, 59, 59, 0 };

// GLOBAL: XW 0x4C95D0
uint8_t g_paiPlan64[7] = { 255, 25, 12, 0, 60, 59, 0 };

// GLOBAL: XW 0x4C95D8
uint8_t g_paiPlan65[7] = { 255, 25, 12, 0, 61, 59, 0 };

// GLOBAL: XW 0x4C95E0
uint8_t g_paiPlan69[7] = { 255, 25, 12, 0, 69, 59, 0 };

// GLOBAL: XW 0x4C95E8
uint8_t g_paiPlan66[7] = { 255, 25, 2, 0, 3, 66, 0 };

// GLOBAL: XW 0x4C95F0
uint8_t g_paiPlan67[5] = { 255, 25, 12, 0, 0 };

// GLOBAL: XW 0x4C95F8
uint8_t* g_planDataPtrs[PAI_PLAN_COUNT] = { g_paiPlan0,
											g_paiPlan1,
											g_paiPlan2,
											g_paiPlan3,
											g_paiPlan4,
											g_paiPlan1,
											g_paiPlan2,
											g_paiPlan3,
											g_paiPlan4,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan16,
											g_paiPlan17,
											g_paiPlan18,
											g_paiPlan19,
											g_paiPlan20,
											g_paiPlan21,
											g_paiPlan22,
											g_paiPlan23,
											g_paiPlan24,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan9,
											g_paiPlan30,
											g_paiPlan30,
											g_paiPlan32,
											g_paiPlan33,
											g_paiPlan34,
											g_paiPlan35,
											g_paiPlan35,
											g_paiPlan37,
											g_paiPlan38,
											g_paiPlan39,
											g_paiPlan40,
											g_paiPlan40,
											g_paiPlan40,
											g_paiPlan40,
											g_paiPlan40,
											g_paiPlan45,
											g_paiPlan46,
											g_paiPlan47,
											g_paiPlan48,
											g_paiPlan49,
											g_paiPlan50,
											g_paiFlyHomePlan,
											g_paiPlan52,
											g_paiPlan53,
											g_paiPlan54,
											g_paiEnterHangarPlan,
											(uint8_t*)&g_paiOutOfHangarPlan,
											g_paiPlan57,
											(uint8_t*)&g_paiOutOfHyperspacePlan,
											g_paiPlan59,
											g_paiPlan60,
											g_paiPlan61,
											g_paiPlan61,
											g_paiPlan63,
											g_paiPlan64,
											g_paiPlan65,
											g_paiPlan66,
											g_paiPlan67,
											g_paiPlan68,
											g_paiPlan69 };

// GLOBAL: XW 0x62AF18
unsigned int g_targetRangeScore = 0;

// GLOBAL: XW 0x62B4F6
uint16_t g_paiLeaderObjectIndex = 0;

// GLOBAL: XW 0x62B910
struct CraftData* g_paiLeaderCraft = NULL;

// GLOBAL: XW 0x62BC90
uint16_t g_paiObjectIndex = 0;

// GLOBAL: XW 0x63ADC8
uint8_t* g_paiPlanCursor = NULL;

// GLOBAL: XW 0x63ADCC
uint8_t g_paiSkillTier = 0;

// GLOBAL: XW 0x63ADCD
uint8_t g_paiInitialManeuverId = 0;

// GLOBAL: XW 0x62BAE8
int g_rotatedY = 0;

// GLOBAL: XW 0x62BAEC
int g_rotatedX = 0;

// GLOBAL: XW 0x62BAF4
int g_rotatedZ = 0;

// FUNCTION: XW 0x412810
void pai_initplan(uint16_t objectIdx) {
	uint8_t* plan;
	int16_t targetSelector;
	g_paiObjectIndex = objectIdx;
	plan = g_planDataPtrs[g_curCraft->aiCurrentPlanId];
	targetSelector = plan[PAI_PLAN_TARGET_BYTE];
	if (targetSelector != PAI_PLAN_PRESERVE_VALUE) {
		if (targetSelector == PAI_PLAN_TARGET_ENABLED_POINT_4) {
			if (g_missionFlightGroups[g_curCraft->flightGroupIndex].waypointEnabled[PAI_WAYPOINT_4] != 0)
				g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE + PAI_WAYPOINT_4;
			else
				g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
		} else if (targetSelector == PAI_PLAN_TARGET_ENABLED_FINAL) {
			if (g_missionFlightGroups[g_curCraft->flightGroupIndex].waypointEnabled[PAI_WAYPOINT_FINAL] != 0)
				g_curCraft->aiTargetRef = PAI_TARGET_FINAL_WAYPOINT;
			else
				g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
		} else if (targetSelector == PAI_PLAN_TARGET_POINT_4) {
			g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE + PAI_WAYPOINT_4;
		} else if (targetSelector == PAI_PLAN_TARGET_POINT_5) {
			g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE + PAI_WAYPOINT_5;
		} else {
			if (g_missionFlightGroups[g_curCraft->flightGroupIndex]
					.waypointEnabled[g_curCraft->aiWaypointIndex] != 0)
				g_curCraft->aiTargetRef = g_curCraft->aiWaypointIndex + PAI_TARGET_WAYPOINT_BASE;
			else
				g_curCraft->aiTargetRef = PAI_TARGET_WAYPOINT_BASE;
		}
		if (g_curCraft->aiTargetRef != PAI_TARGET_NONE)
			pai_settarget();
	}
	g_curCraft->aiManeuverAuxTimerTicks = 0;
	if (plan[PAI_PLAN_MANEUVER_BYTE] != PAI_PLAN_PRESERVE_VALUE) {
		g_curCraft->aiManeuverId = plan[PAI_PLAN_MANEUVER_BYTE];
		paiman_initmaneuver();
	}
	g_curCraft->lastAttackerObjIdx = PAI_TARGET_NONE;
	g_curCraft->aiThinkTimerTicks = g_curCraft->aiThinkIntervalTicks;
}

// FUNCTION: XW 0x412930
void pai_updateplaneai(void) {
	uint16_t objectIndex;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[objectIndex].familyId == XW_OBJECT_FAMILY_CRAFT) {
			uint8_t craftKind;
			g_curCraft = g_objectTable[objectIndex].instanceData;
			craftKind = g_curCraft->objectKind;
			if (craftKind != XW_CRAFT_OBJECT_KIND_3 && craftKind != XW_CRAFT_OBJECT_KIND_4 &&
				g_curCraft->aiThinkTimerTicks == 0) {
				g_paiObjectIndex = objectIndex;
				g_paiLeaderObjectIndex = g_curCraft->aiLeaderObjectIndex;
#ifdef XW_MODERN
				/* Group leaders have no leader object, but still run their AI plans. */
				g_paiLeaderCraft = g_paiLeaderObjectIndex < XW_CRAFT_OBJECT_COUNT
									   ? g_objectTable[g_paiLeaderObjectIndex].instanceData
									   : NULL;
#else
				g_paiLeaderCraft = g_objectTable[g_paiLeaderObjectIndex].instanceData;
#endif
				pai_updatecraftplan();
				g_curCraft->aiThinkTimerTicks = g_curCraft->aiThinkIntervalTicks;
			}
		}
	}
}

// FUNCTION: XW 0x4129B0
void pai_updatecraftplan(void) {
	uint16_t objectIndex = g_paiObjectIndex;
	uint8_t* plan;
	size_t orderOffset;
	g_paiSkillTier = (uint8_t)pai_getprof(g_curCraft->aiSkillQ16);
	plan = g_planDataPtrs[g_curCraft->aiCurrentPlanId];
	g_paiPlanCursor = &plan[PAI_PLAN_MANEUVER_BYTE];
	g_paiInitialManeuverId = plan[PAI_PLAN_MANEUVER_BYTE];
	g_paiPlanCursor = &plan[PAI_PLAN_FIRST_ORDER_BYTE];
	if (objectIndex == g_playerFlightState.objectIndex) {
		int16_t orderClass = g_curCraft->aiOrderPlanId;
		if (orderClass == PAI_PLAN_ESCORT_30 || orderClass == PAI_PLAN_ESCORT_31) {
			paifight_checkescortorder();
		}
	}
	for (orderOffset = PAI_PLAN_FIRST_ORDER_BYTE;; orderOffset += sizeof(XwAiPlanTransition)) {
		uint8_t orderId = plan[orderOffset];
		g_paiPlanCursor = &plan[orderOffset + 1];
		if (orderId == 0) {
			return;
		}
		if (g_orderTable[orderId]() != 0 && *g_paiPlanCursor != 0) {
			g_curCraft->aiCurrentPlanId = *g_paiPlanCursor;
			pai_initplan(objectIndex);
			return;
		}
		g_paiPlanCursor = &plan[orderOffset + sizeof(XwAiPlanTransition)];
	}
}

// FUNCTION: XW 0x412A90
int16_t pai_getprof(uint16_t skillQ16) {
	if (skillQ16 < PAI_SKILL_MIDDLE_THRESHOLD) {
		return PAI_PROFICIENCY_LOW;
	}
	if (skillQ16 < PAI_SKILL_HIGH_THRESHOLD) {
		return PAI_PROFICIENCY_MIDDLE;
	}
	return PAI_PROFICIENCY_HIGH;
}

// FUNCTION: XW 0x413450
int16_t pai_checktargetforattack(uint16_t attackerObjectIndex, uint16_t targetObjectIndex,
								 int16_t extendRange) {
	if (pai_worthytarget(targetObjectIndex) != 0) {
		CraftData* attacker = g_objectTable[attackerObjectIndex].instanceData;
		int maximumRange = (uint16_t)math2_fraction(PAI_ATTACK_SKILL_RANGE, attacker->aiSkillQ16);
		int16_t withinRange;
		maximumRange += PAI_ATTACK_BASE_RANGE;
		if (extendRange != 0) {
			maximumRange += (uint16_t)math2_fraction(maximumRange, PAI_ATTACK_EXTENDED_RANGE_Q16);
		}
		withinRange = pai_roughproximitycheck(targetObjectIndex, maximumRange << PAI_ATTACK_RANGE_SHIFT);
		if (withinRange == 1) {
			return withinRange;
		}
	}
	return 0;
}

// FUNCTION: XW 0x4134E0
int16_t pai_worthytarget(uint16_t objectIndex) {
	if (objectIndex == PAI_TARGET_NONE) {
		return 0;
	}
	if (objectIndex < XW_MISSION_OBJECT_REF_BASE) {
		if (g_objectTable[objectIndex].objectType == XW_OBJ_NONE) {
			return 0;
		}
		if (objectIndex < XW_CRAFT_OBJECT_COUNT) {
			CraftData* craft = g_objectTable[objectIndex].instanceData;
			int16_t objectKind;
			if (craft->aiManeuverId == PAIORDER_MANEUVER_INTO_HYPERSPACE && craft->aiManeuverPhase != 0) {
				return 0;
			}
			objectKind = craft->objectKind;
			if (objectKind == XW_CRAFT_OBJECT_KIND_1 || objectKind == XW_CRAFT_OBJECT_KIND_3 ||
				objectKind == XW_CRAFT_OBJECT_KIND_4) {
				return 0;
			}
			if (objectIndex == g_playerFlightState.objectIndex) {
				return 1;
			}
			return craft->hullDamage < pai_getcraftdoomedlevel(objectIndex);
		}
		return 1;
	}
	return g_missionObjects[objectIndex - XW_MISSION_OBJECT_REF_BASE].objectType != XW_OBJ_NONE;
}

// FUNCTION: XW 0x4145F0
int16_t pai_roughproximitycheck(uint16_t targetObjectIndex, int maximumRange) {
#ifdef XW_MODERN
	int dx = (int32_t)((uint32_t)g_objectTable[g_paiObjectIndex].worldX -
					   (uint32_t)g_objectTable[targetObjectIndex].worldX);
	int dy = (int32_t)((uint32_t)g_objectTable[g_paiObjectIndex].worldY -
					   (uint32_t)g_objectTable[targetObjectIndex].worldY);
	int dz = (int32_t)((uint32_t)g_objectTable[g_paiObjectIndex].worldZ -
					   (uint32_t)g_objectTable[targetObjectIndex].worldZ);
#else
	int dx = g_objectTable[g_paiObjectIndex].worldX - g_objectTable[targetObjectIndex].worldX;
	int dy = g_objectTable[g_paiObjectIndex].worldY - g_objectTable[targetObjectIndex].worldY;
	int dz = g_objectTable[g_paiObjectIndex].worldZ - g_objectTable[targetObjectIndex].worldZ;
#endif
	if (dx < 0) {
#ifdef XW_MODERN
		dx = (int32_t)(0u - (uint32_t)dx);
#else
		dx = -dx;
#endif
	}
	if (dy < 0) {
#ifdef XW_MODERN
		dy = (int32_t)(0u - (uint32_t)dy);
#else
		dy = -dy;
#endif
	}
	if (dz < 0) {
#ifdef XW_MODERN
		dz = (int32_t)(0u - (uint32_t)dz);
#else
		dz = -dz;
#endif
	}
	if (dx > dy) {
#ifdef XW_MODERN
		dx = (int32_t)((uint32_t)dx + (uint32_t)(dy >> 1));
#else
		dx = dx + (dy >> 1);
#endif
	} else {
#ifdef XW_MODERN
		dx = (int32_t)((uint32_t)dy + (uint32_t)(dx >> 1));
#else
		dx = dy + (dx >> 1);
#endif
	}
	if (dx > dz) {
		dz >>= 1;
	} else {
		dx >>= 1;
	}
#ifdef XW_MODERN
	dx = (int32_t)((uint32_t)dx + (uint32_t)dz);
#else
	dx += dz;
#endif
	g_targetRangeScore = dx;
	return maximumRange > dx;
}

// FUNCTION: XW 0x414690
void pai_settarget(void) {
	create_getworldposition(g_curCraft->aiTargetRef, g_curCraft->flightGroupIndex);
	g_curCraft->aiAimPointX = g_resolvedWorldX;
	g_curCraft->aiAimPointY = g_resolvedWorldY;
	g_curCraft->aiAimPointZ = g_resolvedWorldZ;
}

// FUNCTION: XW 0x4146E0
void pai_distancebetween(uint16_t fromRef, uint16_t toRef) {
	int toX;
	int toY;
	int toZ;
	create_getworldposition(toRef, 0);
	toX = g_resolvedWorldX;
	toY = g_resolvedWorldY;
	toZ = g_resolvedWorldZ;
	create_getworldposition(fromRef, 0);
	trig2_ctop((int)((unsigned int)toX - (unsigned int)g_resolvedWorldX),
			   (int)((unsigned int)toY - (unsigned int)g_resolvedWorldY),
			   (int)((unsigned int)toZ - (unsigned int)g_resolvedWorldZ));
}

// FUNCTION: XW 0x414740
void pai_roughdistancebetween(uint16_t fromRef, uint16_t toRef) {
	int dx;
	int dy;
	int dz;
	int approxXY;

	create_getworldposition(fromRef, 0);
	dx = g_resolvedWorldX;
	dy = g_resolvedWorldY;
	dz = g_resolvedWorldZ;
	create_getworldposition(toRef, 0);
#ifdef XW_MODERN
	dx = (int32_t)((uint32_t)dx - (uint32_t)g_resolvedWorldX);
	dy = (int32_t)((uint32_t)dy - (uint32_t)g_resolvedWorldY);
	dz = (int32_t)((uint32_t)dz - (uint32_t)g_resolvedWorldZ);
#else
	dx -= g_resolvedWorldX;
	dy -= g_resolvedWorldY;
	dz -= g_resolvedWorldZ;
#endif
	if (dx < 0) {
#ifdef XW_MODERN
		dx = (int32_t)(0u - (uint32_t)dx);
#else
		dx = -dx;
#endif
	}
	if (dy < 0) {
#ifdef XW_MODERN
		dy = (int32_t)(0u - (uint32_t)dy);
#else
		dy = -dy;
#endif
	}
	if (dz < 0) {
#ifdef XW_MODERN
		dz = (int32_t)(0u - (uint32_t)dz);
#else
		dz = -dz;
#endif
	}
	if (dx > dy) {
#ifdef XW_MODERN
		approxXY = (int32_t)((uint32_t)dx + (uint32_t)(dy >> 1));
#else
		approxXY = (dy >> 1) + dx;
#endif
	} else {
#ifdef XW_MODERN
		approxXY = (int32_t)((uint32_t)dy + (uint32_t)(dx >> 1));
#else
		approxXY = (dx >> 1) + dy;
#endif
	}
	if (approxXY > dz) {
#ifdef XW_MODERN
		g_targetRangeScore = (uint32_t)approxXY + (uint32_t)(dz >> 1);
#else
		g_targetRangeScore = approxXY + (dz >> 1);
#endif
	} else {
#ifdef XW_MODERN
		g_targetRangeScore = (uint32_t)(approxXY >> 1) + (uint32_t)dz;
#else
		g_targetRangeScore = (approxXY >> 1) + dz;
#endif
	}
}

// FUNCTION: XW 0x4147D0
void pai_calcrotatedpoint(struct ObjectRecord* object, int16_t localSide, int16_t localUp,
						  int16_t localForward) {
	int side = localSide;
	int up = localUp;
	int forward = localForward;
	if (object->orientMatrixDirty != 0) {
		fview_calcrotatemove(object->pitch, object->yaw, object);
		fview_calcrotateorient(object->roll, 0, object);
	}
	g_rotatedX = (int32_t)(((int64_t)object->cachedSideX * side) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedX += (int32_t)(((int64_t)object->cachedUpX * up) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedX += (int32_t)(((int64_t)object->cachedForwardX * forward) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedY = (int32_t)(((int64_t)object->cachedSideY * side) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedY += (int32_t)(((int64_t)object->cachedUpY * up) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedY += (int32_t)(((int64_t)object->cachedForwardY * forward) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedZ = (int32_t)(((int64_t)object->cachedSideZ * side) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedZ += (int32_t)(((int64_t)object->cachedUpZ * up) >> FVIEW_MATRIX_FRACTION_BITS);
	g_rotatedZ += (int32_t)(((int64_t)object->cachedForwardZ * forward) >> FVIEW_MATRIX_FRACTION_BITS);
}

// FUNCTION: XW 0x414970
uint16_t pai_getcraftdoomedlevel(uint16_t objectIndex) {
	CraftData* craft = g_objectTable[objectIndex].instanceData;
	return craft->hullMax;
}

// FUNCTION: XW 0x414990
void pai_targetdistance(void) {
	trig2_ctop(
		(int)((unsigned int)g_curCraft->aiAimPointX - (unsigned int)g_objectTable[g_paiObjectIndex].worldX),
		(int)((unsigned int)g_curCraft->aiAimPointY - (unsigned int)g_objectTable[g_paiObjectIndex].worldY),
		(int)((unsigned int)g_curCraft->aiAimPointZ - (unsigned int)g_objectTable[g_paiObjectIndex].worldZ));
}

// FUNCTION: XW 0x4149E0
void pai_RotateLocalVectorToWorldScratch(struct ObjectRecord* object, int localSide, int localUp,
										 int localForward) {
	if (object->orientMatrixDirty != 0) {
		fview_calcrotatemove(object->pitch, object->yaw, object);
		fview_calcrotateorient(object->roll, 0, object);
	}
	g_rotatedX = (int32_t)(((int64_t)object->cachedSideX * localSide) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
	g_rotatedX = (int32_t)((uint32_t)g_rotatedX +
						   (uint32_t)(((int64_t)object->cachedUpX * localUp) >> FVIEW_MATRIX_FRACTION_BITS));
#else
	g_rotatedX += (int32_t)(((int64_t)object->cachedUpX * localUp) >> FVIEW_MATRIX_FRACTION_BITS);
#endif
#ifdef XW_MODERN
	g_rotatedX =
		(int32_t)((uint32_t)g_rotatedX +
				  (uint32_t)(((int64_t)object->cachedForwardX * localForward) >> FVIEW_MATRIX_FRACTION_BITS));
#else
	g_rotatedX += (int32_t)(((int64_t)object->cachedForwardX * localForward) >> FVIEW_MATRIX_FRACTION_BITS);
#endif
	g_rotatedY = (int32_t)(((int64_t)object->cachedSideY * localSide) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
	g_rotatedY = (int32_t)((uint32_t)g_rotatedY +
						   (uint32_t)(((int64_t)object->cachedUpY * localUp) >> FVIEW_MATRIX_FRACTION_BITS));
#else
	g_rotatedY += (int32_t)(((int64_t)object->cachedUpY * localUp) >> FVIEW_MATRIX_FRACTION_BITS);
#endif
#ifdef XW_MODERN
	g_rotatedY =
		(int32_t)((uint32_t)g_rotatedY +
				  (uint32_t)(((int64_t)object->cachedForwardY * localForward) >> FVIEW_MATRIX_FRACTION_BITS));
#else
	g_rotatedY += (int32_t)(((int64_t)object->cachedForwardY * localForward) >> FVIEW_MATRIX_FRACTION_BITS);
#endif
	g_rotatedZ = (int32_t)(((int64_t)object->cachedSideZ * localSide) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
	g_rotatedZ = (int32_t)((uint32_t)g_rotatedZ +
						   (uint32_t)(((int64_t)object->cachedUpZ * localUp) >> FVIEW_MATRIX_FRACTION_BITS));
#else
	g_rotatedZ += (int32_t)(((int64_t)object->cachedUpZ * localUp) >> FVIEW_MATRIX_FRACTION_BITS);
#endif
#ifdef XW_MODERN
	g_rotatedZ =
		(int32_t)((uint32_t)g_rotatedZ +
				  (uint32_t)(((int64_t)object->cachedForwardZ * localForward) >> FVIEW_MATRIX_FRACTION_BITS));
#else
	g_rotatedZ += (int32_t)(((int64_t)object->cachedForwardZ * localForward) >> FVIEW_MATRIX_FRACTION_BITS);
#endif
}
