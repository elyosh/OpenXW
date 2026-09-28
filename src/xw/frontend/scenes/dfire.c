#include "xw/frontend/scenes/dfire.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/dfire_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/view.h>
#include <stdlib.h>

// GLOBAL: XW 0x4F7360
Actor* g_dfireBackgroundActor = NULL;

// GLOBAL: XW 0x4F7368
Rect g_dfirePreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7370
int16_t g_dfireRepeatHalfBackground = 0;

// GLOBAL: XW 0x4F7374
Actor* g_dfireCloseActor = NULL;

// GLOBAL: XW 0x4F7378
Rect g_dfireCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F7380
Film* g_dfireFilm = NULL;

// GLOBAL: XW 0x4F7384
LandruHandle g_dfireBackgroundHandle = 0;

// GLOBAL: XW 0x4F7388
XwSceneMusicHandles g_dfireMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7390
Sound* g_dfireSpeechSounds[DFIRE_SPEECH_COUNT] = { NULL };

// FUNCTION: XW 0x445D10
XwShellSceneResult DFire_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	int16_t scene;
	int16_t noSavedBackground;
	LandruDisplay_SetLowResolutionMode(1);
	scene = shellext_Get_Cur_Scene();
	switch (scene) {
		case XW_SCENE_DEATH_STAR_FIRE_1:
			xfade_AddTimedText("Imperial Death Star on approach to Alderaan.", DFIRE_ARRIVAL_START,
							   DFIRE_ARRIVAL_END, DFIRE_CAPTION_FONT, DFIRE_ARRIVAL_X, DFIRE_ARRIVAL_Y,
							   DFIRE_ARRIVAL_COLOR);
			break;
		case XW_SCENE_DEATH_STAR_FIRE_2:
			xfade_AddTimedText("You may fire when ready.", DFIRE_FIRE_START, DFIRE_FIRE_END,
							   DFIRE_CAPTION_FONT, DFIRE_FIRE_X, DFIRE_FIRE_Y, DFIRE_FIRE_COLOR);
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
			xfade_AddTimedText("You may fire when ready.", DFIRE_ORDER_START, DFIRE_ORDER_END,
							   DFIRE_CAPTION_FONT, DFIRE_ORDER_X, DFIRE_ORDER_Y, DFIRE_ORDER_COLOR);
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
			xfade_AddTimedText("Congratulations, the Death Star is", DFIRE_COMPLETE_FIRST_START,
							   DFIRE_COMPLETE_FIRST_END, DFIRE_CAPTION_FONT, DFIRE_COMPLETE_FIRST_X,
							   DFIRE_COMPLETE_FIRST_Y, DFIRE_COMPLETE_FIRST_COLOR);
			xfade_AddTimedText("complete.  The Emperor will be pleased.", DFIRE_COMPLETE_SECOND_START,
							   DFIRE_COMPLETE_SECOND_END, DFIRE_CAPTION_FONT, DFIRE_COMPLETE_SECOND_X,
							   DFIRE_COMPLETE_SECOND_Y, DFIRE_COMPLETE_SECOND_COLOR);
			xfade_AddTimedText("I think it is time we demonstrated", DFIRE_POWER_FIRST_START,
							   DFIRE_POWER_FIRST_END, DFIRE_CAPTION_FONT, DFIRE_POWER_FIRST_X,
							   DFIRE_POWER_FIRST_Y, DFIRE_POWER_FIRST_COLOR);
			xfade_AddTimedText("the full power of this station.", DFIRE_POWER_SECOND_START,
							   DFIRE_POWER_SECOND_END, DFIRE_CAPTION_FONT, DFIRE_POWER_SECOND_X,
							   DFIRE_POWER_SECOND_Y, DFIRE_POWER_SECOND_COLOR);
			break;
	}
	resourceFile = xres_Open_Resource("dfire.lfd");
	xrect_Set_Rect(&frame, 0, 0, DFIRE_BACKGROUND_WIDTH, DFIRE_BACKGROUND_HEIGHT);
	noSavedBackground = 0;
	if (scene == XW_SCENE_DEATH_STAR_FIRE_3)
		noSavedBackground = 1;
	if (scene == XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3)
		noSavedBackground = 1;
	g_dfireRepeatHalfBackground = 0;
	if (scene == XW_SCENE_DEATH_STAR_FIRE_5)
		g_dfireRepeatHalfBackground = 1;
	if (scene == XW_SCENE_DEATH_STAR_COMPLETION_PRISON)
		g_dfireRepeatHalfBackground = 1;
	if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0 &&
		!noSavedBackground) {
		if (g_dfireRepeatHalfBackground != 0)
			g_dfireBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				DFIRE_BACKGROUND_WIDTH * DFIRE_HALF_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
		else
			g_dfireBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				DFIRE_BACKGROUND_WIDTH * DFIRE_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
		xcanvas_Erase_Canvas();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_dfireBackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DFIRE_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_dfireBackgroundActor, DFire_user_Background);
		xactor_Set_Actor_Draw_Function(g_dfireBackgroundActor, DFire_draw_Background);
		g_dfireCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DFIRE_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_dfireCloseActor, DFire_user_Close);
	}
	switch (scene) {
		case XW_SCENE_DEATH_STAR_FIRE_1:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire1_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire1_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_FIRE_2:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire2_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire2_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_FIRE_3:
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3:
			g_dfireFilm =
				xfilm_Res_Callback_Film(resourceFile, "dfire3_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_FIRE_4:
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE4:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire4_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire4_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_FIRE_5:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire5_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dfire5_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dsdn4a_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dsdn4a_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dsdn4c_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "dsdn4c_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_PRISON:
			if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0)
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "prison_s", &frame, 0, 0, 0, DFire_film_Callback);
			else
				g_dfireFilm =
					xfilm_Res_Callback_Film(resourceFile, "prison_f", &frame, 0, 0, 0, DFire_film_Callback);
			break;
	}
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xfilm_Set_Film_Def_Palette(g_dfireFilm, shell->standardPalette);
	xview_Set_View_Update_Function(DFire_end_View);
	DFire_OpenMusic(resourceFile, g_dfireFilm);
	DFire_LoadSoundEffects(resourceFile, g_dfireFilm);
