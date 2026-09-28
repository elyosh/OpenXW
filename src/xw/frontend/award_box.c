#include "xw/frontend/award_box.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/tooltip.h"
#include "xw/landru_config.h"
#include "xw/render/shade.h"

#include "xw_runtime/integration/awards_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/award_box_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actrect.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D08F0
XwUniformHotspot g_awardBoxHotspots[AWARD_BOX_HOTSPOT_COUNT] = { { 160, 130 }, { 120, 230 }, { 200, 230 },
																 { 120, 305 }, { 200, 305 }, { 120, 374 },
																 { 200, 374 }, { 270, 132 }, { 330, 132 },
																 { 390, 132 }, { 450, 132 }, { 510, 132 },
																 { 416, 317 }, { 308, 317 }, { 524, 317 } };

// GLOBAL: XW 0x4D0930
char g_awardBoxLabels[AWARD_BOX_LABEL_COUNT][AWARD_BOX_LABEL_SIZE] = {
	"B-Wing Flight Badge", "B-Wing Battle Patch", "Tour ", "Ribbon(s)", "The Shield of Yavin",
	"The Talons of Hoth",  "The Unused Medal",    "Four ", "Five ",     "Six "
};

// GLOBAL: XW 0x4D1590
const char* g_awardBoxMusicFilename = "unmusic.lfd";

// GLOBAL: XW 0x4D1594
const char* g_awardBoxMusicName = "uniform";

// GLOBAL: XW 0x4F4C80
int g_awardBoxKeyboardAward = 0;

// GLOBAL: XW 0x4F4C84
Input* g_awardBoxAwardsInput = NULL;

// GLOBAL: XW 0x4F4C88
int16_t g_awardBoxActorStates[AWARD_BOX_ACTOR_STATE_COUNT] = { 0 };

// GLOBAL: XW 0x4F4C98
PushButton* g_awardBoxExitButton = NULL;

// GLOBAL: XW 0x4F4C9C
int16_t g_awardBoxPreviousHoveredAward = 0;

// GLOBAL: XW 0x4F4CA0
int16_t g_awardBoxHoveredAward = 0;

// GLOBAL: XW 0x4F4CA4
Film* g_awardBoxFilm = NULL;

// GLOBAL: XW 0x4F4CA8
int16_t g_awardBoxAwardAvailable[AWARD_BOX_HOTSPOT_COUNT] = { 0 };

// GLOBAL: XW 0x4F4CC8
int16_t g_awardBoxExitAction = 0;

// GLOBAL: XW 0x4F4CCC
Input* g_awardBoxRootInput = NULL;

// GLOBAL: XW 0x4F4CD0
REGISTER_PilotFileRecord g_awardBoxPilot = { 0 };

// GLOBAL: XW 0x4F537C
PushButton* g_awardBoxUniformButton = NULL;

// GLOBAL: XW 0x4F6248
XwSceneMusicHandles g_awardBoxMusicState = { NULL, NULL };

