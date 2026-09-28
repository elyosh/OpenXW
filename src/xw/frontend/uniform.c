#include "xw/frontend/uniform.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/tooltip.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/awards_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/uniform_task.h"
#endif
#include "xw/landru_config.h"
#include "xw/render/shade.h"

#include <landru/actanim.h>
#include <landru/actrect.h>
#include <landru/btnpush.h>
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

// GLOBAL: XW 0x4D8A20
XwUniformHotspot g_uniformHotspots[UNIFORM_HOTSPOT_COUNT] = {
	{ 346, 101 }, { 300, 130 }, { 274, 173 }, { 218, 200 }, { 278, 211 }, { 196, 223 },
	{ 258, 233 }, { 174, 247 }, { 236, 250 }, { 148, 276 }, { 208, 293 }, { 126, 300 },
	{ 186, 322 }, { 102, 324 }, { 164, 348 }, { 72, 362 },  { 130, 386 }, { 44, 391 },
	{ 106, 422 }, { 16, 422 },  { 76, 456 },  { 330, 276 }, { 390, 276 }, { 450, 276 },
	{ 510, 276 }, { 420, 384 }, { 356, 384 }, { 492, 384 }, { 490, 125 }, { 36, 240 },
	{ 38, 180 },  { 74, 170 },  { 36, 125 },  { 98, 146 },  { 38, 156 },  { 6, 170 }
};

// GLOBAL: XW 0x4D8AB0
char g_uniformAwardLabels[UNIFORM_AWARD_LABEL_COUNT][UNIFORM_AWARD_LABEL_SIZE] = { "A-Wing Flight Badge",
																				   "Y-Wing Flight Badge",
																				   "X-Wing Flight Badge",
																				   "A-Wing Battle Patch",
																				   "X-Wing Battle Patch",
																				   "Y-Wing Battle Patch",
																				   "Rank of Flight Cadet",
																				   "Rank of Flight Officer",
																				   "Rank of Lieutenant",
																				   "Rank of Captain",
																				   "Rank of Commander",
																				   "Rank of General",
																				   "Tour ",
																				   "Ribbon(s)",
																				   "The Corellian Cross",
																				   "The Mantooine Medallion",
																				   "The Star of Alderaan",
																				   "The Kalidor Crescent",
																				   "Bronze Cluster",
																				   "Silver Talons",
																				   "Silver Scimitar",
																				   "Golden Wings",
																				   "Diamond Eyes",
																				   "One ",
																				   "Two ",
																				   "Three " };

// GLOBAL: XW 0x4D8D88
const char* g_uniformMusicResourceName = "unmusic.lfd";

// GLOBAL: XW 0x4D8D8C
const char* g_uniformMusicName = "uniform";

// GLOBAL: XW 0x4FBC78
PushButton* g_uniformMedalsButton = NULL;

// GLOBAL: XW 0x4FBC7C
Film* g_uniformFilm = NULL;

// GLOBAL: XW 0x4FBC80
int16_t g_uniformAwardAvailable[UNIFORM_HOTSPOT_COUNT] = { 0 };

// GLOBAL: XW 0x4FBCC8
int16_t g_uniformHoveredAward = 0;

// GLOBAL: XW 0x4FBCCC
Input* g_uniformRootInput = NULL;

// GLOBAL: XW 0x4FBCD0
Input* g_uniformAwardsInput = NULL;

// GLOBAL: XW 0x4FBCD8
REGISTER_PilotFileRecord g_uniformPilot = { 0 };

// GLOBAL: XW 0x4FC384
int16_t g_uniformPreviousHoveredAward = 0;

// GLOBAL: XW 0x4FC386
int16_t g_uniformActorStates[UNIFORM_ACTOR_STATE_COUNT] = { 0 };

// GLOBAL: XW 0x4FC3A8
int16_t g_uniformExitAction = 0;

// GLOBAL: XW 0x4FC3AC
PushButton* g_uniformExitButton = NULL;

