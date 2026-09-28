#ifndef XW_FLIGHT_MISSION_MISSION_H
#define XW_FLIGHT_MISSION_MISSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/flight/object/craft.h>
#include <xw/flight/object/object.h>

typedef struct MissionFlightGroupState MissionFlightGroupState;
typedef struct XwMissionFlightGroup XwMissionFlightGroup;
typedef struct XwMissionHeader XwMissionHeader;
typedef struct XwMissionObjectDiskRecord XwMissionObjectDiskRecord;
typedef struct XwMissionObjectGoalResults XwMissionObjectGoalResults;
typedef struct XwMissionObjectRecord XwMissionObjectRecord;
typedef struct XwMissionRuntimeState XwMissionRuntimeState;

enum {
	MISSION_GOAL_NONE = 0,
	MISSION_GOAL_DESTROY_ALL = 1,
	MISSION_GOAL_COMPLETE_ALL = 2,
	MISSION_GOAL_RECOVER_ALL = 3,
	MISSION_GOAL_BOARD_ALL = 4,
	MISSION_GOAL_DESTROY_SPECIAL = 5,
	MISSION_GOAL_COMPLETE_SPECIAL = 6,
	MISSION_GOAL_RECOVER_SPECIAL = 7,
	MISSION_GOAL_BOARD_SPECIAL = 8,
	MISSION_GOAL_DESTROY_HALF = 9,
	MISSION_GOAL_COMPLETE_HALF = 10,
	MISSION_GOAL_RECOVER_HALF = 11,
	MISSION_GOAL_BOARD_HALF = 12,
	MISSION_GOAL_IDENTIFY_ALL = 13,
	MISSION_GOAL_IDENTIFY_SPECIAL = 14,
	MISSION_GOAL_IDENTIFY_HALF = 15,
	MISSION_GOAL_ARRIVE = 16
};

/* Indices in MissionFlightGroupState.outcomes, verified by goal evaluation. */
enum {
	MISSION_OUTCOME_COMPLETED_FIRST = 0,
	MISSION_OUTCOME_COMPLETED_SECOND = 1,
	MISSION_OUTCOME_SPECIAL_COMPLETED = 2,
	MISSION_OUTCOME_RECOVERED = 3,
	MISSION_OUTCOME_SPECIAL_RECOVERED = 4,
	MISSION_OUTCOME_BOARDED = 5,
	MISSION_OUTCOME_SPECIAL_BOARDED = 6
};

enum {
	MISSION_GOAL_EVALUATION_EXIT_REASON = 3,
	MISSION_GOAL_SCAN_TICKS = 236,
	MISSION_GOAL_OUTCOME_SCAN_TICKS = 14160,
	MISSION_OBJECT_GOAL_DESTROY = 4,
	MISSION_OBJECT_GOAL_PROTECT = 8,
	MISSION_RULE_SURFACE_SPECIAL_TARGET = 4,
	MISSION_SURFACE_GOAL_HEALTH_COUNT = 8,
	MISSION_GOAL_SATELLITE_TYPE_FIRST = 70,
	MISSION_GOAL_SATELLITE_TYPE_SECOND = 83,
	MISSION_GOAL_PROBE_TYPE = 80,
	MISSION_GOAL_MINE_TYPE_FIRST = 75,
	MISSION_GOAL_MINE_TYPE_LAST = 78,
	MISSION_COMPLETION_SOUND_SLOT = 37
};

enum { XW_MISSION_GOAL_STATE_UNCOMPLETED = 1, XW_MISSION_GOAL_STATE_COMPLETED = 2 };

enum { MISSION_FLIGHT_GROUP_COUNT = 16, MISSION_DEPARTURE_MOTHERSHIP = 0, MISSION_DEPARTURE_HYPERSPACE = 1 };

enum { MISSION_ARRIVAL_MOTHERSHIP = 0 };

enum {
	MISSION_TRIGGER_ALWAYS = 0,
	MISSION_TRIGGER_ARRIVED = 1,
	MISSION_TRIGGER_DESTROYED_NO_LIVE_CRAFT = 2,
	MISSION_TRIGGER_ATTACKED_OR_TARGETED = 3,
	MISSION_TRIGGER_CAPTURED = 4,
	MISSION_TRIGGER_INSPECTED = 5,
	MISSION_TRIGGER_DISABLED = 6
};

enum { MISSION_RULE_FORCE_RESCUE = 1 };

enum { MISSION_TARGET_GROUP_UNSPECIFIED = 0xFFFF };