// FUNCTION: XW 0x4373B0
XwShellSceneResult AwardBox_AwardBox(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Actor* backgroundActor;
	Rect frame;
	char pilotFilename[AWARD_BOX_PILOT_FILENAME_CAPACITY];
	strcpy(pilotFilename, g_RegisterShellPilot.name);
	strcat(pilotFilename, ".PLT");
	g_awardBoxPreviousHoveredAward = AWARD_BOX_NO_HOVER;
	g_awardBoxHoveredAward = AWARD_BOX_NO_HOVER;
	g_awardBoxExitAction = 0;
	memset(&g_awardBoxPilot, 0, sizeof(g_awardBoxPilot));
	AwardBox_ReadPilot(pilotFilename);
	resourceFile = xres_Open_Resource("box640.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	g_awardBoxFilm =
		xfilm_Res_Callback_Film(resourceFile, "box_640", &frame, 0, 0, 0, AwardBox_film_Callback);
	xfilm_Set_Film_Def_Palette(g_awardBoxFilm, shell->standardPalette);
	backgroundActor = xactrect_Alloc_Blank_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, AWARD_BOX_BACKGROUND_Z);
	xactor_Non_Refreshable_Actor(backgroundActor);
	backgroundActor->w = AWARD_BOX_WIDTH;
	backgroundActor->h = AWARD_BOX_HEIGHT;
	xrect_Set_Rect(&frame, 0, 0, AWARD_BOX_WIDTH, AWARD_BOX_HEIGHT);
	g_awardBoxRootInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xrect_Set_Rect(&frame, 0, 0, AWARD_BOX_WIDTH, AWARD_BOX_AWARDS_BOTTOM);
	g_awardBoxAwardsInput = xinput_Alloc_Input(g_awardBoxRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_awardBoxAwardsInput, AwardBox_iupdate_Awards);
	xinpattr_Set_Input_User_Function(g_awardBoxAwardsInput, AwardBox_iuser_Awards);
	xinpattr_Set_Input_Draw_Function(g_awardBoxAwardsInput, AwardBox_idraw_Awards);
	g_awardBoxAwardsInput->mouseUsage = allInput;
	xrect_Set_Rect(&frame, AWARD_BOX_UNIFORM_LEFT, AWARD_BOX_BUTTON_TOP, AWARD_BOX_UNIFORM_RIGHT,
				   AWARD_BOX_HEIGHT);
	g_awardBoxUniformButton = xbtnpush_Alloc_Button(g_awardBoxRootInput, &frame, 0, AwardBox_iuser_Button,
													"View Uniform", AWARD_BOX_BUTTON_UNIFORM);
	xinpattr_Set_Input_Update_Function(&g_awardBoxUniformButton->header, XwAwardBox_UpdateButton);
	xinpattr_Set_Input_Draw_Function(&g_awardBoxUniformButton->header, XwAwardsUI_DrawTextButton);
	g_awardBoxUniformButton->header.mouseUsage = allInput;
	xrect_Set_Rect(&frame, AWARD_BOX_EXIT_LEFT, AWARD_BOX_BUTTON_TOP, AWARD_BOX_EXIT_RIGHT, AWARD_BOX_HEIGHT);
	g_awardBoxExitButton = xbtnpush_Alloc_Button(g_awardBoxRootInput, &frame, 0, AwardBox_iuser_Button,
												 "Exit", AWARD_BOX_BUTTON_EXIT);
	xinpattr_Set_Input_Update_Function(&g_awardBoxExitButton->header, XwAwardBox_UpdateButton);
	xinpattr_Set_Input_Draw_Function(&g_awardBoxExitButton->header, XwAwardsUI_DrawTextButton);
	g_awardBoxExitButton->header.mouseUsage = allInput;
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(AwardBox_end_View);
	xio_Set_Key_Buttons();
	AwardBox_OpenMusic(resourceFile, g_awardBoxFilm);
	soundext_RecheckSfxPreference();
#ifdef XW_MODERN
	XwAwardBox_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	AwardBox_CloseMusic();
	soundext_RecheckSfxPreference();
	xcursor_Hide_Cursor();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xres_Close_Resource(resourceFile);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4376D0
void AwardBox_end_View(int time) {
	int16_t key;
	if (time == 0) {
		if ((int16_t)xcursor_Is_Cursor_Visible() == 0)
			xcursor_Show_Cursor();
		Tooltip_BuildScreenPaletteRemap();
	}
	key = xio_Get_Free_Key();
	if (key != 0) {
		int16_t candidateAward = g_awardBoxKeyboardAward;
		if (key == AWARD_BOX_KEY_LEFT || key == AWARD_BOX_KEY_UP) {
			candidateAward = (int16_t)g_awardBoxKeyboardAward - 1;
			if (candidateAward < 0)
				candidateAward += AWARD_BOX_HOTSPOT_COUNT;
			if (candidateAward == AWARD_BOX_KEYBOARD_SKIP_FIRST ||
				candidateAward == AWARD_BOX_KEYBOARD_SKIP_SECOND ||
				candidateAward == AWARD_BOX_KEYBOARD_SKIP_THIRD ||
				candidateAward == AWARD_BOX_KEYBOARD_SKIP_LAST)
				--candidateAward;
			for (; candidateAward != (int16_t)g_awardBoxKeyboardAward;) {
				if (g_awardBoxAwardAvailable[candidateAward] != 0)
					break;
				--candidateAward;
				if (candidateAward < 0)
					candidateAward += AWARD_BOX_HOTSPOT_COUNT;
				if (candidateAward == AWARD_BOX_KEYBOARD_SKIP_FIRST ||
					candidateAward == AWARD_BOX_KEYBOARD_SKIP_SECOND ||
					candidateAward == AWARD_BOX_KEYBOARD_SKIP_THIRD ||
					candidateAward == AWARD_BOX_KEYBOARD_SKIP_LAST)
					--candidateAward;
			}
			key = xio_Get_Key();
		}
		if (key == AWARD_BOX_KEY_RIGHT || key == AWARD_BOX_KEY_DOWN) {
			candidateAward = (int16_t)g_awardBoxKeyboardAward + 1;
			if (candidateAward >= AWARD_BOX_HOTSPOT_COUNT)
				candidateAward -= AWARD_BOX_HOTSPOT_COUNT;
			if (candidateAward == AWARD_BOX_KEYBOARD_SKIP_FIRST ||
				candidateAward == AWARD_BOX_KEYBOARD_SKIP_SECOND ||
				candidateAward == AWARD_BOX_KEYBOARD_SKIP_THIRD ||
				candidateAward == AWARD_BOX_KEYBOARD_SKIP_LAST)
				++candidateAward;
			for (; candidateAward != (int16_t)g_awardBoxKeyboardAward;) {
				if (g_awardBoxAwardAvailable[candidateAward] != 0)
					break;
				++candidateAward;
				if (candidateAward >= AWARD_BOX_HOTSPOT_COUNT)
					candidateAward -= AWARD_BOX_HOTSPOT_COUNT;
				if (candidateAward == AWARD_BOX_KEYBOARD_SKIP_FIRST ||
					candidateAward == AWARD_BOX_KEYBOARD_SKIP_SECOND ||
					candidateAward == AWARD_BOX_KEYBOARD_SKIP_THIRD ||
					candidateAward == AWARD_BOX_KEYBOARD_SKIP_LAST)
					++candidateAward;
			}
			xio_Get_Key();
		}
		if (candidateAward != (int16_t)g_awardBoxKeyboardAward) {
			xio_Set_Mouse_Position(g_awardBoxHotspots[candidateAward].x,
								   g_awardBoxHotspots[candidateAward].y);
			/* Preserve the untouched upper half of the selection global. */
			g_awardBoxKeyboardAward =
				(int)(((uint32_t)g_awardBoxKeyboardAward & ~(uint32_t)UINT16_MAX) | (uint16_t)candidateAward);
		}
	}
	if (g_awardBoxExitAction == AWARD_BOX_EXIT) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_AWARD_CASE_FROM_COMBAT_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_BRIEFING_COMBAT);
				break;
			case XW_SCENE_AWARD_CASE_FROM_TOUR_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_BRIEFING_TOUR);
				break;
			case XW_SCENE_AWARD_CASE_FROM_REGISTER:
				xerror_Set_Landru_Exit(XW_SCENE_REGISTER_RETURN);
				break;
		}
	} else if (g_awardBoxExitAction == AWARD_BOX_VIEW_UNIFORM) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_AWARD_CASE_FROM_COMBAT_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_COMBAT_BRIEFING);
				break;
			case XW_SCENE_AWARD_CASE_FROM_TOUR_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_TOUR_BRIEFING);
				break;
			case XW_SCENE_AWARD_CASE_FROM_REGISTER:
				xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_REGISTER);
				break;
		}
	}
}