// GLOBAL: XW 0x4FC3B0
int g_uniformKeyboardAward = 0;

// GLOBAL: XW 0x4FC3B4
Sound* g_uniformMusic = NULL;

// GLOBAL: XW 0x4FC3B8
Film* g_uniformMusicFilm = NULL;

// FUNCTION: XW 0x4687E0
XwShellSceneResult Uniform_Uniform(struct XwShellContext* shellContext) {
	ResFile* resourceFile;
	Actor* backgroundActor;
	Rect frame;
	char pilotFilename[UNIFORM_PILOT_FILENAME_CAPACITY];
	g_uniformPreviousHoveredAward = UNIFORM_NO_HOVER;
	g_uniformHoveredAward = UNIFORM_NO_HOVER;
	g_uniformExitAction = 0;
	xcanvas_Erase_Canvas();
	strcpy(pilotFilename, g_RegisterShellPilot.name);
	strcat(pilotFilename, ".PLT");
	memset(&g_uniformPilot, 0, sizeof(g_uniformPilot));
	Uniform_ReadPilot(pilotFilename);
	resourceFile = xres_Open_Resource("unfrm640.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	g_uniformFilm = xfilm_Res_Callback_Film(resourceFile, "unifr640", &frame, 0, 0, 0, Uniform_film_Callback);
	xfilm_Set_Film_Def_Palette(g_uniformFilm, shellContext->standardPalette);
	backgroundActor = xactrect_Alloc_Blank_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, UNIFORM_BACKGROUND_Z);
	xactor_Non_Refreshable_Actor(backgroundActor);
	backgroundActor->w = UNIFORM_WIDTH;
	backgroundActor->h = UNIFORM_HEIGHT;
	xrect_Set_Rect(&frame, 0, 0, UNIFORM_WIDTH, UNIFORM_HEIGHT);
	g_uniformRootInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xrect_Set_Rect(&frame, 0, UNIFORM_AWARDS_TOP, UNIFORM_WIDTH, UNIFORM_HEIGHT);
	g_uniformAwardsInput = xinput_Alloc_Input(g_uniformRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_uniformAwardsInput, Uniform_iupdate_Awards);
	xinpattr_Set_Input_User_Function(g_uniformAwardsInput, Uniform_iuser_Awards);
	xinpattr_Set_Input_Draw_Function(g_uniformAwardsInput, Uniform_idraw_Awards);
	g_uniformAwardsInput->mouseUsage = allInput;
	if (shipext_IsTourAvailable(UNIFORM_MEDALS_FIRST_TOUR) != 0 ||
		shipext_IsTourAvailable(UNIFORM_MEDALS_SECOND_TOUR) != 0) {
		xrect_Set_Rect(&frame, UNIFORM_MEDALS_LEFT, 0, UNIFORM_MEDALS_RIGHT, UNIFORM_BUTTON_BOTTOM);
		g_uniformMedalsButton = xbtnpush_Alloc_Button(g_uniformRootInput, &frame, 0, Uniform_iuser_Button,
													  "Medals Case", UNIFORM_BUTTON_MEDALS_CASE);
		xinpattr_Set_Input_Update_Function(&g_uniformMedalsButton->header, Uniform_iupdate_Button);
		xinpattr_Set_Input_Draw_Function(&g_uniformMedalsButton->header, XwAwardsUI_DrawTextButton);
		g_uniformMedalsButton->header.mouseUsage = allInput;
	}
	xrect_Set_Rect(&frame, UNIFORM_EXIT_LEFT, 0, UNIFORM_EXIT_RIGHT, UNIFORM_BUTTON_BOTTOM);
	g_uniformExitButton = xbtnpush_Alloc_Button(g_uniformRootInput, &frame, 0, Uniform_iuser_Button, "Exit",
												UNIFORM_BUTTON_EXIT);
	xinpattr_Set_Input_Update_Function(&g_uniformExitButton->header, Uniform_iupdate_Button);
	xinpattr_Set_Input_Draw_Function(&g_uniformExitButton->header, XwAwardsUI_DrawTextButton);
	g_uniformExitButton->header.mouseUsage = allInput;
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_Update_Function(Uniform_end_View);
	xio_Set_Key_Buttons();
	Uniform_OpenMusic(resourceFile, g_uniformFilm);
	soundext_RecheckSfxPreference();
#ifdef XW_MODERN
	XwUniform_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Uniform_CloseMusic();
	soundext_RecheckSfxPreference();
	xcursor_Hide_Cursor();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xres_Close_Resource(resourceFile);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x468B20
void Uniform_end_View(int time) {
	int16_t key;
	if (time == 0) {
		if ((int16_t)xcursor_Is_Cursor_Visible() == 0)
			xcursor_Show_Cursor();
		Tooltip_BuildScreenPaletteRemap();
	}
	key = xio_Get_Free_Key();
	if (key != 0) {
		int16_t candidateAward = g_uniformKeyboardAward;
		if (key == UNIFORM_KEY_LEFT || key == UNIFORM_KEY_UP) {
			candidateAward = (int16_t)g_uniformKeyboardAward - 1;
			if (candidateAward < 0)
				candidateAward += UNIFORM_HOTSPOT_COUNT;
			if (candidateAward == UNIFORM_KEYBOARD_SKIP_FIRST ||
				candidateAward == UNIFORM_KEYBOARD_SKIP_SECOND ||
				candidateAward == UNIFORM_KEYBOARD_SKIP_LAST)
				--candidateAward;
			for (; candidateAward != (int16_t)g_uniformKeyboardAward;) {
				if (g_uniformAwardAvailable[candidateAward] != 0)
					break;
				--candidateAward;
				if (candidateAward < 0)
					candidateAward += UNIFORM_HOTSPOT_COUNT;
				if (candidateAward == UNIFORM_KEYBOARD_SKIP_FIRST ||
					candidateAward == UNIFORM_KEYBOARD_SKIP_SECOND ||
					candidateAward == UNIFORM_KEYBOARD_SKIP_LAST)
					--candidateAward;
			}
			key = xio_Get_Key();
		}
		if (key == UNIFORM_KEY_RIGHT || key == UNIFORM_KEY_DOWN) {
			candidateAward = (int16_t)g_uniformKeyboardAward + 1;
			if (candidateAward == UNIFORM_KEYBOARD_SKIP_FIRST ||
				candidateAward == UNIFORM_KEYBOARD_SKIP_SECOND ||
				candidateAward == UNIFORM_KEYBOARD_SKIP_LAST)
				++candidateAward;
			if (candidateAward >= UNIFORM_HOTSPOT_COUNT)
				candidateAward -= UNIFORM_HOTSPOT_COUNT;
			for (; candidateAward != (int16_t)g_uniformKeyboardAward;) {
				if (g_uniformAwardAvailable[candidateAward] != 0)
					break;
				++candidateAward;
				if (candidateAward == UNIFORM_KEYBOARD_SKIP_FIRST ||
					candidateAward == UNIFORM_KEYBOARD_SKIP_SECOND ||
					candidateAward == UNIFORM_KEYBOARD_SKIP_LAST)
					++candidateAward;
				if (candidateAward >= UNIFORM_HOTSPOT_COUNT)
					candidateAward -= UNIFORM_HOTSPOT_COUNT;
			}
			xio_Get_Key();
		}
		if (candidateAward != (int16_t)g_uniformKeyboardAward) {
			xio_Set_Mouse_Position(g_uniformHotspots[candidateAward].x, g_uniformHotspots[candidateAward].y);
			/* The original leaves the upper half of this global unchanged. */
			g_uniformKeyboardAward =
				(int)(((uint32_t)g_uniformKeyboardAward & ~(uint32_t)UINT16_MAX) | (uint16_t)candidateAward);
		}
	}
	if (g_uniformExitAction == UNIFORM_EXIT) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_UNIFORM_FROM_COMBAT_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_BRIEFING_COMBAT);
				break;
			case XW_SCENE_UNIFORM_FROM_TOUR_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_BRIEFING_TOUR);
				break;
			case XW_SCENE_UNIFORM_FROM_REGISTER:
				xerror_Set_Landru_Exit(XW_SCENE_REGISTER_RETURN);
				break;
		}
	} else if (g_uniformExitAction == UNIFORM_VIEW_MEDALS_CASE) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_UNIFORM_FROM_COMBAT_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_AWARD_CASE_FROM_COMBAT_BRIEFING);
				break;
			case XW_SCENE_UNIFORM_FROM_TOUR_BRIEFING:
				xerror_Set_Landru_Exit(XW_SCENE_AWARD_CASE_FROM_TOUR_BRIEFING);
				break;
			case XW_SCENE_UNIFORM_FROM_REGISTER:
				xerror_Set_Landru_Exit(XW_SCENE_AWARD_CASE_FROM_REGISTER);
				break;
		}
	}
}

