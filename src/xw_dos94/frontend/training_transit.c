#include "xw_dos94/frontend/training_transit.h"
#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/training_transit.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw_dos94/audio/soundext.h"
#include "xw_runtime/runtime/scene_view_task.h"
#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#include <imuse/lolevel.h>
#endif
#include <landru/actback.h>
#include <landru/actrect.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/view.h>

static ResFile* resource;
static int saved_background;

/* DOS94 0x5c2574: var2 controls the lifetime of the saved region. */
static void save_region(Actor* actor, int time) {
	(void)time;
	if (actor->var1 == 10 && actor->var2 != 0) {
		Rect frame;
		if (actor->var2 == 1) {
			xactback_Stop_Back_Actor(g_trainingTransitBackgroundSaveActor);
			xactor_Refresh_Actors();
			xcanvas_Invalid_Screen_Diff();
		}
		if (actor->var2 == 3)
			xcanvas_Invalid_Screen_Diff();
		xactor_Get_Actor_Rect(actor, &frame);
		xactback_Set_Back_Actor_Rect(g_trainingTransitBackgroundSaveActor, &frame);
	} else if (actor->var1 == 20 && xactor_Is_Actor_Refreshable(actor)) {
		xactor_Non_Refreshable_Actor(actor);
	}
}

/* DOS94 0x5c249a. The fast departure callback is identical to Windows. */
static int16_t facility_callback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		Actor* actor = object->object;
		xactor_Set_Actor_User_Function(actor,
									   actor->var1 == 50 ? TrainingTransit_user_SoundCue : save_region);
	}
	return 0;
}

/* DOS94 0x5c2310: returning fighters first pass through the landing scene. */
static void end_view(int time) {
	int16_t next, section, exit;
	(void)time;
	switch (shellext_Get_Cur_Scene()) {
		case 40:
			next = 41;
			section = 45;
			break;
		case 41:
			next = 42;
			section = 45;
			break;
		case 42:
			next = section = 45;
			break;
		case 43:
		case 53:
			next = section = 30;
			break;
		case 50:
			next = 51;
			section = 60;
			break;
		case 51:
			next = 52;
			section = 60;
			break;
		case 52:
			next = section = 60;
			break;
		case 80:
			next = 81;
			section = 112;
			break;
		case 81:
			next = 82;
			section = 112;
			break;
		case 263:
		case 264:
		case 265:
		case 267:
			next = section = 2;
			break;
		case 273:
		case 274:
		case 275:
		case 277:
			next = section = 3;
			break;
		case 290:
			next = 293;
			section = 115;
			break;
		case 291:
			next = 294;
			section = 115;
			break;
		case 292:
			next = 295;
			section = 115;
			break;
		case 296:
			next = 297;
			section = 115;
			break;
		case 300:
			next = 303;
			section = 116;
			break;
		case 301:
			next = 304;
			section = 116;
			break;
		case 302:
			next = 305;
			section = 116;
			break;
		case 306:
			next = 307;
			section = 116;
			break;
		default:
			return;
	}
	if (shellext_Check_Scene_Exit(&exit, next, section,
								  g_trainingTransitFilm->cur_cel == g_trainingTransitFilm->cels))
		xerror_Set_Landru_Exit(exit);
}

/* DOS94 0x5c2b3a: the DOS call passes the resource sound, not the Windows stub ID. */
static void music_update(Sound* sound, int time) {
	(void)sound;
	(void)time;
	int cel = g_trainingTransitMusicState.film->cur_cel;
	int start = shellext_Get_Cur_Scene() == 80 ? 20 : 10;
	if (cel == start) {
		soundext_Start_Resource_Sound(g_trainingTransitMusicState.sceneSound);
		soundext_SetGroup((intptr_t)g_trainingTransitMusicState.sceneSound, 120);
	} else if (g_trainingTransitMusicState.fadePreviousAtCel12 && cel == 12) {
		soundext_FadeVolume(g_trainingTransitMusicState.previousSound, 0, 480);
	}
}

/* DOS94 0x5c2a3c. */
static void close_music(void) {
	if (!ShellPreferences_GetMusicEnabled())
		return;
	int scene = shellext_Get_Cur_Scene();
	if (scene == 42 || scene == 52) {
		Sound* music = xsound_Find_Gmid("halmarch");
		if (music && !Dos94_soundext_CheckTriggers())
			soundext_FadeVolume(music, 0, 180);
	} else if (scene == 43 || scene == 53 || scene == 82) {
		Sound* music = xsound_Find_Gmid("plans");
		if (music) {
			soundext_FadeVolume(music, 0, 300);
			soundext_SetPriority((intptr_t)music, 0);
		}
	}
}

static void release_view(void) {
	close_music();
	xview_Clear_View_Update_Function();
	if (saved_background)
		xview_Enable_All_View_Erase();
	xres_Close_Resource(resource);
	resource = NULL;
}

static void allocate_background(Rect* frame) {
	g_trainingTransitBackgroundSaveActor = xactback_Alloc_Back_Actor(LANDRU_NULL_HANDLE, frame, 0, 0, 1);
	xactback_Alloc_Back_Actor_Buffer(g_trainingTransitBackgroundSaveActor, 40000);
	g_trainingTransitBlackActor = xactrect_Alloc_Blank_Actor(LANDRU_NULL_HANDLE, frame, 0, 0, 100);
	xactor_Set_Actor_Size(g_trainingTransitBlackActor, 320, 200);
	xactor_Set_Actor_Color(g_trainingTransitBlackActor, 0, 0);
	xactor_Non_Refreshable_Actor(g_trainingTransitBlackActor);
	xview_Disable_All_View_Erase();
}

