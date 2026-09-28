#include "xw/frontend/scenes/training_transit.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/training_departure_task.h"
#include "xw_runtime/runtime/training_facility_task.h"
#endif
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw_runtime/integration/cutscene_callbacks.h"

#include <landru/actback.h>
#include <landru/actcust.h>
#include <landru/actrect.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/memhdl.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <stdlib.h>

// GLOBAL: XW 0x4FAD98
Actor* g_trainingTransitBlackActor = NULL;

// GLOBAL: XW 0x4FAD9C
Actor* g_trainingTransitBackgroundActor = NULL;

// GLOBAL: XW 0x4FADA0
Actor* g_trainingTransitBackgroundSaveActor = NULL;

// GLOBAL: XW 0x4FADA8
Rect g_trainingTransitPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FADB0
Film* g_trainingTransitFilm = NULL;

// GLOBAL: XW 0x4FADB4
Actor* g_trainingTransitCloseActor = NULL;

// GLOBAL: XW 0x4FADB8
Rect g_trainingTransitCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FADC0
LandruHandle g_trainingTransitBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FBC60
XwTrainingTransitMusicState g_trainingTransitMusicState = { NULL, NULL, NULL, 0 };

// GLOBAL: XW 0x4FBC70
int16_t g_trainingTransitUseTransitSfx = 0;

// FUNCTION: XW 0x463350
XwShellSceneResult TrainingTransit_PlayDeparture(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("totr_640.lfd");
	Rect frame;
	int16_t sceneId;
	int16_t isHyperspaceScene;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
		case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
		case XW_SCENE_TOUR_DEPART_INDEPENDENCE: {
			int16_t halfWidth = xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
														 "Leaving the Alliance Flagship Independence.") >>
								1;
			xfade_AddTimedText("Leaving the Alliance Flagship Independence.", TRAINING_TRANSIT_CAPTION_START,
							   TRAINING_TRANSIT_CAPTION_DEPARTURE_END, TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_DEPARTURE_COLOR);
			break;
		}
		case XW_SCENE_TRAINING_HYPERSPACE:
		case XW_SCENE_COMBAT_HYPERSPACE:
		case XW_SCENE_TOUR_HYPERSPACE: {
			int16_t halfWidth = xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
														 "Preparing for the jump to hyperspace.") >>
								1;
			xfade_AddTimedText("Preparing for the jump to hyperspace.", TRAINING_TRANSIT_CAPTION_START,
							   TRAINING_TRANSIT_CAPTION_TRANSIT_END, TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_HYPERSPACE_COLOR);
			break;
		}
		case XW_SCENE_TRAINING_ARRIVE_FACILITY: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
										 "Arriving at the hidden Rebel Training Facilities.") >>
				1;
			xfade_AddTimedText("Arriving at the hidden Rebel Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_TRANSIT_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_TRAINING_COLOR);
			break;
		}
		case XW_SCENE_COMBAT_ARRIVE_FACILITY: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
										 "Arriving at the hidden Rebel Combat Training Facilities.") >>
				1;
			xfade_AddTimedText("Arriving at the hidden Rebel Combat Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_TRANSIT_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_COMBAT_COLOR);
			break;
		}
	}
	xrect_Set_Rect(&frame, 0, 0, TRAINING_TRANSIT_BACKGROUND_WIDTH, TRAINING_TRANSIT_BACKGROUND_HEIGHT);
	sceneId = shellext_Get_Cur_Scene();
	isHyperspaceScene = sceneId == XW_SCENE_TRAINING_HYPERSPACE || sceneId == XW_SCENE_COMBAT_HYPERSPACE ||
						sceneId == XW_SCENE_TOUR_HYPERSPACE;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
		case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
		case XW_SCENE_TOUR_DEPART_INDEPENDENCE:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(sceneResource, "exit_f", &frame, 0, 0, 0,
															TrainingTransit_film_DepartureCallback);
			break;
		case XW_SCENE_TRAINING_HYPERSPACE:
		case XW_SCENE_COMBAT_HYPERSPACE:
		case XW_SCENE_TOUR_HYPERSPACE:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(sceneResource, "hyper_f", &frame, 0, 0, 0,
															TrainingTransit_film_DepartureCallback);
			break;
		case XW_SCENE_TRAINING_ARRIVE_FACILITY:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(sceneResource, "ttfilm_f", &frame, 0, 0, 0,
															TrainingTransit_film_DepartureCallback);
			break;
		case XW_SCENE_COMBAT_ARRIVE_FACILITY:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(sceneResource, "tcfilm_f", &frame, 0, 0, 0,
															TrainingTransit_film_DepartureCallback);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_trainingTransitFilm, shell->standardPalette);
	xview_Set_View_Update_Function(TrainingTransit_end_View);
	TrainingTransit_OpenMusic(sceneResource, g_trainingTransitFilm);
	TrainingTransit_LoadSoundEffects(sceneResource, g_trainingTransitFilm);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
		case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
			FrontendAudio_PlayFile("XwingCD\\music\\halmarch.wav", 0);
			break;
		case XW_SCENE_TOUR_DEPART_INDEPENDENCE:
			FrontendAudio_PlayFile("XwingCD\\music\\plans.wav", 0);
			break;
	}
