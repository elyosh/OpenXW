#include "xw/frontend/scenes/leave2.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/leave2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D5B60
const char* g_leave2MusicFilename = "hr1music.lfd";

// GLOBAL: XW 0x4D5B64
const char* g_leave2MusicName = "hangar";

// GLOBAL: XW 0x4F7998
Actor* g_leave2BackgroundActor = NULL;

// GLOBAL: XW 0x4F799C
Film* g_leave2Film = NULL;

// GLOBAL: XW 0x4F79A0
Rect g_leave2PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F79A8
Actor* g_leave2CloseActor = NULL;

// GLOBAL: XW 0x4F79B0
Rect g_leave2CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F79B8
LandruHandle g_leave2BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F80C4
XwSceneMusicHandles g_leave2MusicState = { NULL, NULL };

// FUNCTION: XW 0x4551A0
XwShellSceneResult Leave2_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	ResFile* yavinResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	sceneResource = xres_Open_Resource("leave2.lfd");
	yavinResource = xres_Open_Resource("yavin1.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_leave2BackgroundHandle = xmemhdl_Alloc_Clear_Handle(LEAVE2_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(LEAVE2_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "leaveb_s", &frame, 0, 0, 0, Leave2_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "leaveb_f", &frame, 0, 0, 0, Leave2_film_Callback);
	g_leave2Film = sceneFilm;
	g_leave2BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LEAVE2_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_leave2BackgroundActor, Leave2_user_Background);
	xactor_Set_Actor_Draw_Function(g_leave2BackgroundActor, Leave2_draw_Background);
	g_leave2CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LEAVE2_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_leave2CloseActor, Leave2_user_Close);
	xactor_Set_Actor_Draw_Function(g_leave2CloseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_leave2Film, shell->standardPalette);
	Leave2_OpenMusic(sceneResource, g_leave2Film);
	Leave2_LoadSoundEffects(sceneResource, g_leave2Film);
	xview_Set_View_Update_Function(Leave2_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwLeave2_RunView(sceneResource, yavinResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_leave2BackgroundHandle);
	xres_Close_Resource(yavinResource);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4553A0
void Leave2_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, shipext_Get_Pending_Medal_Scene(),
								  shipext_Get_Pending_Medal_Scene(),
								  g_leave2Film->cur_cel == g_leave2Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x4553F0
int16_t Leave2_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Leave2_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Leave2_film_Actor_To_Background(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Leave2_user_Sound);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x455470
int16_t Leave2_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_leave2BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						LEAVE2_BACKGROUND_WIDTH, LEAVE2_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_leave2BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x455550
void Leave2_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Leave2_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x455570
void Leave2_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_leave2PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_leave2PreviousDirtyRect, &g_leave2CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_leave2CurrentDirtyRect);
}

// FUNCTION: XW 0x4555C0
int16_t Leave2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_leave2BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_leave2PreviousDirtyRect,
								  g_leave2PreviousDirtyRect.left, g_leave2PreviousDirtyRect.top,
								  LEAVE2_BACKGROUND_WIDTH, LEAVE2_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_leave2BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x455620
void Leave2_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_leave2CurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x455690
void Leave2_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_leave2Film->cur_cel == g_leave2Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_leave2PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_leave2CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x457D10
void Leave2_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_leave2MusicState.film = sceneFilm;
		g_leave2MusicState.sound = xsound_Find_Gmid(g_leave2MusicName);
		if (g_leave2MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_leave2MusicFilename);
			g_leave2MusicState.sound = xsound_Res_Music(musicResource, g_leave2MusicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_leave2MusicState.sound);
			soundext_ScanMidi(g_leave2MusicState.sound, 0, LEAVE2_MUSIC_START_BEAT, LEAVE2_MUSIC_START_TICK);
		}
		xsound_Set_Sound_Keep(g_leave2MusicState.sound);
		xsound_Set_Sound_User_Function(g_leave2MusicState.sound, Leave2_user_Music);
	}
}

// FUNCTION: XW 0x457DC0
void Leave2_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_leave2MusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_YAVIN_DEPARTURE_2 && filmCel == LEAVE2_MUSIC_FADE_CEL)
		soundext_FadeVolume(g_leave2MusicState.sound, 0, LEAVE2_MUSIC_FADE_DURATION);
}

// FUNCTION: XW 0x457E00
void Leave2_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_6, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x457E30
void Leave2_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case XW_LEAVE2_CUE_FLYBY:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_6);
			}
			break;
	}
}
