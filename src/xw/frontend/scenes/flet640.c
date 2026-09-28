#include "xw/frontend/scenes/flet640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/scenes/ds_land.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/flet640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>

// GLOBAL: XW 0x4D3510
const char* g_flet640ResourceFilename = "flet_640.lfd";

// GLOBAL: XW 0x4D3514
const char* g_flet640RebelSlowFilmName = "reb_s";

// GLOBAL: XW 0x4D3518
const char* g_flet640RebelFastFilmName = "reb_f";

// GLOBAL: XW 0x4D351C
const char* g_flet640ImperialSlowFilmName = "empf_s";

// GLOBAL: XW 0x4D3520
const char* g_flet640ImperialFastFilmName = "empf_f";

// GLOBAL: XW 0x4D3524
const char* g_flet640PlansSlowFilmName = "plans3_s";

// GLOBAL: XW 0x4D3528
const char* g_flet640PlansFastFilmName = "plans3_f";

// GLOBAL: XW 0x4D352C
const char* g_flet640DefianceSlowFilmName = "defarriv";

// GLOBAL: XW 0x4D3530
const char* g_flet640DefianceFastFilmName = "defarriv";

// GLOBAL: XW 0x4F74B8
Actor* g_flet640BackgroundActor = NULL;

// GLOBAL: XW 0x4F74BC
Film* g_flet640Film = NULL;

// GLOBAL: XW 0x4F74C0
Rect g_flet640PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F74C8
Actor* g_flet640CloseActor = NULL;

// GLOBAL: XW 0x4F74D0
Rect g_flet640CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F74D8
LandruHandle g_flet640BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F74DC
XwSceneMusicHandles g_flet640MusicState = { NULL, NULL };

// FUNCTION: XW 0x449EE0
XwShellSceneResult Flet640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TOUR_ARRIVE_DEFIANCE: {
			int16_t halfWidth = xfont_Get_String_Width_0(FLET640_CAPTION_FONT,
														 "Arriving at the Calamari Cruiser Defiance.") >>
								1;
			xfade_AddTimedText("Arriving at the Calamari Cruiser Defiance.", FLET640_CAPTION_START,
							   FLET640_CAPTION_END, FLET640_CAPTION_FONT, FLET640_CAPTION_CENTER - halfWidth,
							   FLET640_CAPTION_Y, FLET640_DEFIANCE_COLOR);
			break;
		}
		case XW_SCENE_RESCUE_ARRIVE_SALVATION: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(FLET640_CAPTION_FONT,
										 "Arriving at Rebel Fleet Medical Ship, the Salvation.") >>
				1;
			xfade_AddTimedText("Arriving at Rebel Fleet Medical Ship, the Salvation.", FLET640_CAPTION_START,
							   FLET640_CAPTION_END, FLET640_CAPTION_FONT, FLET640_CAPTION_CENTER - halfWidth,
							   FLET640_CAPTION_Y, FLET640_SALVATION_COLOR);
			break;
		}
		case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR: {
			int16_t halfWidth =
				xfont_Get_String_Width_0(FLET640_CAPTION_FONT,
										 "Arriving at the Executor.  Flagship of the Empire!") >>
				1;
			xfade_AddTimedText("Arriving at the Executor.  Flagship of the Empire!", FLET640_CAPTION_START,
							   FLET640_CAPTION_END, FLET640_CAPTION_FONT, FLET640_CAPTION_CENTER - halfWidth,
							   FLET640_CAPTION_Y, FLET640_EXECUTOR_COLOR);
			break;
		}
		case XW_SCENE_RECOVER_PLANS_FLEET:
			LandruDisplay_SetLowResolutionMode(1);
			xfade_AddTimedText("Imperial Fleet on course for Tatooine.", FLET640_CAPTION_START,
							   FLET640_CAPTION_END, FLET640_PLANS_FONT, FLET640_PLANS_X, FLET640_PLANS_Y,
							   FLET640_PLANS_COLOR);
			break;
	}
	resourceFile = xres_Open_Resource(g_flet640ResourceFilename);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
#ifdef XW_MODERN
		g_flet640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(
			FLET640_BACKGROUND_WIDTH * FLET640_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
#else
		g_flet640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(
			FLET640_LEGACY_BACKGROUND_WIDTH * FLET640_LEGACY_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
#endif
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_RESCUE_ARRIVE_SALVATION:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640RebelSlowFilmName, &frame, 0,
														0, 0, Flet640_film_Callback);
				break;
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640DefianceSlowFilmName, &frame,
														0, 0, 0, Flet640_film_Callback);
				break;
			case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640ImperialSlowFilmName, &frame,
														0, 0, 0, Flet640_film_Callback);
				break;
			case XW_SCENE_RECOVER_PLANS_FLEET:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640PlansSlowFilmName, &frame, 0,
														0, 0, Flet640_film_Callback);
				break;
		}
		g_flet640BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, FLET640_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_flet640BackgroundActor, Flet640_user_Background);
		xactor_Set_Actor_Draw_Function(g_flet640BackgroundActor, Flet640_draw_Background);
		g_flet640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, FLET640_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_flet640CloseActor, Flet640_user_Close);
		xactor_Set_Actor_Draw_Function(g_flet640CloseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_RESCUE_ARRIVE_SALVATION:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640RebelFastFilmName, &frame, 0,
														0, 0, Flet640_film_Callback);
				break;
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640DefianceFastFilmName, &frame,
														0, 0, 0, Flet640_film_Callback);
				break;
			case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640ImperialFastFilmName, &frame,
														0, 0, 0, Flet640_film_Callback);
				break;
			case XW_SCENE_RECOVER_PLANS_FLEET:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640PlansFastFilmName, &frame, 0,
														0, 0, Flet640_film_Callback);
				break;
		}
	}
	xfilm_Set_Film_Def_Palette(g_flet640Film, shell->standardPalette);
	xview_Set_View_Update_Function(Flet640_end_View);
	Flet640_OpenMusic(resourceFile, g_flet640Film);
	DsLand_LoadSoundEffects(resourceFile, g_flet640Film);