#ifdef XW_MODERN
	XwTrainingDeparture_RunView(sceneResource, isHyperspaceScene);
#else
	j_xviewadd_Handle_View();
	if (isHyperspaceScene)
		xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_3);
	TrainingTransit_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if (!isHyperspaceScene)
		xview_Enable_All_View_Erase();
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x463660
XwShellSceneResult TrainingTransit_PlayFacilityFlight(struct XwShellContext* shell) {
	ResFile* resourceFile = xres_Open_Resource("totr_640.lfd");
	Rect frame;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_RETURN_SHUTTLE: {
			int16_t halfWidth = xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
														 "Leaving the hidden Rebel Training Facilities.") >>
								1;
			xfade_AddTimedText("Leaving the hidden Rebel Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_FACILITY_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_TRAINING_COLOR);
			break;
		}
		case XW_SCENE_COMBAT_RETURN_SHUTTLE: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
										 "Leaving the hidden Rebel Combat Training Facilities.") >>
				1;
			xfade_AddTimedText("Leaving the hidden Rebel Combat Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_FACILITY_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_COMBAT_COLOR);
			break;
		}
		case XW_SCENE_TRAINING_LAUNCH_AWING:
		case XW_SCENE_TRAINING_LAUNCH_XWING:
		case XW_SCENE_TRAINING_LAUNCH_YWING:
		case XW_SCENE_TRAINING_LAUNCH_BWING: {
			int16_t halfWidth = xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
														 "Leaving the hidden Rebel Training Facilities.") >>
								1;
			xfade_AddTimedText("Leaving the hidden Rebel Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_FACILITY_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_TRAINING_COLOR);
			break;
		}
		case XW_SCENE_COMBAT_LAUNCH_AWING:
		case XW_SCENE_COMBAT_LAUNCH_XWING:
		case XW_SCENE_COMBAT_LAUNCH_YWING:
		case XW_SCENE_COMBAT_LAUNCH_BWING: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
										 "Leaving the hidden Rebel Combat Training Facilities.") >>
				1;
			xfade_AddTimedText("Leaving the hidden Rebel Combat Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_FACILITY_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_COMBAT_COLOR);
			break;
		}
		case XW_SCENE_TRAINING_RETURN_AWING:
		case XW_SCENE_TRAINING_RETURN_XWING:
		case XW_SCENE_TRAINING_RETURN_YWING:
		case XW_SCENE_TRAINING_RETURN_BWING: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
										 "Returning to the hidden Rebel Training Facilities.") >>
				1;
			xfade_AddTimedText("Returning to the hidden Rebel Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_FACILITY_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_TRAINING_COLOR);
			break;
		}
		case XW_SCENE_COMBAT_RETURN_AWING:
		case XW_SCENE_COMBAT_RETURN_XWING:
		case XW_SCENE_COMBAT_RETURN_YWING:
		case XW_SCENE_COMBAT_RETURN_BWING: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(TRAINING_TRANSIT_CAPTION_FONT,
										 "Returning to the hidden Rebel Combat Training Facilities.") >>
				1;
			xfade_AddTimedText("Returning to the hidden Rebel Combat Training Facilities.",
							   TRAINING_TRANSIT_CAPTION_START, TRAINING_TRANSIT_CAPTION_FACILITY_END,
							   TRAINING_TRANSIT_CAPTION_FONT,
							   TRAINING_TRANSIT_BACKGROUND_WIDTH / 2 - halfWidth, TRAINING_TRANSIT_CAPTION_Y,
							   TRAINING_TRANSIT_CAPTION_COMBAT_COLOR);
			break;
		}
	}
	xrect_Set_Rect(&frame, 0, 0, TRAINING_TRANSIT_BACKGROUND_WIDTH, TRAINING_TRANSIT_BACKGROUND_HEIGHT);
	g_trainingTransitBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
		TRAINING_TRANSIT_BACKGROUND_WIDTH * TRAINING_TRANSIT_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	shellext_Get_Cur_Scene();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_RETURN_SHUTTLE:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "tsret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_RETURN_SHUTTLE:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "csret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_LAUNCH_AWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "tawing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_LAUNCH_XWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "txwing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_LAUNCH_YWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "tywing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_LAUNCH_BWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "tbwing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_LAUNCH_AWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cawing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_LAUNCH_XWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cxwing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_LAUNCH_YWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cywing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_LAUNCH_BWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cbwing_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_RETURN_AWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "taret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_RETURN_XWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "txret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_RETURN_YWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "tyret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_TRAINING_RETURN_BWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "tbret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_RETURN_AWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "caret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_RETURN_XWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cxret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_RETURN_YWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cyret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
		case XW_SCENE_COMBAT_RETURN_BWING:
			g_trainingTransitFilm = xfilm_Res_Callback_Film(resourceFile, "cbret_f", &frame, 0, 0, 0,
															TrainingTransit_film_FacilityCallback);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_trainingTransitFilm, shell->standardPalette);
	g_trainingTransitBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TRAINING_TRANSIT_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_trainingTransitBackgroundActor, TrainingTransit_user_Background);
	xactor_Set_Actor_Draw_Function(g_trainingTransitBackgroundActor, TrainingTransit_draw_Background);
	g_trainingTransitCloseActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TRAINING_TRANSIT_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_trainingTransitCloseActor, TrainingTransit_user_Close);
	xactor_Set_Actor_Draw_Function(g_trainingTransitCloseActor, XwCutscene_DrawCloseOnRefresh);
	g_trainingTransitBackgroundSaveActor =
		xactback_Alloc_Back_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TRAINING_TRANSIT_SAVE_Z);
	xactback_Alloc_Back_Actor_Buffer(g_trainingTransitBackgroundSaveActor,
									 TRAINING_TRANSIT_SAVE_BUFFER_CAPACITY);
	g_trainingTransitBlackActor =
		xactrect_Alloc_Blank_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TRAINING_TRANSIT_BLACK_Z);
	xactor_Set_Actor_Size(g_trainingTransitBlackActor, TRAINING_TRANSIT_BACKGROUND_WIDTH,
						  TRAINING_TRANSIT_BACKGROUND_HEIGHT);
	xactor_Set_Actor_Color(g_trainingTransitBlackActor, 0, 0);
	xactor_Non_Refreshable_Actor(g_trainingTransitBlackActor);
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(TrainingTransit_end_View);
#ifdef XW_MODERN
	XwTrainingFacility_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_2);
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x463C20
void TrainingTransit_end_View(int time) {
	int16_t nextScene;
	int16_t nextSection;
	int16_t exitScene;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
			nextScene = XW_SCENE_TRAINING_HYPERSPACE;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
		case XW_SCENE_TRAINING_HYPERSPACE:
			nextScene = XW_SCENE_TRAINING_ARRIVE_FACILITY;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			if (time == TRAINING_TRANSIT_HYPERSPACE_RATE_CHANGE_TIME)
				xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_5);
			break;
		case XW_SCENE_TRAINING_ARRIVE_FACILITY:
			nextScene = XW_SCENE_PROVING_GROUNDS_ROOM;
			nextSection = XW_SCENE_PROVING_GROUNDS_ROOM;
			break;
		case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
			nextScene = XW_SCENE_COMBAT_HYPERSPACE;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
		case XW_SCENE_COMBAT_HYPERSPACE:
			nextScene = XW_SCENE_COMBAT_ARRIVE_FACILITY;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			if (time == TRAINING_TRANSIT_HYPERSPACE_RATE_CHANGE_TIME)
				xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_5);
			break;
		case XW_SCENE_COMBAT_ARRIVE_FACILITY:
			nextScene = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			nextSection = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			break;
		case XW_SCENE_TRAINING_RETURN_SHUTTLE:
		case XW_SCENE_COMBAT_RETURN_SHUTTLE:
			nextScene = XW_SCENE_CONCOURSE;
			nextSection = XW_SCENE_CONCOURSE;
			break;
		case XW_SCENE_TOUR_DEPART_INDEPENDENCE:
			nextScene = XW_SCENE_TOUR_HYPERSPACE;
			nextSection = XW_SCENE_BRIEFING_TOUR;
			break;
		case XW_SCENE_TRAINING_LAUNCH_XWING:
		case XW_SCENE_TRAINING_LAUNCH_YWING:
		case XW_SCENE_TRAINING_LAUNCH_AWING:
		case XW_SCENE_TRAINING_LAUNCH_BWING:
			nextScene = XW_SCENE_FLIGHT_PROVING_GROUNDS;
			nextSection = XW_SCENE_FLIGHT_PROVING_GROUNDS;
			break;
		case XW_SCENE_COMBAT_LAUNCH_XWING:
		case XW_SCENE_COMBAT_LAUNCH_YWING:
		case XW_SCENE_COMBAT_LAUNCH_AWING:
		case XW_SCENE_COMBAT_LAUNCH_BWING:
			nextScene = XW_SCENE_FLIGHT_COMBAT;
			nextSection = XW_SCENE_FLIGHT_COMBAT;
			break;
		case XW_SCENE_TOUR_HYPERSPACE:
			nextScene = XW_SCENE_TOUR_ARRIVE_DEFIANCE;
			nextSection = XW_SCENE_BRIEFING_TOUR;
			if (time == TRAINING_TRANSIT_HYPERSPACE_RATE_CHANGE_TIME)
				xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_5);
			break;
		case XW_SCENE_TRAINING_RETURN_AWING:
		case XW_SCENE_TRAINING_RETURN_XWING:
		case XW_SCENE_TRAINING_RETURN_YWING:
		case XW_SCENE_TRAINING_RETURN_BWING:
			nextSection = XW_SCENE_DEBRIEF_PROVING_GROUNDS;
			nextScene = XW_SCENE_DEBRIEF_PROVING_GROUNDS;
			break;
		case XW_SCENE_COMBAT_RETURN_AWING:
		case XW_SCENE_COMBAT_RETURN_XWING:
		case XW_SCENE_COMBAT_RETURN_YWING:
		case XW_SCENE_COMBAT_RETURN_BWING:
			nextSection = XW_SCENE_DEBRIEF_COMBAT;
			nextScene = XW_SCENE_DEBRIEF_COMBAT;
			break;
		default:
			nextSection = time;
			nextScene = time;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection,
								  g_trainingTransitFilm->cur_cel == g_trainingTransitFilm->cels) != 0)
		xerror_Set_Landru_Exit(exitScene);
}

