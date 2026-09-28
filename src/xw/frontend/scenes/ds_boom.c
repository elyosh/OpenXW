#include "xw/frontend/scenes/ds_boom.h"

#include "xw/landru_config.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/dsboom_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/paint.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D30D8
const char* g_dsboomMusicFilename = "yvmusic.lfd";

// GLOBAL: XW 0x4D30DC
const char* g_dsboomMusicName = "victory";

// GLOBAL: XW 0x4F73B0
XwSceneMusicHandles g_dsboomMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7408
Film* g_dsboomFilm = NULL;

// GLOBAL: XW 0x4F740C
Actor* g_dsboomBackgroundActor = NULL;

// GLOBAL: XW 0x4F7410
Rect g_dsboomPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7418
Actor* g_dsboomCloseActor = NULL;

// GLOBAL: XW 0x4F7420
Rect g_dsboomCurrentDirtyRect = { 0 };

// FUNCTION: XW 0x4470B0
void DsBoom_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled()) {
		g_dsboomMusicState.film = sceneFilm;
		g_dsboomMusicState.sound = xsound_Find_Gmid(g_dsboomMusicName);
		if (g_dsboomMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_dsboomMusicFilename);
			g_dsboomMusicState.sound = xsound_Res_Music(musicResource, g_dsboomMusicName);
			soundext_Start_Resource_Sound(g_dsboomMusicState.sound);
			xres_Close_Resource(musicResource);
		}
		xsound_Set_Sound_Keep(g_dsboomMusicState.sound);
	}
}

// FUNCTION: XW 0x447130
void DsBoom_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	/* Use the defined preference value, as in DsBoom_CloseSoundEffects. */
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_BIG, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x447170
void DsBoom_CloseSoundEffects(void) {
	/* Use the defined preference value, not the original getter's incidental upper EAX bits. */
	if (ShellPreferences_GetSfxEnabled() != 0)
		soundext_ResetSfxCache(1);
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x447190
void DsBoom_PlaySoundCue(int16_t cue) {
	/* Use the defined preference value, as in DsBoom_CloseSoundEffects. */
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_DS_BOOM_CUE_FLYBY:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
				break;
			case XW_DS_BOOM_CUE_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_BIG);
				break;
		}
	}
}

// FUNCTION: XW 0x447E10
XwShellSceneResult DsBoom_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	resourceFile = xres_Open_Resource("dsboom.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_dsboomFilm = xfilm_Res_Callback_Film(resourceFile, "dsboom_f", &frame, 0, 0, 0, DsBoom_film_Callback);
	g_dsboomBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DS_BOOM_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_dsboomBackgroundActor, DsBoom_user_Background);
	xactor_Set_Actor_Draw_Function(g_dsboomBackgroundActor, XwDsBoom_DrawBackground);
	g_dsboomCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DS_BOOM_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_dsboomCloseActor, DsBoom_user_Close);
	xfilm_Set_Film_Def_Palette(g_dsboomFilm, shell->standardPalette);
	DsBoom_OpenMusic(resourceFile, g_dsboomFilm);
	DsBoom_LoadSoundEffects(resourceFile, g_dsboomFilm);
	xview_Set_View_Update_Function(DsBoom_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwDsBoom_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	DsBoom_CloseSoundEffects();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xview_Clear_View_Update_Function();
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x447F90
void DsBoom_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_YAVIN_APPROACH, shipext_Get_Pending_Medal_Scene(),
								  g_dsboomFilm->cur_cel == g_dsboomFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x447FD0
int16_t DsBoom_film_Callback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
			xactor_Set_Actor_User_Function(actor, DsBoom_user_DirtyBounds);
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, DsBoom_user_Sound);
		}
	}
	return 0;
}

// FUNCTION: XW 0x448020
void DsBoom_user_Sound(Actor* actor, int unusedTime) {
	int16_t soundCue = actor->var2;
	(void)unusedTime;
	if (soundCue != 0) {
		DsBoom_PlaySoundCue(soundCue);
	}
}

// FUNCTION: XW 0x448040
void DsBoom_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_dsboomPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_dsboomPreviousDirtyRect, &g_dsboomCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_dsboomCurrentDirtyRect);
}

// FUNCTION: XW 0x448090
int DsBoom_draw_Background(Actor* unusedActor, Rect* frame, Rect* unusedClip, int16_t unusedX,
						   int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	xpaint_Paint_Clipped_Rect(frame, DS_BOOM_BACKGROUND_COLOR);
	return 1;
}

// FUNCTION: XW 0x4480B0
void DsBoom_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_dsboomCurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x448120
void DsBoom_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_dsboomFilm->cur_cel == g_dsboomFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	} else {
		xrect_Copy_Rect(&frame, &g_dsboomPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_dsboomCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
