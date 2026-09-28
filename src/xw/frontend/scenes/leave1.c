#include "xw/frontend/scenes/leave1.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/leave1_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D5B58
const char* g_leave1MusicFilename = "hr1music.lfd";

// GLOBAL: XW 0x4D5B5C
const char* g_leave1MusicName = "hangar";

// GLOBAL: XW 0x4F7978
Actor* g_leave1BackgroundActor = NULL;

// GLOBAL: XW 0x4F797C
Film* g_leave1Film = NULL;

// GLOBAL: XW 0x4F7980
Rect g_leave1PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7988
Actor* g_leave1CloseActor = NULL;

// GLOBAL: XW 0x4F7990
Rect g_leave1CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F80BC
XwSceneMusicHandles g_leave1MusicState = { NULL, NULL };

// FUNCTION: XW 0x454DA0
XwShellSceneResult Leave1_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	ResFile* yavinResource;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	sceneResource = xres_Open_Resource("leave1.lfd");
	yavinResource = xres_Open_Resource("yavin3.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_leave1Film = xfilm_Res_Callback_Film(sceneResource, "leave1_f", &frame, 0, 0, 0, Leave1_film_Callback);
	g_leave1BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LEAVE1_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_leave1BackgroundActor, Leave1_user_Background);
	xactor_Set_Actor_Draw_Function(g_leave1BackgroundActor, Cutscene_DrawEraseViewClip);
	g_leave1CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LEAVE1_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_leave1CloseActor, Leave1_user_Close);
	xactor_Set_Actor_Draw_Function(g_leave1CloseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_leave1Film, shell->standardPalette);
	Leave1_OpenMusic(sceneResource, g_leave1Film);
	Leave1_LoadSoundEffects(sceneResource, g_leave1Film);
	xview_Set_View_Update_Function(Leave1_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwLeave1_RunView(sceneResource, yavinResource);
#else
	j_xviewadd_Handle_View();
	Leave1_TransitionMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xview_Enable_All_View_Erase();
	xres_Close_Resource(yavinResource);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x454F50
void Leave1_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_YAVIN_DEPARTURE_2, shipext_Get_Pending_Medal_Scene(),
								  g_leave1Film->cur_cel == g_leave1Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x454F90
int16_t Leave1_film_Callback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
			xactor_Set_Actor_User_Function(actor, Leave1_user_DirtyBounds);
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Leave1_user_Sound);
		}
	}
	return 0;
}

// FUNCTION: XW 0x454FE0
void Leave1_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Leave1_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x455000
void Leave1_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_leave1PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_leave1PreviousDirtyRect, &g_leave1CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_leave1CurrentDirtyRect);
}

// FUNCTION: XW 0x455090
void Leave1_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_leave1CurrentDirtyRect, &actorBounds);
	}
}

// FUNCTION: XW 0x455100
void Leave1_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_leave1Film->cur_cel == g_leave1Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_leave1PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_leave1CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x457B90
void Leave1_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_leave1MusicState.film = sceneFilm;
		g_leave1MusicState.sound = xsound_Find_Gmid(g_leave1MusicName);
		if (g_leave1MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(g_leave1MusicFilename);
			g_leave1MusicState.sound = xsound_Res_Music(musicResource, g_leave1MusicName);
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_leave1MusicState.sound);
			soundext_ScanMidi(g_leave1MusicState.sound, 0, XW_LEAVE1_START_BEAT, XW_LEAVE1_START_TICK);
			xsound_Set_Sound_Keep(g_leave1MusicState.sound);
		}
		xsound_Set_Sound_Keep(g_leave1MusicState.sound);
		xsound_Set_Sound_User_Function(g_leave1MusicState.sound, Leave1_user_Music);
	}
}

// FUNCTION: XW 0x457C50
void Leave1_TransitionMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int tick;
		j_lolevel_ImPause();
		tick = soundext_GetMusicParam(g_leave1MusicState.sound, XW_SOUND_QUERY_TICK, 0);
		j_lolevel_ImResume();
		soundext_JumpMidi(g_leave1MusicState.sound, 0, XW_LEAVE1_TRANSITION_BEAT, tick);
	}
}

// FUNCTION: XW 0x457C90
void Leave1_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	if (g_leave1MusicState.film->cur_cel == XW_LEAVE1_MUSIC_CONTROL_CEL) {
		soundext_SetHook(g_leave1MusicState.sound, XW_SOUND_CONTROL_DIRECT, 1, 0);
	}
}

// FUNCTION: XW 0x457CC0
void Leave1_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_2, 0, NULL, 0, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x457CF0
void Leave1_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case XW_LEAVE1_CUE_FLYBY:
			if (ShellPreferences_GetSfxEnabled()) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_2);
			}
			break;
	}
}
