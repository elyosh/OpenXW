#include "xw/frontend/scenes/sun_shot.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/sunshot_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4CFE58
const char g_sunshotResourceFilename[12] = "sunshot.lfd";

// GLOBAL: XW 0x4CFE66
const char g_sunshotFilmName[9] = "sunshot1";

// GLOBAL: XW 0x4F4B08
Film* g_sunshotFilm = NULL;

// GLOBAL: XW 0x4F4B10
Actor* g_sunshotCloseActor = NULL;

// GLOBAL: XW 0x4F4B14
LandruHandle g_sunshotBackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x4331B0
XwShellSceneResult SunShot_Play(struct XwShellContext* context) {
	ResFile* resourceFile = xres_Open_Resource(g_sunshotResourceFilename);
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, SUNSHOT_BACKGROUND_WIDTH, SUNSHOT_BACKGROUND_HEIGHT);
	g_sunshotBackgroundHandle = xmemhdl_Alloc_Clear_Handle(SUNSHOT_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_sunshotFilm =
		xfilm_Res_Callback_Film(resourceFile, g_sunshotFilmName, &frame, 0, 0, 0, SunShot_film_Callback);
	g_sunshotCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, SUNSHOT_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_sunshotCloseActor, SunShot_user_Close);
	xactor_Set_Actor_Draw_Function(g_sunshotCloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_sunshotFilm, context->standardPalette);
	xview_Set_View_Update_Function(SunShot_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Cutscene_EnsureTroFightMusic(resourceFile, g_sunshotFilm);
	SunShot_LoadSounds();
#ifdef XW_MODERN
	XwSunshot_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_sunshotBackgroundHandle);
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x433330
void SunShot_end_View(int time) {
	int16_t exitScene;
	(void)time;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_DOGFIGHT_HRC2, XW_SCENE_REGISTER_INITIAL,
								  g_sunshotFilm->cur_cel == g_sunshotFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x433370
int16_t SunShot_film_Callback(Film* film, FilmObject* object) {
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == SUNSHOT_ACTOR_ROLE_BACKGROUND) {
			SunShot_ActorToBackground(actor);
			xactor_Set_Actor_Draw_Function(actor, SunShot_draw_Background);
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, SunShot_user_Sound);
		}
	}
	return 0;
}

// FUNCTION: XW 0x4333D0
int16_t SunShot_ActorToBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_sunshotBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						SUNSHOT_BACKGROUND_WIDTH, SUNSHOT_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_sunshotBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4334B0
void SunShot_user_Sound(Actor* actor, int time) {
	int16_t soundAction = actor->var2;
	(void)time;
	if (soundAction != 0) {
		SunShot_PlaySoundAction(soundAction);
	}
}

// FUNCTION: XW 0x4334D0
int16_t SunShot_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)clip;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_sunshotBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, frame, x, y, SUNSHOT_BACKGROUND_WIDTH,
								  SUNSHOT_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_sunshotBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x433530
void SunShot_user_Close(Actor* actor, int time) {
	(void)time;
	if (g_sunshotFilm->cur_cel == g_sunshotFilm->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x434D90
void SunShot_LoadSounds(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		if (ShellPreferences_GetSfxEnabled() != 0) {
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_1, 0, NULL, 1, 1);
		} else {
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_2A, 0, NULL, 1, 0);
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_2, 0, NULL, 1, 0);
		}
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x434DE0
void SunShot_PlaySoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case SUNSHOT_SOUND_ACTION_FIRST:
				if (ShellPreferences_GetSfxEnabled() != 0) {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_1);
				} else {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_2A);
				}
				break;
			case SUNSHOT_SOUND_ACTION_SECOND:
				if (ShellPreferences_GetSfxEnabled() == 0) {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_2);
				}
				break;
		}
	}
}
