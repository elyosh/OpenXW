#include "xw_runtime/runtime/shell_flight_task.h"

#include "xw/flight/fediskio.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/inflight_ui.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/xmain.h"
#include "xw/util/shared.h"

#include "xw/flight/replay/replay.h"
#include "xw_runtime/runtime/flight_mode.h"
#include "xw_runtime/runtime/frontend_task.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/storage/replay_format.h"
#include <landru/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ShellFlightPhase {
	SHELL_FLIGHT_FRONTEND,
	SHELL_FLIGHT_AFTER_FRONTEND,
	SHELL_FLIGHT_AFTER_SIMULATION
} ShellFlightPhase;

typedef struct ShellFlightState {
	ShellFlightPhase phase;
	XwShellSceneId nextScene;
	int16_t missionType;
} ShellFlightState;

static void select_flight(ShellFlightState* state, XwShellSceneId flightScene) {
	uint16_t skillValue;
	int pilotSlot;
	if (flightScene > XW_SCENE_FLIGHT_REPLAY) {
		if (flightScene == XW_SCENE_FLIGHT_RESUME)
			g_flightEntryMode = FLIGHT_ENTRY_RESUME_SAVED;
	} else if (flightScene == XW_SCENE_FLIGHT_REPLAY) {
		g_flightEntryMode = FLIGHT_ENTRY_REPLAY_VIEWER;
		state->missionType = XW_SCENE_FLIGHT_REPLAY;
	} else if (flightScene >= XW_SCENE_FLIGHT_PROVING_GROUNDS && flightScene <= XW_SCENE_FLIGHT_TOUR) {
		if (flightScene == XW_SCENE_FLIGHT_PROVING_GROUNDS) {
			g_missionRuntimeState.provingGroundsSelectedCraft = shipext_Get_Train_Ship() + 1;
			skillValue = g_RegisterShellPilot.skillValue;
			for (pilotSlot = 0; pilotSlot < XW_FLIGHT_PILOT_SLOT_COUNT; ++pilotSlot) {
				if (pilotSlot != 0)
					g_PilotSlotNames[pilotSlot][0] = 0;
				else {
					g_pilotSlotFlightGroupIndices[0] = 0;
					g_pilotSlotCraftIndices[0] = 0;
					g_pilotSlotSkillValues[0] = skillValue;
					strcpy(g_PilotSlotNames[0], g_RegisterShellPilot.name);
				}
			}
		}
		g_missionRuntimeState.mode = flightScene - XW_SCENE_FLIGHT_PROVING_GROUNDS;
		if (flightScene == XW_SCENE_FLIGHT_TOUR)
			g_missionRuntimeState.mode = FEDISKIO_MISSION_MODE_TOUR;
		if (flightScene == XW_SCENE_FLIGHT_COMBAT && shipext_Is_Combat_Ship_Tour() != 0)
			g_missionRuntimeState.mode = FEDISKIO_MISSION_MODE_COMBAT_TOUR;
		shipext_Get_Mission_Path(g_currentMissionFile, g_shellMissionName, 1);
		g_flightEntryMode = FLIGHT_ENTRY_NEW_MISSION;
		g_inflightMapPreferencesInitialized = 0;
		state->missionType = flightScene;
	}
}