// FUNCTION: XW 0x4378B0
int16_t AwardBox_film_Callback(Film* film, FilmObject* filmObject) {
	if (filmObject->id == FTC_ACTOR) {
		Actor* actor;
		int role;
		xfilm_Rewind_Actor_Film(film, filmObject, filmObject + 1);
		actor = filmObject->object;
		xactor_Non_Refreshable_Actor(actor);
		if (actor->var1 != 0) {
			xactor_Set_Actor_User_Function(actor, AwardBox_user_AwardState);
			g_awardBoxActorStates[actor->var1 - 1] = AWARD_BOX_KEEP_ACTOR_STATE;
		}
		role = actor->var1;
		switch (role) {
			case AWARD_BOX_ROLE_BWING_BADGE:
				if (g_awardBoxPilot.trainingLevelProgress[AWARD_BOX_BWING_TRAINING_INDEX] >=
					AWARD_BOX_BADGE_TRAINING_LEVEL) {
					g_awardBoxAwardAvailable[role - 1] = 1;
				} else {
					g_awardBoxAwardAvailable[role - 1] = 0;
					xactor_Set_Actor_Time(actor, 0, 0);
				}
				break;
			case AWARD_BOX_ROLE_BWING_PATCHES: {
				int16_t patchIndex;
				xactor_Set_Actor_Draw_Function(actor, AwardBox_draw_BWingPatches);
				for (patchIndex = 0;
					 patchIndex <
					 (int)sizeof(g_awardBoxPilot.combatAwards[AWARD_BOX_BWING_COMBAT_AWARDS].missionPatch);
					 ++patchIndex) {
					g_awardBoxAwardAvailable[patchIndex + AWARD_BOX_FIRST_PATCH_HOTSPOT] =
						g_awardBoxPilot.combatAwards[AWARD_BOX_BWING_COMBAT_AWARDS].missionPatch[patchIndex];
				}
				break;
			}
			case AWARD_BOX_ROLE_TOUR_FOUR:
			case AWARD_BOX_ROLE_TOUR_FIVE:
			case AWARD_BOX_ROLE_TOUR_SIX: {
				int16_t tourProgress = g_awardBoxPilot.tourOperationProgress[role];
				if (tourProgress != 0 && tourProgress != AWARD_BOX_TOUR_PROGRESS_UNAVAILABLE) {
					if (tourProgress > AWARD_BOX_TOUR_INITIAL_PROGRESS_LIMIT) {
						tourProgress = AWARD_BOX_TOUR_INITIAL_PROGRESS_LIMIT;
					}
					g_awardBoxActorStates[role - 1] =
						AWARD_BOX_TOUR_RIBBON_COUNT * tourProgress / AWARD_BOX_TOUR_PROGRESS_DIVISOR +
						AWARD_BOX_TOUR_FIRST_STATE;
					g_awardBoxActorStates[actor->var1 - 1] +=
						AWARD_BOX_TOUR_RIBBON_COUNT * (actor->var1 - AWARD_BOX_ROLE_TOUR_FOUR);
				} else {
					xactor_Set_Actor_Time(actor, 0, 0);
				}
				xactor_Set_Actor_Draw_Function(actor, AwardBox_draw_TourRibbons);
				g_awardBoxAwardAvailable[actor->var1 - AWARD_BOX_ROLE_TOUR_FOUR +
										 AWARD_BOX_FIRST_TOUR_HOTSPOT] = 1;
				g_awardBoxAwardAvailable[actor->var1 - AWARD_BOX_ROLE_TOUR_FOUR +
										 AWARD_BOX_SECOND_TOUR_HOTSPOT] = 1;
				break;
			}
			case AWARD_BOX_ROLE_SHIELD_OF_YAVIN:
			case AWARD_BOX_ROLE_TALONS_OF_HOTH:
			case AWARD_BOX_ROLE_UNUSED_MEDAL:
				if (g_awardBoxPilot.expansionMedals[role - AWARD_BOX_ROLE_SHIELD_OF_YAVIN] > 0u) {
					g_awardBoxAwardAvailable[role - AWARD_BOX_ROLE_SHIELD_OF_YAVIN +
											 AWARD_BOX_FIRST_MEDAL_HOTSPOT] = 1;
				} else {
					g_awardBoxAwardAvailable[role - AWARD_BOX_ROLE_SHIELD_OF_YAVIN +
											 AWARD_BOX_FIRST_MEDAL_HOTSPOT] = 0;
					xactor_Set_Actor_Time(actor, 0, 0);
				}
				break;
		}
	}
	return 0;
}