#ifdef XW_MODERN
	XwDFire_RunView(resourceFile, noSavedBackground);
#else
	j_xviewadd_Handle_View();
	DFire_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0 &&
		!noSavedBackground) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_dfireBackgroundHandle);
	}
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x446220
void DFire_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_DEATH_STAR_FIRE_1:
			nextScene = XW_SCENE_DEATH_STAR_FIRE_2;
			break;
		case XW_SCENE_DEATH_STAR_FIRE_2:
			nextScene = XW_SCENE_DEATH_STAR_FIRE_3;
			break;
		case XW_SCENE_DEATH_STAR_FIRE_3:
			nextScene = XW_SCENE_DEATH_STAR_FIRE_4;
			break;
		case XW_SCENE_DEATH_STAR_FIRE_4:
			nextScene = XW_SCENE_DEATH_STAR_FIRE_5;
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
			nextScene = XW_SCENE_DEATH_STAR_TARKIN_SPEECH;
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
			nextScene = XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3;
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3:
			nextScene = XW_SCENE_DEATH_STAR_COMPLETION_DFIRE4;
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE4:
			nextScene = XW_SCENE_DEATH_STAR_COMPLETION_PRISON;
			break;
		case XW_SCENE_DEATH_STAR_FIRE_5:
		case XW_SCENE_DEATH_STAR_COMPLETION_PRISON:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_dfireFilm->cur_cel == g_dfireFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x446310
int16_t DFire_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				if (xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					xactor_Set_Actor_User_Function(actor, DFire_user_DirtyBounds);
				}
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				if (xio_Is_System_Slower_Than(DFIRE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					DFire_film_Actor_To_Background(actor);
					handled = 1;
				}
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, DFire_user_Sound);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x4463B0
int16_t DFire_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	int16_t backgroundHeight;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_dfireBackgroundHandle);
	backgroundHeight =
		g_dfireRepeatHalfBackground != 0 ? DFIRE_HALF_BACKGROUND_HEIGHT : DFIRE_BACKGROUND_HEIGHT;
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						DFIRE_BACKGROUND_WIDTH, backgroundHeight, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_dfireBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4464A0
void DFire_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0)
		DFire_PlaySoundCue(cue);
}