enum { MISSION_OBJECT_COUNT = 64 };

enum { MISSION_OBJECT_WORLD_COORDINATE_SCALE = 256, MISSION_CRAFT_TYPE_COUNT = 112 };

/* Original IDB size: 20 bytes. */
struct MissionFlightGroupState {
	/* IDB +0x0: Cleared during mission initialization; set before first-wave creation. */
	uint8_t hasArrived;
	/* IDB +0x1: Loaded from mission wave count after first-wave creation; decremented by subsequent-wave
	 * spawning. */
	uint8_t wavesRemaining;
	/* IDB +0x2: Remaining one-second arrival-delay scans; decremented while arrivalDelayPending and not
	 * hasArrived. */
	uint16_t arrivalDelayTimer;
	/* IDB +0x4: Set when arrival condition is met; reset during mission initialization. */
	uint8_t arrivalDelayPending;
	/* IDB +0x5 */
	uint8_t gap05;
	/* IDB +0x6: Encoded current waypoint reference 0x8000+index, not a bare index. CREATE_loadmission
	 * initializes 0x8000/0x8004/0x8005; resolving reference 0x8000 loads this word and decodes by 16-bit
	 * +0x8000 wrap. */
	uint16_t currentWaypointRef;
	/* IDB +0x8 */
	uint8_t spawnedCraftCount;
	/* IDB +0x9: Departure, recovery and boarding counts plus special-craft flags; MISSION_OUTCOME_* indices.
	 */
	uint8_t outcomes[7];
	/* IDB +0x10 */
	uint8_t destroyedCount;
	/* IDB +0x11 */
	uint8_t specialCraftDestroyed;
	/* IDB +0x12 */
	uint8_t inspectedCount;
	/* IDB +0x13 */
	uint8_t specialCraftInspected;
};

/* Original IDB size: 148 bytes. */
struct XwMissionFlightGroup {
	/* IDB +0x0: Flight-group display name. Followed by two 16-byte strings copied to craft +161. */
	char name[16];
	/* IDB +0x10: Default cargo text copied to craft cargoName. */
	char cargo[16];
	/* IDB +0x20: Cargo text for specialCraftIndex. */
	char specialCargo[16];
	/* IDB +0x30: Zero-based craft index choosing specialCargo instead of cargo during spawn. */
	uint16_t specialCraftIndex;
	/* IDB +0x32: Mission-format type index into g_craftTypeToObjectType, not runtime XwObjectTypeId. Named
	 * ship IDs 1..17; other values select static objects/backdrops/course scenery. For flight groups, type2 +
	 * initialStatus>=10 means B-wing; type16 + initialStatus>=10 means Interdictor. */
	XwCraftSpecies craftType;
	/* IDB +0x34: 0=craft default IFF, otherwise object IFF is value-1. */
	uint16_t iffOverride;
	/* IDB +0x36: Spawn status, including warhead/shield reduction and values >=10 selecting alternate craft
	 * models. */
	uint16_t initialStatus;
	/* IDB +0x38: Number of craft in a wave; spawning stops at this count, message display adds craft numbers
	 * when greater than one. */
	uint16_t numberOfCraft;
	/* IDB +0x3A: Extra waves after the initial wave; low byte initializes wavesRemaining; objective totals
	 * use (additionalWaveCount + 1) * numberOfCraft. */
	uint16_t additionalWaveCount;
	/* IDB +0x3C: Arrival trigger selector 0..6. See Mission_UpdateFlightGroupArrivals for the X-Wing-specific
	 * tests. */
	uint16_t arrivalCondition;
	/* IDB +0x3E: Encoded delay: 0..20 is minutes; larger values decode to 6 * value - 120 seconds. Trigger 1
	 * may add 15 seconds. */
	uint16_t arrivalDelay;
	/* IDB +0x40: Flight group tested by arrivalCondition; 0xFFFF means none. */
	uint16_t arrivalTriggerFlightGroupIndex;
	/* IDB +0x42: Mothership flight-group index shared by arrival/departure handling. Field within 148-byte
	 * mission flight-group records. */
	uint16_t mothershipFlightGroupIndex;
	/* IDB +0x44: Arrival method: 0 selects mothership launch. Field within 148-byte mission flight-group
	 * records. */
	uint16_t arrivalMethod;
	/* IDB +0x46: Departure method: 0 selects mothership return; 1 permits the ordinary hyperspace predicate.
	 * Field within 148-byte mission flight-group records. Encoding differs from BoP. */
	uint16_t departureMethod;
	/* IDB +0x48: Seven signed X coordinates; Mission_ResolveObjectOrMissionPointWorldLoc scales by 256. */
	int16_t waypointX[7];
	/* IDB +0x56: Seven signed Y coordinates; same scale as X. */
	int16_t waypointY[7];
	/* IDB +0x64: Seven signed Z coordinates; same scale as X. */
	int16_t waypointZ[7];
	/* IDB +0x72: Seven on-disk 16-bit waypoint enable flags. The advance helper tests nonzero; loaded
	 * verbatim with the 148-byte flight-group record. */
	uint16_t waypointEnabled[7];
	/* IDB +0x80: Formation-table selector copied to g_spawnFormation. */
	uint16_t formation;
	/* IDB +0x82: 0=no player binding, otherwise zero-based craft ordinal+1. */
	uint16_t playerCraftOrdinalPlusOne;
	/* IDB +0x84: Indexes skill and think-interval tables through its low byte. */
	uint16_t aiLevel;
	/* IDB +0x86: Indexes leader/follower plan maps. */
	uint16_t order;
	/* IDB +0x88: Indexes initial throttle map; also stored in craft state. */
	uint16_t throttlePreset;
	/* IDB +0x8A: Low byte copied to ObjectRecord offset 0x32 during spawn. */
	uint16_t markings;
	/* IDB +0x8C */
	uint16_t field_8C;
	/* IDB +0x8E: Flight-group goal selector 0..16; zero means no goal. Evaluated by Mission_UpdateLogic. */
	uint16_t missionGoal;
	/* IDB +0x90: Primary mission target flight-group index; field within 148-byte flight-group records. */
	uint16_t primaryTarget;
	/* IDB +0x92: Secondary mission target flight-group index; field within 148-byte flight-group records. */
	uint16_t secondaryTarget;
};

