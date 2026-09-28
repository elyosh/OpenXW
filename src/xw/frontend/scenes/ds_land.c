#include "xw/frontend/scenes/ds_land.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/dsland_task.h"
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

// GLOBAL: XW 0x4F73A8
XwSceneMusicHandles g_dslandMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7458
Actor* g_dslandBackgroundActor = NULL;

// GLOBAL: XW 0x4F745C
Film* g_dslandFilm = NULL;

// GLOBAL: XW 0x4F7460
Rect g_dslandPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7468
Actor* g_dslandCloseActor = NULL;

// GLOBAL: XW 0x4F7470
Rect g_dslandCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F7478
LandruHandle g_dslandBackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x446FC0
void DsLand_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_dslandMusicState.film = sceneFilm;
		g_dslandMusicState.sound = xsound_Find_Gmid("empire");
		if (g_dslandMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("ddmusic.lfd");
			g_dslandMusicState.sound = xsound_Res_Music(musicResource, "empire");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_dslandMusicState.sound);
			soundext_ScanMidi(g_dslandMusicState.sound, 0, DSLAND_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_dslandMusicState.sound);
		xsound_Set_Sound_User_Function(g_dslandMusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x447060
void DsLand_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_3, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x447090
void DsLand_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_DS_LAND_CUE_SHUTTLE:
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_3);
				break;
		}
	}
}

// FUNCTION: XW 0x448770
XwShellSceneResult DsLand_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("Arrival of Darth Vader, Envoy of the Emperor.", DSLAND_CAPTION_START,
					   DSLAND_CAPTION_END, DSLAND_CAPTION_FONT, DSLAND_CAPTION_X, DSLAND_CAPTION_Y,
					   DSLAND_CAPTION_COLOR);
	sceneResource = xres_Open_Resource("dsland.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_dslandBackgroundHandle = xmemhdl_Alloc_Clear_Handle(DSLAND_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(DSLAND_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "dsland_s", &frame, 0, 0, 0, DsLand_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(sceneResource, "dsland_f", &frame, 0, 0, 0, DsLand_film_Callback);
	g_dslandFilm = sceneFilm;
	xfilm_Set_Film_Def_Palette(sceneFilm, shell->standardPalette);
	g_dslandBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DSLAND_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_dslandBackgroundActor, DsLand_user_Background);
	xactor_Set_Actor_Draw_Function(g_dslandBackgroundActor, DsLand_draw_Background);
	g_dslandCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DSLAND_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_dslandCloseActor, DsLand_user_Close);
	xactor_Set_Actor_Draw_Function(g_dslandCloseActor, XwCutscene_DrawCloseOnRefresh);
	xview_Set_View_Update_Function(DsLand_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	DsLand_OpenMusic(sceneResource, g_dslandFilm);
	DsLand_LoadSoundEffects(sceneResource, g_dslandFilm);
#ifdef XW_MODERN
	XwDsLand_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_dslandBackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x448960
void DsLand_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DEATH_STAR_COMPLETION_SPEECH,
								  shipext_Get_Pending_Medal_Scene(),
								  g_dslandFilm->cur_cel == g_dslandFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x4489A0
int16_t DsLand_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, DsLand_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				DsLand_film_Actor_To_Background(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, DsLand_user_Sound);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x448A20
int16_t DsLand_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_dslandBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						DSLAND_BACKGROUND_WIDTH, DSLAND_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_dslandBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x448B00
void DsLand_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_dslandPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_dslandPreviousDirtyRect, &g_dslandCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_dslandCurrentDirtyRect);
}

// FUNCTION: XW 0x448B50
int16_t DsLand_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_dslandBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_dslandPreviousDirtyRect,
								  g_dslandPreviousDirtyRect.left, g_dslandPreviousDirtyRect.top,
								  DSLAND_BACKGROUND_WIDTH, DSLAND_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_dslandBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x448BB0
void DsLand_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_dslandCurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x448C20
void DsLand_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_dslandFilm->cur_cel == g_dslandFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_dslandPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_dslandCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x44A690
void DsLand_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		DsLand_PlaySoundCue(cue);
	}
}
