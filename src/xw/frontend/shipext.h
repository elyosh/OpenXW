#ifndef XW_FRONTEND_SHIPEXT_H
#define XW_FRONTEND_SHIPEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { SHIPEXT_SHIP_XWING = 0, SHIPEXT_SHIP_YWING = 1, SHIPEXT_SHIP_AWING = 2, SHIPEXT_SHIP_BWING = 3 };

enum {
	SHIPEXT_MISSION_OUTCOME_NONE = 0,
	SHIPEXT_MISSION_OUTCOME_DEAD = 1,
	SHIPEXT_MISSION_OUTCOME_CAPTURED = 2,
	SHIPEXT_MISSION_OUTCOME_RESCUED = 3,
	SHIPEXT_MISSION_OUTCOME_LAND = 4,
	SHIPEXT_MISSION_OUTCOME_LEGACY_15 = 15,
	SHIPEXT_MISSION_OUTCOME_FILM_ROOM = 16,
	SHIPEXT_MISSION_OUTCOME_LEGACY_17 = 17
};

enum {
	SHIPEXT_EXIT_DEAD = 0,
	SHIPEXT_EXIT_CAPTURED = 1,
	SHIPEXT_EXIT_RESCUED = 2,
	SHIPEXT_EXIT_LAND = 3,
	SHIPEXT_EXIT_MAP = 10,
	SHIPEXT_EXIT_DAMAGE_CONTROL = 11,
	SHIPEXT_EXIT_BRIEFING = 12,
	SHIPEXT_EXIT_OPTIONS = 13,
	SHIPEXT_EXIT_REPLAY_14 = 14,
	SHIPEXT_EXIT_REPLAY_16 = 16,
	SHIPEXT_EXIT_INFLIGHT_SWAP_MASK = 3,
	SHIPEXT_EXIT_OUTCOME_COUNT = 4,
	SHIPEXT_TOUR_CUTSCENE_COUNT = 14,
	SHIPEXT_NO_MEDAL = 0xFF
};

enum {
	SHIPEXT_REVIVE_WITH_PENALTY = 0,
	SHIPEXT_CREATE_PILOT = 1,
	SHIPEXT_REVIVED_RANK_LIMIT = 1,
	SHIPEXT_PILOT_UNSET_BYTE = 0xFF
};

enum {
	SHIPEXT_DEFAULT_COMBAT_SELECTION = 4,
	SHIPEXT_DEFAULT_MEDAL_INDEX = 1,
	SHIPEXT_SHIP_COUNT = 6,
	SHIPEXT_TOUR_COUNT = 8,
	SHIPEXT_COMBAT_SELECTION_COUNT = 14
};

enum {
	SHIPEXT_TOUR_OPERATION_TABLE_COUNT = 5,
	SHIPEXT_TOUR_1_OPERATION_COUNT = 12,
	SHIPEXT_TOUR_2_OPERATION_COUNT = 12,
	SHIPEXT_TOUR_3_OPERATION_COUNT = 14,
	SHIPEXT_TOUR_4_OPERATION_COUNT = 20,
	SHIPEXT_TOUR_5_OPERATION_COUNT = 20,
	SHIPEXT_TOUR_NO_ENTRY = 0xFF,
	SHIPEXT_TOUR_STATUS_ACTIVE = 1,
	SHIPEXT_TOUR_STATUS_LOST = 2,
	SHIPEXT_TOUR_STATUS_UNSELECTABLE = 3
};

typedef struct XwHighScoreEntry XwHighScoreEntry;
typedef struct XwMissionHighScores XwMissionHighScores;
typedef struct XwTourOperation XwTourOperation;

/* Preserve the serialized format in native builds; matching uses /Zp1. */
#ifdef XW_MODERN
#pragma pack(push, 1)
#endif
/* Original IDB size: 30 bytes. */
struct XwHighScoreEntry {
	/* IDB +0x0 */
	char pilotName[24];
	/* IDB +0x18 */
	int score;
	/* IDB +0x1C */
	uint16_t kills;
};
#ifdef XW_MODERN
#pragma pack(pop)
#endif
typedef char xw_size_XwHighScoreEntry[(sizeof(XwHighScoreEntry) == 30) ? 1 : -1];
typedef char xw_offset_XwHighScoreEntry_score[(offsetof(XwHighScoreEntry, score) == 24) ? 1 : -1];
typedef char xw_offset_XwHighScoreEntry_kills[(offsetof(XwHighScoreEntry, kills) == 28) ? 1 : -1];

/* Original IDB size: 3 bytes. */
struct XwTourOperation {
	/* IDB +0x0: First mission choice; 255 means absent. */
	uint8_t missionChoiceA;
	/* IDB +0x1: Alternate mission choice; 255 means absent. */
	uint8_t missionChoiceB;
	/* IDB +0x2: Cutscene unlocked by completion; 255 means none. */
	uint8_t cutsceneIndex;
};

/* Original IDB size: 264 bytes. */
struct XwMissionHighScores {
	/* IDB +0x0: Compared with selected mission paragraph text. Score file record stride is 264 bytes. */
	char missionName[24];
	/* IDB +0x18: Eight packed 30-byte entries; reader at 0x440AB0 uses stride 30 and score/kills at entry
	 * +24/+28. */
	struct XwHighScoreEntry entries[8];
};

typedef char xw_size_XwMissionHighScores[(sizeof(XwMissionHighScores) == 264) ? 1 : -1];
typedef char xw_offset_XwMissionHighScores_entries[(offsetof(XwMissionHighScores, entries) == 24) ? 1 : -1];

