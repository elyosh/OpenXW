#include "xw/frontend/shell_test.h"

#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw_runtime/runtime/shell_test_task.h"

#include <landru/error.h>
#include <landru/fade.h>
#include <landru/pal.h>
#include <landru/view.h>
#include <stdlib.h>

// GLOBAL: XW 0x4D9CF4
char g_shellTestOutcomeLabels[SHELL_TEST_OUTCOME_LABEL_BYTES] = "Land\0Rescue\0Capture\0Death\0";

// GLOBAL: XW 0x4D9D10
char g_shellTestSceneLabels[SHELL_TEST_SCENE_LABEL_BYTES] =
	"Mission\0Empire Assault\0Destroy Star Destroyer\0Send Plans\0Recover Plans\0"
	"Complete Death Star\0Death Star Fire\0Destroy Death Star\0";

// FUNCTION: XW 0x47DFA0
XwShellSceneResult ShellTest_Show(struct XwShellContext* shell) {
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_FLIGHT_PROVING_GROUNDS:
			xview_Set_View_Update_Function(ShellTest_UpdateProvingGroundsExit);
			break;
		case XW_SCENE_FLIGHT_COMBAT:
			xview_Set_View_Update_Function(ShellTest_UpdateCombatExit);
			break;
		case XW_SCENE_FLIGHT_TOUR:
			xview_Set_View_Update_Function(ShellTest_UpdateMissionOutcome);
			break;
		default:
			xview_Set_View_Update_Function(ShellTest_UpdateSceneSelection);
			break;
	}
	xpal_Set_Dest_Palette(shell->standardPalette);
	xfade_Start_Full_Fade(FADE_WIPE_INSTANT, FADE_COLOR_TWO_PHASE, 1, 0, 1);
#ifdef XW_MODERN
	XwShellTest_RunView();
