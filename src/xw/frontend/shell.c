#include "xw/frontend/shell.h"
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
#include "xw/frontend/inflight_options.h"
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

#ifdef XW_MODERN
#include "xw_runtime/runtime/frontend_task.h"
#endif

#include <landru/error.h>
#include <landru/stream.h>
#include <stdio.h>

// GLOBAL: XW 0x4DD7A8
int16_t g_installDriveLetter = -1;

// GLOBAL: XW 0x5B8AF0
char g_shellMissionName[XW_SHELL_MISSION_NAME_CAPACITY] = { 0 };

// GLOBAL: XW 0x5B8B40
XwShellContext* g_shellContext = NULL;

// FUNCTION: XW 0x4A57D0
XwShellSceneResult shell_Shell(XwShellSceneId scene, struct XwLegacyMemoryConfig* memory) {
#ifdef XW_MODERN
	XwFrontend_Begin(scene, memory);
#else
	int exitLoop;
	uint16_t landruError;
	XwShellSceneId currentScene;
	XwShellSceneId nextScene;
	int frontendOnlyExit;
	XwShellContext shellContext;
	g_frontendDisplayWndProcMode = 1;
	g_shellContext = &shellContext;
	shellContext.lastScene = XW_SCENE_INITIAL_PREVIOUS_SCENE;
	exitLoop = 0;
	frontendOnlyExit = 1;
	shellext_Open_Landru(memory);
	lolevel_ImPause();
	xstream_Set_Stream_Tick_Counts(g_installDriveLetter, "\\XwingCD\\music\\tour2s1.wav");
	xstream_ConfigureChannel(XW_SHELL_STREAM_CHANNEL, 0, g_installDriveLetter);
	lolevel_ImResume();
	xstream_Init_Stream_Engine(XW_SHELL_STREAM_CHANNEL, XW_SHELL_STREAM_BUFFER_BYTES,
							   XW_SHELL_STREAM_PREFETCH_BYTES);
	landruError = (uint16_t)xerror_Is_Landru_Error();
	currentScene = scene;
	nextScene = scene;
	while (!landruError) {
		if (exitLoop != 0 || g_quitRequested != 0)
			break;
		FlightDisplay_ClearBackAndAuxiliarySurfaces();
		shellext_Open_Landru_Scene(currentScene);
		switch (currentScene) {
			case XW_SCENE_FLIGHT_PROVING_GROUNDS:
			case XW_SCENE_FLIGHT_COMBAT:
			case XW_SCENE_FLIGHT_TOUR:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = currentScene;
				frontendOnlyExit = 0;
				exitLoop = 1;
				g_flightLoadingReplayFilm = 0;
				g_frontendDisplayWndProcMode = 0;
				break;
			case XW_SCENE_FLIGHT_REPLAY:
			case XW_SCENE_FLIGHT_RESUME:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = currentScene;
				frontendOnlyExit = 0;
				exitLoop = 1;
				g_frontendDisplayWndProcMode = 0;
				break;
			case XW_SCENE_DIAGNOSTIC_MENU:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = ShellTest_Show(g_shellContext);
				break;
			case XW_SCENE_STARTUP_LOGO:
				nextScene = Logo640_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_TITLE_CRAWL:
			case XW_SCENE_TOUR_TITLE_CRAWL:
				nextScene = title_Title(g_shellContext);
				break;
			case XW_SCENE_INTRO_OPENING:
				nextScene = Intro1_Opening(g_shellContext);
				break;
			case XW_SCENE_INTRO_FLEET_ATTACK:
				nextScene = Intro2_Attack(g_shellContext);
				break;
			case XW_SCENE_INTRO_IMPERIAL_BRIDGE:
				nextScene = Brdg1_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_REBEL_BRIDGE:
				nextScene = Brdg2_Play(g_shellContext);
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
				nextScene = Hangar3_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_OSSHT7:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Ossht7_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_SUNSHOT:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = SunShot_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_FORMATION:
			case XW_SCENE_TRAINING_RESULTS_AWING:
			case XW_SCENE_TRAINING_RESULTS_XWING:
			case XW_SCENE_TRAINING_RESULTS_YWING:
			case XW_SCENE_TRAINING_RESULTS_BWING:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = B1b640_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_DOGFIGHT_HRC2:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = B2_640_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_DOGFIGHT_HRC3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = B3_640_Play(g_shellContext);
				break;
			case XW_SCENE_INTRO_DOGFIGHT_HRC4:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = B4_640_Play(g_shellContext);
				break;
			case XW_SCENE_XWING_LOGO:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = XLogo_XLogo(g_shellContext);
				break;
			case XW_SCENE_INTRO_CREDITS:
			case XW_SCENE_VICTORY_CREDITS:
				nextScene = credits_Credits(g_shellContext);
				break;
			case XW_SCENE_REGISTER_INITIAL:
			case XW_SCENE_REGISTER_RETURN:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = register_Register(g_shellContext);
				break;
			case XW_SCENE_CONCOURSE:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = mainmenu_Main_Menu(g_shellContext);
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
				nextScene = TrainingTransit_PlayDeparture(g_shellContext);
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
				nextScene = TrainingTransit_PlayFacilityFlight(g_shellContext);
				break;
			case XW_SCENE_PROVING_GROUNDS_ROOM:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = train_Train(g_shellContext);
				break;
			case XW_SCENE_COMBAT_SIMULATOR_ROOM:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = combat_Combat(g_shellContext);
				break;
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
			case XW_SCENE_RESCUE_ARRIVE_SALVATION:
			case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
			case XW_SCENE_RECOVER_PLANS_FLEET:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Flet640_Play(g_shellContext);
				break;
			case XW_SCENE_BRIEFING_LEGACY_110:
			case XW_SCENE_BRIEFING_COMBAT:
			case XW_SCENE_BRIEFING_TOUR:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = brief_Brief(g_shellContext);
				FrontendAudio_StopAndRelease();
				break;
			case XW_SCENE_DEBRIEF_PROVING_GROUNDS:
			case XW_SCENE_DEBRIEF_COMBAT:
			case XW_SCENE_DEBRIEF_TOUR:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = debrief_Debrief(g_shellContext);
				break;
			case XW_SCENE_UNIFORM_FROM_COMBAT_BRIEFING:
			case XW_SCENE_UNIFORM_FROM_TOUR_BRIEFING:
			case XW_SCENE_UNIFORM_FROM_REGISTER:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Uniform_Uniform(g_shellContext);
				break;
			case XW_SCENE_AWARD_CASE_FROM_COMBAT_BRIEFING:
			case XW_SCENE_AWARD_CASE_FROM_TOUR_BRIEFING:
			case XW_SCENE_AWARD_CASE_FROM_REGISTER:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = AwardBox_AwardBox(g_shellContext);
				break;
			case XW_SCENE_PILOT_LOG_FROM_COMBAT_BRIEFING:
			case XW_SCENE_PILOT_LOG_FROM_TOUR_BRIEFING:
			case XW_SCENE_PILOT_LOG_FROM_REGISTER:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = PilotLog_Show(g_shellContext);
				break;
			case XW_SCENE_TOUR_DESK:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = tourdesk_TourDesk(g_shellContext);
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				break;
			case XW_SCENE_FILM_ROOM:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				g_flightLoadingReplayFilm = 1;
				nextScene = filmview_FilmView(g_shellContext);
				break;
			case XW_SCENE_TECH_ROOM:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = blueprnt_Blueprint(g_shellContext);
				break;
			case XW_SCENE_EMPIRE_ASSAULT_1:
				FrontendAudio_PlayFile("XwingCD\\music\\tour1s1.wav", 0);
				/* fall through */
			case XW_SCENE_EMPIRE_ASSAULT_2:
			case XW_SCENE_EMPIRE_ASSAULT_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Assault_Play(g_shellContext);
				break;
			case XW_SCENE_DEATH_STAR_COMPLETED:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour3s1.wav", 0);
				nextScene = DsDone_Play(g_shellContext);
				break;
			case XW_SCENE_DEATH_STAR_HANGAR:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = DsBay_Play(g_shellContext);
				break;
			case XW_SCENE_DEATH_STAR_VADER_ARRIVAL:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = DsLand_Play(g_shellContext);
				break;
			case XW_SCENE_DEATH_STAR_TARKIN_SPEECH:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Tarkin_Play(g_shellContext);
				break;
			case XW_SCENE_DEATH_STAR_FIRE_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour3s2.wav", 0);
				/* fall through */
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
				nextScene = DFire_Play(g_shellContext);
				break;
			case XW_SCENE_DEATH_STAR_DESTRUCTION:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour3s3.wav", 0);
				nextScene = DsBoom_Play(g_shellContext);
				break;
			case XW_SCENE_YAVIN_APPROACH:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Yavin1_Play(g_shellContext);
				break;
			case XW_SCENE_YAVIN_FILM_2:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Yavin2_Play(g_shellContext);
				break;
			case XW_SCENE_YAVIN_HEADQUARTERS:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Yavin3_Play(g_shellContext);
				break;
			case XW_SCENE_DESTROY_STAR_DESTROYER_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour1s2.wav", 0);
				/* fall through */
			case XW_SCENE_DESTROY_STAR_DESTROYER_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Sabotage_Sabotage(g_shellContext);
				break;
			case XW_SCENE_DESTROY_STAR_DESTROYER_2:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Sabotage_MiddleScene(g_shellContext);
				break;
			case XW_SCENE_SEND_PLANS_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour2s1.wav", 0);
				/* fall through */
			case XW_SCENE_SEND_PLANS_2:
			case XW_SCENE_SEND_PLANS_3:
			case XW_SCENE_SEND_PLANS_4:
			case XW_SCENE_SEND_PLANS_5:
			case XW_SCENE_SEND_PLANS_6:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Outpost_Play(g_shellContext);
				break;
			case XW_SCENE_RECOVER_PLANS_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour2s2.wav", 0);
				/* fall through */
			case XW_SCENE_RECOVER_PLANS_2:
			case XW_SCENE_RECOVER_PLANS_VADER:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Plans_Play(g_shellContext);
				break;
			case XW_SCENE_PILOT_RESCUED:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\rescue.wav", 0);
				nextScene = Rescue64_Play(g_shellContext);
				break;
			case XW_SCENE_PILOT_CAPTURED:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\torture.wav", 0);
				nextScene = Rescue64_Play(g_shellContext);
				break;
			case XW_SCENE_PILOT_TORTURE:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Tort640_Play(g_shellContext);
				break;
			case XW_SCENE_PILOT_MEDICAL_RECOVERY:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Med640_Play(g_shellContext);
				break;
			case XW_SCENE_FUNERAL_INTERIOR:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\death.wav", 0);
				/* fall through */
			case XW_SCENE_FUNERAL_EXTERIOR:
			case XW_SCENE_FUNERAL_PLANET:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Death640_Play(g_shellContext);
				break;
			case XW_SCENE_AWARD_CEREMONY_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\recruits.wav", 0);
				/* fall through */
			case XW_SCENE_AWARD_CEREMONY_2:
			case XW_SCENE_AWARD_CEREMONY_3:
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_1:
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_2:
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Awards640_Play(g_shellContext);
				break;
			case XW_SCENE_INFLIGHT_MAP:
			case XW_SCENE_INFLIGHT_BRIEFING:
			case XW_SCENE_INFLIGHT_DAMAGE_CONTROL:
				nextScene = InflightUI_Show(g_shellContext);
				break;
			case XW_SCENE_INFLIGHT_OPTIONS:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = InflightOptions_Show(g_shellContext);
				break;
			case XW_SCENE_YAVIN_DEPARTURE_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour4s1.wav", 0);
				nextScene = Leave1_Play(g_shellContext);
				break;
			case XW_SCENE_YAVIN_DEPARTURE_2:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Leave2_Play(g_shellContext);
				break;
			case XW_SCENE_RAMMING_ATTACK_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour4s4.wav", 0);
				/* fall through */
			case XW_SCENE_RAMMING_ATTACK_2:
			case XW_SCENE_RAMMING_ATTACK_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Boom_Play(g_shellContext);
				break;
			case XW_SCENE_IMPERIAL_DRYDOCK_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour4s3.wav", 0);
				/* fall through */
			case XW_SCENE_IMPERIAL_DRYDOCK_2:
			case XW_SCENE_IMPERIAL_DRYDOCK_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Dock_Play(g_shellContext);
				break;
			case XW_SCENE_GHORIN_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour4s2.wav", 0);
				nextScene = Gov1_Play(g_shellContext);
				break;
			case XW_SCENE_GHORIN_2:
			case XW_SCENE_GHORIN_3:
			case XW_SCENE_GHORIN_4:
			case XW_SCENE_GHORIN_5:
			case XW_SCENE_GHORIN_6:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Gov2_Play(g_shellContext);
				break;
			case XW_SCENE_BWING_ARRIVAL_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour5s1.wav", 0);
				/* fall through */
			case XW_SCENE_BWING_ARRIVAL_2:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = BIntro_BWingArrival(g_shellContext);
				break;
			case XW_SCENE_IMPERIAL_PROBES_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour5s2.wav", 0);
				/* fall through */
			case XW_SCENE_IMPERIAL_PROBES_2:
			case XW_SCENE_IMPERIAL_PROBES_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Probe_Play(g_shellContext);
				break;
			case XW_SCENE_HOTH_1:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				FrontendAudio_PlayFile("XwingCD\\music\\tour5s3.wav", 0);
				/* fall through */
			case XW_SCENE_HOTH_2:
			case XW_SCENE_HOTH_3:
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_Flip();
				nextScene = Hoth_Play(g_shellContext);
				break;
			default:
				exitLoop = 1;
				break;
		}
		if (shellext_GetTransitionsEnabled() == 0)
			nextScene = shellext_Convert_Transition(nextScene, 1);
		shellext_Close_Landru_Scene(currentScene);
		currentScene = nextScene;
		landruError = (uint16_t)xerror_Is_Landru_Error();
	}
	FrontendAudio_StopAndRelease();
	xstream_Exit_Stream_Engine(XW_SHELL_STREAM_CHANNEL);
	shellext_Close_Landru();
	printf("XWing Front End Version 1.4 Alpha\n");
	if (frontendOnlyExit != 0 || g_quitRequested != 0)
		return XW_SCENE_EXIT_SHELL;
	return currentScene;
#endif
}