#ifdef XW_MODERN
	XwFlet640_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Flet640_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_flet640BackgroundHandle);
	}
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x44A400
void Flet640_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
			nextScene = XW_SCENE_BRIEFING_TOUR;
			nextSection = XW_SCENE_BRIEFING_TOUR;
			break;
		case XW_SCENE_RESCUE_ARRIVE_SALVATION:
			nextScene = XW_SCENE_PILOT_MEDICAL_RECOVERY;
			nextSection = shipext_Get_Pending_Tour_Cutscene();
			break;
		case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
			nextScene = XW_SCENE_PILOT_TORTURE;
			nextSection = XW_SCENE_REGISTER_RETURN;
			break;
		case XW_SCENE_RECOVER_PLANS_FLEET:
			nextScene = XW_SCENE_RECOVER_PLANS_VADER;
			nextSection = shipext_Get_Pending_Medal_Scene();
			break;
#ifdef XW_MODERN
		default:
			return;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection,
								  g_flet640Film->cur_cel == g_flet640Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x44A510
int16_t Flet640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				if (xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					xactor_Set_Actor_User_Function(actor, Flet640_user_DirtyBounds);
				}
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				if (xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					Flet640_film_Actor_To_Background(actor);
					consumeObject = 1;
				}
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, DsLand_user_Sound);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x44A5B0
int16_t Flet640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_flet640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						FLET640_BACKGROUND_WIDTH, FLET640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_flet640BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x44A6B0
void Flet640_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_flet640PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_flet640PreviousDirtyRect, &g_flet640CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_flet640CurrentDirtyRect);
}

// FUNCTION: XW 0x44A700
int16_t Flet640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								int16_t unusedY, int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_flet640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_flet640PreviousDirtyRect,
								  g_flet640PreviousDirtyRect.left, g_flet640PreviousDirtyRect.top,
								  FLET640_BACKGROUND_WIDTH, FLET640_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_flet640BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x44A760
void Flet640_user_DirtyBounds(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor)) {
		Rect actorBounds;
		Rect actorFrame;
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_flet640CurrentDirtyRect, &actorBounds);
	}
	if (actor->var2 == FLET640_INVALIDATE_SCREEN_DIFF) {
		xcanvas_Invalid_Screen_Diff();
	}
}

// FUNCTION: XW 0x44A7E0
void Flet640_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_flet640Film->cur_cel == g_flet640Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_flet640PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_flet640CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x44A880
void Flet640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	unsigned int startBeat = 0;
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene = (int16_t)shellext_Get_Cur_Scene();
		const char* musicFilename;
		const char* musicName;
		switch (scene) {
			case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
				musicFilename = "rsmusic.lfd";
				musicName = "torture";
				startBeat = FLET640_MUSIC_CAPTURE_BEAT;
				break;
			case XW_SCENE_RESCUE_ARRIVE_SALVATION:
				musicFilename = "rsmusic.lfd";
				musicName = "rescue";
				startBeat = FLET640_MUSIC_RESCUE_BEAT;
				break;
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
				startBeat = FLET640_MUSIC_DEFIANCE_BEAT;
				/* Fall through to the shared plans music selection. */
			default:
				if (scene == XW_SCENE_RECOVER_PLANS_FLEET) {
					startBeat = FLET640_MUSIC_PLANS_BEAT;
				}
				musicFilename = "pnmusic.lfd";
				musicName = "plans";
				break;
		}
		g_flet640MusicState.film = sceneFilm;
		g_flet640MusicState.sound = xsound_Find_Gmid(musicName);
		if (g_flet640MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(musicFilename);
			g_flet640MusicState.sound = xsound_Res_Music(musicResource, musicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_flet640MusicState.sound);
			if (startBeat != 0) {
				soundext_ScanMidi(g_flet640MusicState.sound, 0, startBeat, 0);
			}
		}
		xsound_Set_Sound_Keep(g_flet640MusicState.sound);
		xsound_Set_Sound_User_Function(g_flet640MusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x44A980
void Flet640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene = shellext_Get_Cur_Scene();
		if (scene == XW_SCENE_TOUR_ARRIVE_DEFIANCE) {
			Sound* music = xsound_Find_Gmid("plans");
			if (music != NULL) {
				soundext_FadeVolume(music, 0, FLET640_MUSIC_FADE_DURATION);
				/* Resource pointers are outside the numeric flight-sound ID range. */
				soundext_SetPriority(0, FLET640_MUSIC_CLOSE_PRIORITY);
			}
		}
	}
}
