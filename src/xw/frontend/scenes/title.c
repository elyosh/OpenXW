#include "xw/frontend/scenes/title.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

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

// GLOBAL: XW 0x4D7720
const int16_t g_titleRowScaleThreshold[TITLE_CRAWL_BITMAP_LINE_HEIGHT] = { 276, 116, 238, 38,  188, 132, 248,
																		   70,  260, 100, 228, 54,  178, 22,
																		   214, 148, 164, 202, 6,   86 };

// GLOBAL: XW 0x4D779C
char g_titleTourParagraphTemplate[TITLE_PARAGRAPH_TEMPLATE_CAPACITY] = "todtxt1";

// GLOBAL: XW 0x4FA1F8
int16_t g_titleLineYVelocityFraction[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FA220
int16_t g_titleScaleSkip[TITLE_CRAWL_SCALE_TABLE_COUNT] = { 0 };

// GLOBAL: XW 0x4FA724
Actor* g_titleCrawlActor = NULL;

// GLOBAL: XW 0x4FA728
FontStruct* g_titleFont = NULL;

// GLOBAL: XW 0x4FA72C
Actor* g_titleAlongActor = NULL;

// GLOBAL: XW 0x4FA730
int16_t g_titleLogoScaleStep = 0;

// GLOBAL: XW 0x4FA738
int16_t g_titleLineActive[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FA75C
Actor* g_titleStarsActor = NULL;

// GLOBAL: XW 0x4FA760
uint16_t g_titleScaleSkipFraction[TITLE_CRAWL_SCALE_TABLE_COUNT] = { 0 };

// GLOBAL: XW 0x4FAC64
int16_t g_titleFilmTime = 0;

// GLOBAL: XW 0x4FAC68
int16_t g_titleLineYFraction[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FAC8C
Actor* g_titleTextPainterActor = NULL;

// GLOBAL: XW 0x4FAC90
int16_t g_titleLineYVelocity[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FACB4
int16_t g_titleFadeColorOffset = 0;

// GLOBAL: XW 0x4FACB8
int16_t g_titleLineDrawn[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FACE0
int16_t g_titleLogoScaleFraction = 0;

// GLOBAL: XW 0x4FACE8
int16_t g_titleLineY[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FAD10
int16_t g_titleLineBitmapY[TITLE_CRAWL_LINE_COUNT] = { 0 };

// GLOBAL: XW 0x4FAD34
int16_t g_titleLineCount = 0;

// GLOBAL: XW 0x4FAD38
Actor* g_titleStarWarsActor = NULL;

// GLOBAL: XW 0x4FAD3C
LandruHandle g_titleTextHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FAD40
LandruHandle g_titleTextBitmapHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FAD50
XwTitleMusicState g_titleMusicState = { NULL, NULL, 0 };

// FUNCTION: XW 0x461A40
XwShellSceneResult title_Title(struct XwShellContext* shell) {
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
	xactor_Set_Actor_Draw_Function(g_titleTextPainterActor, title_draw_Back);
	g_titleCrawlActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_titleCrawlActor, title_user_Title);
	xactor_Set_Actor_Draw_Function(g_titleCrawlActor, XwTitle_DrawCrawl);
	xpal_Set_Dest_Palette(xpal_Res_Palette(resourceFile, "title"));
	xpal_Set_Dest_Palette(shell->standardPalette);
	xfade_Start_Full_Fade(FADE_WIPE_SNAP_ON, FADE_COLOR_TWO_PHASE, 1, 0, 1);
	title_OpenMusic(resourceFile, &g_titleFilmTime);
	soundext_RecheckSfxPreference();
	if (shellext_Get_Cur_Scene() == XW_SCENE_TOUR_TITLE_CRAWL)
		FrontendAudio_PlayFile("XwingCD\\music\\starwars.wav", 0);
	xview_Set_View_Update_Function(title_end_View);
	xview_Disable_Global_View_Erase();
#ifdef XW_MODERN
	XwTitle_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	xview_Enable_Global_View_Erase();
	xview_Clear_View_Update_Function();
	title_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xmemhdl_Free_Handle(g_titleTextBitmapHandle);
	xparagrp_Free_Paragraph(g_titleTextHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, 0, 0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x461DB0
void title_end_View(int unusedTime) {
	int16_t exitScene;
	Rect frame;
	int lineIndex;
	(void)unusedTime;
	if ((int16_t)shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL) {
		if (g_savedShellPreferences.introPlaybackMode == 0) {
			if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_OPENING, XW_SCENE_REGISTER_INITIAL,
										  g_titleFilmTime == TITLE_END_FRAME) != 0)
				xerror_Set_Landru_Exit(exitScene);
		} else if (g_titleFilmTime == TITLE_END_FRAME) {
			xerror_Set_Landru_Exit(XW_SCENE_INTRO_OPENING);
		}
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

// FUNCTION: XW 0x461EF0
void title_user_StarWars(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_titleFilmTime == 0) {
		xactor_Hide_Actor(actor);
	} else {
		if (g_titleFilmTime == TITLE_LOGO_HIDE_FRAME) {
			xpal_Screen_To_Src_Palette(0, 0, TITLE_PALETTE_LAST_COLOR);
			xpal_Screen_To_Dest_Palette(0, 0, TITLE_PALETTE_LAST_COLOR);
			xpal_Set_Dest_Pal_Color(TITLE_LOGO_FIRST_COLOR, TITLE_LOGO_LAST_COLOR, 0, 0, 0);
			xfade_Start_Full_Fade(FADE_WIPE_SNAP_ON, FADE_COLOR_PAL_TO_PAL, 1, 0, 0);
		}
		if (g_titleFilmTime == TITLE_LOGO_LATE_MUSIC_FRAME)
			title_StartMusic();
		if (g_titleFilmTime == TITLE_STARS_SHOW_FRAME) {
			xactor_Show_Actor(actor);
			xactor_Set_Actor_Scale(actor, TITLE_LOGO_INITIAL_SCALE, TITLE_LOGO_INITIAL_SCALE);
			title_StartMusic();
		}
		xactor_Set_Actor_Scale(actor, actor->xscale - g_titleLogoScaleStep,
							   actor->yscale - g_titleLogoScaleStep);
		if (actor->xscale < TITLE_LOGO_MIN_SCALE)
			actor->xscale = TITLE_LOGO_MIN_SCALE;
		if (actor->yscale < TITLE_LOGO_MIN_SCALE)
			actor->yscale = TITLE_LOGO_MIN_SCALE;
		if (g_titleFilmTime > TITLE_STARS_SHOW_FRAME && g_titleFilmTime < TITLE_LOGO_END_FRAME) {
			g_titleLogoScaleFraction += TITLE_LOGO_FRACTION_STEP;
			if (g_titleLogoScaleFraction >= TITLE_LOGO_FRACTION_SCALE) {
				g_titleLogoScaleFraction -= TITLE_LOGO_FRACTION_SCALE;
				--g_titleLogoScaleStep;
			}
		}
		if (g_titleFilmTime == TITLE_LOGO_END_FRAME)
			xactor_Hide_Actor(actor);
	}
}

// FUNCTION: XW 0x462020
void title_user_Slow_StarWars(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_titleFilmTime == 0) {
		xactor_Hide_Actor(actor);
	} else {
		if (g_titleFilmTime == TITLE_STARS_SHOW_FRAME) {
			xactor_Show_Actor(actor);
			title_StartMusic();
		}
		if (g_titleFilmTime == TITLE_LOGO_HIDE_FRAME) {
			xpal_Screen_To_Src_Palette(0, 0, TITLE_PALETTE_LAST_COLOR);
			xpal_Screen_To_Dest_Palette(0, 0, TITLE_PALETTE_LAST_COLOR);
			xpal_Set_Dest_Pal_Color(TITLE_LOGO_FIRST_COLOR, TITLE_LOGO_LAST_COLOR, 0, 0, 0);
			xfade_Start_Full_Fade(FADE_WIPE_SNAP_OFF, FADE_COLOR_PAL_TO_PAL, 1, 0, 1);
			xactor_Hide_Actor(actor);
		}
	}
}

// FUNCTION: XW 0x4620B0
void title_user_Stars(Actor* actor, int time) {
	if (shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL) {
		if (g_titleFilmTime == 0) {
			xactor_Hide_Actor(actor);
		} else {
			if (g_titleFilmTime == TITLE_STARS_FADE_FRAME) {
				xfade_Start_Full_Fade(FADE_WIPE_SNAP_ON, FADE_COLOR_TWO_PHASE, 1, 0, 1);
			}
			if (g_titleFilmTime == TITLE_STARS_SHOW_FRAME) {
				xactor_Show_Actor(actor);
			}
		}
	} else if (time == 0) {
		title_StartMusic();
	}
}

// FUNCTION: XW 0x462120
void title_user_Title(Actor* unusedActor, int unusedTime) {
	int lineIndex;
	int16_t linesLeft = g_titleLineCount;

	(void)unusedActor;
	(void)unusedTime;
	for (lineIndex = 0; linesLeft > 0; ++lineIndex, --linesLeft) {
		if (g_titleLineActive[lineIndex] != 0) {
			int stepsLeft;
			for (stepsLeft = TITLE_CRAWL_STEPS_PER_UPDATE; stepsLeft != 0; --stepsLeft) {
				int16_t velocityFraction = g_titleLineYVelocityFraction[lineIndex];
				g_titleLineYFraction[lineIndex] += velocityFraction;
				if (g_titleLineYFraction[lineIndex] >= TITLE_CRAWL_FRACTION_SCALE) {
					g_titleLineYFraction[lineIndex] -= TITLE_CRAWL_FRACTION_SCALE;
					--g_titleLineY[lineIndex];
				}
				g_titleLineY[lineIndex] -= g_titleLineYVelocity[lineIndex];
				if (g_titleLineY[lineIndex] < TITLE_CRAWL_SCREEN_HEIGHT) {
					g_titleLineYVelocityFraction[lineIndex] = velocityFraction - TITLE_CRAWL_VELOCITY_DECAY;
					if (g_titleLineYVelocityFraction[lineIndex] < 0) {
						g_titleLineYVelocityFraction[lineIndex] += TITLE_CRAWL_FRACTION_SCALE;
						if (g_titleLineYVelocity[lineIndex] > 0) {
							--g_titleLineYVelocity[lineIndex];
						} else {
							g_titleLineActive[lineIndex] = 0;
						}
					}
				}
			}
		}
	}
}

// FUNCTION: XW 0x462210
void title_draw_Title(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
					  int16_t unusedY, int16_t refresh) {
	int16_t lineIndex;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh != 0) {
		const uint8_t* bitmap = xmemhdl_Lock_Handle(g_titleTextBitmapHandle);
		for (lineIndex = 0; lineIndex < g_titleLineCount; ++lineIndex) {
			if (g_titleLineActive[lineIndex] != 0) {
				int16_t destinationY = g_titleLineY[lineIndex];
				int16_t sourceY = g_titleLineBitmapY[lineIndex];
				int16_t destinationWidth = 2 * destinationY - 2 * TITLE_CRAWL_HORIZON_Y;
				int lineScaleThreshold = destinationWidth;
				int rowIndex;
				int rowsLeft;
				if (g_titleLineYFraction[lineIndex] >= TITLE_CRAWL_FRACTION_SCALE / 2)
					--lineScaleThreshold;
				for (rowIndex = 0, rowsLeft = TITLE_CRAWL_BITMAP_LINE_HEIGHT; rowsLeft != 0;
					 ++rowIndex, --rowsLeft) {
					if (g_titleRowScaleThreshold[rowIndex] <= (int16_t)lineScaleThreshold) {
						int16_t destinationX = TITLE_CRAWL_SCREEN_WIDTH / 2 - (destinationWidth >> 1);
						int color = ((destinationY - TITLE_CRAWL_HORIZON_Y) >> 1) - g_titleFadeColorOffset +
									TITLE_CRAWL_MIN_COLOR;
						if ((int16_t)color < TITLE_CRAWL_MIN_COLOR)
							color = TITLE_CRAWL_MIN_COLOR;
						if (destinationY < TITLE_CRAWL_SCREEN_HEIGHT && destinationWidth != 0) {
							slant_Scale_Line(bitmap, 0, sourceY, g_titleScaleSkip[destinationWidth],
											 g_titleScaleSkipFraction[destinationWidth], destinationX,
											 destinationY, destinationWidth, color);
						}
						++destinationY;
						destinationWidth += 2;
					}
					++sourceY;
				}
			}
		}
		xmemhdl_Unlock_Handle(g_titleTextBitmapHandle);
	}
}

// FUNCTION: XW 0x462340
void title_user_Back(Actor* unusedActor, int time) {
	int startOffsetY = TITLE_CRAWL_NORMAL_START_Y;

	(void)unusedActor;
	if (shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL) {
		if (xio_Is_System_Slower_Than(LANDRU_SYSTEM_SPEED_DEFAULT) != 0) {
			startOffsetY = TITLE_CRAWL_SLOW_START_Y;
		}
	} else {
		startOffsetY = TITLE_CRAWL_OTHER_START_Y;
	}
	if (time == 0) {
		int lineIndex = 0;
		int lineY = startOffsetY + TITLE_CRAWL_SCREEN_HEIGHT;
		int linesLeft;
		int fillIndex;

		memset(g_titleLineDrawn, 0, sizeof(g_titleLineDrawn));
		for (fillIndex = 0; fillIndex < TITLE_CRAWL_LINE_COUNT; ++fillIndex) {
			g_titleLineActive[fillIndex] = 1;
		}
		g_titleLineCount = TITLE_CRAWL_LINE_COUNT;
		memset(g_titleLineYVelocityFraction, 0, sizeof(g_titleLineYVelocityFraction));
		for (fillIndex = 0; fillIndex < TITLE_CRAWL_LINE_COUNT; ++fillIndex) {
			g_titleLineYVelocity[fillIndex] = 1;
		}
		memset(g_titleLineYFraction, 0, sizeof(g_titleLineYFraction));
		for (linesLeft = TITLE_CRAWL_LINE_COUNT; linesLeft != 0; ++lineIndex, --linesLeft) {
			g_titleLineY[lineIndex] = lineY;
			lineY += TITLE_CRAWL_LINE_SPACING;
			g_titleLineBitmapY[lineIndex] =
				TITLE_CRAWL_BITMAP_LINE_HEIGHT * (lineIndex % TITLE_CRAWL_BITMAP_LINE_COUNT);
		}
	}
}

// FUNCTION: XW 0x462410
int16_t title_draw_Back(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
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
				xfont_Print_Centered_Text(lineText, &textRowRect, TITLE_CRAWL_FONT, TITLE_CRAWL_TEXT_COLOR);
				g_titleLineDrawn[lineIndex] = 1;
			}
		}
		xcanvas_Pop_Canvas(previousCanvasPixels, &previousCanvasClip, previousCanvasWidth,
						   previousCanvasHeight);
		xmemhdl_Unlock_Handle(g_titleTextBitmapHandle);
	}
	return 1;
}

// FUNCTION: XW 0x462690
void title_OpenMusic(ResFile* unusedResourceFile, int16_t* sceneTime) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* logoMusic;
		ResFile* titleMusicResource;
		g_titleMusicState.sceneTime = sceneTime;
		logoMusic = xsound_Find_Gmid("logo");
		if (logoMusic != NULL)
			soundext_FadeVolume(logoMusic, 0, TITLE_LOGO_MUSIC_FADE_DURATION);
		titleMusicResource = xres_Open_Resource("tlmusic.lfd");
		g_titleMusicState.sound = xsound_Res_Music(titleMusicResource, "starwars");
		xres_Close_Resource(titleMusicResource);
		xsound_Set_Sound_User_Function(g_titleMusicState.sound, title_user_Music);
		if (shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL) {
			ResFile* attackMusicResource = xres_Open_Resource("inmusic.lfd");
			xsound_Res_Music(attackMusicResource, "inattack");
			xres_Close_Resource(attackMusicResource);
		}
	}
}

