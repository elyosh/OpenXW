#include "xw/frontend/scenes/gov1.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/gov1_task.h"
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

// GLOBAL: XW 0x4F7524
Film* g_gov1Film = NULL;

// GLOBAL: XW 0x4F752C
Actor* g_gov1CloseActor = NULL;

// GLOBAL: XW 0x4F7530
LandruHandle g_gov1BackgroundHandle = 0;

// GLOBAL: XW 0x4F7574
XwSceneMusicHandles g_gov1MusicState = { NULL, NULL };

// FUNCTION: XW 0x44B2D0
XwShellSceneResult Gov1_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("Darth Vader's shuttle arrives at Plooriod III.", GOV1_CAPTION_START, GOV1_CAPTION_END,
					   GOV1_CAPTION_FONT, GOV1_CAPTION_X, GOV1_CAPTION_Y, GOV1_CAPTION_COLOR);
	resourceFile = xres_Open_Resource("gov1.lfd");
	xrect_Set_Rect(&frame, 0, 0, GOV1_BACKGROUND_WIDTH, GOV1_BACKGROUND_HEIGHT);
	g_gov1BackgroundHandle = xmemhdl_Alloc_Clear_Handle(GOV1_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	if ((uint16_t)xio_Is_System_Slower_Than(GOV1_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm = xfilm_Res_Callback_Film(resourceFile, "gov1_s", &frame, 0, 0, 0, Gov1_film_Callback);
	else
		sceneFilm = xfilm_Res_Callback_Film(resourceFile, "gov1_f", &frame, 0, 0, 0, Gov1_film_Callback);
	g_gov1Film = sceneFilm;
	g_gov1CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, GOV1_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_gov1CloseActor, Gov1_user_Close);
	xactor_Set_Actor_Draw_Function(g_gov1CloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_gov1Film, shell->standardPalette);
	xview_Set_View_Update_Function(Gov1_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Gov1_OpenMusic(resourceFile, g_gov1Film);
	Gov1_LoadSoundEffects(resourceFile, g_gov1Film);
#ifdef XW_MODERN
	XwGov1_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_gov1BackgroundHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x44B490
void Gov1_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_GHORIN_2, shipext_Get_Pending_Medal_Scene(),
								  g_gov1Film->cur_cel == g_gov1Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x44B4D0
int16_t Gov1_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Gov1_film_Actor_To_Background(actor);
			if (actor->var2 == GOV1_BACKGROUND_TILED) {
				xactor_Set_Actor_Draw_Function(actor, Gov1_draw_TiledBackground);
			}
			consumeObject = actor->var2 == GOV1_BACKGROUND_CONSUME;
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Gov1_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x44B540
int16_t Gov1_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_gov1BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						GOV1_BACKGROUND_WIDTH, GOV1_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_gov1BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x44B620
void Gov1_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Gov1_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x44B640
int16_t Gov1_draw_TiledBackground(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x,
								  int16_t y, int16_t refresh) {
	Rect sourceRect;
	int16_t baseX;
	int16_t baseY;
	int16_t tileX;
	int16_t tileY;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	if (refresh == 0) {
		return 0;
	}
	baseX = x;
	baseY = y;
	if (x >= 0) {
		baseX -= GOV1_BACKGROUND_WIDTH;
	}
	if (y >= 0) {
		baseY -= GOV1_BACKGROUND_HEIGHT;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, GOV1_BACKGROUND_WIDTH, GOV1_BACKGROUND_HEIGHT);
	backgroundPixels = (const uint8_t*)xmemhdl_Lock_Handle(g_gov1BackgroundHandle);
	for (tileY = 0; tileY < GOV1_TILE_ROWS * GOV1_BACKGROUND_HEIGHT; tileY += GOV1_BACKGROUND_HEIGHT) {
		for (tileX = 0; tileX < GOV1_TILE_COLUMNS * GOV1_BACKGROUND_WIDTH; tileX += GOV1_BACKGROUND_WIDTH) {
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, tileX + baseX, tileY + baseY,
										  GOV1_BACKGROUND_WIDTH, GOV1_BACKGROUND_HEIGHT);
		}
	}
	xmemhdl_Unlock_Handle(g_gov1BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x44B720
void Gov1_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_gov1Film->cur_cel == g_gov1Film->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x44C040
void Gov1_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_gov1MusicState.film = sceneFilm;
		g_gov1MusicState.sound = xsound_Find_Gmid("ghorin");
		if (g_gov1MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("gvmusic.lfd");
			g_gov1MusicState.sound = xsound_Res_Music(musicResource, "ghorin");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_gov1MusicState.sound);
		}
		xsound_Set_Sound_Keep(g_gov1MusicState.sound);
		xsound_Set_Sound_User_Function(g_gov1MusicState.sound, Gov1_user_Music);
	}
}

// FUNCTION: XW 0x44C0D0
void Gov1_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_gov1MusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_2 && filmCel == XW_GOV1_MUSIC_CONTROL_CEL) {
		soundext_SetHook(g_gov1MusicState.sound, XW_SOUND_CONTROL_DIRECT, 1, 0);
	}
}

// FUNCTION: XW 0x44C110
void Gov1_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_2, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x44C140
void Gov1_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case XW_GOV1_CUE_TIE_APPROACH:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_2);
			}
			break;
	}
}
