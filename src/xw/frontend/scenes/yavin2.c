#include "xw/frontend/scenes/yavin2.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/yavin1.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/yavin2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4FC410
Film* g_yavin2Film = NULL;

// GLOBAL: XW 0x4FC414
Actor* g_yavin2BackgroundActor = NULL;

// GLOBAL: XW 0x4FC418
Rect g_yavin2PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FC420
Actor* g_yavin2CloseActor = NULL;

// GLOBAL: XW 0x4FC428
Rect g_yavin2CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4FC430
LandruHandle g_yavin2BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FC480
XwSceneMusicHandles g_yavin2MusicState = { NULL, NULL };

// GLOBAL: XW 0x4FC494
Film* g_yavin2SoundEffectsFilm = NULL;

// FUNCTION: XW 0x46A740
XwShellSceneResult Yavin2_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	sceneResource = xres_Open_Resource("yavin2.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_yavin2BackgroundHandle = xmemhdl_Alloc_Clear_Handle(YAVIN2_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(YAVIN2_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "yavin2_s", &frame, 0, 0, 0, Yavin2_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "yavin2_f", &frame, 0, 0, 0, Yavin2_film_Callback);
	g_yavin2Film = sceneFilm;
	g_yavin2BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, YAVIN2_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_yavin2BackgroundActor, Yavin2_user_Background);
	xactor_Set_Actor_Draw_Function(g_yavin2BackgroundActor, Yavin2_draw_Background);
	g_yavin2CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, YAVIN2_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_yavin2CloseActor, Yavin2_user_Close);
	xactor_Set_Actor_Draw_Function(g_yavin2CloseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_yavin2Film, shell->standardPalette);
	Yavin2_OpenMusic(sceneResource, g_yavin2Film);
	Yavin2_LoadSoundEffects(sceneResource, g_yavin2Film);
	xview_Set_View_Update_Function(Yavin2_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwYavin2_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	Yavin2_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_yavin2BackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x46A920
void Yavin2_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_YAVIN_HEADQUARTERS, shipext_Get_Pending_Medal_Scene(),
								  g_yavin2Film->cur_cel == g_yavin2Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x46A960
int16_t Yavin2_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Yavin2_user_DirtyActor);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Yavin2_film_Actor_To_Background(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Yavin1_user_SoundCue);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x46A9E0
int16_t Yavin2_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_yavin2BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						YAVIN2_BACKGROUND_WIDTH, YAVIN2_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_yavin2BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x46AAC0
void Yavin2_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_yavin2PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_yavin2PreviousDirtyRect, &g_yavin2CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_yavin2CurrentDirtyRect);
}

// FUNCTION: XW 0x46AB10
int16_t Yavin2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_yavin2BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_yavin2PreviousDirtyRect,
								  g_yavin2PreviousDirtyRect.left, g_yavin2PreviousDirtyRect.top,
								  YAVIN2_BACKGROUND_WIDTH, YAVIN2_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_yavin2BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x46AB70
void Yavin2_user_DirtyActor(Actor* actor, int unusedTime) {
	Rect visibleBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_yavin2CurrentDirtyRect, &visibleBounds);
	}
}

// FUNCTION: XW 0x46ABE0
void Yavin2_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_yavin2Film->cur_cel == g_yavin2Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_yavin2PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_yavin2CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x46B340
void Yavin2_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_yavin2MusicState.film = sceneFilm;
		g_yavin2MusicState.sound = xsound_Find_Gmid("victory");
		if (g_yavin2MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("yvmusic.lfd");
			g_yavin2MusicState.sound = xsound_Res_Music(musicResource, "victory");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_yavin2MusicState.sound);
			soundext_ScanMidi(g_yavin2MusicState.sound, 0, YAVIN2_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_yavin2MusicState.sound);
	}
}

// FUNCTION: XW 0x46B3D0
void Yavin2_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_yavin2MusicState.sound = xsound_Find_Gmid("victory");
		if (g_yavin2MusicState.sound != NULL) {
			soundext_SetHook(g_yavin2MusicState.sound, XW_SOUND_CONTROL_DIRECT, YAVIN2_MUSIC_CLOSE_CONTROL,
							 0);
			soundext_ClearTriggers();
			soundext_SetTriggerContext(g_yavin2MusicState.sound, YAVIN2_MUSIC_CLOSE_MARKER);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SET_HOOK, g_yavin2MusicState.sound->id, 0,
										 YAVIN2_MUSIC_CLOSE_HOOK, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
			soundext_FadeVolume(g_yavin2MusicState.sound, YAVIN2_MUSIC_CLOSE_VOLUME,
								YAVIN2_MUSIC_CLOSE_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x46B470
void Yavin2_LoadSoundEffects(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	g_yavin2SoundEffectsFilm = sceneFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}