typedef char xw_size_XwMissionFlightGroup[(sizeof(XwMissionFlightGroup) == 148) ? 1 : -1];
typedef char xw_offset_XwMissionFlightGroup_waypointEnabled
	[(offsetof(XwMissionFlightGroup, waypointEnabled) == 114) ? 1 : -1];
typedef char
	xw_offset_XwMissionFlightGroup_missionGoal[(offsetof(XwMissionFlightGroup, missionGoal) == 142) ? 1 : -1];

/* Original IDB size: 206 bytes. */
struct XwMissionHeader {
	/* IDB +0x0 */
	uint16_t field_00;
	/* IDB +0x2: Normal-mission countdown in minutes; zero defaults to 20. Proving-ground timer comes from
	 * object type 202. */
	uint16_t timeLimitMinutes;
	/* IDB +0x4: Mission-rule byte read by flight initialization, goal evaluation, rescue checks and Death
	 * Star cell collisions. */
	uint8_t missionRuleFlags;
	/* IDB +0x5 */
	uint8_t field_05;
	/* IDB +0x6: Temporarily replaces the low 16 bits of the game RNG seed while generating default backdrops.
	 */
	uint16_t backdropSeed;
	/* IDB +0x8: Copied to g_deathStarSurfaceModeActive and selects the light-direction Y sign. */
	uint8_t surfaceMode;
	/* IDB +0x9 */
	uint8_t field_09;
	/* IDB +0xA: Three 64-byte message buffers; nonempty entries are displayed when mission goals complete. */
	char completionMessages[3][64];
	/* IDB +0xCA: Count of 148-byte XwMissionFlightGroup records immediately after this header. Flight loaders
	 * interpret unsigned16; the in-flight map loader uses a signed16 loop comparison. */
	uint16_t flightGroupCount;
	/* IDB +0xCC: Count of 70-byte XwMissionObjectDiskRecord entries after the flight groups. In-flight map
	 * code uses this same count as signed16 but requests 148 bytes per entry, a loader discrepancy rather
	 * than a distinct header format. */
	uint16_t objectRecordCount;
};

typedef char xw_size_XwMissionHeader[(sizeof(XwMissionHeader) == 206) ? 1 : -1];
typedef char
	xw_offset_XwMissionHeader_flightGroupCount[(offsetof(XwMissionHeader, flightGroupCount) == 202) ? 1 : -1];
typedef char xw_offset_XwMissionHeader_objectRecordCount[(offsetof(XwMissionHeader, objectRecordCount) == 204)
															 ? 1
															 : -1];

