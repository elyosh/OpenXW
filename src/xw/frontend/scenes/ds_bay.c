#include "xw/frontend/scenes/ds_bay.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/dsbay_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/view.h>

// GLOBAL: XW 0x4F6A84
XwSceneMusicHandles g_dsbayMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F73E0
Film* g_dsbayFilm = NULL;

// GLOBAL: XW 0x4F73E8
Rect g_dsbayCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F73F0
Actor* g_dsbayBackgroundActor = NULL;

// GLOBAL: XW 0x4F73F4
Actor* g_dsbayScrollActor = NULL;

// GLOBAL: XW 0x4F73F8
Rect g_dsbayRestoreRect = { 0 };

// GLOBAL: XW 0x4F7400
Actor* g_dsbayCloseActor = NULL;

// GLOBAL: XW 0x4F7404
LandruHandle g_dsbayBackgroundHandles[DSBAY_BACKGROUND_TILE_COUNT] = { LANDRU_NULL_HANDLE,
																	   LANDRU_NULL_HANDLE };

// FUNCTION: XW 0x4422B0
void DsBay_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_dsbayMusicState.film = sceneFilm;
		g_dsbayMusicState.sound = xsound_Find_Gmid("empire");
		if (g_dsbayMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("ddmusic.lfd");
			g_dsbayMusicState.sound = xsound_Res_Music(musicResource, "empire");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_dsbayMusicState.sound);
			soundext_ScanMidi(g_dsbayMusicState.sound, 0, DSBAY_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_dsbayMusicState.sound);
		xsound_Set_Sound_User_Function(g_dsbayMusicState.sound, DsBay_user_Music);
	}
}

// FUNCTION: XW 0x442350
void DsBay_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	if (g_dsbayMusicState.film->cur_cel == DSBAY_MUSIC_CONTROL_CEL &&
		shellext_Get_Cur_Scene() == XW_SCENE_DEATH_STAR_HANGAR) {
		soundext_SetHook(g_dsbayMusicState.sound, XW_SOUND_CONTROL_DIRECT, DSBAY_MUSIC_CONTROL_VALUE, 0);
	}
}

// FUNCTION: XW 0x447850
XwShellSceneResult DsBay_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	sceneResource = xres_Open_Resource("dsbay.lfd");
	xrect_Set_Rect(&frame, 0, 0, DSBAY_FRAME_WIDTH, DSBAY_FRAME_HEIGHT);
	g_dsbayBackgroundHandles[0] =
		xmemhdl_Alloc_Clear_Handle(DSBAY_BACKGROUND_TILE_BYTES, LANDRU_MEMORY_RESOURCE);
	g_dsbayBackgroundHandles[1] =
		xmemhdl_Alloc_Clear_Handle(DSBAY_BACKGROUND_TILE_BYTES, LANDRU_MEMORY_RESOURCE);
	xrect_Copy_Rect(&g_dsbayRestoreRect, &frame);
	xrect_Set_Rect(&g_dsbayCurrentDirtyRect, 0, 0, 0, 0);
	if ((uint16_t)xio_Is_System_Slower_Than(DSBAY_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "dsbay_s", &frame, 0, 0, 0, DsBay_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "dsbay_f", &frame, 0, 0, 0, DsBay_film_Callback);
	g_dsbayFilm = sceneFilm;
	xfilm_Set_Film_Def_Palette(sceneFilm, shell->standardPalette);
	g_dsbayBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DSBAY_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_dsbayBackgroundActor, DsBay_user_Background);
	xactor_Set_Actor_Draw_Function(g_dsbayBackgroundActor, DsBay_draw_Background);
	g_dsbayCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DSBAY_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_dsbayCloseActor, DsBay_user_Close);
	xactor_Set_Actor_Draw_Function(g_dsbayCloseActor, Cutscene_DrawConditionalErase);
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(DsBay_end_View);
	DsBay_OpenMusic(sceneResource, g_dsbayFilm);
	soundext_RecheckSfxPreference();
#ifdef XW_MODERN
	XwDsBay_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_dsbayBackgroundHandles[0]);
	xmemhdl_Free_Handle(g_dsbayBackgroundHandles[1]);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x447A60