// FUNCTION: XW 0x468CF0
int16_t Uniform_film_Callback(Film* film, FilmObject* object) {
	Actor* actor;
	int16_t awardRole;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		xactor_Non_Refreshable_Actor(actor);
		if (actor->var1 != 0) {
			xactor_Set_Actor_User_Function(actor, Uniform_user_Award);
			g_uniformActorStates[actor->var1] = UNIFORM_KEEP_ACTOR_STATE;
		}
		awardRole = actor->var1;
		switch (awardRole) {
			case UNIFORM_ROLE_KALIDOR:
				if (g_uniformPilot.kalidorAwardLevel > 0)
					g_uniformAwardAvailable[UNIFORM_HOVER_KALIDOR] = 1;
				else {
					xactor_Set_Actor_Time(actor, 0, 0);
					g_uniformAwardAvailable[UNIFORM_HOVER_KALIDOR] = 0;
				}
				break;
			case UNIFORM_ROLE_EMBELLISHMENTS: {
				uint8_t awardLevel = g_uniformPilot.kalidorAwardLevel;
				int16_t hoverIndex = UNIFORM_HOVER_EMBELLISHMENT_FIRST;
				int level = g_uniformPilot.kalidorAwardLevel;
				for (; hoverIndex < UNIFORM_HOVER_EMBELLISHMENT_DUPLICATE; ++hoverIndex) {
					if (level > hoverIndex - UNIFORM_HOVER_KALIDOR)
						g_uniformAwardAvailable[hoverIndex] = 1;
					else
						g_uniformAwardAvailable[hoverIndex] = 0;
					if (hoverIndex == UNIFORM_HOVER_EMBELLISHMENT_SOURCE)
						g_uniformAwardAvailable[UNIFORM_HOVER_EMBELLISHMENT_DUPLICATE] =
							g_uniformAwardAvailable[hoverIndex];
				}
				if (awardLevel < UNIFORM_EMBELLISHMENT_MIN_LEVEL)
					xactor_Set_Actor_Time(actor, 0, 0);
				else
					g_uniformActorStates[actor->var1] = (int16_t)awardLevel - 1;
				break;
			}
			case UNIFORM_ROLE_BADGE_FIRST:
			case UNIFORM_ROLE_BADGE_FIRST + 1:
			case UNIFORM_ROLE_BADGE_LAST:
				if (g_uniformPilot.trainingLevelProgress[(int16_t)(UNIFORM_ROLE_BADGE_LAST - awardRole)] >=
					UNIFORM_BADGE_TRAINING_LEVEL)
					g_uniformAwardAvailable[awardRole - UNIFORM_ROLE_BADGE_FIRST] = 1;
				else {
					g_uniformAwardAvailable[awardRole - UNIFORM_ROLE_BADGE_FIRST] = 0;
					xactor_Set_Actor_Time(actor, 0, 0);
				}
				break;
			case UNIFORM_ROLE_PATCH_FIRST:
			case UNIFORM_ROLE_PATCH_FIRST + 1:
			case UNIFORM_ROLE_PATCH_LAST: {
				int16_t shipIndex = awardRole - UNIFORM_COMBAT_PATCH_ROLE_BASE;
				int16_t missionIndex;
				if (shipIndex == -1)
					shipIndex = UNIFORM_COMBAT_PATCH_WRAPPED_ROW;
				xactor_Set_Actor_Draw_Function(actor, Uniform_draw_CombatPatches);
				for (missionIndex = 0; missionIndex < UNIFORM_COMBAT_PATCH_COUNT; ++missionIndex) {
					int16_t available = g_uniformPilot.combatAwards[shipIndex].missionPatch[missionIndex];
					g_uniformAwardAvailable[missionIndex + UNIFORM_COMBAT_PATCH_COUNT * actor->var1 -
											UNIFORM_PATCH_HOVER_BIAS] = available;
				}
				break;
			}
			case UNIFORM_ROLE_RIBBON_FIRST:
			case UNIFORM_ROLE_RIBBON_FIRST + 1:
			case UNIFORM_ROLE_RIBBON_LAST: {
				int16_t ribbonCount =
					g_uniformPilot.tourOperationProgress[awardRole - UNIFORM_ROLE_RIBBON_FIRST];
				if (ribbonCount != 0 && ribbonCount != UNIFORM_RIBBON_ABSENT) {
					if (ribbonCount > UNIFORM_RIBBONS_PER_TOUR)
						ribbonCount = UNIFORM_RIBBONS_PER_TOUR;
					g_uniformActorStates[awardRole] = ribbonCount - 1;
					g_uniformActorStates[actor->var1] +=
						UNIFORM_RIBBONS_PER_TOUR * (actor->var1 - UNIFORM_ROLE_RIBBON_FIRST);
				} else
					xactor_Set_Actor_Time(actor, 0, 0);
				g_uniformAwardAvailable[actor->var1 + UNIFORM_RIBBON_HOVER_BIAS] = 1;
				g_uniformAwardAvailable[UNIFORM_HOVER_RIBBON_SUMMARY] = 1;
				break;
			}
			case UNIFORM_ROLE_MEDAL_FIRST:
			case UNIFORM_ROLE_MEDAL_FIRST + 1:
			case UNIFORM_ROLE_MEDAL_LAST:
				if (g_uniformPilot.uniformMedals[awardRole - UNIFORM_ROLE_MEDAL_FIRST] > 0)
					g_uniformAwardAvailable[awardRole + UNIFORM_MEDAL_HOVER_BIAS] = 1;
				else {
					g_uniformAwardAvailable[awardRole + UNIFORM_MEDAL_HOVER_BIAS] = 0;
					xactor_Set_Actor_Time(actor, 0, 0);
				}
				break;
			case UNIFORM_ROLE_RANK:
				g_uniformActorStates[awardRole] = (int16_t)g_uniformPilot.rank + 1;
				if (g_uniformPilot.rank > UNIFORM_RANK_EXTRA_STATE_THRESHOLD)
					++g_uniformActorStates[actor->var1];
				g_uniformAwardAvailable[UNIFORM_HOVER_RANK] = 1;
				break;
		}
	}
	return 0;
}

