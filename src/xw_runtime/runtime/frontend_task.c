#include "xw_runtime/runtime/frontend_task.h"
#include "xw/audio/frontend_audio.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/award_box.h"
#include "xw/frontend/blueprnt.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/combat.h"
#include "xw/frontend/debrief.h"
#include "xw/frontend/filmview.h"
#include "xw/frontend/inflight_ui.h"
#include "xw/frontend/mainmenu.h"
#include "xw/frontend/pilot_log.h"
#include "xw/frontend/register.h"
#include "xw/frontend/scenes/assault.h"
#include "xw/frontend/scenes/awards640.h"
#include "xw/frontend/scenes/b1b640.h"
#include "xw/frontend/scenes/b2_640.h"
#include "xw/frontend/scenes/b3_640.h"
#include "xw/frontend/scenes/b4_640.h"
#include "xw/frontend/scenes/bintro.h"
#include "xw/frontend/scenes/boom.h"
#include "xw/frontend/scenes/brdg1.h"
#include "xw/frontend/scenes/brdg2.h"
#include "xw/frontend/scenes/credits.h"
#include "xw/frontend/scenes/death640.h"
#include "xw/frontend/scenes/dfire.h"
#include "xw/frontend/scenes/dock.h"
#include "xw/frontend/scenes/ds_bay.h"
#include "xw/frontend/scenes/ds_boom.h"
#include "xw/frontend/scenes/ds_done.h"
#include "xw/frontend/scenes/ds_land.h"
#include "xw/frontend/scenes/flet640.h"
#include "xw/frontend/scenes/gov1.h"
#include "xw/frontend/scenes/gov2.h"
#include "xw/frontend/scenes/hangar3.h"
#include "xw/frontend/scenes/hoth.h"
#include "xw/frontend/scenes/intro1.h"
#include "xw/frontend/scenes/intro2.h"
#include "xw/frontend/scenes/leave1.h"
#include "xw/frontend/scenes/leave2.h"
#include "xw/frontend/scenes/logo640.h"
#include "xw/frontend/scenes/med640.h"
#include "xw/frontend/scenes/ossht7.h"
#include "xw/frontend/scenes/outpost.h"
#include "xw/frontend/scenes/plans.h"
#include "xw/frontend/scenes/probe.h"
#include "xw/frontend/scenes/rescue64.h"
#include "xw/frontend/scenes/sabotage.h"
#include "xw/frontend/scenes/sun_shot.h"
#include "xw/frontend/scenes/tarkin.h"
#include "xw/frontend/scenes/title.h"
#include "xw/frontend/scenes/tort640.h"
#include "xw/frontend/scenes/training_transit.h"
#include "xw/frontend/scenes/xlogo.h"
#include "xw/frontend/scenes/yavin1.h"
#include "xw/frontend/scenes/yavin2.h"
#include "xw/frontend/scenes/yavin3.h"
#include "xw/frontend/shell_test.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/tourdesk.h"
#include "xw/frontend/train.h"
#include "xw/frontend/uniform.h"
#include "xw/util/landru_display.h"
#include "xw_dos94/audio/gamesnd.h"
#include "xw_dos94/frontend/dispatch.h"
#include "xw_runtime/runtime/flight_mode.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/storage.h"

#include <aeron/log.h>
#include <landru/error.h>
#include <landru/stream.h>
#include <landru/task.h>
#include <stdio.h>
#include <stdlib.h>

enum { FRONTEND_START_SCENE, FRONTEND_AFTER_SCENE, FRONTEND_CLOSE_SCENE, FRONTEND_AFTER_CLOSE };

typedef struct XwFrontendState {
	XwShellContext context;
	XwShellSceneId currentScene;
	XwShellSceneId nextScene;
	int exitLoop;
	int frontendOnlyExit;
	int phase;
} XwFrontendState;

static XwShellSceneId frontendResult;
static XwFrontendState* active_frontend;

int XwFrontend_IsActive(void) { return active_frontend != NULL; }

