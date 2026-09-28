#ifndef XW_FLIGHT_AI_PAI_H
#define XW_FLIGHT_AI_PAI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { PAI_SKILL_LEVEL_COUNT = 8 };

extern uint16_t g_aiSkillValueQ16ByLevel[PAI_SKILL_LEVEL_COUNT];
extern uint16_t g_aiThinkIntervalBySkill[PAI_SKILL_LEVEL_COUNT];

extern int g_rotatedY;
extern int g_rotatedX;
extern int g_rotatedZ;

struct CraftData;
struct ObjectRecord;

enum {
	PAI_PLAN_RETURN_HOME = 53,
	PAI_PLAN_INTO_HYPERSPACE = 57,
	PAI_PLAN_STARSHIP_RETURN_HOME = 59,
	PAI_PLAN_FLY_HOME = 51,
	PAI_PLAN_55 = 55,
	PAI_PLAN_66 = 66,
	PAI_PLAN_67 = 67
};

enum {
	PAI_PLAN_BOARDING_STAGE_FIRST = 40,
	PAI_PLAN_BOARDING_STAGE_LAST = 46,
	PAI_PLAN_OUT_OF_HANGAR = 56,
	PAI_PLAN_OUT_OF_HYPERSPACE = 58
};

enum { PAI_PLAN_DISABLE_FIRST = 25, PAI_PLAN_DISABLE_LAST = 29 };

enum {
	PAI_PLAN_FIGHTER_OR_TRANSPORT_TARGETS = 9,
	PAI_PLAN_FIGHTER_TARGETS = 11,
	PAI_PLAN_TRANSPORT_TARGETS = 12,
	PAI_PLAN_FREIGHTER_TARGETS = 13,
	PAI_PLAN_STARSHIP_TARGETS = 14,
	PAI_PLAN_DISABLE_FIGHTER_OR_TRANSPORT_TARGETS = 26,
	PAI_PLAN_DISABLE_TRANSPORT_TARGETS = 27,
	PAI_PLAN_DISABLE_FREIGHTER_TARGETS = 28,
	PAI_PLAN_DISABLE_STARSHIP_TARGETS = 29
};

enum { PAI_ORDER_PLAN_COUNT = 40, PAI_PLAN_COUNT = 70 };

enum {
	PAI_PLAN_TARGET_BYTE = 0,
	PAI_PLAN_MANEUVER_BYTE = 1,
	PAI_PLAN_FIRST_ORDER_BYTE = 2,
	PAI_PLAN_PRESERVE_VALUE = 255
};

enum { PAI_ORDER_PREDICATE_COUNT = 70 };

enum {
	PAI_PLAN_TARGET_POINT_4 = 249,
	PAI_PLAN_TARGET_POINT_5 = 250,
	PAI_PLAN_TARGET_ENABLED_POINT_4 = 253,
	PAI_PLAN_TARGET_ENABLED_FINAL = 254,
	PAI_WAYPOINT_4 = 4,
	PAI_WAYPOINT_5 = 5
};

enum {
	PAI_ATTACK_BASE_RANGE = 2560,
	PAI_ATTACK_SKILL_RANGE = 1280,
	PAI_ATTACK_EXTENDED_RANGE_Q16 = 0x5555,
	PAI_ATTACK_RANGE_SHIFT = 8
};

enum { PAI_PLAN_1 = 1, PAI_PLAN_3 = 3, PAI_PLAN_61 = 61 };

enum { PAI_PLAN_ESCORT_30 = 30, PAI_PLAN_ESCORT_31 = 31 };

enum { PAI_WAYPOINT_FIRST = 1, PAI_WAYPOINT_LAST = 3, PAI_WAYPOINT_FINAL = 6 };

enum {
	PAI_TARGET_EVASIVE = 251,
	PAI_TARGET_DAMAGE_SENTINEL = 254,
	PAI_TARGET_NONE = 255,
	PAI_TARGET_WAYPOINT_BASE = 0x8000,
	PAI_TARGET_FINAL_WAYPOINT = 0x8006
};

enum {
	PAI_PROFICIENCY_LOW = 0,
	PAI_PROFICIENCY_MIDDLE = 1,
	PAI_PROFICIENCY_HIGH = 2,
	PAI_PROFICIENCY_COUNT = 3,
	PAI_SKILL_MIDDLE_THRESHOLD = 0x8000,
	PAI_SKILL_HIGH_THRESHOLD = 0xC000
};

typedef struct XwAiPlanTransition XwAiPlanTransition;
typedef struct XwAiSingleTransitionPlan XwAiSingleTransitionPlan;

/* Original IDB size: 2 bytes. */
struct XwAiPlanTransition {
	/* IDB +0x0: Predicate index in g_orderTable. */
	uint8_t orderId;
	/* IDB +0x1: Plan selected if predicate succeeds; zero means no transition. Patched at runtime by exit
	 * maneuver callbacks. */
	uint8_t nextPlanId;
};

