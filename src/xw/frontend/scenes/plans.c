#include "xw/frontend/scenes/plans.h"
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
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/plans_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4F9258
Actor* g_plansBackgroundActor = NULL;

// GLOBAL: XW 0x4F925C
Film* g_plansFilm = NULL;

// GLOBAL: XW 0x4F9260
Rect g_plansPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F9268
Actor* g_plansCloseActor = NULL;

// GLOBAL: XW 0x4F9270
Rect g_plansCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F9278
LandruHandle g_plansBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F9284
XwSceneMusicHandles g_plansMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F9290
Sound* g_plansSpeech[PLANS_SPEECH_COUNT] = { NULL, NULL };

// FUNCTION: XW 0x45AF20
XwShellSceneResult Plans_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Rect frame;
	int16_t useNormalErase;
	LandruDisplay_SetLowResolutionMode(1);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_RECOVER_PLANS_1:
			xfade_AddTimedText("Enroute to the planet Alderaan, the Corellian Corvette",
							   PLANS_INTRO_FIRST_START, PLANS_INTRO_FIRST_END, PLANS_CAPTION_FONT,
							   PLANS_INTRO_FIRST_X, PLANS_INTRO_FIRST_Y, PLANS_INTRO_FIRST_COLOR);
			xfade_AddTimedText("transporting Princess Leia receives a vital message.",
							   PLANS_INTRO_SECOND_START, PLANS_INTRO_SECOND_END, PLANS_CAPTION_FONT,
							   PLANS_INTRO_SECOND_X, PLANS_INTRO_SECOND_Y, PLANS_INTRO_SECOND_COLOR);
			break;
		case XW_SCENE_RECOVER_PLANS_2:
			xfade_AddTimedText("Here are the secret plans, Princess Leia.", PLANS_PLANS_START,
							   PLANS_PLANS_END, PLANS_CAPTION_FONT, PLANS_PLANS_X, PLANS_PLANS_Y,
							   PLANS_PLANS_COLOR);
			xfade_AddTimedText("I will bring them safely to Alderaan.", PLANS_LEIA_START, PLANS_LEIA_END,
							   PLANS_CAPTION_FONT, PLANS_LEIA_X, PLANS_LEIA_Y, PLANS_LEIA_COLOR);
			break;
		case XW_SCENE_RECOVER_PLANS_VADER:
			xfade_AddTimedText("I have you now!", PLANS_VADER_START, PLANS_VADER_END, PLANS_CAPTION_FONT,
							   PLANS_VADER_X, PLANS_VADER_Y, PLANS_VADER_COLOR);
			break;
	}
	sceneResource = xres_Open_Resource("plans.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if (shellext_Get_Cur_Scene() == XW_SCENE_RECOVER_PLANS_2) {
		g_plansBackgroundHandle = xmemhdl_Alloc_Clear_Handle(PLANS_BACKGROUND_WIDTH * PLANS_BACKGROUND_HEIGHT,
															 LANDRU_MEMORY_RESOURCE);
		g_plansFilm = xfilm_Res_Callback_Film(sceneResource, "plans2_f", &frame, 0, 0, 0,
											  Plans_film_SavedBackgroundCallback);
		useNormalErase = 0;
	} else if (shellext_Get_Cur_Scene() == XW_SCENE_RECOVER_PLANS_1) {
		if ((uint16_t)xio_Is_System_Slower_Than(PLANS_SPEED_THRESHOLD) != 0) {
			g_plansBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				PLANS_BACKGROUND_WIDTH * PLANS_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
			g_plansFilm = xfilm_Res_Callback_Film(sceneResource, "plans1_s", &frame, 0, 0, 0,
												  Plans_film_SavedBackgroundCallback);
			useNormalErase = 0;
		} else {
			g_plansFilm = xfilm_Res_Callback_Film(sceneResource, "plans1_f", &frame, 0, 0, 0,
												  Plans_film_NormalCallback);
			useNormalErase = 1;
		}
	} else {
		if ((uint16_t)xio_Is_System_Slower_Than(PLANS_SPEED_THRESHOLD) != 0) {
			g_plansBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				PLANS_BACKGROUND_WIDTH * PLANS_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
			g_plansFilm = xfilm_Res_Callback_Film(sceneResource, "plans4_s", &frame, 0, 0, 0,
												  Plans_film_SavedBackgroundCallback);
			useNormalErase = 0;
		} else {
			g_plansFilm = xfilm_Res_Callback_Film(sceneResource, "plans4_f", &frame, 0, 0, 0,
												  Plans_film_NormalCallback);
			useNormalErase = 1;
		}
	}
	xfilm_Set_Film_Def_Palette(g_plansFilm, shell->standardPalette);
	if (useNormalErase == 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_plansBackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, PLANS_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_plansBackgroundActor, Plans_user_Background);
		xactor_Set_Actor_Draw_Function(g_plansBackgroundActor, Plans_draw_Background);
		g_plansCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, PLANS_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_plansCloseActor, Plans_user_Close);
		xactor_Set_Actor_Draw_Function(g_plansCloseActor, XwCutscene_DrawCloseOnRefresh);
	}
	xview_Set_View_Update_Function(Plans_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Plans_OpenMusic(sceneResource, g_plansFilm);
	Plans_LoadSoundEffects(sceneResource, g_plansFilm);
#ifdef XW_MODERN
	XwPlans_RunView(sceneResource, useNormalErase);
#else
	j_xviewadd_Handle_View();
	Plans_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if (useNormalErase == 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_plansBackgroundHandle);
	}
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x45B250
void Plans_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_RECOVER_PLANS_1:
			nextScene = XW_SCENE_RECOVER_PLANS_2;
			break;
		case XW_SCENE_RECOVER_PLANS_2:
			nextScene = XW_SCENE_RECOVER_PLANS_FLEET;
			break;
		case XW_SCENE_RECOVER_PLANS_VADER:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