static void run_view(struct XwShellContext* shell) {
	xfilm_Set_Film_Def_Palette(g_trainingTransitFilm, shell->standardPalette);
	xview_Set_View_Update_Function(end_view);
	TrainingTransit_OpenMusic(resource, g_trainingTransitFilm);
	int scene = shellext_Get_Cur_Scene();
	if (ShellPreferences_GetMusicEnabled() && g_trainingTransitMusicState.sceneSound &&
		(scene == 43 || scene == 53 || scene == 80 || scene >= 290))
		xsound_Set_Sound_User_Function(g_trainingTransitMusicState.sceneSound, music_update);
	XwScene_RunView(NULL, release_view);
}

/* DOS94 0x5c197c. Captions have authored positions, not measured centering. */
XwShellSceneResult Dos94_TrainingTransit_PlayDeparture(struct XwShellContext* shell) {
	int scene = shellext_Get_Cur_Scene();
	int hyper = scene == 41 || scene == 51 || scene == 81;
	const char* name;
	Rect frame;
	switch (scene) {
		case 40:
		case 50:
		case 80:
			xfade_AddTimedText("Leaving the Alliance Flagship Independence.", 4, 30, 0, 60, 176, 16);
			break;
		case 41:
		case 51:
		case 81:
			xfade_AddTimedText("Preparing for the jump to hyperspace.", 4, 50, 0, 80, 176, 16);
			break;
		case 42:
			xfade_AddTimedText("Arriving at the hidden Rebel Training Facilities.", 4, 50, 0, 30, 176, 16);
			break;
		case 52:
			xfade_AddTimedText("Arriving at the hidden Rebel Combat Training Facilities.", 4, 50, 0, 14, 176,
							   16);
			break;
	}
	resource = xres_Open_Resource("totrain.lfd");
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	saved_background = xio_Is_System_Slower_Than(2) && !hyper;
	if (hyper)
		name = "hyper_f";
	else if (scene == 42)
		name = saved_background ? "ttfilm_s" : "ttfilm_f";
	else if (scene == 52)
		name = saved_background ? "tcfilm_s" : "tcfilm_f";
	else
		name = saved_background ? "exit_s" : "exit_f";
	g_trainingTransitFilm = xfilm_Res_Callback_Film(
		resource, name, &frame, 0, 0, 0,
		saved_background ? facility_callback : TrainingTransit_film_DepartureCallback);
	if (saved_background)
		allocate_background(&frame);
	if (scene == 40 || scene == 50)
		FrontendAudio_PlayFile("XwingCD\\music\\halmarch.wav", 0);
	else if (scene == 80)
		FrontendAudio_PlayFile("XwingCD\\music\\plans.wav", 0);
	run_view(shell);
}

/* DOS94 0x5c1d48. B-wing films live in their expansion archive. */
XwShellSceneResult Dos94_TrainingTransit_PlayFacilityFlight(struct XwShellContext* shell) {
	int scene = shellext_Get_Cur_Scene();
	const char* name;
	const char* caption;
	int x;
	Rect frame;
	switch (scene) {
		case 43:
			name = "tsret_f";
			break;
		case 53:
			name = "csret_f";
			break;
		case 263:
			name = "txwing_f";
			break;
		case 264:
			name = "tywing_f";
			break;
		case 265:
			name = "tawing_f";
			break;
		case 267:
			name = "tbwing_f";
			break;
		case 273:
			name = "cxwing_f";
			break;
		case 274:
			name = "cywing_f";
			break;
		case 275:
			name = "cawing_f";
			break;
		case 277:
			name = "cbwing_f";
			break;
		case 290:
			name = "taret_f";
			break;
		case 291:
			name = "txret_f";
			break;
		case 292:
			name = "tyret_f";
			break;
		case 296:
			name = "tbret_f";
			break;
		case 300:
			name = "caret_f";
			break;
		case 301:
			name = "cxret_f";
			break;
		case 302:
			name = "cyret_f";
			break;
		case 306:
			name = "cbret_f";
			break;
		default:
			return;
	}
	if (scene == 43 || (scene >= 263 && scene <= 267)) {
		caption = "Leaving the hidden Rebel Training Facilities.";
		x = scene == 43 ? 30 : 40;
	} else if (scene == 53 || (scene >= 273 && scene <= 277)) {
		caption = "Leaving the hidden Rebel Combat Training Facilities.";
		x = scene == 53 ? 16 : 26;
	} else if (scene < 300) {
		caption = "Returning to the hidden Rebel Training Facilities.";
		x = 20;
	} else {
		caption = "Returning to the hidden Rebel Combat Training Facilities.";
		x = 6;
	}
	xfade_AddTimedText(caption, 4, 40, 0, x, 176, 16);
	resource = xres_Open_Resource("totrain.lfd");
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	ResFile* film_resource = resource;
	if (scene == 267 || scene == 277 || scene == 296 || scene == 306)
		film_resource = xres_Open_Resource("bwing.lfd");
	g_trainingTransitFilm = xfilm_Res_Callback_Film(film_resource, name, &frame, 0, 0, 0, facility_callback);
	if (film_resource != resource)
		xres_Close_Resource(film_resource);
	saved_background = 1;
	allocate_background(&frame);
	run_view(shell);
}
