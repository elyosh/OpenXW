#include "xw/frontend/scenes/gov2.h"
#include "xw_dos94/frontend/scenes.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/gov2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/pal.h>
#include <landru/view.h>
#include <landru/viewadd.h>

static const char* const resource_names[] = { "gov2.lfd", "gov2_f", "gov2_f", "gov3_s", "gov3_f", "gov4_f",
											  "gov4_f",   "gov5_f", "gov5_f", "gov6_f", "gov6_f" };

/* DOS94 0x6025a2. */
static int16_t draw_background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							   int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	if (g_gov2PreviousScrollX != g_gov2ScrollActor->x || g_gov2ScrollActor->var2 != 0) {
		Rect canvasBounds;
		uint8_t* screenPixels = xcanvas_Get_Screen_Buffer();
		const uint8_t* backgroundPixels;
		xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
		canvasBounds.left = g_gov2PreviousScrollX;
		xcanvas_Scroll_Clipped_Buffer(screenPixels, &canvasBounds,
									  g_gov2ScrollActor->x - g_gov2PreviousScrollX, 0,
									  xcanvas_Get_Current_Canvas_Bitmap()->w, GOV2_BACKGROUND_HEIGHT);
		xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
		backgroundPixels = xmemhdl_Lock_Handle(g_gov2BackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &canvasBounds,
									  g_gov2ScrollActor->x + GOV2_BACKGROUND_WIDTH, 0,
									  xcanvas_Get_Current_Canvas_Bitmap()->w, GOV2_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_gov2BackgroundHandle);
		g_gov2PreviousScrollX = g_gov2ScrollActor->x;
		xcanvas_Invalid_Screen_Diff();
	}
	if (xrect_Empty_Rect(&g_gov2RestoreRect) == 0) {
		const uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_gov2BackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_gov2RestoreRect, g_gov2RestoreRect.left,
									  g_gov2RestoreRect.top, GOV2_BACKGROUND_WIDTH, GOV2_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_gov2BackgroundHandle);
	}
	return 1;
}

/* DOS94 0x601fa6. */
XwShellSceneResult Dos94_Gov2_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	int16_t slowFilmSlot;
	if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_3) {
		xfade_AddTimedText("Lord Vader!  What a great privilege to have", GOV2_GREETING_FIRST_START,
						   GOV2_GREETING_FIRST_END, GOV2_CAPTION_FONT, GOV2_GREETING_FIRST_X,
						   GOV2_GREETING_FIRST_Y, GOV2_CAPTION_COLOR);
		xfade_AddTimedText("you unexpectedly grace my humble domain.", GOV2_GREETING_SECOND_START,
						   GOV2_GREETING_SECOND_END, GOV2_CAPTION_FONT, GOV2_GREETING_SECOND_X,
						   GOV2_GREETING_SECOND_Y, GOV2_CAPTION_COLOR);
		xfade_AddTimedText("How may I serve you?", GOV2_QUESTION_START, GOV2_QUESTION_END, GOV2_CAPTION_FONT,
						   GOV2_QUESTION_X, GOV2_QUESTION_Y, GOV2_CAPTION_COLOR);
	}
	if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_4) {
		xfade_AddTimedText("We received your grain shipments, Ghorin...", GOV2_SHIPMENT_START,
						   GOV2_SHIPMENT_END, GOV2_CAPTION_FONT, GOV2_SHIPMENT_X, GOV2_SHIPMENT_Y,
						   GOV2_CAPTION_COLOR);
		xfade_AddTimedText("and the Emperor has sent me here to", GOV2_REPLY_FIRST_START,
						   GOV2_REPLY_FIRST_END, GOV2_CAPTION_FONT, GOV2_REPLY_FIRST_X, GOV2_REPLY_FIRST_Y,
						   GOV2_CAPTION_COLOR);
		xfade_AddTimedText("personally repay you for your treachery.", GOV2_REPLY_SECOND_START,
						   GOV2_REPLY_SECOND_END, GOV2_CAPTION_FONT, GOV2_REPLY_SECOND_X, GOV2_REPLY_SECOND_Y,
						   GOV2_CAPTION_COLOR);
	}
	resourceFile = xres_Open_Resource(resource_names[GOV2_RESOURCE_FILE]);
	xrect_Set_Rect(&frame, 0, 0, GOV2_BACKGROUND_WIDTH, GOV2_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xrect_Clear_Rect(&g_gov2RestoreRect);
	g_gov2PreviousScrollX = 0;
	g_gov2FullRefreshFrames = 0;
	g_gov2BackgroundHandle =
		xmemhdl_Alloc_Clear_Handle(GOV2_BACKGROUND_WIDTH * GOV2_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_GHORIN_2:
			slowFilmSlot = GOV2_SCENE_2_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_3:
			slowFilmSlot = GOV2_SCENE_3_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_4:
			slowFilmSlot = GOV2_SCENE_4_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_5:
			slowFilmSlot = GOV2_SCENE_5_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_6:
			slowFilmSlot = GOV2_SCENE_6_SLOW_FILM;
			break;
		default:
			slowFilmSlot = GOV2_SCENE_2_SLOW_FILM;
			break;
	}
	if ((uint16_t)xio_Is_System_Slower_Than(GOV2_SPEED_THRESHOLD) != 0)
		g_gov2Film = xfilm_Res_Callback_Film(resourceFile, resource_names[slowFilmSlot], &frame, 0, 0, 0,
											 Gov2_film_Callback);
	else
		g_gov2Film =
			xfilm_Res_Callback_Film(resourceFile, resource_names[slowFilmSlot + GOV2_FAST_FILM_OFFSET],
									&frame, 0, 0, 0, Gov2_film_Callback);
	xfilm_Set_Film_Def_Palette(g_gov2Film, shell->standardPalette);
	g_gov2BackgroundActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, GOV2_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_gov2BackgroundActor, draw_background);
	g_gov2CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, GOV2_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_gov2CloseActor, Gov2_user_Close);
	xactor_Set_Actor_Draw_Function(g_gov2CloseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_gov2CurrentDirtyRect);
	xview_Set_View_Update_Function(Gov2_end_View);
	Gov2_OpenMusic(resourceFile, g_gov2Film);
	XwGov2_RunView(resourceFile);
}