/* Original IDB size: 70 bytes. */
struct XwMissionObjectDiskRecord {
	/* IDB +0x0: Prefix not interpreted by the mission loaders. */
	uint8_t gap00[50];
	/* IDB +0x32: Mission-format type index into g_craftTypeToObjectType, not runtime XwObjectTypeId. Named
	 * ship IDs 1..17; other values select static objects/backdrops/course scenery. For flight groups, type2 +
	 * initialStatus>=10 means B-wing; type16 + initialStatus>=10 means Interdictor. */
	XwCraftSpecies craftType;
	/* IDB +0x34 */
	uint16_t field_34;
	/* IDB +0x36: Genus-specific options: low byte supplies gate target flags or type-202 seconds; value 1
	 * preserves the specified asteroid type. Low two bits choose the grid plane, and bits 2..3 become static
	 * goal flags. */
	uint16_t stateOrSeconds;
	/* IDB +0x38: Asteroid count or grid side length; type 202 supplies countdown minutes. Other course gates
	 * set state bit 7 when this value exceeds 1. */
	uint16_t countOrMinutes;
	/* IDB +0x3A: Signed coordinate for statics; backdrop records use its low nibble as a packed direction
	 * component. */
	int16_t worldX;
	/* IDB +0x3C: Signed coordinate for statics; backdrop records use its low nibble as a packed direction
	 * component. */
	int16_t worldY;
	/* IDB +0x3E: Signed coordinate for statics; backdrop records instead select face 0..5 (unsigned values
	 * above 5 clamp to 5). */
	int16_t worldZ;
	/* IDB +0x40: Low byte is used. Course gates add 128 during load. */
	uint16_t yawAngle8;
	/* IDB +0x42: Low byte is used. Course gates convert to 128 minus this byte. */
	uint16_t pitchAngle8;
	/* IDB +0x44: Low byte is used. Course gates negate this byte. */
	uint16_t rollAngle8;
};

typedef char xw_size_XwMissionObjectDiskRecord[(sizeof(XwMissionObjectDiskRecord) == 70) ? 1 : -1];
typedef char xw_offset_XwMissionObjectDiskRecord_craftType
	[(offsetof(XwMissionObjectDiskRecord, craftType) == 50) ? 1 : -1];
typedef char
	xw_offset_XwMissionObjectDiskRecord_worldX[(offsetof(XwMissionObjectDiskRecord, worldX) == 58) ? 1 : -1];

/* Original IDB size: 6 bytes. */
struct XwMissionObjectGoalResults {
	/* IDB +0x0: 0 absent;1 failed;2 completed. Surface missions reuse first two category pairs. */
	uint8_t protectMines;
	/* IDB +0x1: 0 absent;1 failed;2 completed. Surface missions reuse first two category pairs. */
	uint8_t destroyMines;
	/* IDB +0x2: 0 absent;1 failed;2 completed. Surface missions reuse first two category pairs. */
	uint8_t protectSatellites;
	/* IDB +0x3: 0 absent;1 failed;2 completed. Surface missions reuse first two category pairs. */
	uint8_t destroySatellites;
	/* IDB +0x4: 0 absent;1 failed;2 completed. Surface missions reuse first two category pairs. */
	uint8_t protectProbes;
	/* IDB +0x5: 0 absent;1 failed;2 completed. Surface missions reuse first two category pairs. */
	uint8_t destroyProbes;
};

/* Original IDB size: 15 bytes. */
struct XwMissionObjectRecord {
	/* IDB +0x0 */
	uint8_t field_00;
	/* IDB +0x1: While objectType!=0: XwObjectGenus. static_laserhitstatic replaces this byte with the former
	 * objectType before clearing objectType. Preserve unsigned-byte storage; genus enum applies only to
	 * active-object reads/writes. */
	uint8_t genusId;
	/* IDB +0x2: Runtime species/model-table index after translating the disk craftType; zero means inactive.
	 * On destruction static_laserhitstatic preserves this ID in the genusId byte. Non-ship values are mostly
	 * unnamed in the current species enum. */
	XwObjectTypeId objectType;
	/* IDB +0x3: Signed world coordinate in units of 256; multiply by 256 to match ObjectRecord.worldX. */
	int16_t worldX;
	/* IDB +0x5: Same coordinate scale as worldX. */
	int16_t worldY;
	/* IDB +0x7: Same coordinate scale as worldX. */
	int16_t worldZ;
	/* IDB +0x9: 8-bit angle; shifted left 8 for the 16-bit full-turn rotation routines. */
	uint8_t yawAngle8;
	/* IDB +0xA: 8-bit angle; shifted left 8 for the 16-bit full-turn rotation routines. */
	uint8_t pitchAngle8;
	/* IDB +0xB: 8-bit angle; shifted left 8 for the 16-bit full-turn rotation routines. */
	uint8_t rollAngle8;
	/* IDB +0xC: Disk stateOrSeconds bits 2/3: destroy/protect objective flags. */
	uint8_t goalFlags;
	/* IDB +0xD: Genus 14 gate guns: bit 7 enables firing; bits 0..5 enable meshes 4..9. Loaded at
	 * 0x40FEB7/0x40FEC2 and read by GATE_updategateguns. Other object genera may reuse this byte. */
	uint8_t stateByte;
	/* IDB +0xE: For genus-7 mines: unsigned firing cooldown, decremented by elapsedTicks/2 and reset to 236.
	 * Other object types reuse this byte. */
	uint8_t typeSpecificByte;
};