// FUNCTION: XW 0x463EF0
int16_t TrainingTransit_film_FacilityCallback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 != CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, TrainingTransit_user_SaveRegion);
		} else {
			xactor_Set_Actor_User_Function(actor, TrainingTransit_user_SoundCue);
		}
	}
	return 0;
}

// FUNCTION: XW 0x463F40
int16_t TrainingTransit_film_DepartureCallback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND)
			xactor_Set_Actor_User_Function(actor, TrainingTransit_user_SoundCue);
	}
	return 0;
}

// FUNCTION: XW 0x463F80
void TrainingTransit_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		TrainingTransit_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x463FA0
void TrainingTransit_user_SaveRegion(Actor* actor, int unusedTime) {
	(void)unusedTime;
	switch (actor->var1) {
		case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS: {
			Rect saveRect;
			xactback_Stop_Back_Actor(g_trainingTransitBackgroundSaveActor);
			xactor_Refresh_Actors();
			xcanvas_Invalid_Screen_Diff();
			xactor_Get_Actor_Rect(actor, &saveRect);
			xactback_Set_Back_Actor_Rect(g_trainingTransitBackgroundSaveActor, &saveRect);
			break;
		}
		case TRAINING_TRANSIT_ACTOR_ROLE_NON_REFRESHABLE:
			if (xactor_Is_Actor_Refreshable(actor)) {
				xactor_Non_Refreshable_Actor(actor);
			}
			break;
	}
}