/* Original IDB size: 5 bytes. */
struct XwAiSingleTransitionPlan {
	/* IDB +0x0: First byte of packed plan stream; consumed when a plan is initialized. */
	uint8_t targetSelector;
	/* IDB +0x1: Maneuver selected on entry. */
	uint8_t maneuverId;
	/* IDB +0x2: One predicate/next-plan pair consumed by pai_updatecraftplan. */
	struct XwAiPlanTransition transition;
	/* IDB +0x4: Zero order byte terminates the predicate stream. */
	uint8_t endOrder;
};

extern uint8_t g_orderLeaderPlanId[PAI_ORDER_PLAN_COUNT];
extern uint8_t g_orderFollowerPlanId[PAI_ORDER_PLAN_COUNT];
extern uint8_t g_paiPlan0[3];
extern uint8_t g_paiPlan1[9];
extern uint8_t g_paiPlan2[11];
extern uint8_t g_paiPlan3[15];
extern uint8_t g_paiPlan4[17];
extern uint8_t g_paiPlan9[19];
extern uint8_t g_paiPlan16[21];
extern uint8_t g_paiPlan17[19];
extern uint8_t g_paiPlan18[19];
extern uint8_t g_paiPlan19[5];
extern uint8_t g_paiPlan20[23];
extern uint8_t g_paiPlan21[21];
extern uint8_t g_paiPlan22[17];
extern uint8_t g_paiPlan23[17];
extern uint8_t g_paiPlan24[5];
extern uint8_t g_paiPlan30[19];
extern uint8_t g_paiPlan32[19];
extern uint8_t g_paiPlan33[17];
extern uint8_t g_paiPlan34[5];
extern uint8_t g_paiPlan35[23];
extern uint8_t g_paiPlan37[19];
extern uint8_t g_paiPlan38[17];
extern uint8_t g_paiPlan39[5];
extern uint8_t g_paiPlan40[9];
extern uint8_t g_paiPlan45[7];
extern uint8_t g_paiPlan46[11];
extern uint8_t g_paiPlan47[11];
extern uint8_t g_paiPlan48[7];
extern uint8_t g_paiPlan49[9];
extern uint8_t g_paiPlan50[7];
extern uint8_t g_paiPlan53[13];
extern uint8_t g_paiPlan54[13];
extern uint8_t g_paiFlyHomePlan[9];
extern uint8_t g_paiPlan52[9];
extern uint8_t g_paiEnterHangarPlan[7];
extern XwAiSingleTransitionPlan g_paiOutOfHangarPlan;
extern uint8_t g_paiPlan57[5];
extern XwAiSingleTransitionPlan g_paiOutOfHyperspacePlan;
extern uint8_t g_paiPlan59[9];
extern uint8_t g_paiPlan60[5];
extern uint8_t g_paiPlan61[9];
extern uint8_t g_paiPlan68[11];
extern uint8_t g_paiPlan63[7];
extern uint8_t g_paiPlan64[7];
extern uint8_t g_paiPlan65[7];
extern uint8_t g_paiPlan69[7];
extern uint8_t g_paiPlan66[7];
extern uint8_t g_paiPlan67[5];
extern uint8_t* g_planDataPtrs[PAI_PLAN_COUNT];
extern unsigned int g_targetRangeScore;
extern uint16_t g_paiLeaderObjectIndex;
extern struct CraftData* g_paiLeaderCraft;
extern uint16_t g_paiObjectIndex;
extern uint8_t g_paiSkillTier;
extern uint8_t* g_paiPlanCursor;
extern uint8_t g_paiInitialManeuverId;

/* Declarations follow ascending original IDB address. */

/* 0x412810 */
void pai_initplan(uint16_t objectIdx);

/* 0x412930 */
void pai_updateplaneai(void);

/* 0x4129B0 */
void pai_updatecraftplan(void);

/* 0x412A90 */
int16_t pai_getprof(uint16_t skillQ16);

/* 0x413450 */
int16_t pai_checktargetforattack(uint16_t attackerObjectIndex, uint16_t targetObjectIndex,
								 int16_t extendRange);

/* 0x4134E0 */
int16_t pai_worthytarget(uint16_t objectIndex);

/* 0x4145F0 */
int16_t pai_roughproximitycheck(uint16_t targetObjectIndex, int maximumRange);

/* 0x414690 */
void pai_settarget(void);

/* 0x4146E0 */
void pai_distancebetween(uint16_t fromRef, uint16_t toRef);

/* 0x414740 */
void pai_roughdistancebetween(uint16_t fromRef, uint16_t toRef);

/* 0x4147D0 */
void pai_calcrotatedpoint(struct ObjectRecord* object, int16_t localSide, int16_t localUp,
						  int16_t localForward);

/* 0x414970 */
uint16_t pai_getcraftdoomedlevel(uint16_t objectIndex);

/* 0x414990 */
void pai_targetdistance(void);

/* 0x4149E0 */
void pai_RotateLocalVectorToWorldScratch(struct ObjectRecord* object, int localSide, int localUp,
										 int localForward);

#ifdef __cplusplus
}
#endif

#endif
