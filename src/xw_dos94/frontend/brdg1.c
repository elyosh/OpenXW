#include "xw/frontend/scenes/brdg1.h"
#include "xw_dos94/frontend/intro.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/brdg1_task.h"
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

/* DOS94 0x4c15d8. */
XwShellSceneResult Dos94_Brdg1_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	xfade_AddTimedText("Sir, our TIE Interceptors have located a", BRDG1_REPORT_START, BRDG1_REPORT_END, 0,
					   12, 5, BRDG1_OFFICER_COLOR);
	xfade_AddTimedText("Rebel Fleet orbiting the planet Turkana.", BRDG1_REPORT_START, BRDG1_REPORT_END, 0,
					   12, 15, BRDG1_OFFICER_COLOR);
	xfade_AddTimedText("Excellent!  Prepare the attack.", BRDG1_ATTACK_START, BRDG1_ATTACK_END, 0, 140, 25,
					   BRDG1_COMMANDER_COLOR);
	xfade_AddTimedText("Move our Star Destroyers within range", BRDG1_ORDERS_START, BRDG1_ORDERS_END, 0, 110,
					   20, BRDG1_COMMANDER_COLOR);
	xfade_AddTimedText("and launch all TIE Fighter Squadrons.", BRDG1_ORDERS_START, BRDG1_ORDERS_END, 0, 110,
					   30, BRDG1_COMMANDER_COLOR);
	xfade_AddTimedText("At once, Sir!", BRDG1_ACK_START, BRDG1_ACK_END, 0, 30, 5, BRDG1_OFFICER_COLOR);
	sceneResource = xres_Open_Resource("bridge1.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_bridge1BackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	if ((uint16_t)xio_Is_System_Slower_Than(BRDG1_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm =
			xfilm_Res_Callback_Film(sceneResource, "brdg1_s", &frame, 0, 0, 0, Dos94_Brdg1_film_Callback);
	else
		sceneFilm =
			xfilm_Res_Callback_Film(sceneResource, "brdg1_f", &frame, 0, 0, 0, Dos94_Brdg1_film_Callback);
	g_bridge1Film = sceneFilm;
	g_bridge1BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG1_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_bridge1BackgroundActor, Brdg1_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_bridge1BackgroundActor, Dos94_Brdg1_draw_Background);
	g_bridge1EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BRDG1_ERASE_Z);
	xactor_Set_Actor_User_Function(g_bridge1EraseActor, Brdg1_user_Erase);
	xactor_Set_Actor_Draw_Function(g_bridge1EraseActor, XwCutscene_DrawCloseOnRefresh);
	xfilm_Set_Film_Def_Palette(g_bridge1Film, shell->standardPalette);
	Brdg1_OpenMusic(sceneResource, g_bridge1Film);
	xview_Set_View_Update_Function(Dos94_Brdg1_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	XwBrdg1_RunView(sceneResource);
}

/* DOS94 0x4c18da. */
void Dos94_Brdg1_end_View(int unusedTime) {
	int16_t exitScene;
	(void)unusedTime;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_FLEET_ATTACK, XW_SCENE_REGISTER_INITIAL,
								  g_bridge1Film->cur_cel == g_bridge1Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

/* DOS94 0x4c1920. */
int16_t Dos94_Brdg1_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Brdg1_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Dos94_Brdg1_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Brdg1_user_SpeechAction);
				break;
		}
	}
	return handled;
}

/* DOS94 0x4c19a2. */
int16_t Dos94_Brdg1_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge1BackgroundHandle);
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
	xmemhdl_Unlock_Handle(g_bridge1BackgroundHandle);
	return drawResult;
}

/* DOS94 0x4c1af6. */
int16_t Dos94_Brdg1_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_bridge1BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_bridge1PreviousDirtyRect,
								  g_bridge1PreviousDirtyRect.left, g_bridge1PreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_bridge1BackgroundHandle);
	return 1;
}