#ifdef XW_MODERN
		default:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_plansFilm->cur_cel == g_plansFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x45B2C0
int16_t Plans_film_SavedBackgroundCallback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Plans_user_DirtyActor);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Plans_film_Actor_To_Background(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Plans_user_SoundCue);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x45B340
int16_t Plans_film_NormalCallback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND)
			xactor_Set_Actor_User_Function(actor, Plans_user_SoundCue);
	}
	return 0;
}

// FUNCTION: XW 0x45B380
int16_t Plans_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_plansBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						PLANS_BACKGROUND_WIDTH, PLANS_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_plansBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x45B460
void Plans_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0)
		Plans_PlaySoundCue(cue);
}

// FUNCTION: XW 0x45B480
void Plans_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_plansPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_plansPreviousDirtyRect, &g_plansCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_plansCurrentDirtyRect);
}

// FUNCTION: XW 0x45B4D0
int16_t Plans_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_plansBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_plansPreviousDirtyRect, g_plansPreviousDirtyRect.left,
								  g_plansPreviousDirtyRect.top, PLANS_BACKGROUND_WIDTH,
								  PLANS_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_plansBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x45B530
void Plans_user_DirtyActor(Actor* actor, int unusedTime) {
	Rect visibleBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_plansCurrentDirtyRect, &visibleBounds);
	}
	if (actor->var2 != 0) {
		g_plansFilm->var1 = PLANS_REFRESH_FULL_CANVAS;
	}
}

// FUNCTION: XW 0x45B5B0
void Plans_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_plansFilm->cur_cel == g_plansFilm->cels || g_plansFilm->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		g_plansFilm->var1 = 0;
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_plansPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_plansCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x45B660
void Plans_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_plansMusicState.film = sceneFilm;
		g_plansMusicState.sound = xsound_Find_Gmid("plans");
		if (g_plansMusicState.sound == NULL) {
			unsigned int startBeat;
			ResFile* musicResource;
			switch (shellext_Get_Cur_Scene()) {
				case XW_SCENE_RECOVER_PLANS_2:
					startBeat = PLANS_MUSIC_SECOND_START_BEAT;
					break;
				case XW_SCENE_RECOVER_PLANS_VADER:
					startBeat = PLANS_MUSIC_VADER_START_BEAT;
					break;
				default:
					startBeat = 0;
					break;
			}
			musicResource = xres_Open_Resource("pnmusic.lfd");
			g_plansMusicState.sound = xsound_Res_Music(musicResource, "plans");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_plansMusicState.sound);
			if (startBeat != 0) {
				soundext_ScanMidi(g_plansMusicState.sound, 0, startBeat, 0);
			}
		}
		soundext_SetHook(g_plansMusicState.sound, XW_SOUND_CONTROL_PACKED, 1, PLANS_MUSIC_CHANNEL);
		xsound_Set_Sound_Keep(g_plansMusicState.sound);
		xsound_Set_Sound_User_Function(g_plansMusicState.sound, Plans_user_Music);
	}
}

