#include "xw/frontend/scenes/ds_done.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/dsdone_task.h"
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

// GLOBAL: XW 0x4F6A8C
XwSceneMusicHandles g_dsdoneMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7430
Actor* g_dsdoneBackgroundActor = NULL;

// GLOBAL: XW 0x4F7438
Rect g_dsdonePreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7440
Film* g_dsdoneFilm = NULL;

// GLOBAL: XW 0x4F7444
Actor* g_dsdoneCloseActor = NULL;

// GLOBAL: XW 0x4F7448
Rect g_dsdoneCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F7450
LandruHandle g_dsdoneBackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x442380
void DsDone_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_dsdoneMusicState.film = sceneFilm;
		g_dsdoneMusicState.sound = xsound_Find_Gmid("empire");
		if (g_dsdoneMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("ddmusic.lfd");
			g_dsdoneMusicState.sound = xsound_Res_Music(musicResource, "empire");
			soundext_Start_Resource_Sound(g_dsdoneMusicState.sound);
			xres_Close_Resource(musicResource);
		}
		soundext_SetHook(g_dsdoneMusicState.sound, XW_SOUND_CONTROL_DIRECT, DSDONE_MUSIC_CONTROL_VALUE, 0);
		xsound_Set_Sound_Keep(g_dsdoneMusicState.sound);
		xsound_Set_Sound_User_Function(g_dsdoneMusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x442420
void DsDone_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_1, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x442450
void DsDone_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_DS_DONE_CUE_TIE_APPROACH:
				soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_1);
				break;
		}
	}
}

// FUNCTION: XW 0x4481B0
XwShellSceneResult DsDone_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("Imperial Death Star in final stages of construction.", DSDONE_CAPTION_START,
					   DSDONE_CAPTION_END, DSDONE_CAPTION_FONT, DSDONE_CAPTION_X, DSDONE_CAPTION_Y,
					   DSDONE_CAPTION_COLOR);
	sceneResource = xres_Open_Resource("dsdone.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(DSDONE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_dsdoneBackgroundHandle =
			xmemhdl_Alloc_Clear_Handle(DSDONE_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
		g_dsdoneFilm =
			xfilm_Res_Callback_Film(sceneResource, "dsdone_s", &frame, 0, 0, 0, DsDone_film_Callback);
		g_dsdoneBackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DSDONE_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_dsdoneBackgroundActor, DsDone_user_Background);
		xactor_Set_Actor_Draw_Function(g_dsdoneBackgroundActor, DsDone_draw_Background);
		g_dsdoneCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DSDONE_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_dsdoneCloseActor, DsDone_user_Close);
		xactor_Set_Actor_Draw_Function(g_dsdoneCloseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		g_dsdoneFilm =
			xfilm_Res_Callback_Film(sceneResource, "dsdone_f", &frame, 0, 0, 0, DsDone_film_Callback);
	}
	xfilm_Set_Film_Def_Palette(g_dsdoneFilm, shell->standardPalette);
	DsDone_OpenMusic(sceneResource, g_dsdoneFilm);
	DsDone_LoadSoundEffects(sceneResource, g_dsdoneFilm);
	xview_Set_View_Update_Function(DsDone_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwDsDone_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DSDONE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_dsdoneBackgroundHandle);
	}
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4483D0
void DsDone_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DEATH_STAR_HANGAR, shipext_Get_Pending_Medal_Scene(),
								  g_dsdoneFilm->cur_cel == g_dsdoneFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x448410
int16_t DsDone_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (xio_Is_System_Slower_Than(DSDONE_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
				DsDone_film_Actor_To_Background(actor);
				consumeObject = 1;
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
				xactor_Set_Actor_User_Function(actor, DsDone_user_DirtyBounds);
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
				xactor_Set_Actor_User_Function(actor, DsDone_user_Sound);
			}
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, DsDone_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4484B0
int16_t DsDone_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_dsdoneBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						DSDONE_BACKGROUND_WIDTH, DSDONE_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_dsdoneBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x448590
void DsDone_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		DsDone_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x4485B0
void DsDone_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_dsdonePreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_dsdonePreviousDirtyRect, &g_dsdoneCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_dsdoneCurrentDirtyRect);
}

// FUNCTION: XW 0x448600
int16_t DsDone_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_dsdoneBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_dsdonePreviousDirtyRect,
								  g_dsdonePreviousDirtyRect.left, g_dsdonePreviousDirtyRect.top,
								  DSDONE_BACKGROUND_WIDTH, DSDONE_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_dsdoneBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x448660
void DsDone_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_dsdoneCurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x4486D0
void DsDone_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_dsdoneFilm->cur_cel == g_dsdoneFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_dsdonePreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_dsdoneCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