// FUNCTION: XW 0x437A80
int16_t AwardBox_iupdate_Awards(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)clip;
	if (key != 0) {
		return 0;
	}
	if (leftEvent == 0 && rightEvent == 0) {
		int16_t bestDistance = AWARD_BOX_INITIAL_HOVER_DISTANCE;
		int16_t nearestAward = AWARD_BOX_NO_HOVER;
		int16_t awardIndex;
		for (awardIndex = 0; awardIndex < AWARD_BOX_HOTSPOT_COUNT; ++awardIndex) {
			if (g_awardBoxAwardAvailable[awardIndex] != 0) {
				int16_t distanceX = abs(x + frame->left - g_awardBoxHotspots[awardIndex].x);
				int16_t distanceY = abs(frame->top + y - g_awardBoxHotspots[awardIndex].y);
				int16_t distance;
				if (distanceX < distanceY) {
					distanceX >>= 1;
					distance = distanceX + distanceY;
				} else {
					distanceY >>= 1;
					distance = distanceY + distanceX;
				}
				if (distance < bestDistance) {
					bestDistance = distance;
					nearestAward = awardIndex;
				}
			}
		}
		if (bestDistance < AWARD_BOX_HOVER_DISTANCE) {
			g_awardBoxHoveredAward = nearestAward;
		} else {
			g_awardBoxHoveredAward = AWARD_BOX_NO_HOVER;
		}
	}
	return 1;
}

