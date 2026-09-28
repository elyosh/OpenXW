#include "xw_dos94/frontend/credits.h"
#include "xw/frontend/scenes/credits.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/credits_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/font.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x343880. */
XwShellSceneResult Dos94_credits_Credits(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("credits.lfd");
	Rect frame;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_creditsBackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	g_creditsFilm =
		xfilm_Res_Callback_Film(sceneResource, "crdts_s", &frame, 0, 0, 0, Dos94_credits_film_Callback);
	g_creditsBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, CREDITS_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_creditsBackgroundActor, credits_user_Background);
	xactor_Set_Actor_Draw_Function(g_creditsBackgroundActor, Dos94_credits_draw_Background);
	g_creditsOverlayActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, CREDITS_OVERLAY_Z);
	xactor_Set_Actor_User_Function(g_creditsOverlayActor, credits_user_Close);
	xactor_Set_Actor_Draw_Function(g_creditsOverlayActor, XwCutscene_DrawCloseOnRefresh);
	xrect_Inset_Rect(&frame, CREDITS_TEXT_INSET_X, 0);
	g_creditsTextActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, CREDITS_TEXT_Z);
	xactor_Set_Actor_User_Function(g_creditsTextActor, Dos94_credits_user_Credit);
	xactor_Set_Actor_Draw_Function(g_creditsTextActor, Dos94_credits_draw_Credit);
	g_creditsText = xparagrp_Res_Paragraph(sceneResource, "credits");
	g_creditsParagraphCount = xparagrp_Count_Paragraphs(g_creditsText);
	xfilm_Set_Film_Def_Palette(g_creditsFilm, shell->standardPalette);
	credits_OpenMusic(sceneResource, g_creditsFilm);
	soundext_RecheckSfxPreference();
	xview_Set_View_Update_Function(Dos94_credits_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	XwCredits_RunView(sceneResource);
}

/* DOS94 0x343b4e. */
void Dos94_credits_end_View(int time) {
	(void)time;
	int16_t exit_scene;
	int16_t next = shellext_Get_Cur_Scene() == 25 ? 26 : 117;
	if (shellext_Check_Scene_Exit(&exit_scene, next, next, g_creditsFilm->cur_cel == g_creditsFilm->cels))
		xerror_Set_Landru_Exit(exit_scene);
}

/* DOS94 0x343bba. */
int16_t Dos94_credits_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, credits_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Dos94_credits_Credit_Actor_To_Buffer(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, credits_user_Sound);
				break;
		}
	}
	return handled;
}

/* DOS94 0x343c3c. */
int16_t Dos94_credits_Credit_Actor_To_Buffer(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_creditsBackgroundHandle);
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
	xmemhdl_Unlock_Handle(g_creditsBackgroundHandle);
	return drawResult;
}

/* DOS94 0x343d90. */
int16_t Dos94_credits_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_creditsBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_creditsPreviousDirtyRect,
								  g_creditsPreviousDirtyRect.left, g_creditsPreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_creditsBackgroundHandle);
	return 1;
}

/* DOS94 0x343df6. */
void Dos94_credits_user_Credit(Actor* actor, int unusedTime) {
	(void)unusedTime;
	actor->var1 += 1;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		Rect dirtyFrame;
		Rect actorFrame;
		xactor_Get_Actor_Frame(actor, &dirtyFrame);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&dirtyFrame);
		xrect_Clip_Rect(&dirtyFrame, &actorFrame);
		xrect_Enclose_Rect(&g_creditsCurrentDirtyRect, &dirtyFrame);
	}
}

/* DOS94 0x343e74. */
int16_t Dos94_credits_draw_Credit(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								  int16_t unusedY, int16_t refresh) {
	Rect lineRect;
	char lineText[CREDITS_LINE_CAPACITY];
	int16_t paragraphIndex, lineIndex;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	xrect_Set_Rect(&lineRect, 0, 200 - actor->var1, 320, 200 + 10 - actor->var1);
	for (paragraphIndex = 0; paragraphIndex < g_creditsParagraphCount - 1; ++paragraphIndex) {
		if (lineRect.top >= 200)
			break;
		g_creditsCurrentParagraphLineCount = xparagrp_Count_Paragraph_Strings(g_creditsText, paragraphIndex);
		for (lineIndex = 0; lineIndex < g_creditsCurrentParagraphLineCount; ++lineIndex) {
			if (lineRect.bottom > 0) {
				int16_t textColor;
				xparagrp_Get_Paragraph_String(g_creditsText, lineText, paragraphIndex, lineIndex);
				textColor = CREDITS_COLOR_MAX;
				if (lineRect.bottom < CREDITS_TOP_FADE_LIMIT)
					textColor = (lineRect.bottom >> 1) + CREDITS_COLOR_MIN;
				if (lineRect.top > 136)
					textColor = ((200 - lineRect.top) >> 1) + CREDITS_COLOR_MIN;
				if (lineIndex == 0 && textColor >= CREDITS_COLOR_MAX)
					textColor = CREDITS_COLOR_MAX;
				if (lineIndex != 0 && textColor >= CREDITS_BODY_COLOR_THRESHOLD)
					textColor = CREDITS_BODY_COLOR;
				xfont_Print_Centered_Text(lineText, &lineRect, 0, textColor);
				if (lineIndex == 0) {
					int16_t halfWidth = 96;
					int16_t underlineX, underlineWidth;
					if (lineRect.bottom < 13)
						halfWidth = CREDITS_UNDERLINE_SCALE * lineRect.bottom;
					if (lineRect.top > 177)
						halfWidth = CREDITS_UNDERLINE_SCALE * (190 - lineRect.top);
					if (halfWidth > 96)
						halfWidth = 96;
					underlineX = 320 / 2 - halfWidth;
					underlineWidth = 2 * (320 / 2 - underlineX);
					if (underlineWidth > 0)
						xpaint_Horiz_Clipped_Line(underlineX, lineRect.bottom + 1, underlineWidth,
												  CREDITS_UNDERLINE_COLOR);
				}
			}
			if (lineIndex == 0)
				xrect_Offset_Rect(&lineRect, 0, 14);
			else
				xrect_Offset_Rect(&lineRect, 0, 10);
		}
		xrect_Offset_Rect(&lineRect, 0, 20);
	}
	if (actor->var1 >= 1220) {
		int16_t finalTextColor;
		xrect_Set_Rect(&lineRect, 0, 90, 320, 90 + 10);
		finalTextColor = actor->var1 - 1204;
		if (finalTextColor > CREDITS_COLOR_MAX)
			finalTextColor = CREDITS_COLOR_MAX;
		if (actor->var1 > 1290)
			finalTextColor = 1338 - actor->var1;
		if (finalTextColor >= CREDITS_COLOR_MIN && finalTextColor < CREDITS_COLOR_MAX + 1) {
			xparagrp_Get_Paragraph_String(g_creditsText, lineText, g_creditsParagraphCount - 1, 0);
			xfont_Print_Centered_Text(lineText, &lineRect, 0, finalTextColor);
			xrect_Offset_Rect(&lineRect, 0, 10);
			xparagrp_Get_Paragraph_String(g_creditsText, lineText, g_creditsParagraphCount - 1, 1);
			xfont_Print_Centered_Text(lineText, &lineRect, 0, finalTextColor);
		}
	}
	return 1;
}