// FUNCTION: XW 0x464010
void TrainingTransit_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_trainingTransitPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_trainingTransitPreviousDirtyRect, &g_trainingTransitCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_trainingTransitCurrentDirtyRect);
}

// FUNCTION: XW 0x464060
int16_t TrainingTransit_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
										int16_t unusedX, int16_t unusedY, int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_trainingTransitBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_trainingTransitPreviousDirtyRect,
								  g_trainingTransitPreviousDirtyRect.left,
								  g_trainingTransitPreviousDirtyRect.top, TRAINING_TRANSIT_BACKGROUND_WIDTH,
								  TRAINING_TRANSIT_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_trainingTransitBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x4640C0
void TrainingTransit_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_trainingTransitFilm->cur_cel == g_trainingTransitFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_trainingTransitPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_trainingTransitCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x4680C0
void TrainingTransit_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	int startSwirlsLater = 0;
	const char* previousMusicName = NULL;
	const char* secondaryMusicName;
	Sound* secondaryMusic;
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() == 0)
		return;
	g_trainingTransitMusicState.sceneSound = NULL;
	g_trainingTransitMusicState.film = sceneFilm;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TRAINING_DEPART_INDEPENDENCE:
		case XW_SCENE_TRAINING_HYPERSPACE:
		case XW_SCENE_TRAINING_ARRIVE_FACILITY:
			previousMusicName = "halmarch";
			secondaryMusicName = "rebels";
			TrainingTransit_EnsureMusic(previousMusicName, "mmmusic.lfd", TRAINING_TRANSIT_MARCH_BEAT);
			break;
		case XW_SCENE_COMBAT_DEPART_INDEPENDENCE:
		case XW_SCENE_COMBAT_HYPERSPACE:
		case XW_SCENE_COMBAT_ARRIVE_FACILITY:
			previousMusicName = "halmarch";
			secondaryMusicName = "mission";
			TrainingTransit_EnsureMusic(previousMusicName, "mmmusic.lfd", TRAINING_TRANSIT_MARCH_BEAT);
			break;
		case XW_SCENE_TRAINING_RETURN_SHUTTLE:
			previousMusicName = "rebels";
			/* Fall through to load plans while retaining the previous track. */
		case XW_SCENE_COMBAT_RETURN_SHUTTLE:
			if (previousMusicName == NULL)
				previousMusicName = "mission";
			/* Fall through. */
		case XW_SCENE_TOUR_DEPART_INDEPENDENCE: {
			ResFile* plansResource;
			if (previousMusicName == NULL)
				previousMusicName = "tourdesk";
			secondaryMusicName = "plans";
			plansResource = xres_Open_Resource("pnmusic.lfd");
			g_trainingTransitMusicState.sceneSound = xsound_Res_Music(plansResource, secondaryMusicName);
			xres_Close_Resource(plansResource);
			g_trainingTransitMusicState.fadePreviousAtCel12 = 1;
			break;
		}
		case XW_SCENE_TOUR_HYPERSPACE:
			secondaryMusicName = "plans";
			previousMusicName = NULL;
			TrainingTransit_EnsureMusic(secondaryMusicName, "pnmusic.lfd", TRAINING_TRANSIT_PLANS_BEAT);
			break;
		case XW_SCENE_TRAINING_LAUNCH_XWING:
		case XW_SCENE_TRAINING_LAUNCH_YWING:
		case XW_SCENE_TRAINING_LAUNCH_AWING:
		case XW_SCENE_TRAINING_LAUNCH_BWING:
		case XW_SCENE_COMBAT_LAUNCH_XWING:
		case XW_SCENE_COMBAT_LAUNCH_YWING:
		case XW_SCENE_COMBAT_LAUNCH_AWING:
		case XW_SCENE_COMBAT_LAUNCH_BWING:
		case XW_SCENE_LEGACY_TOUR_TRANSITION_281:
		case XW_SCENE_LEGACY_TOUR_TRANSITION_283:
		case XW_SCENE_LEGACY_TOUR_TRANSITION_284:
		case XW_SCENE_LEGACY_TOUR_TRANSITION_285:
			previousMusicName = "launch";
			secondaryMusicName = NULL;
			g_trainingTransitMusicState.sceneSound =
				TrainingTransit_EnsureMusic(previousMusicName, "hr2music.lfd", TRAINING_TRANSIT_LAUNCH_BEAT);
			break;
		case XW_SCENE_TOUR_RETURN_AWING:
		case XW_SCENE_TOUR_RETURN_XWING:
		case XW_SCENE_TOUR_RETURN_YWING:
		case XW_SCENE_TOUR_RETURN_BWING:
		case XW_SCENE_TRAINING_RETURN_AWING:
		case XW_SCENE_TRAINING_RETURN_XWING:
		case XW_SCENE_TRAINING_RETURN_YWING:
		case XW_SCENE_TRAINING_RETURN_BWING:
		case XW_SCENE_COMBAT_RETURN_AWING:
		case XW_SCENE_COMBAT_RETURN_XWING:
		case XW_SCENE_COMBAT_RETURN_YWING:
		case XW_SCENE_COMBAT_RETURN_BWING: {
			ResFile* swirlsResource = xres_Open_Resource("ldmusic.lfd");
			g_trainingTransitMusicState.sceneSound = xsound_Res_Music(swirlsResource, "swirls");
			xres_Close_Resource(swirlsResource);
			xsound_Set_Sound_Keep(g_trainingTransitMusicState.sceneSound);
			secondaryMusicName = NULL;
			startSwirlsLater = 1;
			break;
		}
		default:
			previousMusicName = "tourdesk";
			secondaryMusicName = "wrkmarch";
			break;
	}
	if (previousMusicName != NULL)
		g_trainingTransitMusicState.previousSound = xsound_Find_Gmid(previousMusicName);
	else
		g_trainingTransitMusicState.previousSound = NULL;
	if (secondaryMusicName != NULL)
		secondaryMusic = xsound_Find_Gmid(secondaryMusicName);
	else
		secondaryMusic = NULL;
	if (secondaryMusic != NULL)
		xsound_Set_Sound_Keep(secondaryMusic);
	if (g_trainingTransitMusicState.previousSound != NULL)
		xsound_Set_Sound_Keep(g_trainingTransitMusicState.previousSound);
	if ((g_trainingTransitMusicState.sceneSound != NULL && secondaryMusicName != NULL) ||
		startSwirlsLater != 0)
		xsound_Set_Sound_User_Function(g_trainingTransitMusicState.sceneSound, TrainingTransit_user_Music);
}