// FUNCTION: XW 0x437B60
void AwardBox_iuser_Awards(Input* input, int context) {
	(void)input;
	(void)context;
	if (g_awardBoxPreviousHoveredAward != g_awardBoxHoveredAward ||
		g_awardBoxHoveredAward != AWARD_BOX_NO_HOVER) {
		g_awardBoxPreviousHoveredAward = g_awardBoxHoveredAward;
		xview_Refresh_View();
	}
}

// FUNCTION: XW 0x437B90
void AwardBox_idraw_Awards(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect textRect;
	Rect tooltipRect;
	char ribbonCountText[AWARD_BOX_COUNT_TEXT_SIZE];
	char lines[AWARD_BOX_TEXT_LINES][AWARD_BOX_TEXT_LINE_SIZE];
	int16_t hoveredAward;
	int16_t mouseX, mouseY;
	int16_t lineCount;
	int16_t maxWidth;
	int lineIndex;
	int linesRemaining;
	(void)input;
	(void)frame;
	(void)clip;
	if (refresh == 0)
		return;
	hoveredAward = g_awardBoxHoveredAward;
	if (hoveredAward == AWARD_BOX_NO_HOVER)
		return;
	for (lineIndex = 0; lineIndex < AWARD_BOX_TEXT_LINES; ++lineIndex)
		lines[lineIndex][0] = 0;
	if (hoveredAward == 0)
		strcpy(lines[0], g_awardBoxLabels[AWARD_BOX_LABEL_BADGE]);
	if (hoveredAward >= AWARD_BOX_FIRST_PATCH_HOTSPOT && hoveredAward < AWARD_BOX_FIRST_TOUR_HOTSPOT) {
		strcpy(lines[0], g_awardBoxLabels[AWARD_BOX_LABEL_PATCH]);
		sprintf(lines[1], "Mission %d", hoveredAward);
		hoveredAward = g_awardBoxHoveredAward;
	}
	if (hoveredAward >= AWARD_BOX_FIRST_TOUR_HOTSPOT && hoveredAward < AWARD_BOX_FIRST_MEDAL_HOTSPOT) {
		int16_t ribbonLineCount = 0;
		int tourIndex;
		int toursRemaining;
		for (tourIndex = 0, toursRemaining = AWARD_BOX_TEXT_LINES; toursRemaining != 0;
			 ++tourIndex, --toursRemaining) {
			int16_t tourProgress =
				g_awardBoxPilot.tourOperationProgress[AWARD_BOX_FIRST_EXPANSION_TOUR + tourIndex];
			if (tourProgress > 1 && tourProgress != AWARD_BOX_TOUR_PROGRESS_UNAVAILABLE) {
				sprintf(
					ribbonCountText, "%d ",
					(int16_t)(AWARD_BOX_TOUR_RIBBON_COUNT * tourProgress / AWARD_BOX_TOUR_PROGRESS_DIVISOR));
				strcpy(lines[ribbonLineCount], ribbonCountText);
				strcat(lines[ribbonLineCount], g_awardBoxLabels[AWARD_BOX_LABEL_TOUR]);
				strcat(lines[ribbonLineCount], g_awardBoxLabels[AWARD_BOX_LABEL_FIRST_TOUR + tourIndex]);
				strcat(lines[ribbonLineCount], g_awardBoxLabels[AWARD_BOX_LABEL_RIBBONS]);
				hoveredAward = g_awardBoxHoveredAward;
				++ribbonLineCount;
			}
		}
	}
	if (hoveredAward >= AWARD_BOX_FIRST_MEDAL_HOTSPOT && hoveredAward < AWARD_BOX_HOTSPOT_COUNT) {
		strcpy(lines[0],
			   g_awardBoxLabels[hoveredAward - AWARD_BOX_FIRST_MEDAL_HOTSPOT + AWARD_BOX_LABEL_FIRST_MEDAL]);
	}
	mouseX = xio_Mouse_X();
	mouseY = xio_Mouse_Y();
	lineCount = 0;
	if (strlen(lines[0]) != 0) {
		lineCount = 1;
		if (strlen(lines[1]) != 0) {
			lineCount = 2;
			if (strlen(lines[2]) != 0)
				lineCount = 3;
		}
	}
	if (lineCount != 0) {
		maxWidth = 0;
		for (lineIndex = 0, linesRemaining = lineCount; linesRemaining != 0; ++lineIndex, --linesRemaining) {
			int16_t lineWidth = xfont_Get_String_Width_0(AWARD_BOX_TOOLTIP_FONT, lines[lineIndex]);
			if (lineWidth > maxWidth)
				maxWidth = lineWidth;
		}
		if (mouseX >= AWARD_BOX_TOOLTIP_MIDPOINT) {
			xrect_Set_Rect(&textRect, mouseX - maxWidth - AWARD_BOX_TOOLTIP_LEFT_OFFSET,
						   mouseY - AWARD_BOX_TEXT_HALF_HEIGHT * lineCount,
						   mouseX - AWARD_BOX_TOOLTIP_LEFT_OFFSET,
						   mouseY + AWARD_BOX_TEXT_HEIGHT - AWARD_BOX_TEXT_HALF_HEIGHT * lineCount);
		} else {
			xrect_Set_Rect(&textRect, mouseX + AWARD_BOX_TOOLTIP_RIGHT_OFFSET,
						   mouseY - AWARD_BOX_TEXT_HALF_HEIGHT * lineCount,
						   maxWidth + mouseX + AWARD_BOX_TOOLTIP_RIGHT_OFFSET,
						   mouseY + AWARD_BOX_TEXT_HEIGHT - AWARD_BOX_TEXT_HALF_HEIGHT * lineCount);
		}
		if (textRect.left < AWARD_BOX_TOOLTIP_MARGIN)
			xrect_Offset_Rect(&textRect, AWARD_BOX_TOOLTIP_MARGIN - textRect.left, 0);
		if (textRect.right > AWARD_BOX_TOOLTIP_RIGHT)
			xrect_Offset_Rect(&textRect, AWARD_BOX_TOOLTIP_RIGHT - textRect.right, 0);
		if (textRect.top < AWARD_BOX_TOOLTIP_MARGIN)
			xrect_Offset_Rect(&textRect, 0, AWARD_BOX_TOOLTIP_MARGIN - textRect.top);
		if (textRect.bottom > AWARD_BOX_TOOLTIP_BOTTOM)
			xrect_Offset_Rect(&textRect, 0, AWARD_BOX_TOOLTIP_BOTTOM - textRect.bottom);
		xrect_Copy_Rect(&tooltipRect, &textRect);
		xrect_Inset_Rect(&tooltipRect, -AWARD_BOX_TOOLTIP_MARGIN, -AWARD_BOX_TOOLTIP_INSET_Y);
		tooltipRect.bottom += AWARD_BOX_TEXT_SPACING * lineCount - AWARD_BOX_TEXT_SPACING;
		xpaint_Frame_Clipped_Rect(&tooltipRect, AWARD_BOX_TOOLTIP_FRAME_COLOR);
		xrect_Inset_Rect(&tooltipRect, 1, 1);
		shade_RemapClippedRect(&tooltipRect);
		xfont_Enable_FontID_Shadow(AWARD_BOX_TOOLTIP_FONT);
		for (lineIndex = 0, linesRemaining = lineCount; linesRemaining != 0; ++lineIndex, --linesRemaining) {
			xfont_Print_Centered_Text(lines[lineIndex], &textRect, AWARD_BOX_TOOLTIP_FONT,
									  AWARD_BOX_TOOLTIP_TEXT_COLOR);
			xrect_Offset_Rect(&textRect, 0, AWARD_BOX_TEXT_SPACING);
		}
		xfont_Disable_FontID_Shadow(AWARD_BOX_TOOLTIP_FONT);
	}
}