// FUNCTION: XW 0x4464C0
void DFire_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_dfirePreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_dfirePreviousDirtyRect, &g_dfireCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_dfireCurrentDirtyRect);
	g_dfireBackgroundActor->var1 = 0;
}

// FUNCTION: XW 0x446510
int16_t DFire_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh) {
	Rect halfRect;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	if (g_dfireBackgroundActor->var1 != 0) {
		xcanvas_Erase_Canvas();
		return 1;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_dfireBackgroundHandle);
	if (g_dfireRepeatHalfBackground != 0) {
		if (g_dfirePreviousDirtyRect.top < DFIRE_HALF_BACKGROUND_HEIGHT) {
			xrect_Copy_Rect(&halfRect, &g_dfirePreviousDirtyRect);
			if (halfRect.bottom > DFIRE_HALF_BACKGROUND_HEIGHT)
				halfRect.bottom = DFIRE_HALF_BACKGROUND_HEIGHT;
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &halfRect, halfRect.left, halfRect.top,
										  DFIRE_BACKGROUND_WIDTH, DFIRE_HALF_BACKGROUND_HEIGHT);
		}
		if (g_dfirePreviousDirtyRect.bottom > DFIRE_HALF_BACKGROUND_HEIGHT) {
			xrect_Copy_Rect(&halfRect, &g_dfirePreviousDirtyRect);
			xrect_Offset_Rect(&halfRect, 0, -DFIRE_HALF_BACKGROUND_HEIGHT);
			if (halfRect.top < 0)
				halfRect.top = 0;
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &halfRect, halfRect.left,
										  halfRect.top + DFIRE_HALF_BACKGROUND_HEIGHT, DFIRE_BACKGROUND_WIDTH,
										  DFIRE_HALF_BACKGROUND_HEIGHT);
		}
	} else {
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_dfirePreviousDirtyRect,
									  g_dfirePreviousDirtyRect.left, g_dfirePreviousDirtyRect.top,
									  DFIRE_BACKGROUND_WIDTH, DFIRE_BACKGROUND_HEIGHT);
	}
	xmemhdl_Unlock_Handle(g_dfireBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x446640
void DFire_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_dfireCurrentDirtyRect, &actorBounds);
	}
	if (actor->var2 != 0) {
		g_dfireBackgroundActor->var1 = DFIRE_ERASE_FULL_CANVAS;
	}
}

// FUNCTION: XW 0x4466C0
void DFire_user_Close(Actor* unusedActor, int unusedTime) {
	Rect frame;
	(void)unusedActor;
	(void)unusedTime;
	if (g_dfireBackgroundActor->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	} else {
		xrect_Copy_Rect(&frame, &g_dfirePreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_dfireCurrentDirtyRect);
	}
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x446740
void DFire_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene;
		g_dfireMusicState.film = sceneFilm;
		g_dfireMusicState.sound = xsound_Find_Gmid("empire");
		scene = (int16_t)shellext_Get_Cur_Scene();
		if (g_dfireMusicState.sound == NULL) {
			unsigned int startBeat;
			ResFile* musicResource;
			switch (scene) {
				case XW_SCENE_DEATH_STAR_FIRE_2:
					startBeat = DFIRE_MUSIC_FIRE_2_BEAT;
					break;
				case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
					startBeat = DFIRE_MUSIC_SPEECH_BEAT;
					break;
				case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
					startBeat = DFIRE_MUSIC_FIRE_ORDER_BEAT;
					break;
				case XW_SCENE_DEATH_STAR_FIRE_3:
				case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3:
					startBeat = DFIRE_MUSIC_FIRE_3_BEAT;
					break;
				case XW_SCENE_DEATH_STAR_FIRE_4:
				case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE4:
					startBeat = DFIRE_MUSIC_FIRE_4_BEAT;
					break;
				case XW_SCENE_DEATH_STAR_FIRE_5:
				case XW_SCENE_DEATH_STAR_COMPLETION_PRISON:
					startBeat = DFIRE_MUSIC_FIRE_5_BEAT;
					break;
				default:
					startBeat = 0;
					break;
			}
			musicResource = xres_Open_Resource("ddmusic.lfd");
			g_dfireMusicState.sound = xsound_Res_Music(musicResource, "empire");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_dfireMusicState.sound);
			if (startBeat != 0)
				soundext_ScanMidi(g_dfireMusicState.sound, 0, startBeat, 0);
		}
		if (scene == XW_SCENE_DEATH_STAR_FIRE_1)
			soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT, DFIRE_MUSIC_INITIAL_CONTROL,
							 0);
		xsound_Set_Sound_Keep(g_dfireMusicState.sound);
		xsound_Set_Sound_User_Function(g_dfireMusicState.sound, DFire_user_Music);
	}
}