static LandruTaskStepResult shell_flight_step(void* self) {
	ShellFlightState* state = self;
	if (state->phase == SHELL_FLIGHT_AFTER_SIMULATION) {
		g_shellFlightPhaseState = 1;
		nullsub_SharedNoOp();
		state->nextScene =
			shipext_Mission_Exit(state->missionType, g_missionRuntimeState.flightExitReason,
								 g_missionRuntimeState.newMedal, g_missionRuntimeState.tourCutsceneIndex);
		if (state->nextScene == XW_SCENE_EXIT_SHELL)
			return LANDRU_TASK_STEP_DONE;
		state->phase = SHELL_FLIGHT_FRONTEND;
	}
	if (state->phase == SHELL_FLIGHT_FRONTEND) {
		if (shellext_GetTransitionsEnabled() == 0)
			state->nextScene = shellext_Convert_Transition(state->nextScene, 0);
		ShellFlight_ResetSceneState(state->nextScene);
		state->phase = SHELL_FLIGHT_AFTER_FRONTEND;
		xmain_main(state->nextScene, &g_legacyMemoryConfig);
		return LANDRU_TASK_STEP_YIELD;
	}
	XwShellSceneId flightScene = XwFrontend_Result();
	if (flightScene == XW_SCENE_EXIT_SHELL)
		return LANDRU_TASK_STEP_DONE;
	char error[256];
	if (flightScene == XW_SCENE_FLIGHT_REPLAY || flightScene == XW_SCENE_FLIGHT_RESUME) {
		char path[REPLAY_CLIP_PATH_CAPACITY];
		XwReplayMetadata metadata;
		bool film = flightScene == XW_SCENE_FLIGHT_REPLAY;
		if (film) {
			XwFlightMode_ReleaseMission();
			snprintf(path, sizeof path, "%s.clp", g_ReplayClipName);
		} else
			snprintf(path, sizeof path, "+savegame.rpy");
		if (!XwReplayFormat_CheckFile(path, film ? XW_REPLAY_FILM : XW_REPLAY_CHECKPOINT,
									  XwProfile_MissionFlight()->version, &metadata)) {
			XwFlightMode_ReportError(XwReplayFormat_Error());
			state->nextScene = film ? XW_SCENE_FILM_ROOM : XW_SCENE_CONCOURSE;
			state->phase = SHELL_FLIGHT_FRONTEND;
			return LANDRU_TASK_STEP_YIELD;
		}
		if (!XwProfile_PinRequestedMission(error, sizeof error) ||
			!XwProfile_RestoreMissionContent(metadata.classic) ||
			!XwProfile_RestoreMissionTiming(metadata.update_rate)) {
			XwFlightMode_ReportError("The recording uses a different mission content selection.");
			state->nextScene = film ? XW_SCENE_FILM_ROOM : XW_SCENE_CONCOURSE;
			state->phase = SHELL_FLIGHT_FRONTEND;
			return LANDRU_TASK_STEP_YIELD;
		}
	}
	if (!XwFlightMode_PrepareMission(error, sizeof error)) {
		XwFlightMode_ReportError(error);
		state->nextScene = XW_SCENE_CONCOURSE;
		state->phase = SHELL_FLIGHT_FRONTEND;
		return LANDRU_TASK_STEP_YIELD;
	}
	select_flight(state, flightScene);
	ShellFlight_ResetLegacyBlock();
	g_shellFlightPhaseState = 0;
	state->phase = SHELL_FLIGHT_AFTER_SIMULATION;
	Xw_simulator(g_flightEntryMode);
	return LANDRU_TASK_STEP_YIELD;
}

static void shell_flight_end(void* self) {
	ShellFlightState* state = self;
	if (state->phase == SHELL_FLIGHT_AFTER_SIMULATION)
		g_shellFlightPhaseState = 1;
	XwFlightMode_ReleaseMission();
	ShellFlight_Shutdown();
}

static const LandruTaskVtable shell_flight_vtable = { shell_flight_step, shell_flight_end, NULL, NULL };

void XwShellFlight_Begin(void) {
	if (ShellFlight_AllocateLegacyMemory(XW_LEGACY_MEMORY_SELECTOR) == 0) {
		ShellFlight_Shutdown();
		XwPort_Fail("Cannot allocate shell memory");
		return;
	}
	ShellFlightState* state = landru_task_push(&shell_flight_vtable);
	if (state == NULL) {
		ShellFlight_Shutdown();
		XwPort_Fail("Cannot start shell task");
		return;
	}
	*state = (ShellFlightState) { SHELL_FLIGHT_FRONTEND, XW_SCENE_STARTUP_LOGO, XW_SCENE_EXIT_SHELL };
	ShellFlight_Initialize();
	if (XwPort_SkipIntro())
		state->nextScene = XW_SCENE_REGISTER_INITIAL;
	XwPort_RebaseClock();
}