// FUNCTION: XW 0x462740
void title_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && shellext_Get_Cur_Scene() == XW_SCENE_INTRO_TITLE_CRAWL &&
		(shellext_Is_Sudden_Scene_End() != 0 || xerror_Is_Landru_Exit() != 0)) {
		ResFile* musicResource = xres_Open_Resource("tlmusic.lfd");
		Sound* kludgeMusic = xsound_Res_Music(musicResource, "kludge");
		xres_Close_Resource(musicResource);
		soundext_Start_Resource_Sound(kludgeMusic);
#ifdef XW_MODERN
		XwTitle_WaitForMusic(kludgeMusic);
#else
		while (soundext_Count_Resource_Instances(kludgeMusic) == 1) {
		}
#endif
	}
}

// FUNCTION: XW 0x4627C0
void title_user_Music(Sound* sound, int time) {
	if (shellext_Get_Cur_Scene() == XW_SCENE_TOUR_TITLE_CRAWL) {
		if (time == TITLE_MUSIC_TOUR_FADE_TIME)
			soundext_FadeVolume(sound, 0, TITLE_MUSIC_END_FADE_DURATION);
	} else {
		Sound* attackMusic;
		int16_t beat;
		if (time == TITLE_MUSIC_INTRO_FADE_TIME)
			soundext_FadeVolume(sound, 0, TITLE_MUSIC_END_FADE_DURATION);
		attackMusic = xsound_Find_Gmid("inattack");
		beat = soundext_GetMusicParam(sound, XW_SOUND_QUERY_BEAT, 0);
		if (beat != XW_SOUND_QUERY_UNSUPPORTED && beat > TITLE_MUSIC_ATTACK_BEAT_THRESHOLD &&
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

// FUNCTION: XW 0x4628B0
void title_StartMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		soundext_Start_Resource_Sound(g_titleMusicState.sound);
		if (shellext_Get_Cur_Scene() == XW_SCENE_TOUR_TITLE_CRAWL) {
/* Resource pointers are outside the numeric flight-sound ID range. */
#ifdef XW_MODERN
			Dos94_soundext_SetVolume(g_titleMusicState.sound, 0);
#else
			soundext_SetVolume(0, 0);
#endif
			soundext_FadeVolume(g_titleMusicState.sound, TITLE_MUSIC_FADE_VOLUME, TITLE_MUSIC_FADE_DURATION);
		}
	}
}
