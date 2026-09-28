#include "xw/frontend/shipext.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/profile.h"
#endif

#include "xw/frontend/register.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shellext.h"

#include <landru/file.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4CF7A0
XwTourOperation g_tour1Operations[SHIPEXT_TOUR_1_OPERATION_COUNT] = {
	{ 0, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },  { 1, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 2, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },  { 3, SHIPEXT_TOUR_NO_ENTRY, 0 },
	{ 4, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },  { 5, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 6, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },  { 7, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 8, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },  { 9, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 10, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY }, { 11, SHIPEXT_TOUR_NO_ENTRY, 1 },
};

// GLOBAL: XW 0x4CF7C8
XwTourOperation g_tour2Operations[SHIPEXT_TOUR_2_OPERATION_COUNT] = {
	{ 0, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 1, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 2, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 3, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 4, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 5, SHIPEXT_TOUR_NO_ENTRY, 2 },
	{ 6, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 7, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 8, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 9, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 10, SHIPEXT_TOUR_NO_ENTRY, 3 },
	{ 11, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
};

// GLOBAL: XW 0x4CF7F0
XwTourOperation g_tour3Operations[SHIPEXT_TOUR_3_OPERATION_COUNT] = {
	{ 0, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 1, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 2, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 3, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 4, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 5, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 6, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 7, SHIPEXT_TOUR_NO_ENTRY, 4 },
	{ 8, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 9, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 10, SHIPEXT_TOUR_NO_ENTRY, 5 },
	{ 11, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 12, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 13, SHIPEXT_TOUR_NO_ENTRY, 6 },
};

// GLOBAL: XW 0x4CF820
XwTourOperation g_tour4Operations[SHIPEXT_TOUR_4_OPERATION_COUNT] = {
	{ 0, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 1, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 2, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 3, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 4, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 5, 6, 7 },
	{ 7, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 8, 9, SHIPEXT_TOUR_NO_ENTRY },
	{ 10, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 11, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 12, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 13, 14, SHIPEXT_TOUR_NO_ENTRY },
	{ 15, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 16, SHIPEXT_TOUR_NO_ENTRY, 8 },
	{ 17, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 18, 19, SHIPEXT_TOUR_NO_ENTRY },
	{ 20, SHIPEXT_TOUR_NO_ENTRY, 9 },
	{ 21, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 22, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 23, SHIPEXT_TOUR_NO_ENTRY, 10 },
};

// GLOBAL: XW 0x4CF860
XwTourOperation g_tour5Operations[SHIPEXT_TOUR_5_OPERATION_COUNT] = {
	{ 0, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 1, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 2, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 3, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 4, SHIPEXT_TOUR_NO_ENTRY, 11 },
	{ 5, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 6, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 7, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 8, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 9, SHIPEXT_TOUR_NO_ENTRY, 12 },
	{ 10, 11, SHIPEXT_TOUR_NO_ENTRY },
	{ 12, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 13, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 14, 15, SHIPEXT_TOUR_NO_ENTRY },
	{ 16, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 17, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 18, 19, SHIPEXT_TOUR_NO_ENTRY },
	{ 20, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 21, SHIPEXT_TOUR_NO_ENTRY, SHIPEXT_TOUR_NO_ENTRY },
	{ 22, 23, 13 },
};

// GLOBAL: XW 0x4CF8A8
XwTourOperation* g_tourOperationTables[SHIPEXT_TOUR_OPERATION_TABLE_COUNT] = {
	g_tour1Operations, g_tour2Operations, g_tour3Operations, g_tour4Operations, g_tour5Operations
};

// GLOBAL: XW 0x4DC5F8
const int16_t g_trainingExitScenes[SHIPEXT_SHIP_COUNT] = {
	XW_SCENE_TRAINING_RESULTS_XWING, XW_SCENE_TRAINING_RESULTS_YWING, XW_SCENE_TRAINING_RESULTS_AWING,
	XW_SCENE_TRAINING_RESULTS_BWING, XW_SCENE_TRAINING_RESULTS_AWING, XW_SCENE_TRAINING_RESULTS_AWING
};

// GLOBAL: XW 0x4DC608
const int16_t g_combatExitScenes[SHIPEXT_SHIP_COUNT][SHIPEXT_EXIT_OUTCOME_COUNT] = {
	{ XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT,
	  XW_SCENE_COMBAT_RETURN_XWING },
	{ XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT,
	  XW_SCENE_COMBAT_RETURN_YWING },
	{ XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT,
	  XW_SCENE_COMBAT_RETURN_AWING },
	{ XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT,
	  XW_SCENE_COMBAT_RETURN_BWING },
	{ XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT,
	  XW_SCENE_COMBAT_RETURN_AWING },
	{ XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT, XW_SCENE_DEBRIEF_COMBAT,
	  XW_SCENE_COMBAT_RETURN_AWING }
};

// GLOBAL: XW 0x4DC638
const int16_t g_tourExitScenes[SHIPEXT_SHIP_COUNT][SHIPEXT_EXIT_OUTCOME_COUNT] = {
	{ XW_SCENE_FUNERAL_INTERIOR, XW_SCENE_PILOT_CAPTURED, XW_SCENE_PILOT_RESCUED,
	  XW_SCENE_TOUR_RETURN_XWING },
	{ XW_SCENE_FUNERAL_INTERIOR, XW_SCENE_PILOT_CAPTURED, XW_SCENE_PILOT_RESCUED,
	  XW_SCENE_TOUR_RETURN_YWING },
	{ XW_SCENE_FUNERAL_INTERIOR, XW_SCENE_PILOT_CAPTURED, XW_SCENE_PILOT_RESCUED,
	  XW_SCENE_TOUR_RETURN_AWING },
	{ XW_SCENE_FUNERAL_INTERIOR, XW_SCENE_PILOT_CAPTURED, XW_SCENE_PILOT_RESCUED,
	  XW_SCENE_TOUR_RETURN_BWING },
	{ XW_SCENE_FUNERAL_INTERIOR, XW_SCENE_PILOT_CAPTURED, XW_SCENE_PILOT_RESCUED,
	  XW_SCENE_TOUR_RETURN_AWING },
	{ XW_SCENE_FUNERAL_INTERIOR, XW_SCENE_PILOT_CAPTURED, XW_SCENE_PILOT_RESCUED, XW_SCENE_TOUR_RETURN_AWING }
};

// GLOBAL: XW 0x4DC668
const int16_t g_tourCutsceneScenes[SHIPEXT_TOUR_CUTSCENE_COUNT] = { XW_SCENE_EMPIRE_ASSAULT_1,
																	XW_SCENE_DESTROY_STAR_DESTROYER_1,
																	XW_SCENE_SEND_PLANS_1,
																	XW_SCENE_RECOVER_PLANS_1,
																	XW_SCENE_DEATH_STAR_COMPLETED,
																	XW_SCENE_DEATH_STAR_FIRE_1,
																	XW_SCENE_DEATH_STAR_DESTRUCTION,
																	XW_SCENE_YAVIN_DEPARTURE_1,
																	XW_SCENE_GHORIN_1,
																	XW_SCENE_IMPERIAL_DRYDOCK_1,
																	XW_SCENE_RAMMING_ATTACK_1,
																	XW_SCENE_BWING_ARRIVAL_1,
																	XW_SCENE_IMPERIAL_PROBES_1,
																	XW_SCENE_HOTH_1 };

// GLOBAL: XW 0x4DC684
int16_t g_combatSelectedShip = SHIPEXT_DEFAULT_COMBAT_SELECTION;

// GLOBAL: XW 0x4DC688
int16_t g_pendingTourCutscene = XW_SCENE_TOUR_DESK;

// GLOBAL: XW 0x4DC68C
int16_t g_pendingMedalScene = XW_SCENE_TOUR_DESK;

// GLOBAL: XW 0x4DC690
int16_t g_postAwardScene = XW_SCENE_TOUR_DESK;

// GLOBAL: XW 0x4DC694
int16_t g_pendingMedalIndex = SHIPEXT_DEFAULT_MEDAL_INDEX;

// GLOBAL: XW 0x4DC698
int16_t g_availableShips[SHIPEXT_SHIP_COUNT] = { 1, 1, 1, 0, 0, 0 };

// GLOBAL: XW 0x4DC6A8
int16_t g_availableTours[SHIPEXT_TOUR_COUNT] = { 1, 1, 1, 0, 0, 0, 0, 0 };

// GLOBAL: XW 0x5667C8
int16_t g_lastBriefedTourOperation = 0;

// GLOBAL: XW 0x5667E8
int16_t g_combatSelectionIsTour = 0;

// GLOBAL: XW 0x5667EC
int16_t g_trainingSelectedShip = 0;

// GLOBAL: XW 0x5667F0
int16_t g_trainingSelectedLevel = 0;

// GLOBAL: XW 0x5667F8
int16_t g_combatMissionCursors[SHIPEXT_COMBAT_SELECTION_COUNT] = { 0 };

// GLOBAL: XW 0x566814
int16_t g_shellMissionOutcome = 0;

// GLOBAL: XW 0x566818
int16_t g_shellMissionShip = 0;

// GLOBAL: XW 0x56681C
int16_t g_combatSelectedSourceIndex = 0;

// GLOBAL: XW 0x4CF8A0
uint8_t g_tourOperationCounts[SHIPEXT_TOUR_COUNT] = { SHIPEXT_TOUR_1_OPERATION_COUNT,
													  SHIPEXT_TOUR_2_OPERATION_COUNT,
													  SHIPEXT_TOUR_3_OPERATION_COUNT,
													  SHIPEXT_TOUR_4_OPERATION_COUNT,
													  SHIPEXT_TOUR_5_OPERATION_COUNT,
													  0,
													  0,
													  0 };

// FUNCTION: XW 0x45DD80
int16_t shipext_Load_Pilot(const char* filename, int unused) {
	LandruFile* pilotFile;
	(void)unused;
	pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, filename, "rb");
	if (pilotFile != NULL) {
		register_ReadPilotRecord(pilotFile, &g_RegisterPilotData);
		xfile_Close_File(pilotFile);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x45DDC0
void shipext_Revive_Pilot(const char* filename, int16_t mode) {
	LandruFile* stream;
	if (mode == SHIPEXT_CREATE_PILOT) {
		memset(&g_RegisterPilotData, 0, sizeof(g_RegisterPilotData));
		memset(g_RegisterPilotData.tourReplayUnlockMission, SHIPEXT_PILOT_UNSET_BYTE,
			   sizeof(g_RegisterPilotData.tourReplayUnlockMission));
	} else {
		uint16_t tourIndex;
		g_RegisterPilotData.lost_status = 0;
		for (tourIndex = 0; tourIndex < SHIPEXT_TOUR_COUNT; ++tourIndex) {
			if (g_RegisterPilotData.tour_status[tourIndex] == SHIPEXT_TOUR_STATUS_LOST) {
				g_RegisterPilotData.tour_status[tourIndex] = SHIPEXT_TOUR_STATUS_ACTIVE;
			}
		}
		if (mode == SHIPEXT_REVIVE_WITH_PENALTY) {
			if (g_RegisterPilotData.rank > SHIPEXT_REVIVED_RANK_LIMIT) {
				g_RegisterPilotData.rank = SHIPEXT_REVIVED_RANK_LIMIT;
			}
			g_RegisterPilotData.score = 0;
			g_RegisterPilotData.skillValue = 0;
		}
	}
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, filename, "wb");
	if (stream != NULL) {
		xfile_Write_Data_To_File(stream, &g_RegisterPilotData, sizeof(g_RegisterPilotData));
		xfile_Close_File(stream);
	}
}

// FUNCTION: XW 0x4A6B90
void shipext_ResetMissionSelections(void) {
	memset(g_combatMissionCursors, 0, sizeof(g_combatMissionCursors));
	g_trainingSelectedShip = 0;
	g_trainingSelectedLevel = 0;
	g_combatSelectedShip = 0;
	g_combatSelectionIsTour = 0;
}

// FUNCTION: XW 0x4A6BC0
void shipext_Set_Train_Ship(int16_t shipIndex) { g_trainingSelectedShip = shipIndex; }

// FUNCTION: XW 0x4A6BD0
int16_t shipext_Get_Train_Ship(void) { return g_trainingSelectedShip; }

// FUNCTION: XW 0x4A6BE0
void shipext_Set_Train_Level(int16_t levelIndex) { g_trainingSelectedLevel = levelIndex; }

// FUNCTION: XW 0x4A6BF0
int16_t shipext_Get_Train_Level(void) { return g_trainingSelectedLevel; }

// FUNCTION: XW 0x4A6C00
void shipext_Set_Combat_Ship(int16_t selectionIndex, int16_t isTour) {
	g_combatSelectedShip = selectionIndex;
	g_combatSelectionIsTour = isTour;
}

// FUNCTION: XW 0x4A6C20
int16_t shipext_Get_Combat_Ship(void) { return g_combatSelectedShip; }

// FUNCTION: XW 0x4A6C30
int16_t shipext_Is_Combat_Ship_Tour(void) { return g_combatSelectionIsTour; }

// FUNCTION: XW 0x4A6C40
void shipext_Set_Combat_Mission(int16_t missionIndex) {
	g_combatMissionCursors[g_combatSelectedShip] = missionIndex;
}

// FUNCTION: XW 0x4A6C60
int16_t shipext_Get_Combat_Mission(void) { return g_combatMissionCursors[g_combatSelectedShip]; }

// FUNCTION: XW 0x4A6C70
void shipext_Set_Mission_Ship(int16_t shipIndex) { g_shellMissionShip = shipIndex; }

// FUNCTION: XW 0x4A6C80
int16_t shipext_Get_Mission_Ship(void) { return g_shellMissionShip; }

// FUNCTION: XW 0x4A6C90
int16_t shipext_IsShipAvailable(int16_t shipIndex) { return g_availableShips[shipIndex]; }

// FUNCTION: XW 0x4A6CA0
int16_t shipext_IsTourAvailable(int16_t tourIndex) { return g_availableTours[tourIndex]; }

// FUNCTION: XW 0x4A6CB0
void shipext_Set_Combat_Source_Index(int16_t sourceIndex) { g_combatSelectedSourceIndex = sourceIndex; }

// FUNCTION: XW 0x4A6CC0
int16_t shipext_Get_Combat_Source_Index(void) { return g_combatSelectedSourceIndex; }

// FUNCTION: XW 0x4A6CD0
void shipext_Set_Mission_Outcome(int16_t outcome) { g_shellMissionOutcome = outcome; }

// FUNCTION: XW 0x4A6CE0
int16_t shipext_Get_Mission_Outcome(void) { return g_shellMissionOutcome; }

// FUNCTION: XW 0x4A6CF0
void shipext_Set_Last_Briefed_Tour_Operation(int16_t operationIndex) {
	g_lastBriefedTourOperation = operationIndex;
}

// FUNCTION: XW 0x4A6D00
int16_t shipext_Get_Last_Briefed_Tour_Operation(void) { return g_lastBriefedTourOperation; }

// FUNCTION: XW 0x4A6D10
void shipext_Set_Pending_Tour_Cutscene(int16_t sceneId) { g_pendingTourCutscene = sceneId; }

// FUNCTION: XW 0x4A6D20
int16_t shipext_Get_Pending_Tour_Cutscene(void) { return g_pendingTourCutscene; }

// FUNCTION: XW 0x4A6D30
void shipext_Set_Pending_Medal(int16_t sceneId, int16_t medalIndex) {
	g_pendingMedalScene = sceneId;
	g_pendingMedalIndex = medalIndex;
}

// FUNCTION: XW 0x4A6D50
int16_t shipext_Get_Pending_Medal_Scene(void) { return g_pendingMedalScene; }

// FUNCTION: XW 0x4A6D60
int16_t shipext_Get_Pending_Medal_Index(void) { return g_pendingMedalIndex; }

// FUNCTION: XW 0x4A6D70
void shipext_Set_Post_Award_Scene(int16_t sceneId) { g_postAwardScene = sceneId; }

// FUNCTION: XW 0x4A6D80
int16_t shipext_Get_Post_Award_Scene(void) { return g_postAwardScene; }

// FUNCTION: XW 0x4A6D90
int16_t shipext_Mission_Exit(int16_t missionType, int16_t exitCode, int16_t newMedal,
							 int16_t tourCutsceneIndex) {
	int16_t nextScene = XW_SCENE_COMBAT_SIMULATOR_ROOM;
	if (exitCode >= SHIPEXT_EXIT_MAP && exitCode <= SHIPEXT_EXIT_OPTIONS) {
		if (exitCode == SHIPEXT_EXIT_DAMAGE_CONTROL || exitCode == SHIPEXT_EXIT_BRIEFING) {
			nextScene =
				((exitCode - SHIPEXT_EXIT_MAP) ^ SHIPEXT_EXIT_INFLIGHT_SWAP_MASK) + XW_SCENE_INFLIGHT_MAP;
		} else {
			nextScene = exitCode + (XW_SCENE_INFLIGHT_MAP - SHIPEXT_EXIT_MAP);
		}
	} else {
		shipext_Set_Mission_Outcome(exitCode + 1);
		switch (missionType) {
			case XW_SCENE_FLIGHT_TOUR:
				if (exitCode >= SHIPEXT_EXIT_RESCUED) {
					shipext_Set_Post_Award_Scene(XW_SCENE_DEBRIEF_TOUR);
					if (tourCutsceneIndex != SHIPEXT_TOUR_NO_ENTRY) {
						shipext_Set_Pending_Tour_Cutscene(g_tourCutsceneScenes[tourCutsceneIndex]);
						if (g_tourCutsceneScenes[tourCutsceneIndex] == XW_SCENE_DEATH_STAR_DESTRUCTION) {
							shipext_Set_Post_Award_Scene(XW_SCENE_VICTORY_CREDITS);
							newMedal = SHIPEXT_NO_MEDAL;
						}
					} else if (newMedal != SHIPEXT_NO_MEDAL) {
						shipext_Set_Pending_Tour_Cutscene(XW_SCENE_AWARD_CEREMONY_1);
					} else {
						shipext_Set_Pending_Tour_Cutscene(XW_SCENE_DEBRIEF_TOUR);
					}
					if (newMedal != SHIPEXT_NO_MEDAL) {
						shipext_Set_Pending_Medal(XW_SCENE_AWARD_CEREMONY_1, newMedal);
					} else {
						shipext_Set_Pending_Medal(XW_SCENE_DEBRIEF_TOUR, SHIPEXT_NO_MEDAL);
					}
					if (exitCode == SHIPEXT_EXIT_LAND &&
						(newMedal != SHIPEXT_NO_MEDAL || tourCutsceneIndex != SHIPEXT_TOUR_NO_ENTRY)) {
						if (tourCutsceneIndex != SHIPEXT_TOUR_NO_ENTRY) {
							nextScene = shipext_Get_Pending_Tour_Cutscene();
						} else {
							nextScene = shipext_Get_Pending_Medal_Scene();
						}
						break;
					}
				}
				nextScene = g_tourExitScenes[shipext_Get_Mission_Ship()][exitCode];
				break;
			case XW_SCENE_FLIGHT_COMBAT:
				nextScene = g_combatExitScenes[shipext_Get_Mission_Ship()][exitCode];
				break;
			case XW_SCENE_FLIGHT_PROVING_GROUNDS:
				nextScene = g_trainingExitScenes[shipext_Get_Train_Ship()];
				break;
			case XW_SCENE_FLIGHT_REPLAY:
				if (exitCode == SHIPEXT_EXIT_REPLAY_14 || exitCode == SHIPEXT_EXIT_REPLAY_16) {
					nextScene = XW_SCENE_FILM_ROOM;
				} else {
					nextScene = XW_SCENE_CONCOURSE;
				}
				break;
			case XW_SCENE_FLIGHT_RESUME:
				nextScene = XW_SCENE_CONCOURSE;
				break;
		}
	}
	return nextScene;
}

// FUNCTION: XW 0x4A6F60
void shipext_Get_Mission_Path(char* outPath, const char* missionName, int16_t isMissionData) {
#ifdef XW_MODERN
	if (XwProfile_MissionClassic()) {
#else
	if (shellext_GetClassicMissions() != 0) {
#endif
		strcpy(outPath, ";classic\\");
	} else {
		strcpy(outPath, ";mission\\");
	}
	strcat(outPath, missionName);
	strcat(outPath, isMissionData != 0 ? ".xwi" : ".brf");
}
