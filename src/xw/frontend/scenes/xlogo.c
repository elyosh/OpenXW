#include "xw/frontend/scenes/xlogo.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/shared.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/xlogo_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D8E90
const char* g_xlogoMusicResourceName = "bl4music.lfd";

// GLOBAL: XW 0x4D8E94
const char* g_xlogoMusicName = "runs2";

// GLOBAL: XW 0x4FC3BC
Sound* g_xlogoMusic = NULL;

// GLOBAL: XW 0x4FC3C0
Film* g_xlogoMusicFilm = NULL;

// GLOBAL: XW 0x4FC3C8
Film* g_xlogoFilm = NULL;

// GLOBAL: XW 0x4FC3CC
Actor* g_xlogoUpdateActor = NULL;

// GLOBAL: XW 0x4FC3D0
Rect g_xlogoPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4FC3D8
Actor* g_xlogoCloseActor = NULL;

// GLOBAL: XW 0x4FC3E0
Rect g_xlogoCurrentDirtyRect = { 0 };

// FUNCTION: XW 0x469C70
void XLogo_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled()) {
		g_xlogoMusicFilm = film;
		g_xlogoMusic = xsound_Find_Gmid(g_xlogoMusicName);
		if (g_xlogoMusic == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_xlogoMusicResourceName);
			g_xlogoMusic = xsound_Res_Music(musicResource, g_xlogoMusicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_xlogoMusic);
		}
		xsound_Set_Sound_Keep(g_xlogoMusic);
		xsound_Set_Sound_User_Function(g_xlogoMusic, XLogo_user_Music);
	}
}

// FUNCTION: XW 0x469D10
void XLogo_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	if (g_xlogoMusicFilm->cur_cel == XLOGO_MUSIC_CONTROL_CEL) {
		soundext_SetHook(g_xlogoMusic, XW_SOUND_CONTROL_DIRECT, 1, 0);
	}
}

// FUNCTION: XW 0x469D40
XwShellSceneResult XLogo_XLogo(struct XwShellContext* shellContext) {
	ResFile* resourceFile = xres_Open_Resource("xlogo.lfd");
	Rect frame;
	Film* film;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	if (Shared_ReturnZero() != 0)
		film = xfilm_Res_Callback_Film(resourceFile, "xlogo_l", &frame, 0, 0, 0, XLogo_film_Callback);
	else
		film = xfilm_Res_Callback_Film(resourceFile, "xlogo640", &frame, 0, 0, 0, XLogo_film_Callback);
	g_xlogoFilm = film;
	g_xlogoUpdateActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, XLOGO_UPDATE_Z);
	xactor_Set_Actor_User_Function(g_xlogoUpdateActor, XLogo_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_xlogoUpdateActor, Cutscene_DrawEraseViewClip);
	g_xlogoCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, XLOGO_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_xlogoCloseActor, XLogo_user_Close);
	xactor_Set_Actor_Draw_Function(g_xlogoCloseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_xlogoFilm, shellContext->standardPalette);
	XLogo_OpenMusic(resourceFile, g_xlogoFilm);
	XLogo_LoadSoundEffects();
	xview_Set_View_Update_Function(XLogo_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwXLogo_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xview_Enable_All_View_Erase();
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x469EE0
void XLogo_end_View(int time) {
	int16_t exitScene;
	(void)time;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_CREDITS, XW_SCENE_REGISTER_INITIAL,
									  g_xlogoFilm->cur_cel == g_xlogoFilm->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_xlogoFilm->cur_cel == g_xlogoFilm->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_CREDITS);
	}
}

// FUNCTION: XW 0x469F40
int16_t XLogo_film_Callback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
			xactor_Set_Actor_User_Function(actor, XLogo_user_AccumulateDirtyRect);
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, XLogo_user_Sound);
		}
	}
	return 0;
}

// FUNCTION: XW 0x469F90
void XLogo_user_Sound(Actor* actor, int time) {
	int16_t action = actor->var2;
	(void)time;
	if (action != 0) {
		XLogo_HandleSoundAction(action);
	}
}

// FUNCTION: XW 0x469FB0
void XLogo_user_BeginDirtyFrame(Actor* actor, int time) {
	(void)actor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_xlogoPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_xlogoPreviousDirtyRect, &g_xlogoCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_xlogoCurrentDirtyRect);
}

// FUNCTION: XW 0x46A000
void XLogo_user_AccumulateDirtyRect(Actor* actor, int time) {
	Rect actorRect;
	Rect actorFrame;
	(void)time;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Clip_Rect(&actorRect, &actorFrame);
		xrect_Enclose_Rect(&g_xlogoCurrentDirtyRect, &actorRect);
	}
}

// FUNCTION: XW 0x46A070
void XLogo_user_Close(Actor* actor, int time) {
	Rect frame;
	(void)time;
	if (g_xlogoFilm->cur_cel == g_xlogoFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_xlogoPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_xlogoCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x46A110
void XLogo_LoadSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_2, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x46A150
void XLogo_HandleSoundAction(int16_t action) {
	switch (action) {
		case XLOGO_ACTION_FLYBY_2:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_2);
			}
			break;
		case XLOGO_ACTION_FLYBY_5:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_5);
			}
			break;
	}
}
