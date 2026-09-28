#include "xw/frontend/scenes/brdg2.h"
#include "xw_dos94/frontend/intro.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/brdg2_task.h"
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

/* DOS94 0x4c1efc. */
XwShellSceneResult Dos94_Brdg2_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	xfade_AddTimedText("We are under attack by Imperial Star Destroyers!", BRDG2_CAPTION_START,
					   BRDG2_CAPTION_END, 0, 12, 5, BRDG2_CAPTION_COLOR);
	xfade_AddTimedText("Begin evasive maneuvers.  Launch the X-Wing fighters.", BRDG2_CAPTION_START,
					   BRDG2_CAPTION_END, 0, 12, 15, BRDG2_CAPTION_COLOR);
	sceneResource = xres_Open_Resource("bridge2.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_bridge2BackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(BRDG2_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm =
			xfilm_Res_Callback_Film(sceneResource, "brdg2_s", &frame, 0, 0, 0, Dos94_Brdg2_film_Callback);
	else
		sceneFilm =
			xfilm_Res_Callback_Film(sceneResource, "brdg2_f", &frame, 0, 0, 0, Dos94_Brdg2_film_Callback);
	g_bridge2Film = sceneFilm;
	g_bridge2BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG2_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_bridge2BackgroundActor, Brdg2_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_bridge2BackgroundActor, Dos94_Brdg2_draw_Background);
	g_bridge2EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG2_ERASE_Z);
	xactor_Set_Actor_User_Function(g_bridge2EraseActor, Brdg2_user_Erase);
	xactor_Set_Actor_Draw_Function(g_bridge2EraseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_bridge2Film, shell->standardPalette);
	Brdg2_OpenMusic(sceneResource, g_bridge2Film);
	xview_Set_View_Update_Function(Dos94_Brdg2_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	XwBrdg2_RunView(sceneResource);
}

/* DOS94 0x4c2172. */
void Dos94_Brdg2_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, 16, XW_SCENE_REGISTER_INITIAL,
								  g_bridge2Film->cur_cel == g_bridge2Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

/* DOS94 0x4c21b8. */
int16_t Dos94_Brdg2_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Brdg2_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Dos94_Brdg2_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Brdg2_user_SpeechAction);
				break;
		}
	}
	return handled;
}

/* DOS94 0x4c223a. */
int16_t Dos94_Brdg2_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge2BackgroundHandle);
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
	xmemhdl_Unlock_Handle(g_bridge2BackgroundHandle);
	return drawResult;
}

/* DOS94 0x4c238e. */
int16_t Dos94_Brdg2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge2BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_bridge2PreviousDirtyRect,
								  g_bridge2PreviousDirtyRect.left, g_bridge2PreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_bridge2BackgroundHandle);
	return 1;
}
