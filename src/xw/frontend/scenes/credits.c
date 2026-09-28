#include "xw/frontend/scenes/credits.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

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

// GLOBAL: XW 0x4D1610
const char* g_creditsMusicFilename = "cdmusic.lfd";

// GLOBAL: XW 0x4D1614
const char* g_creditsMusicName = "medley";

// GLOBAL: XW 0x4F626C
XwSceneMusicHandles g_creditsMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F6A40
Actor* g_creditsTextActor = NULL;

// GLOBAL: XW 0x4F6A44
Actor* g_creditsBackgroundActor = NULL;

// GLOBAL: XW 0x4F6A48
int16_t g_creditsCurrentParagraphLineCount = 0;

// GLOBAL: XW 0x4F6A50
Rect g_creditsPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F6A58
int16_t g_creditsParagraphCount = 0;

// GLOBAL: XW 0x4F6A5C
Actor* g_creditsOverlayActor = NULL;

// GLOBAL: XW 0x4F6A60
Film* g_creditsFilm = NULL;

// GLOBAL: XW 0x4F6A64
LandruHandle g_creditsText = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F6A68
Rect g_creditsCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F6A70
LandruHandle g_creditsBackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x43EA70
void credits_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		ResFile* resourceFile;
		Sound* music;
		g_creditsMusicState.film = sceneFilm;
		resourceFile = xres_Open_Resource(g_creditsMusicFilename);
		g_creditsMusicState.sound = xsound_Res_Music(resourceFile, g_creditsMusicName);
		xres_Close_Resource(resourceFile);
		soundext_Start_Resource_Sound(g_creditsMusicState.sound);
/* Resource pointers are outside the numeric flight-sound ID range. */
#ifdef XW_MODERN
		Dos94_soundext_SetVolume(g_creditsMusicState.sound, 0);
#else
		soundext_SetVolume(0, 0);
#endif
		music = g_creditsMusicState.sound;
		soundext_FadeVolume(music, CREDITS_MUSIC_START_VOLUME, CREDITS_MUSIC_START_FADE_DURATION);
		music = g_creditsMusicState.sound;
		xsound_Set_Sound_User_Function(music, credits_user_Music);
	}
}

// FUNCTION: XW 0x43EB10
void credits_user_Music(Sound* unusedSound, int time) {
	(void)unusedSound;
	if (time == XW_CREDITS_MUSIC_FADE_TIME)
		soundext_FadeVolume(g_creditsMusicState.sound, 0, XW_CREDITS_MUSIC_FADE_DURATION);
}

