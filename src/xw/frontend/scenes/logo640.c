#include "xw/frontend/scenes/logo640.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/logo640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/cursor.h>
#include <landru/viewadd.h>

#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>

// GLOBAL: XW 0x4D5388
const char* g_logo640MusicResourceNames[LOGO640_MUSIC_RESOURCE_NAME_COUNT] = { "lgmusic.lfd", "logo" };

// GLOBAL: XW 0x4D5AB8
const char* g_logo640ResourceNames[LOGO640_RESOURCE_NAME_COUNT] = { "logo640.lfd", "logo640s", "logo640f" };

// GLOBAL: XW 0x4F79BC
XwSceneMusicHandles g_logo640MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F8090
Actor* g_logo640BackgroundActor = NULL;

// GLOBAL: XW 0x4F8098
Rect g_logo640PreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F80A0
Actor* g_logo640EraseActor = NULL;

// GLOBAL: XW 0x4F80A4
Film* g_logo640Film = NULL;

// GLOBAL: XW 0x4F80A8
Rect g_logo640DirtyRect = { 0 };

// GLOBAL: XW 0x4F80B0
LandruHandle g_logo640BackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x455730
void Logo640_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		ResFile* musicResource;
		g_logo640MusicState.film = film;
		musicResource = xres_Open_Resource(g_logo640MusicResourceNames[LOGO640_MUSIC_FILE]);
		g_logo640MusicState.sound =
			xsound_Res_Music(musicResource, g_logo640MusicResourceNames[LOGO640_MUSIC_NAME]);
		xres_Close_Resource(musicResource);
		soundext_Start_Resource_Sound(g_logo640MusicState.sound);
		xsound_Set_Sound_Keep(g_logo640MusicState.sound);
	}
}

// FUNCTION: XW 0x4557A0
void Logo640_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_PING_1, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x4557D0
void Logo640_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled()) {
		switch (action) {
			case XW_LOGO640_ACTION_PING:
				soundext_Play_SFX(XW_SHELL_SFX_PING_1);
				break;
		}
	}
}

// FUNCTION: XW 0x457500
XwShellSceneResult Logo640_Play(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource(g_logo640ResourceNames[LOGO640_RESOURCE_FILE]);
	Rect frame;
	LandruDisplay_SetLowResolutionMode(0);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_logo640BackgroundHandle =
			xmemhdl_Alloc_Clear_Handle(LOGO640_BACKGROUND_ALLOCATION_BYTES, LANDRU_MEMORY_RESOURCE);
		g_logo640Film =
			xfilm_Res_Callback_Film(sceneResource, g_logo640ResourceNames[LOGO640_RESOURCE_SLOW_FILM], &frame,
									0, 0, 0, Logo640_film_Callback);
		g_logo640BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LOGO640_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_logo640BackgroundActor, Logo640_user_BeginDirtyFrame);
		xactor_Set_Actor_Draw_Function(g_logo640BackgroundActor, Logo640_draw_Background);
		g_logo640EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LOGO640_ERASE_Z);
		xactor_Set_Actor_User_Function(g_logo640EraseActor, Logo640_user_Erase);
		xactor_Set_Actor_Draw_Function(g_logo640EraseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		g_logo640Film =
			xfilm_Res_Callback_Film(sceneResource, g_logo640ResourceNames[LOGO640_RESOURCE_FAST_FILM], &frame,
									0, 0, 0, Logo640_film_Callback);
	}
	xfilm_Set_Film_Def_Palette(g_logo640Film, shell->standardPalette);
	Logo640_OpenMusic(sceneResource, g_logo640Film);
	Logo640_LoadSoundEffects(sceneResource, g_logo640Film);
	xview_Set_View_Update_Function(Logo640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwLogo640_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_logo640BackgroundHandle);
	}
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x457700
void Logo640_end_View(int time) {
	if (time == LOGO640_MUSIC_START_TIME)
		FrontendAudio_PlayFile("XwingCD\\music\\xwintro.wav", 0);
	if (g_logo640Film->cur_cel == g_logo640Film->cels)
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_TITLE_CRAWL);
}

// FUNCTION: XW 0x457730
int16_t Logo640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
				Logo640_StampBackground(actor);
				consumeObject = 1;
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
				xactor_Set_Actor_User_Function(actor, Logo640_user_DirtyBounds);
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
				xactor_Set_Actor_User_Function(actor, Logo640_user_SoundAction);
			}
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Logo640_user_SoundAction);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4577D0
int16_t Logo640_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_logo640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						LOGO640_BACKGROUND_WIDTH, LOGO640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_logo640BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4578B0
void Logo640_user_SoundAction(Actor* actor, int unusedTime) {
	int16_t action = actor->var2;
	(void)unusedTime;
	if (action != 0) {
		Logo640_HandleSoundAction(action);
	}
}

// FUNCTION: XW 0x4578D0
void Logo640_user_BeginDirtyFrame(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_logo640PreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_logo640PreviousDirtyRect, &g_logo640DirtyRect);
	}
	xrect_Clear_Rect(&g_logo640DirtyRect);
}

// FUNCTION: XW 0x457920
int16_t Logo640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_logo640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_logo640PreviousDirtyRect,
								  g_logo640PreviousDirtyRect.left, g_logo640PreviousDirtyRect.top,
								  LOGO640_BACKGROUND_WIDTH, LOGO640_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_logo640BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x457980
void Logo640_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect actorRect;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Clip_Rect(&actorRect, &actorFrame);
		xrect_Enclose_Rect(&g_logo640DirtyRect, &actorRect);
	}
}

// FUNCTION: XW 0x4579F0
void Logo640_user_Erase(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_logo640Film->cur_cel == g_logo640Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_logo640PreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_logo640DirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
