#include "xw/frontend/scenes/dock.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/dock_task.h"
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

// GLOBAL: XW 0x4D3098
const char* g_dockMusicFilename = "ddmusic.lfd";

// GLOBAL: XW 0x4D309C
const char* g_dockMusicName = "empire";

// GLOBAL: XW 0x4D3100
char g_dockResourceNames[DOCK_RESOURCE_NAME_COUNT][DOCK_RESOURCE_NAME_SIZE] = {
	"dock.lfd", "docka_s", "docka_f", "dockb_s", "dockb_f", "dockc_s", "dockc_f"
};

// GLOBAL: XW 0x4F73A0
XwSceneMusicHandles g_dockMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F73B8
Actor* g_dockBackgroundActor = NULL;

// GLOBAL: XW 0x4F73C0
Rect g_dockPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F73C8
Actor* g_dockCloseActor = NULL;

// GLOBAL: XW 0x4F73D0
Rect g_dockCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F73D8
Film* g_dockFilm = NULL;

// GLOBAL: XW 0x4F73DC
LandruHandle g_dockBackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x446E20
void Dock_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_dockMusicState.film = sceneFilm;
		g_dockMusicState.sound = xsound_Find_Gmid(g_dockMusicName);
		if (g_dockMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_dockMusicFilename);
			g_dockMusicState.sound = xsound_Res_Music(musicResource, g_dockMusicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_dockMusicState.sound);
			soundext_ScanMidi(g_dockMusicState.sound, 0, DOCK_MUSIC_START_BEAT, DOCK_MUSIC_START_TICK);
			soundext_SetHook(g_dockMusicState.sound, 0, DOCK_MUSIC_INITIAL_CONTROL, 0);
		}
		xsound_Set_Sound_Keep(g_dockMusicState.sound);
		xsound_Set_Sound_User_Function(g_dockMusicState.sound, Dock_user_Music);
	}
}

// FUNCTION: XW 0x446EE0
void Dock_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_dockMusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_IMPERIAL_DRYDOCK_3 && filmCel == DOCK_MUSIC_FADE_CEL)
		soundext_FadeVolume(g_dockMusicState.sound, 0, DOCK_MUSIC_FADE_DURATION);
}

// FUNCTION: XW 0x446F20
void Dock_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_2, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_4, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x446F60
void Dock_CloseSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_ResetSfxCache(1);
	}
}

// FUNCTION: XW 0x446F80
void Dock_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case DOCK_CUE_SHUTTLE_2:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_2);
			}
			break;
		case DOCK_CUE_SHUTTLE_4:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_4);
			}
			break;
	}
}