// FUNCTION: XW 0x437FF0
int16_t AwardBox_iupdate_Button(PushButton* button, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								int rightEvent, int16_t x, int16_t y) {
	int16_t result;
	result = 0;
	if (leftEvent != 0 || rightEvent != 0) {
		result = xbtnpush_iupdate_Button(&button->header, frame, clip, key, leftEvent, rightEvent, x, y);
	}
	g_awardBoxHoveredAward = AWARD_BOX_NO_HOVER;
	return result;
}

// FUNCTION: XW 0x438040
void AwardBox_iuser_Button(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input)) {
		g_awardBoxExitAction =
			input->id == AWARD_BOX_BUTTON_UNIFORM ? AWARD_BOX_VIEW_UNIFORM : AWARD_BOX_EXIT;
	}
}

// FUNCTION: XW 0x438070
void AwardBox_user_AwardState(Actor* actor, int time) {
	if (time == 0) {
		int16_t state = g_awardBoxActorStates[actor->var1 - 1];
		if (state != AWARD_BOX_KEEP_ACTOR_STATE) {
			xactor_Set_Actor_State(actor, state, 0);
		}
	}
}

// FUNCTION: XW 0x4380A0
int16_t AwardBox_draw_BWingPatches(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh) {
	if (refresh != 0) {
		int16_t patchIndex;
		for (patchIndex = 0;
			 patchIndex <
			 (int16_t)sizeof(g_awardBoxPilot.combatAwards[AWARD_BOX_BWING_COMBAT_AWARDS].missionPatch);
			 ++patchIndex) {
			if (g_awardBoxPilot.combatAwards[AWARD_BOX_BWING_COMBAT_AWARDS].missionPatch[patchIndex] != 0) {
				xactor_Set_Actor_State(actor, patchIndex + 1, 0);
				xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x438110
int16_t AwardBox_draw_TourRibbons(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								  int16_t refresh) {
	if (refresh != 0) {
		int16_t baseState = AWARD_BOX_TOUR_RIBBON_COUNT * actor->var1 - AWARD_BOX_TOUR_RIBBON_STATE_OFFSET;
		int16_t ribbonCount = AWARD_BOX_TOUR_RIBBON_COUNT *
							  g_awardBoxPilot.tourOperationProgress[actor->var1] /
							  AWARD_BOX_TOUR_PROGRESS_DIVISOR;
		int16_t ribbonIndex;
		for (ribbonIndex = 0; ribbonIndex < AWARD_BOX_TOUR_RIBBON_COUNT; ++ribbonIndex) {
			if (ribbonCount > ribbonIndex) {
				xactor_Set_Actor_State(actor, ribbonIndex + baseState, 0);
				xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x4381A0
int16_t AwardBox_ReadPilot(const char* filename) {
	LandruFile* pilotFile;
	pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, filename, "rb");
	if (pilotFile != NULL) {
		register_ReadPilotRecord(pilotFile, &g_awardBoxPilot);
		xfile_Close_File(pilotFile);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x43E390
void AwardBox_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_awardBoxMusicState.film = film;
		g_awardBoxMusicState.sound = xsound_Find_Gmid(g_awardBoxMusicName);
		if (g_awardBoxMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\unmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource(g_awardBoxMusicFilename);
			g_awardBoxMusicState.sound = xsound_Res_Music(musicResource, g_awardBoxMusicName);
			soundext_Start_Resource_Sound(g_awardBoxMusicState.sound);
			xres_Close_Resource(musicResource);
		} else {
			soundext_FadeVolume(g_awardBoxMusicState.sound, AWARD_BOX_MUSIC_RESUME_VOLUME,
								AWARD_BOX_MUSIC_RESUME_DURATION);
		}
		xsound_Set_Sound_Keep(g_awardBoxMusicState.sound);
		xsound_Set_Sound_User_Function(g_awardBoxMusicState.sound, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x43E450
void AwardBox_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid(g_awardBoxMusicName);
		g_awardBoxMusicState.sound = music;
		if (music != NULL) {
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(g_awardBoxMusicState.sound, 0);
#else
			soundext_SetPriority(0, 0);
#endif
			soundext_FadeVolume(g_awardBoxMusicState.sound, 0, AWARD_BOX_MUSIC_FADE_DURATION);
		}
	}
}
