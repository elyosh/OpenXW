#include "xw/frontend/scenes/yavin1.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/yavin1_task.h"
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

// GLOBAL: XW 0x4FC3E8
Actor* g_yavin1BackgroundActor = NULL;

// GLOBAL: XW 0x4FC3EC
Film* g_yavin1Film = NULL;

// GLOBAL: XW 0x4FC3F0
Rect g_yavin1PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FC3F8
Actor* g_yavin1CloseActor = NULL;

// GLOBAL: XW 0x4FC400
Rect g_yavin1CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FC408
LandruHandle g_yavin1BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FC468
XwSceneMusicHandles g_yavin1MusicState = { NULL, NULL };

// GLOBAL: XW 0x4FC47C
Film* g_yavin1SoundEffectsFilm = NULL;

// FUNCTION: XW 0x46A190
XwShellSceneResult Yavin1_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("Approaching Yavin Moon Base.", YAVIN1_CAPTION_START, YAVIN1_CAPTION_END,
					   YAVIN1_CAPTION_FONT, YAVIN1_CAPTION_X, YAVIN1_CAPTION_Y, YAVIN1_CAPTION_COLOR);
	sceneResource = xres_Open_Resource("yavin1.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_yavin1BackgroundHandle = xmemhdl_Alloc_Clear_Handle(YAVIN1_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(YAVIN1_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "yavin1_s", &frame, 0, 0, 0, Yavin1_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "yavin1_f", &frame, 0, 0, 0, Yavin1_film_Callback);
	g_yavin1Film = sceneFilm;
	g_yavin1BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, YAVIN1_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_yavin1BackgroundActor, Yavin1_user_Background);
	xactor_Set_Actor_Draw_Function(g_yavin1BackgroundActor, Yavin1_draw_Background);
	g_yavin1CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, YAVIN1_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_yavin1CloseActor, Yavin1_user_Close);
	xactor_Set_Actor_Draw_Function(g_yavin1CloseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_yavin1Film, shell->standardPalette);
	Yavin1_OpenMusic(sceneResource, g_yavin1Film);
	Yavin1_LoadSoundEffects(sceneResource, g_yavin1Film);
	xview_Set_View_Update_Function(Yavin1_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwYavin1_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_yavin1BackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x46A390
void Yavin1_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_YAVIN_FILM_2, shipext_Get_Pending_Medal_Scene(),
								  g_yavin1Film->cur_cel == g_yavin1Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x46A3D0
int16_t Yavin1_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Yavin1_user_DirtyActor);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Yavin1_film_Actor_To_Background(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Yavin1_user_SoundCue);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x46A450
int16_t Yavin1_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_yavin1BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						YAVIN1_BACKGROUND_WIDTH, YAVIN1_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_yavin1BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x46A530
void Yavin1_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Yavin1_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x46A550
void Yavin1_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_yavin1PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_yavin1PreviousDirtyRect, &g_yavin1CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_yavin1CurrentDirtyRect);
}

// FUNCTION: XW 0x46A5A0
int16_t Yavin1_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_yavin1BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_yavin1PreviousDirtyRect,
								  g_yavin1PreviousDirtyRect.left, g_yavin1PreviousDirtyRect.top,
								  YAVIN1_BACKGROUND_WIDTH, YAVIN1_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_yavin1BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x46A600
void Yavin1_user_DirtyActor(Actor* actor, int unusedTime) {
	Rect visibleBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_yavin1CurrentDirtyRect, &visibleBounds);
	}
}

// FUNCTION: XW 0x46A670
void Yavin1_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_yavin1Film->cur_cel == g_yavin1Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_yavin1PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_yavin1CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x46B260
void Yavin1_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_yavin1MusicState.film = sceneFilm;
		g_yavin1MusicState.sound = xsound_Find_Gmid("victory");
		if (g_yavin1MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("yvmusic.lfd");
			g_yavin1MusicState.sound = xsound_Res_Music(musicResource, "victory");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_yavin1MusicState.sound);
			soundext_ScanMidi(g_yavin1MusicState.sound, 0, YAVIN1_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_yavin1MusicState.sound);
	}
}

// FUNCTION: XW 0x46B2F0
void Yavin1_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	g_yavin1SoundEffectsFilm = sceneFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x46B320
void Yavin1_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_YAVIN1_CUE_FLYBY:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_5);
				break;
		}
	}
}