/* Declarations follow ascending original IDB address. */

extern XwTourOperation g_tour1Operations[SHIPEXT_TOUR_1_OPERATION_COUNT];
extern XwTourOperation g_tour2Operations[SHIPEXT_TOUR_2_OPERATION_COUNT];
extern XwTourOperation g_tour3Operations[SHIPEXT_TOUR_3_OPERATION_COUNT];
extern XwTourOperation g_tour4Operations[SHIPEXT_TOUR_4_OPERATION_COUNT];
extern XwTourOperation g_tour5Operations[SHIPEXT_TOUR_5_OPERATION_COUNT];
extern XwTourOperation* g_tourOperationTables[SHIPEXT_TOUR_OPERATION_TABLE_COUNT];
extern uint8_t g_tourOperationCounts[SHIPEXT_TOUR_COUNT];

extern const int16_t g_trainingExitScenes[SHIPEXT_SHIP_COUNT];
extern const int16_t g_combatExitScenes[SHIPEXT_SHIP_COUNT][SHIPEXT_EXIT_OUTCOME_COUNT];
extern const int16_t g_tourExitScenes[SHIPEXT_SHIP_COUNT][SHIPEXT_EXIT_OUTCOME_COUNT];
extern const int16_t g_tourCutsceneScenes[SHIPEXT_TOUR_CUTSCENE_COUNT];
extern int16_t g_combatSelectedShip;
extern int16_t g_pendingTourCutscene;
extern int16_t g_pendingMedalScene;
extern int16_t g_postAwardScene;
extern int16_t g_pendingMedalIndex;
extern int16_t g_availableShips[SHIPEXT_SHIP_COUNT];
extern int16_t g_availableTours[SHIPEXT_TOUR_COUNT];
extern int16_t g_lastBriefedTourOperation;
extern int16_t g_combatSelectionIsTour;
extern int16_t g_trainingSelectedShip;
extern int16_t g_trainingSelectedLevel;
extern int16_t g_combatMissionCursors[SHIPEXT_COMBAT_SELECTION_COUNT];
extern int16_t g_shellMissionOutcome;
extern int16_t g_shellMissionShip;
extern int16_t g_combatSelectedSourceIndex;

/* 0x45DD80 */
int16_t shipext_Load_Pilot(const char* filename, int unused);

/* 0x45DDC0 */
void shipext_Revive_Pilot(const char* filename, int16_t mode);

/* 0x4A6B90 */
void shipext_ResetMissionSelections(void);

/* 0x4A6BC0 */
void shipext_Set_Train_Ship(int16_t shipIndex);

/* 0x4A6BD0 */
int16_t shipext_Get_Train_Ship(void);

/* 0x4A6BE0 */
void shipext_Set_Train_Level(int16_t levelIndex);

/* 0x4A6BF0 */
int16_t shipext_Get_Train_Level(void);

/* 0x4A6C00 */
void shipext_Set_Combat_Ship(int16_t selectionIndex, int16_t isTour);

/* 0x4A6C20 */
int16_t shipext_Get_Combat_Ship(void);

/* 0x4A6C30 */
int16_t shipext_Is_Combat_Ship_Tour(void);

/* 0x4A6C40 */
void shipext_Set_Combat_Mission(int16_t missionIndex);

/* 0x4A6C60 */
int16_t shipext_Get_Combat_Mission(void);

/* 0x4A6C70 */
void shipext_Set_Mission_Ship(int16_t shipIndex);

/* 0x4A6C80 */
int16_t shipext_Get_Mission_Ship(void);

/* 0x4A6C90 */
int16_t shipext_IsShipAvailable(int16_t shipIndex);

/* 0x4A6CA0 */
int16_t shipext_IsTourAvailable(int16_t tourIndex);

/* 0x4A6CB0 */
void shipext_Set_Combat_Source_Index(int16_t sourceIndex);

/* 0x4A6CC0 */
int16_t shipext_Get_Combat_Source_Index(void);

/* 0x4A6CD0 */
void shipext_Set_Mission_Outcome(int16_t outcome);

/* 0x4A6CE0 */
int16_t shipext_Get_Mission_Outcome(void);

/* 0x4A6CF0 */
void shipext_Set_Last_Briefed_Tour_Operation(int16_t operationIndex);

/* 0x4A6D00 */
int16_t shipext_Get_Last_Briefed_Tour_Operation(void);

/* 0x4A6D10 */
void shipext_Set_Pending_Tour_Cutscene(int16_t sceneId);

/* 0x4A6D20 */
int16_t shipext_Get_Pending_Tour_Cutscene(void);

/* 0x4A6D30 */
void shipext_Set_Pending_Medal(int16_t sceneId, int16_t medalIndex);

/* 0x4A6D50 */
int16_t shipext_Get_Pending_Medal_Scene(void);

/* 0x4A6D60 */
int16_t shipext_Get_Pending_Medal_Index(void);

/* 0x4A6D70 */
void shipext_Set_Post_Award_Scene(int16_t sceneId);

/* 0x4A6D80 */
int16_t shipext_Get_Post_Award_Scene(void);

/* 0x4A6D90 */
int16_t shipext_Mission_Exit(int16_t missionType, int16_t exitCode, int16_t newMedal,
							 int16_t tourCutsceneIndex);

/* 0x4A6F60 */
void shipext_Get_Mission_Path(char* outPath, const char* missionName, int16_t isMissionData);

#ifdef __cplusplus
}
#endif

#endif