// FUNCTION: XW 0x4683F0
Sound* TrainingTransit_EnsureMusic(const char* name, const char* filename, unsigned int beatIndex) {
	Sound* music = xsound_Find_Gmid(name);
	if (music == NULL) {
		ResFile* musicResource = xres_Open_Resource(filename);
		music = xsound_Res_Music(musicResource, name);
		xres_Close_Resource(musicResource);
		xsound_Set_Sound_Keep(music);
	} else {
		xsound_Set_Sound_Keep(music);
		if (soundext_Count_Resource_Instances(music) == 1) {
			return music;
		}
	}
	soundext_Start_Resource_Sound(music);
	soundext_ScanMidi(music, 0, beatIndex, 0);
	soundext_SetHook(music, XW_SOUND_CONTROL_DIRECT, 1, 0);
	soundext_ClearTriggers();
	return music;
}

// FUNCTION: XW 0x468480
void TrainingTransit_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_TRAINING_ARRIVE_FACILITY:
			case XW_SCENE_COMBAT_ARRIVE_FACILITY: {
				Sound* music = xsound_Find_Gmid("halmarch");
				if (music != NULL && soundext_ClearTriggers() == 0)
					soundext_FadeVolume(music, 0, TRAINING_TRANSIT_MARCH_CLOSE_DURATION);
				break;
			}
			case XW_SCENE_TRAINING_RETURN_SHUTTLE:
			case XW_SCENE_COMBAT_RETURN_SHUTTLE:
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE: {
				Sound* music = xsound_Find_Gmid("plans");
				if (music != NULL) {
					soundext_FadeVolume(music, 0, TRAINING_TRANSIT_PLANS_CLOSE_DURATION);
					/* Resource pointers are outside the numeric flight-sound ID range. */
					soundext_SetPriority(0, 0);
				}
				break;
			}
			case XW_SCENE_TOUR_RETURN_AWING:
			case XW_SCENE_TOUR_RETURN_XWING:
			case XW_SCENE_TOUR_RETURN_YWING:
			case XW_SCENE_TOUR_RETURN_BWING:
				return;
			case XW_SCENE_TRAINING_LAUNCH_XWING:
			case XW_SCENE_TRAINING_LAUNCH_YWING:
			case XW_SCENE_TRAINING_LAUNCH_AWING:
				return;
			case XW_SCENE_TRAINING_LAUNCH_BWING:
				return;
			case XW_SCENE_COMBAT_LAUNCH_XWING:
			case XW_SCENE_COMBAT_LAUNCH_YWING:
			case XW_SCENE_COMBAT_LAUNCH_AWING:
				return;
			case XW_SCENE_COMBAT_LAUNCH_BWING:
				return;
			case XW_SCENE_TRAINING_RETURN_AWING:
			case XW_SCENE_TRAINING_RETURN_XWING:
			case XW_SCENE_TRAINING_RETURN_YWING:
				return;
			case XW_SCENE_TRAINING_RETURN_BWING:
				return;
			case XW_SCENE_COMBAT_RETURN_AWING:
			case XW_SCENE_COMBAT_RETURN_XWING:
			case XW_SCENE_COMBAT_RETURN_YWING:
				return;
			case XW_SCENE_COMBAT_RETURN_BWING:
				return;
		}
	}
}