// FUNCTION: XW 0x468FC0
int16_t Uniform_iupdate_Awards(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
							   int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)clip;
	if (key != 0) {
		return 0;
	}
	if (leftEvent == 0 && rightEvent == 0) {
		int16_t bestDistance = UNIFORM_INITIAL_HOVER_DISTANCE;
		int16_t nearestAward = UNIFORM_NO_HOVER;
		int16_t awardIndex;
		for (awardIndex = 0; awardIndex < UNIFORM_HOTSPOT_COUNT; ++awardIndex) {
			if (g_uniformAwardAvailable[awardIndex] != 0) {
				int16_t distanceX = abs(x + frame->left - g_uniformHotspots[awardIndex].x);
				int16_t distanceY = abs(frame->top + y - g_uniformHotspots[awardIndex].y);
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
		if (bestDistance < UNIFORM_HOVER_DISTANCE) {
			g_uniformHoveredAward = nearestAward;
		} else {
			g_uniformHoveredAward = UNIFORM_NO_HOVER;
		}
	}
	return 1;
}

// FUNCTION: XW 0x4690A0
void Uniform_iuser_Awards(Input* input, int context) {
	(void)input;
	(void)context;
	if (g_uniformPreviousHoveredAward != g_uniformHoveredAward || g_uniformHoveredAward != UNIFORM_NO_HOVER) {
		g_uniformPreviousHoveredAward = g_uniformHoveredAward;
		xview_Refresh_View();
	}
}

// FUNCTION: XW 0x4690D0
void Uniform_idraw_Awards(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	int16_t award, lineCount, maxWidth, mouseX, mouseY;
	int16_t index;
	Rect textRect, boxRect;
	char countText[UNIFORM_COUNT_TEXT_SIZE];
	char lines[UNIFORM_TOOLTIP_LINES][UNIFORM_TOOLTIP_LINE_SIZE];
	(void)input;
	(void)frame;
	(void)clip;
	if (refresh == 0)
		return;
	award = g_uniformHoveredAward;
	if (award == UNIFORM_NO_HOVER)
		return;
	for (index = 0; index < UNIFORM_TOOLTIP_LINES; ++index)
		lines[index][0] = 0;
	if (award < UNIFORM_HOVER_PATCH_FIRST) {
		strcpy(lines[0], g_uniformAwardLabels[award]);
	}
	if (award >= UNIFORM_HOVER_PATCH_FIRST && award < UNIFORM_HOVER_RIBBON_FIRST) {
		int patch = award - UNIFORM_HOVER_PATCH_FIRST;
		strcpy(
			lines[0],
			g_uniformAwardLabels[(int16_t)(patch / UNIFORM_COMBAT_PATCH_COUNT) + UNIFORM_LABEL_PATCH_FIRST]);
		sprintf(lines[1], "Mission %d", (int16_t)(patch % UNIFORM_COMBAT_PATCH_COUNT + 1));
		award = g_uniformHoveredAward;
	}
	if (award >= UNIFORM_HOVER_RIBBON_FIRST && award < UNIFORM_HOVER_MEDAL_FIRST) {
		int16_t line = 0;
		for (index = 0; index < UNIFORM_TOOLTIP_LINES; ++index) {
			int16_t ribbons = g_uniformPilot.tourOperationProgress[index];
			if (ribbons != 0 && ribbons != UNIFORM_RIBBON_ABSENT) {
				if (ribbons > UNIFORM_RIBBONS_PER_TOUR)
					ribbons = UNIFORM_RIBBONS_PER_TOUR;
				sprintf(countText, "%d ", ribbons);
				strcpy(lines[line], countText);
				strcat(lines[line], g_uniformAwardLabels[UNIFORM_LABEL_TOUR_PREFIX]);
				strcat(lines[line], g_uniformAwardLabels[UNIFORM_LABEL_TOUR_FIRST + index]);
				strcat(lines[line], g_uniformAwardLabels[UNIFORM_LABEL_RIBBON_SUFFIX]);
				award = g_uniformHoveredAward;
				++line;
			}
		}
	}
	if (award >= UNIFORM_HOVER_MEDAL_FIRST && award < UNIFORM_HOVER_RANK)
		strcpy(lines[0], g_uniformAwardLabels[award - UNIFORM_LABEL_MEDAL_BIAS]);
	if (award == UNIFORM_HOVER_RANK)
		strcpy(lines[0], g_uniformAwardLabels[g_uniformPilot.rank + UNIFORM_LABEL_RANK_FIRST]);
	if (award >= UNIFORM_HOVER_KALIDOR && award < UNIFORM_HOVER_EMBELLISHMENT_DUPLICATE)
		strcpy(lines[0], g_uniformAwardLabels[award - UNIFORM_LABEL_EMBELLISHMENT_BIAS]);
	if (award == UNIFORM_HOVER_EMBELLISHMENT_DUPLICATE)
		strcpy(lines[0], g_uniformAwardLabels[UNIFORM_LABEL_SILVER_TALONS]);
	mouseX = xio_Mouse_X();
	mouseY = xio_Mouse_Y();
	lineCount = 0;
	if (strlen(lines[0]) != 0) {
		lineCount = 1;
		if (strlen(lines[1]) != 0) {
			lineCount = 2;
			if (strlen(lines[2]) != 0)
				lineCount = UNIFORM_TOOLTIP_LINES;
		}
	}
	if (lineCount != 0) {
		maxWidth = 0;
		for (index = 0; index < lineCount; ++index) {
			int16_t width = xfont_Get_String_Width_0(UNIFORM_TOOLTIP_FONT, lines[index]);
			if (width > maxWidth)
				maxWidth = width;
		}
		if (mouseX >= UNIFORM_TOOLTIP_HALF_SCREEN)
			xrect_Set_Rect(&textRect, mouseX - maxWidth - UNIFORM_TOOLTIP_LEFT_GAP,
						   mouseY - UNIFORM_TOOLTIP_HALF_LINE * lineCount, mouseX - UNIFORM_TOOLTIP_LEFT_GAP,
						   mouseY + UNIFORM_TOOLTIP_HALF_LINE * (2 - lineCount));
		else
			xrect_Set_Rect(&textRect, mouseX + UNIFORM_TOOLTIP_RIGHT_GAP,
						   mouseY - UNIFORM_TOOLTIP_HALF_LINE * lineCount,
						   maxWidth + mouseX + UNIFORM_TOOLTIP_RIGHT_GAP,
						   mouseY + UNIFORM_TOOLTIP_HALF_LINE * (2 - lineCount));
		if (textRect.left < UNIFORM_TOOLTIP_MIN)
			xrect_Offset_Rect(&textRect, UNIFORM_TOOLTIP_MIN - textRect.left, 0);
		if (textRect.right > UNIFORM_TOOLTIP_MAX_X)
			xrect_Offset_Rect(&textRect, UNIFORM_TOOLTIP_MAX_X - textRect.right, 0);
		if (textRect.top < UNIFORM_TOOLTIP_MIN)
			xrect_Offset_Rect(&textRect, 0, UNIFORM_TOOLTIP_MIN - textRect.top);
		if (textRect.bottom > UNIFORM_TOOLTIP_MAX_Y)
			xrect_Offset_Rect(&textRect, 0, UNIFORM_TOOLTIP_MAX_Y - textRect.bottom);
		xrect_Copy_Rect(&boxRect, &textRect);
		xrect_Inset_Rect(&boxRect, -UNIFORM_TOOLTIP_BORDER_X, -UNIFORM_TOOLTIP_BORDER_Y);
		boxRect.bottom += UNIFORM_TOOLTIP_LINE_HEIGHT * lineCount - UNIFORM_TOOLTIP_LINE_HEIGHT;
		xpaint_Frame_Clipped_Rect(&boxRect, UNIFORM_TOOLTIP_FRAME_COLOR);
		xrect_Inset_Rect(&boxRect, 1, 1);
		shade_RemapClippedRect(&boxRect);
		xfont_Enable_FontID_Shadow(UNIFORM_TOOLTIP_FONT);
		for (index = 0; index < lineCount; ++index) {
			xfont_Print_Centered_Text(lines[index], &textRect, UNIFORM_TOOLTIP_FONT,
									  UNIFORM_TOOLTIP_TEXT_COLOR);
			xrect_Offset_Rect(&textRect, 0, UNIFORM_TOOLTIP_LINE_HEIGHT);
		}
		xfont_Disable_FontID_Shadow(UNIFORM_TOOLTIP_FONT);
	}
}

// FUNCTION: XW 0x469600
int16_t Uniform_iupdate_Button(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
							   int rightEvent, int16_t x, int16_t y) {
	int16_t result;
	result = 0;
	if (leftEvent != 0 || rightEvent != 0) {
		result = xbtnpush_iupdate_Button(input, frame, clip, key, leftEvent, rightEvent, x, y);
	}
	g_uniformHoveredAward = UNIFORM_NO_HOVER;
	return result;
}

// FUNCTION: XW 0x469650
void Uniform_iuser_Button(Input* input, int context) {
	(void)context;
	if (xinpattr_Get_Input_Selected(input)) {
		g_uniformExitAction =
			input->id == UNIFORM_BUTTON_MEDALS_CASE ? UNIFORM_VIEW_MEDALS_CASE : UNIFORM_EXIT;
	}
}

// FUNCTION: XW 0x469680
void Uniform_user_Award(Actor* actor, int time) {
	if (time == 0) {
		int16_t initialState = g_uniformActorStates[actor->var1];
		if (initialState != UNIFORM_KEEP_ACTOR_STATE) {
			xactor_Set_Actor_State(actor, initialState, 0);
		}
	}
}

// FUNCTION: XW 0x4696B0
int16_t Uniform_draw_CombatPatches(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh) {
	if (refresh != 0) {
		int16_t shipIndex = actor->var1 - UNIFORM_COMBAT_PATCH_ROLE_BASE;
		int16_t missionIndex;
		const XwUniformCombatAwards* awards;
		if (shipIndex == -1) {
			shipIndex = UNIFORM_COMBAT_PATCH_WRAPPED_ROW;
		}
		missionIndex = 0;
		awards = &g_uniformPilot.combatAwards[shipIndex];
		for (; missionIndex < UNIFORM_COMBAT_PATCH_COUNT; ++missionIndex) {
			if (awards->missionPatch[missionIndex] != 0) {
				xactor_Set_Actor_State(actor,
									   UNIFORM_COMBAT_PATCH_COUNT * actor->var1 - missionIndex -
										   UNIFORM_COMBAT_PATCH_STATE_OFFSET,
									   0);
				xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
			}
		}
	}
	return refresh != 0;
}

// FUNCTION: XW 0x469740
int16_t Uniform_ReadPilot(const char* filename) {
	LandruFile* pilotFile;
	pilotFile = xfile_Open_File(LANDRU_FILE_ROOT_USER, filename, "rb");
	if (pilotFile != NULL) {
		register_ReadPilotRecord(pilotFile, &g_uniformPilot);
		xfile_Close_File(pilotFile);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x469780
void Uniform_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_uniformMusicFilm = film;
		g_uniformMusic = xsound_Find_Gmid(g_uniformMusicName);
		if (g_uniformMusic == NULL) {
			ResFile* resourceFile = xres_Open_Resource(":X-Wing Data\\RESOURCE\\unmusic.lfd");
			if (resourceFile == NULL)
				resourceFile = xres_Open_Resource(g_uniformMusicResourceName);
			g_uniformMusic = xsound_Res_Music(resourceFile, g_uniformMusicName);
			soundext_Start_Resource_Sound(g_uniformMusic);
			xres_Close_Resource(resourceFile);
		} else {
			soundext_FadeVolume(g_uniformMusic, UNIFORM_MUSIC_RESUME_VOLUME, UNIFORM_MUSIC_RESUME_DURATION);
		}
		xsound_Set_Sound_Keep(g_uniformMusic);
		xsound_Set_Sound_User_Function(g_uniformMusic, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x469840
void Uniform_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid(g_uniformMusicName);
		g_uniformMusic = music;
		if (music != NULL) {
			/* Resource pointers are outside the numeric flight-sound ID range. */
			soundext_SetPriority(0, 0);
			soundext_FadeVolume(g_uniformMusic, 0, UNIFORM_MUSIC_FADE_DURATION);
			xsound_Clear_Sound_Keep(g_uniformMusic);
			xsound_Free_Sound(g_uniformMusic);
		}
	}
}