// FUNCTION: XW 0x4468A0
void DFire_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene = shellext_Get_Cur_Scene();
		if (scene == XW_SCENE_DEATH_STAR_COMPLETION_PRISON || scene == XW_SCENE_DEATH_STAR_FIRE_5) {
			Sound* music = xsound_Find_Gmid("empire");
			g_dfireMusicState.sound = music;
			if (music != NULL) {
#ifdef XW_MODERN
				Dos94_soundext_SetPriority(g_dfireMusicState.sound, 0);
#else
				soundext_SetPriority(0, 0);
#endif
				soundext_FadeVolume(g_dfireMusicState.sound, 0, DFIRE_MUSIC_FADE_DURATION);
			}
		}
	}
}

// FUNCTION: XW 0x446900
void DFire_user_Music(Sound* unusedSound, int unusedTime) {
	uint16_t filmCel = g_dfireMusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_DEATH_STAR_FIRE_1:
			if (filmCel == DFIRE_MUSIC_FIRE_TRANSITION_CEL) {
				soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT, DFIRE_MUSIC_FIRE_1_CONTROL,
								 0);
			}
			break;
		case XW_SCENE_DEATH_STAR_FIRE_2:
			if (filmCel == DFIRE_MUSIC_FIRE_TRANSITION_CEL) {
				soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT, DFIRE_MUSIC_FIRE_2_CONTROL,
								 0);
			}
			break;
		case XW_SCENE_DEATH_STAR_FIRE_5:
			if (filmCel == DFIRE_MUSIC_FIRE_5_START_CEL) {
				soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT,
								 DFIRE_MUSIC_FINALE_START_CONTROL, 0);
			} else if (filmCel == DFIRE_MUSIC_FIRE_5_END_CEL) {
				soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT,
								 DFIRE_MUSIC_FINALE_END_CONTROL, 0);
			}
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
			if (filmCel == DFIRE_MUSIC_DIALOGUE_START_CEL) {
				if (ShellPreferences_GetSfxEnabled() != 0)
					soundext_FadeVolume(g_dfireMusicState.sound, DFIRE_MUSIC_SPEECH_VOLUME,
										DFIRE_MUSIC_DIALOGUE_FADE_DURATION);
			} else if (filmCel == DFIRE_MUSIC_SPEECH_END_CEL) {
				if (ShellPreferences_GetSfxEnabled() != 0)
					soundext_FadeVolume(g_dfireMusicState.sound, DFIRE_MUSIC_FULL_VOLUME,
										DFIRE_MUSIC_DIALOGUE_FADE_DURATION);
			}
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
			if (filmCel == DFIRE_MUSIC_DIALOGUE_START_CEL) {
				if (ShellPreferences_GetSfxEnabled() != 0)
					soundext_FadeVolume(g_dfireMusicState.sound, DFIRE_MUSIC_FIRE_ORDER_VOLUME,
										DFIRE_MUSIC_DIALOGUE_FADE_DURATION);
			} else if (filmCel == DFIRE_MUSIC_FIRE_ORDER_END_CEL) {
				if (ShellPreferences_GetSfxEnabled() != 0)
					soundext_FadeVolume(g_dfireMusicState.sound, DFIRE_MUSIC_FULL_VOLUME,
										DFIRE_MUSIC_DIALOGUE_FADE_DURATION);
			}
			break;
		case XW_SCENE_DEATH_STAR_COMPLETION_PRISON:
			if (filmCel == DFIRE_MUSIC_PRISON_START_CEL) {
				soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT,
								 DFIRE_MUSIC_FINALE_START_CONTROL, 0);
			} else if (filmCel == DFIRE_MUSIC_PRISON_END_CEL) {
				soundext_SetHook(g_dfireMusicState.sound, XW_SOUND_CONTROL_DIRECT,
								 DFIRE_MUSIC_FINALE_END_CONTROL, 0);
			}
			break;
		default:
			return;
	}
}