/* Original IDB size: 306 bytes. */
struct XwMissionRuntimeState {
	/* IDB +0x0: Nonzero when loaded mission records contain genus-14 proving-ground gates. Snapshot block 8,
	 * offset +0x0. Existing field purpose checked against its X-Wing users. */
	uint8_t provingGroundsActive;
	/* IDB +0x1: Training-ship selection plus one. Value 4 maps to craft 2 with initialStatus 10; also indexes
	 * pilot course scores. Snapshot block 8, offset +0x1. Existing field purpose checked against its X-Wing
	 * users. */
	uint8_t provingGroundsSelectedCraft;
	/* IDB +0x2: Snapshot block 8, offset +0x2. Existing field purpose checked against its X-Wing users. */
	uint8_t provingGroundsLevel;
	/* IDB +0x3: Snapshot block 8, offset +0x3. Existing field purpose checked against its X-Wing users. */
	int provingGroundsScore;
	/* IDB +0x7: Static mission-object index shown on radar and used for course progression; -1 before the
	 * start gate, 0 at course start. Snapshot block 8, offset +0x7. Existing field purpose checked against
	 * its X-Wing users. */
	int16_t provingGroundsCurrentCheckpointIndex;
	/* IDB +0x9: Snapshot block 8, offset +0x9. Existing field purpose checked against its X-Wing users. */
	uint16_t provingGroundsCheckpointsPassed;
	/* IDB +0xB: Snapshot block 8, offset +0xB. Existing field purpose checked against its X-Wing users. */
	uint16_t provingGroundsCheckpointsMissed;
	/* IDB +0xD: Snapshot block 8, offset +0xD. Existing field purpose checked against its X-Wing users. */
	uint16_t provingGroundsCheckpointsRemaining;
	/* IDB +0xF: Snapshot block 8, offset +0xF. Existing field purpose checked against its X-Wing users. */
	uint16_t provingGroundsTargetsDestroyed;
	/* IDB +0x11: Snapshot block 8, offset +0x11. Existing field purpose checked against its X-Wing users. */
	uint16_t provingGroundsTimeBonus;
	/* IDB +0x13: 24 legacy craft-statistics categories selected by sub_423A10; not direct model/object-type
	 * indices. Snapshot block 8, offset +0x13. Existing field purpose checked against its X-Wing users. */
	uint8_t inspectedCountsByCategory[24];
	/* IDB +0x2B: Pilot scoring indexes iff*24+craft type and increments persistent captures_by_type from
	 * these bytes; adjoining destroyed table has same dimensions. Indirect capture writer still to verify.
	 * Snapshot block 8, offset +0x2B. Existing field purpose checked against its X-Wing users. */
	uint8_t captureCountsByIffAndType[3][24];
	/* IDB +0x73: Destroyed craft counts by victim IFF and legacy statistics category; credited in sub_4052C0
	 * with byte saturation at 255. Snapshot block 8, offset +0x73. Existing field purpose checked against its
	 * X-Wing users. */
	uint8_t destroyedCountsByIffAndCategory[3][24];
	/* IDB +0xBB: Debrief flight-badge announcement byte. Cleared on flight initialization; no nonzero direct
	 * writer found. Snapshot block 8, offset +0xBB. Existing field purpose checked against its X-Wing users.
	 */
	uint8_t flightBadgeAnnouncement;
	/* IDB +0xBC: 0=no new patch; otherwise completed historic mission + 6*ship + 1. updatepilotrecord
	 * excludes ship4. Snapshot block 8, offset +0xBC. Existing field purpose checked against its X-Wing
	 * users. */
	uint8_t newBattlePatch;
	/* IDB +0xBD: New rank awarded by pilot updater, consumed by debrief display. Snapshot block 8, offset
	 * +0xBD. Existing field purpose checked against its X-Wing users. */
	uint8_t newRank;
	/* IDB +0xBE: New medal id: performance1..6 or completed-tour index+7. Snapshot block 8, offset +0xBE.
	 * Existing field purpose checked against its X-Wing users. */
	uint8_t newMedal;
	/* IDB +0xBF: Tour-step cutscene index: FEDISKIO_updatepilotrecord copies byte 2 of the completed
	 * three-byte tour-step record; initialized to 0xFF and passed to SHIPEXT_Mission_Exit. Snapshot block 8,
	 * offset +0xBF. Existing field purpose checked against its X-Wing users. */
	uint8_t tourCutsceneIndex;
	/* IDB +0xC0: Mission mode used by score/progression logic:1 historical combat,3 tour. Snapshot block 8,
	 * offset +0xC0. Existing field purpose checked against its X-Wing users. */
	uint8_t mode;
	/* IDB +0xC1: Reason passed back to frontend routing; 10..13 preserve the CD-music remaining time across
	 * the flight interruption. Other numeric meanings remain unresolved. Snapshot block 8, offset +0xC1.
	 * Existing field purpose checked against its X-Wing users. */
	uint8_t flightExitReason;
	/* IDB +0xC2: Nonzero ends the XW_doframe loop; value 2 is normalized to exit reason 3 at cleanup.
	 * Snapshot block 8, offset +0xC2. Existing field purpose checked against its X-Wing users. */
	uint8_t flightExitRequested;
	/* IDB +0xC3: Serialized bytes with no identified direct semantic use. Do not assume padding or assign
	 * field meaning. */
	uint8_t gap_C3[1];
	/* IDB +0xC4: Latched to 1 when all required goals complete; consumed by dynamic music and mission outcome
	 * handling. Snapshot block 8, offset +0xC4. Existing field purpose checked against its X-Wing users. */
	uint8_t objectivesCompleted;
	/* IDB +0xC5: Serialized bytes with no identified direct semantic use. Do not assume padding or assign
	 * field meaning. */
	uint8_t gap_C5[2];
	/* IDB +0xC7: Set when no remaining goal-bearing craft/static objects can complete the mission; selects
	 * the failure music track. Snapshot block 8, offset +0xC7. Existing field purpose checked against its
	 * X-Wing users. */
	uint8_t objectivesUnfinishable;
	/* IDB +0xC8: 16 mission flight-group goal results:0 absent,1 incomplete,2 completed. Written by
	 * Mission_UpdateLogic and consumed by debrief goal sections. Snapshot block 8, offset +0xC8. Existing
	 * field purpose checked against its X-Wing users. */
	uint8_t flightGroupGoalState[16];
	/* IDB +0xD8: Six byte results:0 absent,1 failed,2 completed. Updated by Mission_UpdateLogic. Surface
	 * mission reuses destroyMines/destroySatellites for exhaust port/laser towers. Snapshot block 8, offset
	 * +0xD8. Existing field purpose checked against its X-Wing users. */
	struct XwMissionObjectGoalResults objectGoalResults;
	/* IDB +0xDE: 28 cached display distances by object slot. Frontend displays integer/100 and integer%100;
	 * populated only for occupied slots. Snapshot block 8, offset +0xDE. Existing field purpose checked
	 * against its X-Wing users. */
	uint16_t snapshotCraftDisplayDistances[28];
	/* IDB +0x116: 28 cached PANEL_getcraftstatus codes by object slot, used by the saved-flight frontend.
	 * Snapshot block 8, offset +0x116. Existing field purpose checked against its X-Wing users. */
	uint8_t snapshotCraftStatusCodes[28];
};

/* Declarations follow ascending original IDB address. */

extern const XwObjectTypeId g_craftTypeToObjectType[MISSION_CRAFT_TYPE_COUNT];
extern XwMissionHeader g_missionHeader;
extern XwMissionRuntimeState g_missionRuntimeState;
extern MissionFlightGroupState g_missionFlightGroupStates[MISSION_FLIGHT_GROUP_COUNT];
extern XwMissionFlightGroup g_missionFlightGroups[MISSION_FLIGHT_GROUP_COUNT];
extern XwMissionObjectRecord g_missionObjects[MISSION_OBJECT_COUNT];

/* 0x408530 */
void Mission_UpdateLogic(void);

#ifdef __cplusplus
}
#endif

#endif