// FUNCTION: XW 0x4471C0
XwShellSceneResult Dock_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Rect frame;
	int16_t slowFilmSlot;
	LandruDisplay_SetLowResolutionMode(1);
	sceneResource = xres_Open_Resource(g_dockResourceNames[DOCK_RESOURCE_FILE]);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_IMPERIAL_DRYDOCK_1:
			xfade_AddTimedText("Imperial Drydock IV, Greater Plooriad Sector.", DOCK_CAPTION_START,
							   DOCK_FIRST_CAPTION_END, DOCK_CAPTION_FONT, DOCK_CAPTION_X,
							   DOCK_FIRST_CAPTION_Y, DOCK_CAPTION_COLOR);
			slowFilmSlot = DOCK_FIRST_SLOW_FILM;
			break;
		case XW_SCENE_IMPERIAL_DRYDOCK_2:
			xfade_AddTimedText("Interdictor Cruisers respond to Fleet Summons.", DOCK_CAPTION_START,
							   DOCK_SECOND_CAPTION_END, DOCK_CAPTION_FONT, DOCK_CAPTION_X,
							   DOCK_BOTTOM_CAPTION_Y, DOCK_CAPTION_COLOR);
			slowFilmSlot = DOCK_SECOND_SLOW_FILM;
			break;
		case XW_SCENE_IMPERIAL_DRYDOCK_3:
			xfade_AddTimedText("Outer Rim Imperial Fleet preparing for battle.", DOCK_CAPTION_START,
							   DOCK_THIRD_CAPTION_END, DOCK_CAPTION_FONT, DOCK_CAPTION_X,
							   DOCK_THIRD_CAPTION_Y, DOCK_CAPTION_COLOR);
			xfade_AddTimedText("Interdictors join the fleet for attack on Rebels.", DOCK_CAPTION_START,
							   DOCK_THIRD_CAPTION_END, DOCK_CAPTION_FONT, DOCK_CAPTION_X,
							   DOCK_BOTTOM_CAPTION_Y, DOCK_CAPTION_COLOR);
			slowFilmSlot = DOCK_THIRD_SLOW_FILM;
			break;
		default:
			/* The original derives an invalid slot from shell pointer bits here. */
			slowFilmSlot = DOCK_FIRST_SLOW_FILM;
			break;
	}
	if ((uint16_t)xio_Is_System_Slower_Than(DOCK_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_dockBackgroundHandle = xmemhdl_Alloc_Clear_Handle(DOCK_BACKGROUND_WIDTH * DOCK_BACKGROUND_HEIGHT,
															LANDRU_MEMORY_RESOURCE);
		g_dockFilm = xfilm_Res_Callback_Film(sceneResource, g_dockResourceNames[slowFilmSlot], &frame, 0, 0,
											 0, Dock_film_Callback);
		g_dockBackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DOCK_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_dockBackgroundActor, Dock_user_Background);
		xactor_Set_Actor_Draw_Function(g_dockBackgroundActor, Dock_draw_Background);
		g_dockCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DOCK_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_dockCloseActor, Dock_user_Close);
		xactor_Set_Actor_Draw_Function(g_dockCloseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		g_dockFilm =
			xfilm_Res_Callback_Film(sceneResource, g_dockResourceNames[slowFilmSlot + DOCK_FAST_FILM_OFFSET],
									&frame, 0, 0, 0, Dock_film_Callback);
	}
	xfilm_Set_Film_Def_Palette(g_dockFilm, shell->standardPalette);
	Dock_OpenMusic(sceneResource, g_dockFilm);
	Dock_LoadSoundEffects(sceneResource, g_dockFilm);
	xview_Set_View_Update_Function(Dock_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwDock_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	Dock_CloseSoundEffects();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(DOCK_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_dockBackgroundHandle);
	}
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x447480
void Dock_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_IMPERIAL_DRYDOCK_1:
			nextScene = XW_SCENE_IMPERIAL_DRYDOCK_2;
			break;
		case XW_SCENE_IMPERIAL_DRYDOCK_2:
			nextScene = XW_SCENE_IMPERIAL_DRYDOCK_3;
			break;
		case XW_SCENE_IMPERIAL_DRYDOCK_3:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_dockFilm->cur_cel == g_dockFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x4474F0
int16_t Dock_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (xio_Is_System_Slower_Than(DOCK_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
				Dock_film_Actor_To_Background(actor);
				consumeObject = 1;
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
				xactor_Set_Actor_User_Function(actor, Dock_user_DirtyBounds);
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
				xactor_Set_Actor_User_Function(actor, Dock_user_Sound);
			}
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Dock_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x447590
int16_t Dock_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_dockBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						DOCK_BACKGROUND_WIDTH, DOCK_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_dockBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x447670
void Dock_user_Sound(Actor* actor, int unusedTime) {
	int16_t soundCue = actor->var2;
	(void)unusedTime;
	if (soundCue != 0) {
		Dock_PlaySoundCue(soundCue);
	}
}

// FUNCTION: XW 0x447690
void Dock_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_dockPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_dockPreviousDirtyRect, &g_dockCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_dockCurrentDirtyRect);
}

// FUNCTION: XW 0x4476E0
int16_t Dock_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_dockBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_dockPreviousDirtyRect, g_dockPreviousDirtyRect.left,
								  g_dockPreviousDirtyRect.top, DOCK_BACKGROUND_WIDTH, DOCK_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_dockBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x447740
void Dock_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_dockCurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x4477B0
void Dock_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_dockFilm->cur_cel == g_dockFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_dockPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_dockCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