// FUNCTION: XW 0x446AB0
void DFire_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	int16_t scene;
	(void)unusedResourceFile;
	(void)unusedFilm;
	scene = shellext_Get_Cur_Scene();
	switch (scene) {
		case XW_SCENE_DEATH_STAR_FIRE_2:
			if (ShellPreferences_GetSfxEnabled())
				soundext_LoadSfx(XW_SHELL_SFX_BEEP_5_SECONDARY, 0, NULL, 0, 0);
			if (ShellPreferences_GetSfxEnabled())
				g_dfireSpeechSounds[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_TARKIN_1, 0, NULL, 0);
			return;
		case XW_SCENE_DEATH_STAR_FIRE_3:
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE3:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_LoadSfx(XW_SHELL_SFX_ALARM, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_BEEP_6_SECONDARY, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_STAR_FIRE_1, 0, NULL, 1, 0);
			}
			return;
		case XW_SCENE_DEATH_STAR_FIRE_4:
		case XW_SCENE_DEATH_STAR_COMPLETION_DFIRE4:
			if (ShellPreferences_GetSfxEnabled())
				soundext_LoadSfx(XW_SHELL_SFX_STAR_FIRE_2, 0, NULL, 0, 0);
			return;
		case XW_SCENE_DEATH_STAR_FIRE_5:
		case XW_SCENE_DEATH_STAR_COMPLETION_PRISON:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_LoadSfx(XW_SHELL_SFX_STAR_FIRE_3, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_BIG_SECONDARY, 0, NULL, 0, 1);
			}
			return;
		case XW_SCENE_DEATH_STAR_COMPLETION_SPEECH:
			if (ShellPreferences_GetSfxEnabled())
				soundext_LoadSfx(XW_SHELL_SFX_BEEP_5_SECONDARY, 0, NULL, 0, 0);
			if (ShellPreferences_GetSfxEnabled()) {
				g_dfireSpeechSounds[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_VADRGRAT, 0, NULL, 0);
				g_dfireSpeechSounds[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_COMPLETE, 0, NULL, 0);
				g_dfireSpeechSounds[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_PLEASED, 0, NULL, 0);
				g_dfireSpeechSounds[3] = soundext_LoadSpeech(XW_SHELL_SPEECH_TARKIN_2, 0, NULL, 0);
			}
			return;
		case XW_SCENE_DEATH_STAR_COMPLETION_FIRE_ORDER:
			if (ShellPreferences_GetSfxEnabled())
				g_dfireSpeechSounds[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_TARKIN_1, 0, NULL, 0);
			return;
		default:
			return;
	}
}

// FUNCTION: XW 0x446C80
void DFire_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case DFIRE_CUE_BEEP_5:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_5_SECONDARY);
			break;
		case DFIRE_CUE_BEEP_6:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_6_SECONDARY);
			break;
		case DFIRE_CUE_EXPLOSION:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_BIG_SECONDARY);
			break;
		case DFIRE_CUE_ALARM:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_ALARM);
			break;
		case DFIRE_CUE_FIRE_1:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_STAR_FIRE_1);
			break;
		case DFIRE_CUE_FIRE_2:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_STAR_FIRE_2);
			break;
		case DFIRE_CUE_FIRE_3:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_STAR_FIRE_3);
			break;
		case DFIRE_CUE_STOP_FIRE_3:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Stop_SFX(XW_SHELL_SFX_STAR_FIRE_3);
			break;
		case DFIRE_CUE_SPEECH_1:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_dfireSpeechSounds[0]);
			break;
		case DFIRE_CUE_SPEECH_2:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_dfireSpeechSounds[1]);
			break;
		case DFIRE_CUE_SPEECH_3:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_dfireSpeechSounds[2]);
			break;
		case DFIRE_CUE_SPEECH_4:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_dfireSpeechSounds[3]);
			break;
		case DFIRE_CUE_REPEAT_SPEECH_1:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_dfireSpeechSounds[0]);
			break;
	}
}