// FUNCTION: XW 0x45B740
void Plans_CloseMusic(void) {
	int scene = shellext_Get_Cur_Scene();
	if (ShellPreferences_GetMusicEnabled() != 0 && scene == XW_SCENE_RECOVER_PLANS_VADER) {
		Sound* music = xsound_Find_Gmid("plans");
		g_plansMusicState.sound = music;
		if (music != NULL) {
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(g_plansMusicState.sound, 0);
#else
			soundext_SetPriority(0, 0);
#endif
			soundext_FadeVolume(g_plansMusicState.sound, 0, PLANS_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x45B7A0
void Plans_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_RECOVER_PLANS_2:
			switch (g_plansMusicState.film->cur_cel) {
				case PLANS_MUSIC_SECOND_QUIET_CEL:
					if (ShellPreferences_GetSfxEnabled() != 0)
						soundext_FadeVolume(g_plansMusicState.sound, PLANS_MUSIC_QUIET_VOLUME,
											PLANS_MUSIC_LEVEL_DURATION);
					break;
				case PLANS_MUSIC_SECOND_LOUD_CEL:
					if (ShellPreferences_GetSfxEnabled() != 0)
						soundext_FadeVolume(g_plansMusicState.sound, PLANS_MUSIC_LOUD_VOLUME,
											PLANS_MUSIC_LEVEL_DURATION);
					break;
			}
			break;
		case XW_SCENE_RECOVER_PLANS_VADER:
			switch (g_plansMusicState.film->cur_cel) {
				case PLANS_MUSIC_VADER_QUIET_CEL:
					if (ShellPreferences_GetSfxEnabled() != 0)
						soundext_FadeVolume(g_plansMusicState.sound, PLANS_MUSIC_QUIET_VOLUME,
											PLANS_MUSIC_LEVEL_DURATION);
					break;
				case PLANS_MUSIC_VADER_LOUD_CEL:
					if (ShellPreferences_GetSfxEnabled() != 0)
						soundext_FadeVolume(g_plansMusicState.sound, PLANS_MUSIC_LOUD_VOLUME,
											PLANS_MUSIC_LEVEL_DURATION);
					break;
			}
			break;
	}
}

// FUNCTION: XW 0x45B870
void Plans_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_RECOVER_PLANS_1:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_MESSAGE, 0, NULL, 0, 0);
			}
			break;
		case XW_SCENE_RECOVER_PLANS_2:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				g_plansSpeech[PLANS_SPEECH_FIRST] = soundext_LoadSpeech(XW_SHELL_SPEECH_ANTILLES, 0, NULL, 0);
				g_plansSpeech[PLANS_SPEECH_SECOND] =
					soundext_LoadSpeech(XW_SHELL_SPEECH_PRINCESS, 0, NULL, 0);
			}
			break;
		case XW_SCENE_RECOVER_PLANS_VADER:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_MESSAGE, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_BREATH, 0, NULL, 0, 1);
			}
			if (ShellPreferences_GetSfxEnabled() != 0) {
				g_plansSpeech[PLANS_SPEECH_FIRST] =
					soundext_LoadSpeech(XW_SHELL_SPEECH_HAVE_YOU_NOW, 0, NULL, 0);
			}
			break;
	}
}

// FUNCTION: XW 0x45B930
void Plans_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case PLANS_CUE_MESSAGE:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_MESSAGE);
			break;
		case PLANS_CUE_BREATH:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Play_SFX(XW_SHELL_SFX_BREATH);
			break;
		case PLANS_CUE_SPEECH_FIRST:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Start_Resource_SFX(g_plansSpeech[PLANS_SPEECH_FIRST]);
			break;
		case PLANS_CUE_SPEECH_SECOND:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Start_Resource_SFX(g_plansSpeech[PLANS_SPEECH_SECOND]);
			break;
		case PLANS_CUE_SPEECH_FIRST_REPEAT:
			if (ShellPreferences_GetSfxEnabled())
				soundext_Start_Resource_SFX(g_plansSpeech[PLANS_SPEECH_FIRST]);
			break;
	}
}
