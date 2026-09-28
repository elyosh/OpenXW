#include "xw/frontend/scenes/title.h"
#include "xw_dos94/frontend/intro.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/register.h"
#include "xw/frontend/scenes/slant.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/title_task.h"
#include "xw_runtime/runtime/title_view_task.h"
#endif

#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/pal.h>
#include <landru/paragrp.h>
#include <landru/view.h>
#include <stdlib.h>
#include <string.h>

/* DOS94 0x5c0000. */
XwShellSceneResult Dos94_title_Title(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	char paragraphName[TITLE_PARAGRAPH_NAME_CAPACITY];
	int16_t outputWidth;
	LandruDisplay_SetLowResolutionMode(1);
	resourceFile = xres_Open_Resource("title.lfd");
	if (shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL)
		g_titleTextHandle = xparagrp_Res_Paragraph(resourceFile, "title");
	else {
		strcpy(paragraphName, g_titleTourParagraphTemplate);
		paragraphName[TITLE_PARAGRAPH_TOUR_DIGIT] = g_RegisterShellPilot.current_tour + '1';
		g_titleTextHandle = xparagrp_Res_Paragraph(resourceFile, paragraphName);
	}
	g_titleFont = xfont_Res_Font(resourceFile, "helv-20");
	g_titleFadeColorOffset = 0;
	g_titleLogoScaleStep = TITLE_LOGO_INITIAL_STEP;
	g_titleLogoScaleFraction = 0;
	g_titleFilmTime = shellext_Get_Cur_Scene() != XW_SCENE_INTRO_TITLE_CRAWL ? TITLE_TOUR_START_FRAME : 0;
	for (outputWidth = 0; outputWidth <= TITLE_CRAWL_SCREEN_WIDTH; ++outputWidth) {
		if (outputWidth != 0) {
			g_titleScaleSkip[outputWidth] = TITLE_CRAWL_SCREEN_WIDTH / outputWidth - 1;
			g_titleScaleSkipFraction[outputWidth] =
				((TITLE_CRAWL_SCREEN_WIDTH % outputWidth) << TITLE_SCALE_FRACTION_BITS) / outputWidth;
		} else {
			g_titleScaleSkip[0] = 0;
			g_titleScaleSkipFraction[0] = 0;
		}
	}
	xrect_Set_Rect(&frame, 0, 0, TITLE_CRAWL_SCREEN_WIDTH, TITLE_CRAWL_SCREEN_HEIGHT);
	g_titleTextBitmapHandle = xmemhdl_Alloc_Clear_Handle(TITLE_CRAWL_SCREEN_WIDTH * TITLE_CRAWL_SCREEN_HEIGHT,
														 LANDRU_MEMORY_RESOURCE);
	if (shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL) {
		g_titleAlongActor = xactdelt_Res_Delta_Actor(resourceFile, "along", &frame, 0, 0, TITLE_LOGO_Z);
		xactor_Set_Actor_Time(g_titleAlongActor, 0, TITLE_STARS_FADE_FRAME);
		if ((uint16_t)xio_Is_System_Slower_Than(TITLE_LOGO_SPEED_THRESHOLD) != 0) {
			g_titleStarWarsActor = xactdelt_Res_Delta_Actor(resourceFile, "starwars", &frame, TITLE_LOGO_X,
															TITLE_LOGO_Y, TITLE_LOGO_Z);
			xactor_Set_Actor_User_Function(g_titleStarWarsActor, title_user_Slow_StarWars);
		} else {
			g_titleStarWarsActor = xactdelt_Res_Delta_Actor(resourceFile, "starwars", &frame, TITLE_LOGO_X,
															TITLE_LOGO_Y, TITLE_LOGO_Z);
			xactor_Set_Actor_User_Function(g_titleStarWarsActor, title_user_StarWars);
		}
	}
	g_titleStarsActor = xactdelt_Res_Delta_Actor(shell->resourceFile, "stars-4", &frame, 0, 0, TITLE_STARS_Z);
	xactor_Set_Actor_User_Function(g_titleStarsActor, title_user_Stars);
	g_titleTextPainterActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TITLE_PAINTER_Z);
	xactor_Set_Actor_User_Function(g_titleTextPainterActor, title_user_Back);
	xactor_Set_Actor_Draw_Function(g_titleTextPainterActor, Dos94_title_draw_Back);
	g_titleCrawlActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_titleCrawlActor, title_user_Title);
	xactor_Set_Actor_Draw_Function(g_titleCrawlActor, XwTitle_DrawCrawl);
	xpal_Set_Dest_Palette(xpal_Res_Palette(resourceFile, "title"));
	xpal_Set_Dest_Palette(shell->standardPalette);
	xfade_Start_Full_Fade(FADE_WIPE_SNAP_ON, FADE_COLOR_TWO_PHASE, 1, 0, 1);
	title_OpenMusic(resourceFile, &g_titleFilmTime);
	if (ShellPreferences_GetMusicEnabled())
		xsound_Set_Sound_User_Function(g_titleMusicState.sound, Dos94_title_user_Music);
	soundext_RecheckSfxPreference();
	if (shellext_Get_Cur_Scene() == XW_SCENE_TOUR_TITLE_CRAWL)
		FrontendAudio_PlayFile("XwingCD\\music\\starwars.wav", 0);
	xview_Set_View_Update_Function(Dos94_title_end_View);
	xview_Disable_Global_View_Erase();
	XwTitle_RunView(resourceFile);
}