void DsBay_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DEATH_STAR_VADER_ARRIVAL,
								  shipext_Get_Pending_Medal_Scene(),
								  g_dsbayFilm->cur_cel == g_dsbayFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
	xrect_Set_Rect(&g_dsbayCurrentDirtyRect, 0, 0, 0, 0);
}

// FUNCTION: XW 0x447AC0
int16_t DsBay_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			DsBay_film_Actor_To_Background(actor);
			if (actor->var2 == DSBAY_BACKGROUND_SCROLL_CONTROLLER) {
				actor->draw = NULL;
				g_dsbayScrollActor = actor;
			} else {
				consumeObject = 1;
			}
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
			xactor_Set_Actor_User_Function(actor, DsBay_user_DirtyBounds);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x447B40
int16_t DsBay_film_Actor_To_Background(Actor* actor) {
	int16_t previousHeight;
	int16_t previousWidth;
	uint8_t* previousPixels;
	Rect canvasBounds;
	Rect previousClip;
	int16_t drawResult = 0;
	int16_t tileOffsetX = 0;
	int tileIndex;
	int remainingTiles;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (tileIndex = 0, remainingTiles = DSBAY_BACKGROUND_TILE_COUNT; remainingTiles != 0;
		 ++tileIndex, --remainingTiles) {
		uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_dsbayBackgroundHandles[tileIndex]);
		xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
							DSBAY_BACKGROUND_TILE_WIDTH, DSBAY_BACKGROUND_TILE_HEIGHT, 0);
		if (actor->draw) {
			drawResult =
				actor->draw(actor, &canvasBounds, &canvasBounds, actor->x + tileOffsetX, actor->y, 1);
		}
		xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
		xmemhdl_Unlock_Handle(g_dsbayBackgroundHandles[tileIndex]);
		tileOffsetX -= DSBAY_BACKGROUND_TILE_WIDTH;
	}
	return drawResult;
}

// FUNCTION: XW 0x447C20
void DsBay_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Enclose_Rect(&g_dsbayCurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x447C70
void DsBay_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_dsbayFilm->cur_cel == g_dsbayFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_dsbayCurrentDirtyRect);
		xrect_Enclose_Rect(&frame, &g_dsbayRestoreRect);
		actor->var1 = 0;
	}
	xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
	xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
}

// FUNCTION: XW 0x447CF0
void DsBay_user_Background(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_dsbayScrollActor->xv != 0 || g_dsbayScrollActor->yv != 0 || g_dsbayScrollActor->xvf != 0 ||
		g_dsbayScrollActor->yvf != 0) {
		actor->var1 = DSBAY_BACKGROUND_RESTORE_FRAMES;
	}
	if (actor->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_dsbayRestoreRect);
		--actor->var1;
	}
}

// FUNCTION: XW 0x447D40
int16_t DsBay_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh) {
	Rect tileRect;
	Rect savedClip;
	int16_t tileX;
	int remainingTiles;
	int tileIndex;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	tileX = g_dsbayScrollActor->x;
	xcanvas_Get_Raster_Clip(&savedClip);
	xcanvas_Set_Drawing_Canvas_Clip(&g_dsbayRestoreRect);
	xrect_Set_Rect(&tileRect, 0, 0, DSBAY_BACKGROUND_TILE_WIDTH, DSBAY_BACKGROUND_TILE_HEIGHT);
	for (tileIndex = 0, remainingTiles = DSBAY_BACKGROUND_TILE_COUNT; remainingTiles != 0;
		 ++tileIndex, --remainingTiles) {
		const uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_dsbayBackgroundHandles[tileIndex]);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &tileRect, tileX, 0, DSBAY_BACKGROUND_TILE_WIDTH,
									  DSBAY_BACKGROUND_TILE_HEIGHT);
		xmemhdl_Unlock_Handle(g_dsbayBackgroundHandles[tileIndex]);
		tileX += DSBAY_BACKGROUND_TILE_WIDTH;
	}
	xcanvas_Set_Drawing_Canvas_Clip(&savedClip);
	xrect_Copy_Rect(&g_dsbayRestoreRect, &g_dsbayCurrentDirtyRect);
	return 1;
}