// FUNCTION: XW 0x43EB30
int16_t j_ShellPreferences_GetSfxEnabled(int16_t unusedSoundId) {
	(void)unusedSoundId;
	return ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x441770
XwShellSceneResult credits_Credits(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("credits.lfd");
	Rect frame;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_creditsBackgroundHandle = xmemhdl_Alloc_Clear_Handle(CREDITS_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	g_creditsFilm = xfilm_Res_Callback_Film(sceneResource, "crdts_f", &frame, 0, 0, 0, credits_film_Callback);
	g_creditsBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, CREDITS_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_creditsBackgroundActor, credits_user_Background);
	xactor_Set_Actor_Draw_Function(g_creditsBackgroundActor, credits_draw_Background);
	g_creditsOverlayActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, CREDITS_OVERLAY_Z);
	xactor_Set_Actor_User_Function(g_creditsOverlayActor, credits_user_Close);
	xactor_Set_Actor_Draw_Function(g_creditsOverlayActor, XwCutscene_DrawCloseOnRefresh);
	xrect_Inset_Rect(&frame, CREDITS_TEXT_INSET_X, 0);
	g_creditsTextActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, CREDITS_TEXT_Z);
	xactor_Set_Actor_User_Function(g_creditsTextActor, credits_user_Credit);
	xactor_Set_Actor_Draw_Function(g_creditsTextActor, credits_draw_Credit);
	g_creditsText = xparagrp_Res_Paragraph(sceneResource, "credits");
	g_creditsParagraphCount = xparagrp_Count_Paragraphs(g_creditsText);
	xfilm_Set_Film_Def_Palette(g_creditsFilm, shell->standardPalette);
	credits_OpenMusic(sceneResource, g_creditsFilm);
	soundext_RecheckSfxPreference();
	xview_Set_View_Update_Function(credits_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
#ifdef XW_MODERN
	XwCredits_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xparagrp_Free_Paragraph(g_creditsText);
	xmemhdl_Free_Handle(g_creditsBackgroundHandle);
	xres_Close_Resource(sceneResource);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4419A0
void credits_end_View(int time) {
	int16_t exitScene;
	(void)time;
	if (shellext_Get_Cur_Scene() == XW_SCENE_INTRO_CREDITS) {
		if (g_savedShellPreferences.introPlaybackMode == 0) {
			if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_REGISTER_INITIAL, XW_SCENE_REGISTER_INITIAL,
										  g_creditsFilm->cur_cel == g_creditsFilm->cels) != 0)
				xerror_Set_Landru_Exit(exitScene);
		} else if (g_creditsFilm->cur_cel == g_creditsFilm->cels) {
			xerror_Set_Landru_Exit(XW_SCENE_STARTUP_LOGO);
		}
	} else if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DEBRIEF_TOUR, XW_SCENE_DEBRIEF_TOUR,
										 g_creditsFilm->cur_cel == g_creditsFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x441A40
int16_t credits_film_Callback(Film* film, FilmObject* object) {
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
				credits_Credit_Actor_To_Buffer(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, credits_user_Sound);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x441AC0
int16_t credits_Credit_Actor_To_Buffer(Actor* actor) {
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
						CREDITS_BACKGROUND_WIDTH, CREDITS_BACKGROUND_HEIGHT, 0);
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

// FUNCTION: XW 0x441BA0
void credits_user_Sound(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (actor->var2 != 0) {
		j_ShellPreferences_GetSfxEnabled(actor->var2);
	}
}

// FUNCTION: XW 0x441BC0
void credits_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_creditsPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_creditsPreviousDirtyRect, &g_creditsCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_creditsCurrentDirtyRect);
}

