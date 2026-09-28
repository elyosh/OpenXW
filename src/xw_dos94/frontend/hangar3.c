#include "xw_dos94/frontend/hangar3.h"
#include "xw/frontend/scenes/hangar3.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/hangar3_task.h"
#include "xw_runtime/runtime/hangar3_view_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x484694. */
int16_t Dos94_Hangar3_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Hangar3_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Dos94_Hangar3_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Hangar3_user_SoundAction);
				break;
		}
	}
	return handled;
}

/* DOS94 0x484716. */
int16_t Dos94_Hangar3_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_hangar3BackgroundHandle);
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
	xmemhdl_Unlock_Handle(g_hangar3BackgroundHandle);
	return drawResult;
}

/* DOS94 0x48486a. */
int16_t Dos94_Hangar3_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_hangar3BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_hangar3PreviousDirtyRect,
								  g_hangar3PreviousDirtyRect.left, g_hangar3PreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_hangar3BackgroundHandle);
	return 1;
}

/* DOS94 0x4845d4. Individual return scenes precede the campaign cutscene. */
static void end_view(int32_t time) {
	(void)time;
	int scene = shellext_Get_Cur_Scene();
	int16_t next, section, exit_scene;
	if (scene == 17) {
		next = 27;
		section = 26;
	} else if (scene == 83)
		next = section = 30;
	else if (scene >= 250 && scene <= 253) {
		next = scene + 5;
		section = shipext_Get_Pending_Tour_Cutscene();
	} else if (scene == 282 || (scene >= 286 && scene <= 288))
		next = section = 4;
	else
		return;
	if (shellext_Check_Scene_Exit(&exit_scene, next, section, g_hangar3Film->cur_cel == g_hangar3Film->cels))
		xerror_Set_Landru_Exit(exit_scene);
}

/* DOS94 0x484232. */
void Dos94_Hangar3(XwShellContext* shell) {
	int scene = shellext_Get_Cur_Scene();
	if (scene == 83)
		xfade_AddTimedText("Returning to Independence.", 4, 40, 0, 98, 176, 16);
	else if (scene >= 250 && scene <= 253)
		xfade_AddTimedText("Returning to the Defiance.", 4, 40, 0, 60, 176, 16);
	else if (scene == 282 || (scene >= 286 && scene <= 288))
		xfade_AddTimedText("Leaving the Defiance.", 4, 40, 0, 98, 176, 16);
	ResFile* resource = xres_Open_Resource("hanger3.lfd");
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_hangar3BackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	int index = 1;
	switch (scene) {
		case 17:
			xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_4);
			break;
		case 83:
		case 286:
			index = 3;
			break;
		case 287:
			index = 5;
			break;
		case 288:
			index = 7;
			break;
		case 250:
			index = 13;
			break;
		case 251:
			index = 9;
			break;
		case 252:
			index = 11;
			break;
		case 253:
			index = 15;
			break;
		case 282:
			index = 17;
			break;
	}
	if (!xio_Is_System_Slower_Than(1))
		++index;
	ResFile* source = index >= 15 && index < 19 ? xres_Open_Resource("bwing.lfd") : resource;
	g_hangar3Film = xfilm_Res_Callback_Film(source, g_hangar3ResourceNames[index], &frame, 0, 0, 0,
											Dos94_Hangar3_film_Callback);
	if (source != resource)
		xres_Close_Resource(source);
	xfilm_Set_Film_Def_Palette(g_hangar3Film, shell->standardPalette);
	g_hangar3BackgroundActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 200);
	xactor_Set_Actor_User_Function(g_hangar3BackgroundActor, Hangar3_user_BeginDirtyFrame);
	xactor_Set_Actor_Draw_Function(g_hangar3BackgroundActor, Dos94_Hangar3_draw_Background);
	g_hangar3EraseActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, -200);
	xactor_Set_Actor_User_Function(g_hangar3EraseActor, Hangar3_user_Erase);
	xactor_Set_Actor_Draw_Function(g_hangar3EraseActor, XwCutscene_DrawCloseOnRefresh);
	Hangar3_OpenMusic(resource, g_hangar3Film);
	xview_Set_View_Update_Function(end_view);
	if (xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	XwHangar3_RunView(resource, 1);
}
