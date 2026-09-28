#include "xw/frontend/scenes/death640.h"
#include "xw_dos94/frontend/recovery_scenes.h"

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
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x481a98. */
XwShellSceneResult Dos94_Death640_Play(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("funeral.lfd");
	Rect frame;
	int16_t noSavedBackground;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if (shellext_Get_Cur_Scene() == 230 || xio_Is_System_Slower_Than(1)) {
		g_death640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
		const char* name = shellext_Get_Cur_Scene() == 230 ? "fun1_f" : "fun2_s";
		g_death640Film = xfilm_Res_Callback_Film(sceneResource, name, &frame, 0, 0, 0,
												 Dos94_Death640_film_InteriorCallback);
		noSavedBackground = 0;
	} else {
		g_death640Film =
			xfilm_Res_Callback_Film(sceneResource, "fun2_f", &frame, 0, 0, 0, Death640_film_ExteriorCallback);
		noSavedBackground = 1;
	}
	xfilm_Set_Film_Def_Palette(g_death640Film, shell->standardPalette);
	if (noSavedBackground == 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_death640BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DEATH640_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_death640BackgroundActor, Death640_user_Background);
		xactor_Set_Actor_Draw_Function(g_death640BackgroundActor, Dos94_Death640_draw_Background);
		g_death640CloseActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, DEATH640_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_death640CloseActor, Death640_user_Close);
		xactor_Set_Actor_Draw_Function(g_death640CloseActor, XwCutscene_DrawCloseOnRefresh);
	}
	xview_Set_View_Update_Function(Dos94_Death640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Death640_OpenMusic(sceneResource, g_death640Film);
	XwDeath640_RunView(sceneResource, noSavedBackground);
}

/* DOS94 0x481d24. */
void Dos94_Death640_end_View(int time) {
	(void)time;
	int16_t exit_scene;
	int16_t next = shellext_Get_Cur_Scene() == 230 ? 231 : 28;
	if (shellext_Check_Scene_Exit(&exit_scene, next, 28, g_death640Film->cur_cel == g_death640Film->cels))
		xerror_Set_Landru_Exit(exit_scene);
}

/* DOS94 0x481d8c. */
int16_t Dos94_Death640_film_InteriorCallback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = (Actor*)object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Dos94_Death640_film_Actor_To_Background(actor);
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

/* DOS94 0x481e78. */
int16_t Dos94_Death640_film_Actor_To_Background(Actor* actor) {
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
						320, 200, 0);
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

/* DOS94 0x481fcc. */
int16_t Dos94_Death640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									   int16_t unusedX, int16_t unusedY, int16_t refresh) {
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
								  g_death640PreviousDirtyRect.left, g_death640PreviousDirtyRect.top, 320,
								  200);
	xmemhdl_Unlock_Handle(g_death640BackgroundHandle);
	return 1;
}

/* DOS94 0x48229c. */
void Dos94_Death640_CloseMusic(void) {
	int scene = shellext_Get_Cur_Scene();
	if (ShellPreferences_GetMusicEnabled() != 0 && scene != XW_SCENE_FUNERAL_INTERIOR) {
		Sound* music = xsound_Find_Gmid("death");
		g_death640MusicState.sound = music;
		if (music != NULL) {
			soundext_SetPriority((intptr_t)music, 0);
			soundext_FadeVolume(g_death640MusicState.sound, 0, DEATH640_MUSIC_FADE_DURATION);
		}
	}
}
