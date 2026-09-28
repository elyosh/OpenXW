#include "xw/frontend/scenes/logo640.h"
#include "xw_dos94/frontend/intro.h"

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

/* DOS94 0x500000. */
XwShellSceneResult Dos94_Logo640_Play(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("logo.lfd");
	Rect frame;
	LandruDisplay_SetLowResolutionMode(0);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_logo640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
		g_logo640Film =
			xfilm_Res_Callback_Film(sceneResource, "logo_s", &frame, 0, 0, 0, Dos94_Logo640_film_Callback);
		g_logo640BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LOGO640_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_logo640BackgroundActor, Logo640_user_BeginDirtyFrame);
		xactor_Set_Actor_Draw_Function(g_logo640BackgroundActor, Dos94_Logo640_draw_Background);
		g_logo640EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, LOGO640_ERASE_Z);
		xactor_Set_Actor_User_Function(g_logo640EraseActor, Logo640_user_Erase);
		xactor_Set_Actor_Draw_Function(g_logo640EraseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		g_logo640Film =
			xfilm_Res_Callback_Film(sceneResource, "logo_f", &frame, 0, 0, 0, Dos94_Logo640_film_Callback);
	}
	xfilm_Set_Film_Def_Palette(g_logo640Film, shell->standardPalette);
	Logo640_OpenMusic(sceneResource, g_logo640Film);
	xview_Set_View_Update_Function(Dos94_Logo640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	XwLogo640_RunView(sceneResource);
}

/* DOS94 0x500258. */
void Dos94_Logo640_end_View(int time) {
	if (time == LOGO640_MUSIC_START_TIME)
		FrontendAudio_PlayFile("XwingCD\\music\\xwintro.wav", 0);
	if (g_logo640Film->cur_cel == g_logo640Film->cels)
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_TITLE_CRAWL);
}

/* DOS94 0x500278. */
int16_t Dos94_Logo640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (xio_Is_System_Slower_Than(LOGO640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
				Dos94_Logo640_StampBackground(actor);
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

/* DOS94 0x500328. */
int16_t Dos94_Logo640_StampBackground(Actor* actor) {
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
						320, 200, 0);
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

/* DOS94 0x50047c. */
int16_t Dos94_Logo640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_logo640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_logo640PreviousDirtyRect,
								  g_logo640PreviousDirtyRect.left, g_logo640PreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_logo640BackgroundHandle);
	return 1;
}