#else
	j_xviewadd_Handle_View();
	xview_Clear_View_Update_Function();
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x47E010
void ShellTest_UpdateProvingGroundsExit(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;

	(void)unusedTime;
	shipext_Set_Mission_Outcome(SHIPEXT_MISSION_OUTCOME_LAND);
	switch (shipext_Get_Train_Ship()) {
		case SHIPEXT_SHIP_AWING:
			nextScene = XW_SCENE_TRAINING_RESULTS_AWING;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
		case SHIPEXT_SHIP_XWING:
			nextScene = XW_SCENE_TRAINING_RESULTS_XWING;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
		case SHIPEXT_SHIP_YWING:
			nextScene = XW_SCENE_TRAINING_RESULTS_YWING;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
		case SHIPEXT_SHIP_BWING:
			nextScene = XW_SCENE_TRAINING_RESULTS_BWING;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
#ifdef XW_MODERN
		default:
			nextScene = XW_SCENE_PROVING_GROUNDS_ROOM;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection, 1) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x47E0A0
void ShellTest_UpdateCombatExit(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;

	(void)unusedTime;
	shipext_Set_Mission_Outcome(SHIPEXT_MISSION_OUTCOME_LAND);
	switch (shipext_Get_Train_Ship()) {
		case SHIPEXT_SHIP_AWING:
			nextScene = XW_SCENE_COMBAT_RETURN_AWING;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
		case SHIPEXT_SHIP_XWING:
			nextScene = XW_SCENE_COMBAT_RETURN_XWING;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
		case SHIPEXT_SHIP_YWING:
			nextScene = XW_SCENE_COMBAT_RETURN_YWING;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
		case SHIPEXT_SHIP_BWING:
			nextScene = XW_SCENE_COMBAT_RETURN_BWING;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
#ifdef XW_MODERN
		default:
			nextScene = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection, 1) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x47E130
void ShellTest_UpdateMissionOutcome(int unusedTime) {
	int16_t choice;
	int16_t outcome;
	int16_t nextScene;
	int16_t nextSection;
	int16_t exitScene;
	(void)unusedTime;
	if (!XwShellTest_ReadOutcomeChoice(g_shellTestOutcomeLabels, &choice))
		return;
	switch (choice) {
		case SHELL_TEST_OUTCOME_EXIT:
			outcome = SHIPEXT_MISSION_OUTCOME_NONE;
			break;
		case SHELL_TEST_OUTCOME_LAND:
			outcome = SHIPEXT_MISSION_OUTCOME_LAND;
			break;
		case SHELL_TEST_OUTCOME_RESCUE:
			outcome = SHIPEXT_MISSION_OUTCOME_RESCUED;
			break;
		case SHELL_TEST_OUTCOME_CAPTURE:
			outcome = SHIPEXT_MISSION_OUTCOME_CAPTURED;
			break;
		case SHELL_TEST_OUTCOME_DEATH:
			outcome = SHIPEXT_MISSION_OUTCOME_DEAD;
			break;
#ifdef XW_MODERN
		default:
			outcome = SHIPEXT_MISSION_OUTCOME_NONE;
			break;
#endif
	}
	shipext_Set_Mission_Outcome(outcome);
	switch (shipext_Get_Mission_Ship()) {
		case SHIPEXT_SHIP_AWING:
			switch (outcome) {
				case SHIPEXT_MISSION_OUTCOME_NONE:
					nextScene = XW_SCENE_TOUR_DESK;
					break;
				case SHIPEXT_MISSION_OUTCOME_DEAD:
					nextScene = XW_SCENE_TOUR_RETURN_AWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_CAPTURED:
					nextScene = XW_SCENE_TOUR_RETURN_AWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_RESCUED:
					nextScene = XW_SCENE_TOUR_RETURN_AWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_LAND:
					nextScene = XW_SCENE_TOUR_RETURN_AWING;
					break;
				default:
#ifdef XW_MODERN
					nextScene = XW_SCENE_TOUR_DESK;
#endif
					break;
			}
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHIPEXT_SHIP_XWING:
			switch (outcome) {
				case SHIPEXT_MISSION_OUTCOME_NONE:
					nextScene = XW_SCENE_TOUR_DESK;
					break;
				case SHIPEXT_MISSION_OUTCOME_DEAD:
					nextScene = XW_SCENE_TOUR_RETURN_XWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_CAPTURED:
					nextScene = XW_SCENE_TOUR_RETURN_XWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_RESCUED:
					nextScene = XW_SCENE_TOUR_RETURN_XWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_LAND:
					nextScene = XW_SCENE_TOUR_RETURN_XWING;
					break;
				default:
#ifdef XW_MODERN
					nextScene = XW_SCENE_TOUR_DESK;
#endif
					break;
			}
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHIPEXT_SHIP_YWING:
			switch (outcome) {
				case SHIPEXT_MISSION_OUTCOME_NONE:
					nextScene = XW_SCENE_TOUR_DESK;
					break;
				case SHIPEXT_MISSION_OUTCOME_DEAD:
					nextScene = XW_SCENE_TOUR_RETURN_YWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_CAPTURED:
					nextScene = XW_SCENE_TOUR_RETURN_YWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_RESCUED:
					nextScene = XW_SCENE_TOUR_RETURN_YWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_LAND:
					nextScene = XW_SCENE_TOUR_RETURN_YWING;
					break;
				default:
#ifdef XW_MODERN
					nextScene = XW_SCENE_TOUR_DESK;
#endif
					break;
			}
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHIPEXT_SHIP_BWING:
			switch (outcome) {
				case SHIPEXT_MISSION_OUTCOME_NONE:
					nextScene = XW_SCENE_TOUR_DESK;
					break;
				case SHIPEXT_MISSION_OUTCOME_DEAD:
					nextScene = XW_SCENE_TOUR_RETURN_BWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_CAPTURED:
					nextScene = XW_SCENE_TOUR_RETURN_BWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_RESCUED:
					nextScene = XW_SCENE_TOUR_RETURN_BWING;
					break;
				case SHIPEXT_MISSION_OUTCOME_LAND:
					nextScene = XW_SCENE_TOUR_RETURN_BWING;
					break;
				default:
#ifdef XW_MODERN
					nextScene = XW_SCENE_TOUR_DESK;
#endif
					break;
			}
			nextSection = XW_SCENE_TOUR_DESK;
			break;
#ifdef XW_MODERN
		default:
			nextScene = XW_SCENE_TOUR_DESK;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection, 1) != 0)
		xerror_Set_Landru_Exit(exitScene);
}

// FUNCTION: XW 0x47E2E0
void ShellTest_UpdateSceneSelection(int unusedTime) {
	int16_t choice;
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;
	(void)unusedTime;
	if (!XwShellTest_ReadSceneChoice(g_shellTestSceneLabels, &choice)) {
		return;
	}
	switch (choice) {
		case SHELL_TEST_CHOICE_EXIT:
			nextScene = XW_SCENE_TOUR_DESK;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_MISSION:
			nextScene = XW_SCENE_TOUR_DEPART_INDEPENDENCE;
			nextSection = XW_SCENE_BRIEFING_TOUR;
			break;
		case SHELL_TEST_CHOICE_EMPIRE_ASSAULT:
			nextScene = XW_SCENE_EMPIRE_ASSAULT_1;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_DESTROY_STAR_DESTROYER:
			nextScene = XW_SCENE_DESTROY_STAR_DESTROYER_1;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_SEND_PLANS:
			nextScene = XW_SCENE_SEND_PLANS_1;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_RECOVER_PLANS:
			nextScene = XW_SCENE_RECOVER_PLANS_1;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_DEATH_STAR_COMPLETED:
			nextScene = XW_SCENE_DEATH_STAR_COMPLETED;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_DEATH_STAR_FIRE:
			nextScene = XW_SCENE_DEATH_STAR_FIRE_1;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		case SHELL_TEST_CHOICE_DEATH_STAR_DESTRUCTION:
			nextScene = XW_SCENE_DEATH_STAR_DESTRUCTION;
			nextSection = XW_SCENE_TOUR_DESK;
			break;
		default:
#ifdef XW_MODERN
			nextScene = XW_SCENE_TOUR_DESK;
			nextSection = XW_SCENE_TOUR_DESK;
#else
			nextScene = exitScene;
			nextSection = exitScene;
#endif
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection, 1) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}