/* DOS94 0x5c0432. */
void Dos94_title_end_View(int unusedTime) {
	int16_t exitScene;
	Rect frame;
	int lineIndex;
	(void)unusedTime;
	if ((int16_t)shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_OPENING, XW_SCENE_REGISTER_INITIAL,
									  g_titleFilmTime == TITLE_END_FRAME) != 0)
			xerror_Set_Landru_Exit(exitScene);
	} else if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_TOUR_DEPART_INDEPENDENCE,
										 XW_SCENE_BRIEFING_TOUR, g_titleFilmTime == TITLE_END_FRAME) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
	if (g_titleFilmTime == TITLE_SET_VIEW_FRAME) {
		xrect_Set_Rect(&frame, 0, TITLE_VIEW_TOP, TITLE_CRAWL_SCREEN_WIDTH, TITLE_CRAWL_SCREEN_HEIGHT);
		xview_Set_View_Frame(TITLE_VIEW_ID, &frame);
		xview_Set_View_Pos(TITLE_VIEW_ID, frame.left, frame.top);
	}
	if (g_titleFilmTime >= TITLE_FADE_START_FRAME && (g_titleFilmTime & 1) != 0)
		++g_titleFadeColorOffset;
	if (g_titleFilmTime == TITLE_END_FRAME - 1) {
		int16_t linesLeft = g_titleLineCount;
		for (lineIndex = 0; linesLeft > 0; ++lineIndex, --linesLeft)
			g_titleLineActive[lineIndex] = 0;
	}
	++g_titleFilmTime;
	if (g_titleFilmTime == TITLE_SLOW_SKIP_FROM_FRAME &&
		(uint16_t)xio_Is_System_Slower_Than(LANDRU_SYSTEM_SPEED_DEFAULT) != 0)
		g_titleFilmTime = TITLE_SLOW_SKIP_TO_FRAME;
}

/* DOS94 0x5c0a64. */
int16_t Dos94_title_draw_Back(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh) {
	int16_t previousCanvasWidth;
	int16_t previousCanvasHeight;
	uint8_t* previousCanvasPixels;
	Rect textRowRect;
	Rect previousCanvasClip;
	char lineText[TITLE_CRAWL_TEXT_CAPACITY];
	int16_t lineIndex;

	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh != 0) {
		uint8_t* bitmapPixels = xmemhdl_Lock_Handle(g_titleTextBitmapHandle);
		xcanvas_Push_Canvas(&previousCanvasPixels, bitmapPixels, &previousCanvasClip, &previousCanvasWidth,
							&previousCanvasHeight, TITLE_CRAWL_SCREEN_WIDTH, TITLE_CRAWL_SCREEN_HEIGHT, 0);
		for (lineIndex = 0; lineIndex < g_titleLineCount; ++lineIndex) {
			if (g_titleLineDrawn[lineIndex] == 0 && g_titleLineY[lineIndex] <= TITLE_CRAWL_SCREEN_HEIGHT) {
				xrect_Set_Rect(&textRowRect, 0, g_titleLineBitmapY[lineIndex], TITLE_CRAWL_SCREEN_WIDTH,
							   g_titleLineBitmapY[lineIndex] + TITLE_CRAWL_BITMAP_LINE_HEIGHT);
				xpaint_Paint_Clipped_Rect(&textRowRect, 0);
				xparagrp_Get_Paragraph_String(g_titleTextHandle, lineText, 0, lineIndex);
				xfont_Print_Centered_Text(lineText, &textRowRect, 2, TITLE_CRAWL_TEXT_COLOR);
				g_titleLineDrawn[lineIndex] = 1;
			}
		}
		xcanvas_Pop_Canvas(previousCanvasPixels, &previousCanvasClip, previousCanvasWidth,
						   previousCanvasHeight);
		xmemhdl_Unlock_Handle(g_titleTextBitmapHandle);
	}
	return 1;
}

/* DOS94 0x5c0cee. */
void Dos94_title_user_Music(Sound* sound, int time) {
	if (shellext_Get_Cur_Scene() == XW_SCENE_TOUR_TITLE_CRAWL) {
		if (time == TITLE_MUSIC_TOUR_FADE_TIME)
			soundext_FadeVolume(sound, 0, TITLE_MUSIC_END_FADE_DURATION);
	} else {
		Sound* attackMusic;
		int16_t beat;
		attackMusic = xsound_Find_Gmid("inattack");
		beat = soundext_GetMusicParam(sound, XW_SOUND_QUERY_BEAT, 0);
		if (beat != XW_SOUND_QUERY_UNSUPPORTED && (uint16_t)beat > TITLE_MUSIC_ATTACK_BEAT_THRESHOLD &&
			g_titleMusicState.attackTransitionQueued == 0) {
			soundext_SetTriggerContext(g_titleMusicState.sound, TITLE_MUSIC_ATTACK_MARKER);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, attackMusic->id, 0, 0, 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
			xsound_Set_Sound_Keep(g_titleMusicState.sound);
			xsound_Set_Sound_Keep(attackMusic);
			g_titleMusicState.attackTransitionQueued = 1;
		}
	}
}
