#ifndef XW_FLIGHT_AI_PAIFIGHT_H
#define XW_FLIGHT_AI_PAIFIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/flight/ai/pai.h"

#include <stddef.h>
#include <stdint.h>

enum {
	PAIFIGHT_SHOOT_PARALLEL_ANGLE = 0x2000,
	PAIFIGHT_SHOOT_CROSSING_ANGLE = 0x5000,
	PAIFIGHT_SHOOT_PARALLEL_RANGE_REDUCTION = 0x4000,
	PAIFIGHT_SHOOT_CROSSING_RANGE_REDUCTION = 0x2000,
	PAIFIGHT_SHOOT_CANNON_ANGLE = 0x800,
	PAIFIGHT_SHOOT_ALL_RANGE = 0x2000,
	PAIFIGHT_SHOOT_PAIR_RANGE = 0x4000,
	PAIFIGHT_SHOOT_CAPITAL_LAUNCH_LIMIT = 2,
	PAIFIGHT_SHOOT_CAPITAL_INBOUND_LIMIT = 6,
	PAIFIGHT_SHOOT_CAPITAL_WARHEAD_RANGE = 203610,
	PAIFIGHT_SHOOT_SMALL_LAUNCH_LIMIT = 1,
	PAIFIGHT_SHOOT_SMALL_INBOUND_LIMIT = 2,
	PAIFIGHT_SHOOT_SMALL_WARHEAD_RANGE = 101805,
	PAIFIGHT_SHOOT_WARHEAD_ANGLE = 0x300,
	PAIFIGHT_SHOOT_LOCK_FRACTION = 0xC000,
	PAIFIGHT_SHOOT_LOCK_TICKS = 472,
	PAIFIGHT_SHOOT_FIRST_LAUNCHER = 1,
	PAIFIGHT_SHOOT_SECOND_LAUNCHER = 0x81
};

extern unsigned int g_aiFighterShootMaxRangeBySkill[PAI_PROFICIENCY_COUNT];
extern uint8_t g_aiFighterShootBurstCountBySkill[PAI_PROFICIENCY_COUNT];

enum {
	PAIFIGHT_TURRET_NO_TARGET = -1,
	PAIFIGHT_TURRET_CAPITAL_RANGE = 0x18000,
	PAIFIGHT_TURRET_OTHER_RANGE = 0x10000,
	PAIFIGHT_TURRET_GROUP_ORDER_FIRST = 60,
	PAIFIGHT_TURRET_GROUP_ORDER_LAST = 62
};

enum { PAIFIGHT_COVER_PLAYER_DISTANCE_LIMIT = 0x10000 };

enum { PAIFIGHT_CLOSE_ESCORT_RANGE_LIMIT = 0x10000, PAIFIGHT_ESCORT_RANGE_LIMIT = 0x40000 };

enum { PAIFIGHT_PLAYER_LEADER_RANGE_LIMIT = 0x50000 };

enum { PAIFIGHT_WAYPOINT_PROGRESS_REQUIRED = 2 };

enum { PAIFIGHT_LOW_ALTITUDE_LIMIT = 1280, PAIFIGHT_LOW_ALTITUDE_START_MASK = 15 };

enum {
	PAIFIGHT_STARSHIP_ATTACKER_LIMIT = 6,
	PAIFIGHT_FREIGHTER_ATTACKER_LIMIT = 4,
	PAIFIGHT_PLAYER_IFF_ATTACKER_LIMIT = 3,
	PAIFIGHT_OTHER_IFF_ATTACKER_LIMIT = 2
};

enum { PAIFIGHT_ESCORT_OBJECT_LIMIT = 28, PAIFIGHT_NO_FLIGHT_GROUP = -1, PAIFIGHT_NO_OBJECT = 0xFFFF };

enum { PAIFIGHT_PRIMARY_LEADER_NOT_FOUND = -1, PAIFIGHT_ESCORT_HOLD_WAYPOINT = 1 };

/* Declarations follow ascending original IDB address. */

/* 0x414BA0 */
int16_t paifight_SelectTurretTargets(void);

/* 0x414EC0 */
int16_t paifight_scanfortargetorder(void);

/* 0x415090 */
int16_t paifight_findtargetingroup(int16_t flightGroupIndex, int16_t requireWorkingSubsystems);

/* 0x415160 */
int16_t paifight_FindNearestTargetOfGenus(int16_t genusId, int16_t requireWorkingSubsystems);

/* 0x415250 */
int16_t paifight_findescorterofgroup(int16_t escortedFlightGroup);

/* 0x415300 */
int paifight_countattackers(uint16_t targetObjIdx);

/* 0x4153B0 */
int16_t paifight_escorttargetorder(void);

/* 0x415570 */
int16_t paifight_fightershootorder(void);

/* 0x4159A0 */
int16_t paifight_coverleaderorder(void);

/* 0x415B00 */
int16_t paifight_followleadatkorder(void);

/* 0x415CF0 */
int16_t paifight_checkescortorder(void);

/* 0x415E30 */
int16_t paifight_OrderHasNoRemainingTargets(void);

/* 0x415F70 */
int16_t paifight_HasLiveOpponentsOrPendingGenus(int16_t genusId, int16_t requireWorkingSubsystems);

/* 0x416060 */
int16_t paifight_SelectLowAltitudeTarget(void);

/* 0x416130 */
int16_t paifight_AdvanceWaypointIfProgressComplete(void);

/* 0x416160 */
int16_t paifight_AdvanceWaypoint(void);

/* 0x416180 */
uint16_t paifight_FindEscortableFlightGroupLeader(int16_t flightGroupIndex);

#ifdef __cplusplus
}
#endif

#endif
