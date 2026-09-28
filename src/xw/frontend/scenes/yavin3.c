#include "xw/frontend/scenes/yavin3.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/yavin3_task.h"
#endif

#include <landru/actcust.h>
#include <landru/cursor.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/viewadd.h>

#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/view.h>

// GLOBAL: XW 0x4FC438
Rect g_yavin3CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FC440
Actor* g_yavin3BackgroundActor = NULL;

// GLOBAL: XW 0x4FC444
Actor* g_yavin3ScrollActor = NULL;

// GLOBAL: XW 0x4FC448
Film* g_yavin3Film = NULL;

// GLOBAL: XW 0x4FC450
Rect g_yavin3PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FC458
Actor* g_yavin3CloseActor = NULL;

// GLOBAL: XW 0x4FC45C
LandruHandle g_yavin3BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FC460
int16_t g_yavin3FullRefreshCountdown = 0;

// GLOBAL: XW 0x4FC464
int16_t g_yavin3LastDrawnX = 0;

// GLOBAL: XW 0x4FC498
XwSceneMusicHandles g_yavin3MusicState = { NULL, NULL };

// GLOBAL: XW 0x4FC4A8
Film* g_yavin3SoundEffectsFilm = NULL;

// FUNCTION: XW 0x46AC80
XwShellSceneResult Yavin3_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("Yavin Rebel Base.  Headquarters of the Rebellion.", YAVIN3_CAPTION_START,
					   YAVIN3_CAPTION_END, YAVIN3_CAPTION_FONT, YAVIN3_CAPTION_X, YAVIN3_CAPTION_Y,
					   YAVIN3_CAPTION_COLOR);
	sceneResource = xres_Open_Resource("yavin3.lfd");
	xrect_Set_Rect(&frame, 0, 0, YAVIN3_BACKGROUND_WIDTH, YAVIN3_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	g_yavin3LastDrawnX = 0;
	g_yavin3FullRefreshCountdown = 0;
	g_yavin3BackgroundHandle = xmemhdl_Alloc_Clear_Handle(YAVIN3_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	if ((uint16_t)xio_Is_System_Slower_Than(YAVIN3_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "yavin3_s", &frame, 0, 0, 0, Yavin3_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "yavin3_f", &frame, 0, 0, 0, Yavin3_film_Callback);
	g_yavin3Film = sceneFilm;
	xfilm_Set_Film_Def_Palette(sceneFilm, shell->standardPalette);
	g_yavin3BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, YAVIN3_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_yavin3BackgroundActor, Yavin3_draw_Background);
	g_yavin3CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, YAVIN3_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_yavin3CloseActor, Yavin3_user_Close);
	xactor_Set_Actor_Draw_Function(g_yavin3CloseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_yavin3CurrentDirtyRect);
	xrect_Clear_Rect(&g_yavin3PreviousDirtyRect);
	xview_Set_View_Update_Function(Yavin3_end_View);
	Yavin3_OpenMusic(sceneResource, g_yavin3Film);
	Yavin3_LoadSoundEffects(sceneResource, g_yavin3Film);
#ifdef XW_MODERN
	XwYavin3_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_yavin3BackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x46AE90
void Yavin3_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_VICTORY_MEDAL_CEREMONY_1,
								  shipext_Get_Pending_Medal_Scene(),
								  g_yavin3Film->cur_cel == g_yavin3Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x46AED0
int16_t Yavin3_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Yavin3_user_DirtyActor);
				break;
			case YAVIN3_ACTOR_ROLE_SCROLL:
				g_yavin3ScrollActor = actor;
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Yavin3_film_Actor_To_Background(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Yavin3_user_SoundCue);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x46AFA0
int16_t Yavin3_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_yavin3BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						YAVIN3_BACKGROUND_WIDTH, YAVIN3_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		drawResult = actor->draw(actor, &canvasBounds, &canvasBounds, actor->x, actor->y, 1);
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_yavin3BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x46B050
void Yavin3_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Yavin3_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x46B070
int16_t Yavin3_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh) {
	Rect canvasBounds;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_yavin3BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &canvasBounds, g_yavin3ScrollActor->x, 0,
								  YAVIN3_BACKGROUND_WIDTH, YAVIN3_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_yavin3BackgroundHandle);
	g_yavin3LastDrawnX = g_yavin3ScrollActor->x;
	return 1;
}

// FUNCTION: XW 0x46B0F0
void Yavin3_user_DirtyActor(Actor* actor, int unusedTime) {
	Rect visibleBounds;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Enclose_Rect(&g_yavin3CurrentDirtyRect, &visibleBounds);
	}
}

// FUNCTION: XW 0x46B140
void Yavin3_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_yavin3Film->cur_cel == g_yavin3Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = YAVIN3_CLOSE_FILM_COMPLETE;
	} else {
		if (g_yavin3ScrollActor->xv != 0 || g_yavin3ScrollActor->yv != 0 || g_yavin3ScrollActor->xvf != 0 ||
			g_yavin3ScrollActor->yvf != 0)
			g_yavin3FullRefreshCountdown = YAVIN3_FULL_REFRESH_FRAMES;
		if (g_yavin3FullRefreshCountdown != 0 || g_yavin3ScrollActor->var2 != 0) {
			xcanvas_Get_Drawing_Canvas_Bounds(&frame);
			if (g_yavin3FullRefreshCountdown != 0)
				--g_yavin3FullRefreshCountdown;
		} else {
			xrect_Copy_Rect(&frame, &g_yavin3CurrentDirtyRect);
			xrect_Enclose_Rect(&frame, &g_yavin3PreviousDirtyRect);
			xcanvas_Clip_Rect_To_Canvas(&frame);
		}
		actor->var1 = 0;
	}
	xrect_Copy_Rect(&g_yavin3PreviousDirtyRect, &g_yavin3CurrentDirtyRect);
	xrect_Clear_Rect(&g_yavin3CurrentDirtyRect);
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x46B4C0
void Yavin3_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_yavin3MusicState.film = sceneFilm;
		g_yavin3MusicState.sound = xsound_Find_Gmid("victory");
		if (g_yavin3MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("yvmusic.lfd");
			g_yavin3MusicState.sound = xsound_Res_Music(musicResource, "victory");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_yavin3MusicState.sound);
			soundext_ScanMidi(g_yavin3MusicState.sound, 0, YAVIN3_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_yavin3MusicState.sound);
		xsound_Set_Sound_User_Function(g_yavin3MusicState.sound, Yavin3_user_Music);
		g_yavin3MusicState.sound->var1 = 0;
	}
}

// FUNCTION: XW 0x46B570
void Yavin3_user_Music(Sound* sound, int unusedTime) {
	(void)unusedTime;
	if (soundext_GetMusicParam(sound, XW_SOUND_QUERY_BEAT, 0) > YAVIN3_MUSIC_FULL_LEVEL_AFTER_BEAT &&
		sound->var1 == 0) {
		soundext_FadeVolume(sound, YAVIN3_MUSIC_FULL_LEVEL, 0);
		sound->var1 = 1;
	}
}

// FUNCTION: XW 0x46B5B0
void Yavin3_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	g_yavin3SoundEffectsFilm = sceneFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x46B5E0
void Yavin3_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_YAVIN3_CUE_FLYBY:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
				break;
		}
	}
}
