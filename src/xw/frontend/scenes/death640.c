#include "xw/frontend/scenes/death640.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/death640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D3750
const char* g_death640ResourceFilename = "death640.lfd";

// GLOBAL: XW 0x4D3754
const char* g_death640InteriorFilmName = "int_640";

// GLOBAL: XW 0x4D3758
const char* g_death640ExteriorFilmName = "ext_640";

// GLOBAL: XW 0x4D375C
const char* g_death640PlanetFilmName = "planet";

// GLOBAL: XW 0x4F74E4
XwSceneMusicHandles g_death640MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F74F0
Actor* g_death640BackgroundActor = NULL;

// GLOBAL: XW 0x4F74F8
Rect g_death640PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F7500
Actor* g_death640CloseActor = NULL;

// GLOBAL: XW 0x4F7508
Rect g_death640CurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F7510
Film* g_death640Film = NULL;

// GLOBAL: XW 0x4F7514
LandruHandle g_death640BackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x44A9D0
void Death640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_death640MusicState.film = sceneFilm;
		g_death640MusicState.sound = xsound_Find_Gmid("death");
		if (g_death640MusicState.sound == NULL) {
			unsigned int startBeat = shellext_Get_Cur_Scene() != XW_SCENE_FUNERAL_INTERIOR
										 ? DEATH640_MUSIC_EXTERIOR_BEAT
										 : DEATH640_MUSIC_INTERIOR_BEAT;
			ResFile* musicResource = xres_Open_Resource("rsmusic.lfd");
			g_death640MusicState.sound = xsound_Res_Music(musicResource, "death");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_death640MusicState.sound);
			soundext_ScanMidi(g_death640MusicState.sound, 0, startBeat, 0);
		}
		xsound_Set_Sound_Keep(g_death640MusicState.sound);
		xsound_Set_Sound_User_Function(g_death640MusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x44AA90
void Death640_CloseMusic(void) {
	int scene = shellext_Get_Cur_Scene();
	if (ShellPreferences_GetMusicEnabled() != 0 && scene != XW_SCENE_FUNERAL_INTERIOR) {
		Sound* music = xsound_Find_Gmid("death");
		g_death640MusicState.sound = music;
		if (music != NULL) {
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, 0);
			soundext_FadeVolume(g_death640MusicState.sound, 0, DEATH640_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x44AAF0
void Death640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_1, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x44AB20
void Death640_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (cue) {
			case XW_DEATH640_CUE_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_1);
				break;
		}
	}
}

// FUNCTION: XW 0x44AB40
XwShellSceneResult Death640_Play(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource(g_death640ResourceFilename);
	Rect frame;
	int16_t noSavedBackground;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_FUNERAL_INTERIOR:
			g_death640BackgroundHandle =
				xmemhdl_Alloc_Clear_Handle(DEATH640_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
			g_death640Film = xfilm_Res_Callback_Film(sceneResource, g_death640InteriorFilmName, &frame, 0, 0,
													 0, Death640_film_InteriorCallback);
			noSavedBackground = 0;
			break;
		case XW_SCENE_FUNERAL_EXTERIOR:
			g_death640Film = xfilm_Res_Callback_Film(sceneResource, g_death640ExteriorFilmName, &frame, 0, 0,
													 0, Death640_film_ExteriorCallback);
			noSavedBackground = 1;
			break;
		case XW_SCENE_FUNERAL_PLANET:
			g_death640Film = xfilm_Res_Callback_Film(sceneResource, g_death640PlanetFilmName, &frame, 0, 0, 0,
													 Death640_film_ExteriorCallback);
			noSavedBackground = 1;
			break;
		default:
			/* The original derives this flag from incidental shell-pointer bits. */
			noSavedBackground = 1;
			break;
	}
	xfilm_Set_Film_Def_Palette(g_death640Film, shell->standardPalette);
	if (noSavedBackground == 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_death640BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DEATH640_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_death640BackgroundActor, Death640_user_Background);
		xactor_Set_Actor_Draw_Function(g_death640BackgroundActor, Death640_draw_Background);
		g_death640CloseActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DEATH640_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_death640CloseActor, Death640_user_Close);
		xactor_Set_Actor_Draw_Function(g_death640CloseActor, XwCutscene_DrawCloseOnRefresh);
	}
	xview_Set_View_Update_Function(Death640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Death640_OpenMusic(sceneResource, g_death640Film);
	Death640_LoadSoundEffects(sceneResource, g_death640Film);
#ifdef XW_MODERN
	XwDeath640_RunView(sceneResource, noSavedBackground);
#else
	j_xviewadd_Handle_View();
	Death640_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if (noSavedBackground == 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_death640BackgroundHandle);
	}
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x44AD70
void Death640_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
#ifdef XW_MODERN
	nextScene = XW_SCENE_REGISTER_RETURN;
#endif
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_FUNERAL_INTERIOR:
			nextScene = XW_SCENE_FUNERAL_EXTERIOR;
			break;
		case XW_SCENE_FUNERAL_EXTERIOR:
			nextScene = XW_SCENE_FUNERAL_PLANET;
			break;
		case XW_SCENE_FUNERAL_PLANET:
			nextScene = XW_SCENE_REGISTER_RETURN;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, XW_SCENE_REGISTER_RETURN,
								  g_death640Film->cur_cel == g_death640Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x44ADE0
int16_t Death640_film_InteriorCallback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = (Actor*)object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Death640_film_Actor_To_Background(actor);
			consumeObject = 1;
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
			xactor_Set_Actor_User_Function(actor, Death640_user_DirtyBounds);
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Death640_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x44AE50
int16_t Death640_film_ExteriorCallback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND)
			xactor_Set_Actor_User_Function(actor, Death640_user_Sound);
	}
	return 0;
}

// FUNCTION: XW 0x44AE90
int16_t Death640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_death640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						DEATH640_BACKGROUND_WIDTH, DEATH640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_death640BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x44AF70
void Death640_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Death640_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x44AF90
void Death640_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_death640PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_death640PreviousDirtyRect, &g_death640CurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_death640CurrentDirtyRect);
}

// FUNCTION: XW 0x44AFE0
int16_t Death640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_death640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_death640PreviousDirtyRect,
								  g_death640PreviousDirtyRect.left, g_death640PreviousDirtyRect.top,
								  DEATH640_BACKGROUND_WIDTH, DEATH640_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_death640BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x44B040
void Death640_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Clip_Rect(&actorBounds, &actorFrame);
		xrect_Enclose_Rect(&g_death640CurrentDirtyRect, &actorBounds);
	}
	if (actor->var2 != 0) {
		g_death640Film->var1 = DEATH640_REFRESH_FULL_CANVAS;
	}
}

// FUNCTION: XW 0x44B0C0
void Death640_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_death640Film->cur_cel == g_death640Film->cels || g_death640Film->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		g_death640Film->var1 = 0;
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_death640PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_death640CurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