// FUNCTION: XW 0x468660
void TrainingTransit_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_trainingTransitMusicState.film->cur_cel;
	int startCel = TRAINING_TRANSIT_MUSIC_START_CEL;
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_TOUR_DEPART_INDEPENDENCE) {
		startCel = TRAINING_TRANSIT_MUSIC_INDEPENDENCE_START_CEL;
	}
	if (filmCel == startCel) {
		soundext_Start_Resource_Sound(g_trainingTransitMusicState.sceneSound);
		/* Selector 4 is unsupported; resource pointers are not numeric flight-sound IDs. */
		soundext_SetGroup(0, TRAINING_TRANSIT_MUSIC_PARAM4_VALUE);
	} else if (g_trainingTransitMusicState.fadePreviousAtCel12 != 0 &&
			   filmCel == TRAINING_TRANSIT_MUSIC_PREVIOUS_FADE_CEL) {
		Sound* previousSound = g_trainingTransitMusicState.previousSound;
		soundext_FadeVolume(previousSound, 0, TRAINING_TRANSIT_MUSIC_PREVIOUS_FADE_DURATION);
	}
}

// FUNCTION: XW 0x4686D0
void TrainingTransit_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_1, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_2, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_3, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_4, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_5, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
	g_trainingTransitUseTransitSfx = 1;
}

// FUNCTION: XW 0x468750
void TrainingTransit_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		if (g_trainingTransitUseTransitSfx != 0) {
			switch (cue) {
				case TRAINING_TRANSIT_SOUND_CUE_1:
					soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_1);
					break;
				case TRAINING_TRANSIT_SOUND_CUE_2:
					soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_2);
					break;
				case TRAINING_TRANSIT_SOUND_CUE_3:
					soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_3);
					break;
				case TRAINING_TRANSIT_SOUND_CUE_4:
					soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_4);
					break;
				case TRAINING_TRANSIT_SOUND_CUE_5:
					soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_5);
					break;
			}
		} else {
			switch (cue) {
				case TRAINING_TRANSIT_SOUND_CUE_1:
					soundext_Play_SFX(XW_SHELL_SFX_FLYBY_8);
					break;
				case TRAINING_TRANSIT_SOUND_CUE_2:
					soundext_Play_SFX(XW_SHELL_SFX_FLYBY_2);
					break;
			}
		}
	}
}