// FUNCTION: XW 0x441C10
int16_t credits_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	backgroundPixels = xmemhdl_Lock_Handle(g_creditsBackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_creditsPreviousDirtyRect,
								  g_creditsPreviousDirtyRect.left, g_creditsPreviousDirtyRect.top,
								  CREDITS_BACKGROUND_WIDTH, CREDITS_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_creditsBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x441C70
void credits_user_Credit(Actor* actor, int unusedTime) {
	(void)unusedTime;
	actor->var1 += CREDITS_SCROLL_STEP;
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

// FUNCTION: XW 0x441CE0
int16_t credits_draw_Credit(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
	xrect_Set_Rect(&lineRect, 0, CREDITS_BACKGROUND_HEIGHT - actor->var1, CREDITS_BACKGROUND_WIDTH,
				   CREDITS_BACKGROUND_HEIGHT + CREDITS_BODY_SPACING - actor->var1);
	for (paragraphIndex = 0; paragraphIndex < g_creditsParagraphCount - 1; ++paragraphIndex) {
		if (lineRect.top >= CREDITS_BACKGROUND_HEIGHT)
			break;
		g_creditsCurrentParagraphLineCount = xparagrp_Count_Paragraph_Strings(g_creditsText, paragraphIndex);
		for (lineIndex = 0; lineIndex < g_creditsCurrentParagraphLineCount; ++lineIndex) {
			if (lineRect.bottom > 0) {
				int16_t textColor;
				xparagrp_Get_Paragraph_String(g_creditsText, lineText, paragraphIndex, lineIndex);
				textColor = CREDITS_COLOR_MAX;
				if (lineRect.bottom < CREDITS_TOP_FADE_LIMIT)
					textColor = (lineRect.bottom >> 1) + CREDITS_COLOR_MIN;
				if (lineRect.top > CREDITS_BOTTOM_FADE_START)
					textColor = ((CREDITS_BACKGROUND_HEIGHT - lineRect.top) >> 1) + CREDITS_COLOR_MIN;
				if (lineIndex == 0 && textColor >= CREDITS_COLOR_MAX)
					textColor = CREDITS_COLOR_MAX;
				if (lineIndex != 0 && textColor >= CREDITS_BODY_COLOR_THRESHOLD)
					textColor = CREDITS_BODY_COLOR;
				xfont_Print_Centered_Text(lineText, &lineRect, CREDITS_FONT, textColor);
				if (lineIndex == 0) {
					int16_t halfWidth = CREDITS_UNDERLINE_MAX_HALF_WIDTH;
					int16_t underlineX, underlineWidth;
					if (lineRect.bottom < CREDITS_UNDERLINE_TOP_LIMIT)
						halfWidth = CREDITS_UNDERLINE_SCALE * lineRect.bottom;
					if (lineRect.top > CREDITS_UNDERLINE_BOTTOM_START)
						halfWidth = CREDITS_UNDERLINE_SCALE * (CREDITS_UNDERLINE_BOTTOM_END - lineRect.top);
					if (halfWidth > CREDITS_UNDERLINE_MAX_HALF_WIDTH)
						halfWidth = CREDITS_UNDERLINE_MAX_HALF_WIDTH;
					underlineX = CREDITS_BACKGROUND_WIDTH / 2 - halfWidth;
					underlineWidth = 2 * (CREDITS_BACKGROUND_WIDTH / 2 - underlineX);
					if (underlineWidth > 0)
						xpaint_Horiz_Clipped_Line(underlineX, lineRect.bottom + 1, underlineWidth,
												  CREDITS_UNDERLINE_COLOR);
				}
			}
			if (lineIndex == 0)
				xrect_Offset_Rect(&lineRect, 0, CREDITS_HEADING_SPACING);
			else
				xrect_Offset_Rect(&lineRect, 0, CREDITS_BODY_SPACING);
		}
		xrect_Offset_Rect(&lineRect, 0, CREDITS_PARAGRAPH_SPACING);
	}
	if (actor->var1 >= CREDITS_FINAL_SCROLL_START) {
		int16_t finalTextColor;
		xrect_Set_Rect(&lineRect, 0, CREDITS_FINAL_TOP, CREDITS_BACKGROUND_WIDTH,
					   CREDITS_FINAL_TOP + CREDITS_BODY_SPACING);
		finalTextColor = actor->var1 - CREDITS_FINAL_COLOR_OFFSET;
		if (finalTextColor > CREDITS_COLOR_MAX)
			finalTextColor = CREDITS_COLOR_MAX;
		if (finalTextColor >= CREDITS_COLOR_MIN && finalTextColor < CREDITS_COLOR_MAX + 1) {
			xparagrp_Get_Paragraph_String(g_creditsText, lineText, g_creditsParagraphCount - 1, 0);
			xfont_Print_Centered_Text(lineText, &lineRect, CREDITS_FONT, finalTextColor);
			xrect_Offset_Rect(&lineRect, 0, CREDITS_BODY_SPACING);
			xparagrp_Get_Paragraph_String(g_creditsText, lineText, g_creditsParagraphCount - 1, 1);
			xfont_Print_Centered_Text(lineText, &lineRect, CREDITS_FONT, finalTextColor);
		}
	}
	return 1;
}

// FUNCTION: XW 0x441F70
void credits_user_DirtyBounds(Actor* actor, int unusedTime) {
	Rect dirtyBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &dirtyBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&dirtyBounds);
		xrect_Clip_Rect(&dirtyBounds, &actorFrame);
		xrect_Enclose_Rect(&g_creditsCurrentDirtyRect, &dirtyBounds);
	}
}

// FUNCTION: XW 0x441FE0
void credits_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_creditsFilm->cur_cel == g_creditsFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_creditsPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_creditsCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