static void frontend_end(void* self) {
	XwFrontendState* state = self;
	/* Closing Landru still owns the frontend surfaces, even on a flight return. */
	g_frontendDisplayWndProcMode = 1;
	FrontendAudio_StopAndRelease();
	if (XwStorage_HasInstallation(XW_GAME_VERSION_98))
		xstream_Exit_Stream_Engine(XW_SHELL_STREAM_CHANNEL);
	if (g_shellContext == &state->context)
		shellext_Close_Landru();
	g_shellContext = NULL;
	active_frontend = NULL;
	XwProfile_SelectFrontend(false);
	LandruDisplay_SetLowResolutionMode(0);
	XwPresentation_Invalidate();
	XwPort_RebaseClock();
}

static LandruTaskStepResult frontend_step(void* self) {
	XwFrontendState* state = self;
	if (state->phase == FRONTEND_AFTER_SCENE) {
		state->nextScene = xerror_Get_Landru_Exit();
		if (state->currentScene == XW_SCENE_TOUR_DESK) {
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
		}
		if (state->currentScene == XW_SCENE_BRIEFING_LEGACY_110 ||
			state->currentScene == XW_SCENE_BRIEFING_COMBAT || state->currentScene == XW_SCENE_BRIEFING_TOUR)
			FrontendAudio_StopAndRelease();
		state->phase = FRONTEND_CLOSE_SCENE;
	}
	if (state->phase == FRONTEND_CLOSE_SCENE) {
		if (shellext_GetTransitionsEnabled() == 0)
			state->nextScene = shellext_Convert_Transition(state->nextScene, 1);
		state->phase = FRONTEND_AFTER_CLOSE;
		shellext_Close_Landru_Scene(state->currentScene);
		return LANDRU_TASK_STEP_YIELD;
	}
	if (state->phase == FRONTEND_AFTER_CLOSE) {
		state->currentScene = state->nextScene;
		state->phase = FRONTEND_START_SCENE;
	}
	if ((uint16_t)xerror_Is_Landru_Error() != 0 || state->exitLoop || g_quitRequested) {
		printf("XWing Front End Version 1.4 Alpha\n");

		frontendResult =
			state->frontendOnlyExit || g_quitRequested ? XW_SCENE_EXIT_SHELL : state->currentScene;
		return LANDRU_TASK_STEP_DONE;
	}
	if (XwFlightMode_IsSuspended() && state->currentScene != XW_SCENE_FLIGHT_RESUME &&
		(state->currentScene < XW_SCENE_INFLIGHT_MAP || state->currentScene > XW_SCENE_INFLIGHT_OPTIONS))
		XwFlightMode_DiscardSuspended();
	/* Catalogs are mission content even when opened by the other frontend. */
	switch (state->currentScene) {
		case XW_SCENE_CONCOURSE:
		case XW_SCENE_REGISTER_INITIAL:
		case XW_SCENE_REGISTER_RETURN:
		case XW_SCENE_FILM_ROOM:
			XwFlightMode_ReleaseMission();
			break;
		case XW_SCENE_PROVING_GROUNDS_ROOM:
		case XW_SCENE_COMBAT_SIMULATOR_ROOM:
		case XW_SCENE_TOUR_DESK:
			XwFlightMode_ReleaseMission();
			/* fall through */
		case XW_SCENE_BRIEFING_LEGACY_110:
		case XW_SCENE_BRIEFING_COMBAT:
		case XW_SCENE_BRIEFING_TOUR: {
			char error[256];
			if (!XwFlightMode_PrepareMission(error, sizeof error)) {
				XwFlightMode_ReportError(error);
				state->currentScene = XW_SCENE_CONCOURSE;
			}
			break;
		}
		default:
			break;
	}
	/* Mission selection can change the content installation without changing the frontend. */
	shellext_DetectInstalledContent();
	if (state->currentScene == XW_SCENE_BRIEFING_TOUR &&
		!shipext_IsTourAvailable(g_RegisterShellPilot.current_tour))
		state->currentScene = XW_SCENE_TOUR_DESK;
	FlightDisplay_ClearBackAndAuxiliarySurfaces();
	Aeron_LogInfo("xw.frontend", "Opening scene %d", state->currentScene);
	shellext_Open_Landru_Scene(state->currentScene);
	state->phase = FRONTEND_CLOSE_SCENE;
	if (XwProfile_DosFrontend() && Dos94_OpenScene(state->currentScene, g_shellContext)) {
		state->phase = FRONTEND_AFTER_SCENE;
		XwPort_RebaseClock();
		return LANDRU_TASK_STEP_YIELD;
	}
	if (XwProfile_DosFrontend() && state->currentScene != 2 && state->currentScene != 3 &&
		state->currentScene != 4 && state->currentScene != 350 && state->currentScene != 351) {
		state->exitLoop = 1;
		return LANDRU_TASK_STEP_YIELD;
	}
	switch (state->currentScene) {
		case XW_SCENE_FLIGHT_PROVING_GROUNDS:
		case XW_SCENE_FLIGHT_COMBAT:
		case XW_SCENE_FLIGHT_TOUR:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			state->nextScene = state->currentScene;
			state->frontendOnlyExit = 0;
			state->exitLoop = 1;
			g_flightLoadingReplayFilm = 0;
			break;
		case XW_SCENE_DIAGNOSTIC_MENU:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			ShellTest_Show(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_STARTUP_LOGO:
			Logo640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_TITLE_CRAWL:
		case XW_SCENE_TOUR_TITLE_CRAWL:
			title_Title(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_OPENING:
			Intro1_Opening(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_FLEET_ATTACK:
			Intro2_Attack(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_IMPERIAL_BRIDGE:
			Brdg1_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_REBEL_BRIDGE:
			Brdg2_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_HANGAR_LAUNCH:
		case XW_SCENE_TOUR_RETURN_INDEPENDENCE:
		case XW_SCENE_TOUR_RETURN_AWING:
		case XW_SCENE_TOUR_RETURN_XWING:
		case XW_SCENE_TOUR_RETURN_YWING:
		case XW_SCENE_TOUR_RETURN_BWING:
		case XW_SCENE_TOUR_LAUNCH_BWING:
		case XW_SCENE_TOUR_LAUNCH_XWING:
		case XW_SCENE_TOUR_LAUNCH_YWING:
		case XW_SCENE_TOUR_LAUNCH_AWING:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Hangar3_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_OSSHT7:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Ossht7_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_DOGFIGHT_HRC2:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			B2_640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_DOGFIGHT_HRC3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			B3_640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_DOGFIGHT_HRC4:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			B4_640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_XWING_LOGO:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			XLogo_XLogo(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_CREDITS:
		case XW_SCENE_VICTORY_CREDITS:
			credits_Credits(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_REGISTER_INITIAL:
		case XW_SCENE_REGISTER_RETURN:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			register_Register(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_FORMATION:
		case XW_SCENE_TRAINING_RESULTS_AWING:
		case XW_SCENE_TRAINING_RESULTS_XWING:
		case XW_SCENE_TRAINING_RESULTS_YWING:
		case XW_SCENE_TRAINING_RESULTS_BWING:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			B1b640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_CONCOURSE:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			mainmenu_Main_Menu(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
		case XW_SCENE_TRAINING_HYPERSPACE:
		case XW_SCENE_TRAINING_ARRIVE_FACILITY:
		case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
		case XW_SCENE_COMBAT_HYPERSPACE:
		case XW_SCENE_COMBAT_ARRIVE_FACILITY:
		case XW_SCENE_TOUR_DEPART_INDEPENDENCE:
		case XW_SCENE_TOUR_HYPERSPACE:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			TrainingTransit_PlayDeparture(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_TRAINING_RETURN_SHUTTLE:
		case XW_SCENE_COMBAT_RETURN_SHUTTLE:
		case XW_SCENE_TRAINING_LAUNCH_XWING:
		case XW_SCENE_TRAINING_LAUNCH_YWING:
		case XW_SCENE_TRAINING_LAUNCH_AWING:
		case XW_SCENE_TRAINING_LAUNCH_BWING:
		case XW_SCENE_COMBAT_LAUNCH_XWING:
		case XW_SCENE_COMBAT_LAUNCH_YWING:
		case XW_SCENE_COMBAT_LAUNCH_AWING:
		case XW_SCENE_COMBAT_LAUNCH_BWING:
		case XW_SCENE_TRAINING_RETURN_AWING:
		case XW_SCENE_TRAINING_RETURN_XWING:
		case XW_SCENE_TRAINING_RETURN_YWING:
		case XW_SCENE_TRAINING_RETURN_BWING:
		case XW_SCENE_COMBAT_RETURN_AWING:
		case XW_SCENE_COMBAT_RETURN_XWING:
		case XW_SCENE_COMBAT_RETURN_YWING:
		case XW_SCENE_COMBAT_RETURN_BWING:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			TrainingTransit_PlayFacilityFlight(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_PROVING_GROUNDS_ROOM:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			train_Train(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_COMBAT_SIMULATOR_ROOM:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			combat_Combat(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_TOUR_DESK:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			tourdesk_TourDesk(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
		case XW_SCENE_RESCUE_ARRIVE_SALVATION:
		case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
		case XW_SCENE_RECOVER_PLANS_FLEET:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Flet640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_FILM_ROOM:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			g_flightLoadingReplayFilm = 1;
			filmview_FilmView(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_TECH_ROOM:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			blueprnt_Blueprint(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_BRIEFING_LEGACY_110:
		case XW_SCENE_BRIEFING_COMBAT:
		case XW_SCENE_BRIEFING_TOUR:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			brief_Brief(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
		case XW_SCENE_DEBRIEF_COMBAT:
		case XW_SCENE_DEBRIEF_TOUR:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			debrief_Debrief(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_UNIFORM_FROM_COMBAT_BRIEFING:
		case XW_SCENE_UNIFORM_FROM_TOUR_BRIEFING:
		case XW_SCENE_UNIFORM_FROM_REGISTER:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Uniform_Uniform(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_PILOT_LOG_FROM_COMBAT_BRIEFING:
		case XW_SCENE_PILOT_LOG_FROM_TOUR_BRIEFING:
		case XW_SCENE_PILOT_LOG_FROM_REGISTER:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			PilotLog_Show(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_AWARD_CASE_FROM_COMBAT_BRIEFING:
		case XW_SCENE_AWARD_CASE_FROM_TOUR_BRIEFING:
		case XW_SCENE_AWARD_CASE_FROM_REGISTER:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			AwardBox_AwardBox(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DESTROY_STAR_DESTROYER_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour1s2.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Sabotage_Sabotage(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DESTROY_STAR_DESTROYER_2:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Sabotage_MiddleScene(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DESTROY_STAR_DESTROYER_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Sabotage_Sabotage(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_FIRE_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour3s2.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			DFire_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_FIRE_2:
		case XW_SCENE_DEATH_STAR_FIRE_3:
		case XW_SCENE_DEATH_STAR_FIRE_4:
		case XW_SCENE_DEATH_STAR_FIRE_5:
		case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
		case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3:
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE4:
		case XW_SCENE_DEATH_STAR_COMPLETION_PRISON:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			DFire_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_COMPLETED:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour3s1.wav", 0);
			DsDone_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_HANGAR:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			DsBay_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_VADER_ARRIVAL:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			DsLand_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_TARKIN_SPEECH:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Tarkin_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_DEATH_STAR_DESTRUCTION:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour3s3.wav", 0);
			DsBoom_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_YAVIN_APPROACH:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Yavin1_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_YAVIN_FILM_2:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Yavin2_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_YAVIN_HEADQUARTERS:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Yavin3_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_SEND_PLANS_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour2s1.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Outpost_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_SEND_PLANS_2:
		case XW_SCENE_SEND_PLANS_3:
		case XW_SCENE_SEND_PLANS_4:
		case XW_SCENE_SEND_PLANS_5:
		case XW_SCENE_SEND_PLANS_6:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Outpost_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_RECOVER_PLANS_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour2s2.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Plans_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_RECOVER_PLANS_2:
		case XW_SCENE_RECOVER_PLANS_VADER:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Plans_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_EMPIRE_ASSAULT_1:
			FrontendAudio_PlayFile("XwingCD\\music\\tour1s1.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Assault_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_EMPIRE_ASSAULT_2:
		case XW_SCENE_EMPIRE_ASSAULT_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Assault_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_PILOT_RESCUED:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\rescue.wav", 0);
			Rescue64_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_PILOT_CAPTURED:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\torture.wav", 0);
			Rescue64_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_PILOT_TORTURE:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Tort640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_PILOT_MEDICAL_RECOVERY:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Med640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_FUNERAL_INTERIOR:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\death.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Death640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_FUNERAL_EXTERIOR:
		case XW_SCENE_FUNERAL_PLANET:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Death640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_AWARD_CEREMONY_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\recruits.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Awards640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_AWARD_CEREMONY_2:
		case XW_SCENE_AWARD_CEREMONY_3:
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_1:
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_2:
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Awards640_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_FLIGHT_REPLAY:
		case XW_SCENE_FLIGHT_RESUME:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			state->nextScene = state->currentScene;
			state->frontendOnlyExit = 0;
			state->exitLoop = 1;
			break;
		case XW_SCENE_INFLIGHT_MAP:
		case XW_SCENE_INFLIGHT_BRIEFING:
		case XW_SCENE_INFLIGHT_DAMAGE_CONTROL:
			InflightUI_Show(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_IMPERIAL_DRYDOCK_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s3.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Dock_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_IMPERIAL_DRYDOCK_2:
		case XW_SCENE_IMPERIAL_DRYDOCK_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Dock_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_YAVIN_DEPARTURE_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s1.wav", 0);
			Leave1_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_YAVIN_DEPARTURE_2:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Leave2_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_GHORIN_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s2.wav", 0);
			Gov1_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_GHORIN_2:
		case XW_SCENE_GHORIN_3:
		case XW_SCENE_GHORIN_4:
		case XW_SCENE_GHORIN_5:
		case XW_SCENE_GHORIN_6:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Gov2_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_RAMMING_ATTACK_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s4.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Boom_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_RAMMING_ATTACK_2:
		case XW_SCENE_RAMMING_ATTACK_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Boom_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_HOTH_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour5s3.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Hoth_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_HOTH_2:
		case XW_SCENE_HOTH_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Hoth_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_BWING_ARRIVAL_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour5s1.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			BIntro_BWingArrival(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_BWING_ARRIVAL_2:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			BIntro_BWingArrival(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_IMPERIAL_PROBES_1:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			FrontendAudio_PlayFile("XwingCD\\music\\tour5s2.wav", 0);
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Probe_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_IMPERIAL_PROBES_2:
		case XW_SCENE_IMPERIAL_PROBES_3:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			Probe_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		case XW_SCENE_INTRO_SUNSHOT:
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			SunShot_Play(g_shellContext);
			state->phase = FRONTEND_AFTER_SCENE;
			break;
		default:
			state->exitLoop = 1;
			break;
	}
	XwPort_RebaseClock();
	return LANDRU_TASK_STEP_YIELD;
}

static const LandruTaskVtable frontend_vtable = { frontend_step, frontend_end, NULL, NULL };

void XwFrontend_Begin(XwShellSceneId scene, struct XwLegacyMemoryConfig* memory) {
	XwFrontendState* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&frontend_vtable);
	if (state == NULL)
		abort();
	active_frontend = state;
	XwProfile_SelectFrontend(scene >= XW_SCENE_INFLIGHT_MAP && scene <= XW_SCENE_INFLIGHT_DAMAGE_CONTROL);
	Dos94_gamesnd_game_Set_Front_Sound();
	XwPresentation_Invalidate();
	XwPort_RebaseClock();
	frontendResult = XW_SCENE_EXIT_SHELL;
	g_frontendDisplayWndProcMode = 1;
	g_shellContext = &state->context;
	LandruDisplay_SetLowResolutionMode(XwProfile_DosFrontend());
	state->context.lastScene = XW_SCENE_INITIAL_PREVIOUS_SCENE;
	state->exitLoop = 0;
	state->frontendOnlyExit = 1;
	shellext_Open_Landru(memory);
	if (XwStorage_HasInstallation(XW_GAME_VERSION_98)) {
		lolevel_ImPause();
		xstream_Set_Stream_Tick_Counts(g_installDriveLetter, "\\XwingCD\\music\\tour2s1.wav");
		xstream_ConfigureChannel(XW_SHELL_STREAM_CHANNEL, 0, g_installDriveLetter);
		lolevel_ImResume();
		xstream_Init_Stream_Engine(XW_SHELL_STREAM_CHANNEL, XW_SHELL_STREAM_BUFFER_BYTES,
								   XW_SHELL_STREAM_PREFETCH_BYTES);
	}
	state->currentScene = scene;
	state->nextScene = scene;
	state->phase = FRONTEND_START_SCENE;
}

XwShellSceneId XwFrontend_Result(void) { return frontendResult; }
