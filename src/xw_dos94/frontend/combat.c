#include "xw_dos94/frontend/combat.h"
#include "xw/frontend/combat.h"
#include "xw_dos94/frontend/textext.h"
#include "xw_runtime/integration/landru_adapter.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/textext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/combat_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/combat_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/actrect.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/style.h>
#include <landru/view.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DOS94 data at 0x2c4f8e / 0x2c4f94. */
static const int16_t enemy_spec_x[3] = { 10, 110, 4 };
static const int16_t enemy_spec_y[3] = { 40, 40, 40 };
static void create_navigation(void);
XwShellSceneResult Dos94_combat_Combat(struct XwShellContext* shell);
static void Dos94_combat_end_Combat_View(int time);
static int16_t Dos94_combat_film_Combat_Callback(Film* film, FilmObject* filmObject);
static void Dos94_combat_idraw_DoorLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_combat_idraw_Combat(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_combat_user_Stars(Actor* actor, int time);
static void Dos94_combat_user_Door(Actor* actor, int time);
static void Dos94_combat_user_ShipIconFrame(Actor* actor, int time);
static void Dos94_combat_user_TieFighter(Actor* actor, int time);
static void Dos94_combat_user_TieInterceptor(Actor* actor, int time);
static void Dos94_combat_user_TieBomber(Actor* actor, int time);
static int16_t Dos94_combat_Draw_Combat_Screen_Flyby(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													 int16_t y, int16_t refresh);
static void Dos94_combat_user_Score(Actor* actor, int time);
static int16_t Dos94_combat_Draw_Combat_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													 int16_t y, int16_t refresh);
static void Dos94_combat_user_Welcome(Actor* actor, int time);
static int16_t Dos94_combat_draw_Welcome(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										 int16_t refresh);
static int16_t Dos94_combat_Draw_Combat_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													   int16_t y, int16_t refresh);
static void Dos94_combat_DrawSoundTypewriterLine(const char* text, uint16_t fontId, int16_t x, int16_t y,
												 int16_t revealSteps);

/* DOS94 0x2c0000. */
XwShellSceneResult Dos94_combat_Combat(struct XwShellContext* shell) {
	ResFile* missionResource;
	Rect rect;
	char paragraphName[COMBAT_PARAGRAPH_NAME_CAPACITY];
	int16_t unavailableCount;
	int unavailableTours;
	int unavailableShips;
	int16_t sourceIndex;
	int16_t missionListIndex;
	int16_t enemyIndex;
	Input* monitorInput;
	g_combatMonitorTime = 0;
	g_combatKeyboardFocusIndex = COMBAT_INITIAL_FOCUS;
	xio_Set_Mouse_Position(COMBAT_INITIAL_MOUSE_X, COMBAT_INITIAL_MOUSE_Y);
	g_combatScoreSelection = -1;
	combat_Load_Combat_High_Scores();
	combat_ReadPilot();
	missionResource = XwLandru_OpenMissionResource(":X-Wing Data\\Resource\\missions.lfd");
	if (missionResource == NULL)
		missionResource = XwLandru_OpenMissionResource("missions.lfd");
	g_combatShipListText = xparagrp_Res_Paragraph(missionResource, "comships");
	g_combatAvailableShipCount =
		xparagrp_Count_Paragraph_Strings(g_combatShipListText, COMBAT_SHIP_NAMES_PARAGRAPH);
	g_combatAvailableTourCount =
		xparagrp_Count_Paragraph_Strings(g_combatShipListText, COMBAT_TOUR_NAMES_PARAGRAPH);
	unavailableCount = 0;
	unavailableShips = 0;
	unavailableTours = 0;
	for (sourceIndex = 0; sourceIndex < g_combatAvailableShipCount + g_combatAvailableTourCount;
		 ++sourceIndex) {
		if (sourceIndex < g_combatAvailableShipCount) {
			if (shipext_IsShipAvailable(sourceIndex)) {
				int selectionIndex;
				xparagrp_Get_Paragraph_String(g_combatShipListText, paragraphName,
											  COMBAT_RESOURCE_NAMES_PARAGRAPH, sourceIndex);
				selectionIndex = sourceIndex - unavailableCount;
				g_combatMissionParagraphs[selectionIndex] =
					xparagrp_Res_Paragraph(missionResource, paragraphName);
				g_combatSelectionSourceIndices[selectionIndex] = sourceIndex;
			} else {
				++unavailableShips;
				++unavailableCount;
			}
		} else {
			/* A pilot may have completed this tour in another installation. */
			if (!shipext_IsTourAvailable(sourceIndex - g_combatAvailableShipCount)) {
				++unavailableTours;
				++unavailableCount;
				continue;
			}
			if (g_combatPilotData.tour_status[sourceIndex - g_combatAvailableShipCount] != 0 &&
				g_combatPilotData.tourReplayUnlockMission[sourceIndex - g_combatAvailableShipCount] !=
					SHIPEXT_PILOT_UNSET_BYTE) {
				int selectionIndex;
				xparagrp_Get_Paragraph_String(g_combatShipListText, paragraphName,
											  COMBAT_RESOURCE_NAMES_PARAGRAPH, sourceIndex);
				selectionIndex = sourceIndex - unavailableCount;
				g_combatMissionParagraphs[selectionIndex] =
					xparagrp_Res_Paragraph(missionResource, paragraphName);
				g_combatSelectionSourceIndices[selectionIndex] = sourceIndex;
			} else {
				++unavailableTours;
				++unavailableCount;
			}
		}
	}
	g_combatAvailableTourCount -= unavailableTours;
	g_combatAvailableShipCount -= unavailableShips;
	for (missionListIndex = 0; missionListIndex < g_combatAvailableShipCount + g_combatAvailableTourCount;
		 ++missionListIndex) {
		LandruHandle missionParagraph = g_combatMissionParagraphs[missionListIndex];
		if (missionParagraph != LANDRU_NULL_HANDLE) {
			int16_t missionCount = xparagrp_Count_Paragraph_Strings(missionParagraph, 0);
			g_combatMissionCounts[missionListIndex] = missionCount;
			if (missionListIndex >= g_combatAvailableShipCount) {
				int16_t tourIndex = g_combatSelectionSourceIndices[missionListIndex] - SHIPEXT_SHIP_COUNT;
				int16_t operationCount = g_combatPilotData.tourOperationProgress[tourIndex];
				int16_t availableVariantCount = 0;
				int16_t operationIndex;
				for (operationIndex = 0; operationIndex < operationCount; ++operationIndex) {
					if (g_tourOperationTables[tourIndex][operationIndex].missionChoiceB !=
						SHIPEXT_TOUR_NO_ENTRY)
						++availableVariantCount;
					++availableVariantCount;
				}
				if (missionCount > (int16_t)availableVariantCount)
					g_combatMissionCounts[missionListIndex] = availableVariantCount;
			}
		}
	}
	xres_Close_Resource(missionResource);
	g_combatResourceFile = xres_Open_Resource("combat.lfd");
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&rect, COMBAT_CANVAS_LEFT, COMBAT_CANVAS_TOP, 320, 200);
	g_combatFilm = xfilm_Res_Callback_Film(g_combatResourceFile, "combat", &rect, 0, 0, 0,
										   Dos94_combat_film_Combat_Callback);
	xfilm_Set_Film_Def_Palette(g_combatFilm, shell->standardPalette);
	ResFile* icons = shipext_IsTourAvailable(4) ? xres_Open_Resource("bwing.lfd") : g_combatResourceFile;
	g_combatShipIconsActor = xactanim_Res_Anim_Actor(icons, "ships", &rect, 0, 0, 0);
	if (icons != g_combatResourceFile)
		xres_Close_Resource(icons);
	xactor_Set_Actor_Time(g_combatShipIconsActor, 0, 0);
	g_combatFallbackIconActor = xactdelt_Res_Delta_Actor(g_combatResourceFile, "logo", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_combatFallbackIconActor, 0, 0);
	xrect_Set_Rect(&rect, 80, 33, 239, 134);
	g_combatStarsActor =
		xactdelt_Res_Delta_Actor(shell->resourceFile, "stars-4", &rect, 0, 0, COMBAT_STARS_Z);
	g_combatStarsWrapActor =
		xactdelt_Res_Delta_Actor(shell->resourceFile, "stars-4", &rect, 320, 0, COMBAT_STARS_Z);
	xactor_Set_Actor_User_Function(g_combatStarsActor, Dos94_combat_user_Stars);
	xactor_Set_Actor_User_Function(g_combatStarsWrapActor, Dos94_combat_user_Stars);
	g_combatMonitorBackgroundActor = xactrect_Alloc_Blank_Actor(0, &rect, 80, 33, COMBAT_BACKGROUND_Z);
	xactor_Set_Actor_Size(g_combatMonitorBackgroundActor, 239 - 80, 134 - 33);
	xactor_Set_Actor_Color(g_combatMonitorBackgroundActor, 0, 0);
	g_combatShipInfoActor = xactcust_Alloc_Custom_Actor(0, &rect, 80, 33, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_Update_Function(g_combatShipInfoActor, combat_update_ShipInfo);
	xactor_Set_Actor_User_Function(g_combatShipInfoActor, combat_user_ShipInfo);
	xactor_Set_Actor_Draw_Function(g_combatShipInfoActor, Dos94_combat_Draw_Combat_Screen_Flyby);
	g_combatShipInfoActor->var1 = 0;
	g_combatScoreActor = xactcust_Alloc_Custom_Actor(0, &rect, 80, 33, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_User_Function(g_combatScoreActor, Dos94_combat_user_Score);
	xactor_Set_Actor_Draw_Function(g_combatScoreActor, Dos94_combat_Draw_Combat_Screen_Score);
	g_combatScoreActor->var1 = 0;
	g_combatWelcomeTextActor = xactcust_Alloc_Custom_Actor(0, &rect, 80, 33, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_User_Function(g_combatWelcomeTextActor, Dos94_combat_user_Welcome);
	xactor_Set_Actor_Draw_Function(g_combatWelcomeTextActor, Dos94_combat_draw_Welcome);
	g_combatWelcomeTextActor->var1 = 0;
	g_combatMissionTextActor = xactcust_Alloc_Custom_Actor(0, &rect, 80, 33, COMBAT_MONITOR_TEXT_Z);
	xactor_Set_Actor_User_Function(g_combatMissionTextActor, combat_user_MissionText);
	xactor_Set_Actor_Draw_Function(g_combatMissionTextActor, Dos94_combat_Draw_Combat_Screen_Mission);
	g_combatMissionTextActor->var1 = 0;
	xrect_Set_Rect(&rect, COMBAT_CANVAS_LEFT, COMBAT_CANVAS_TOP, 320, 200);
	g_combatRootInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xrect_Set_Rect(&rect, 80, 33, 239, 134);
	monitorInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(monitorInput, XwCombat_UpdateMonitor);
	xinpattr_Set_Input_User_Function(monitorInput, combat_iuser_Combat_Screen);
	xrect_Set_Rect(&rect, COMBAT_LEFT_DOOR_LEFT, 42, 34, 182);
	g_combatLeftDoorInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_combatLeftDoorInput, combat_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_combatLeftDoorInput, combat_iuser_Door);
	g_combatLeftDoorInput->mouseUsage = allInput;
	g_combatLeftDoorInput->id = 0;
	xrect_Set_Rect(&rect, 286, 42, 320, 182);
	g_combatRightDoorInput = xinput_Alloc_Input(g_combatRootInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_combatRightDoorInput, combat_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_combatRightDoorInput, combat_iuser_Door);
	g_combatRightDoorInput->mouseUsage = allInput;
	g_combatRightDoorInput->id = 1;
	create_navigation();
	xrect_Set_Rect(&rect, 80, 33, 239, 134);
	g_combatEnemyPreviewActors[0] = xactanim_Res_Anim_Actor(
		g_combatResourceFile, g_combatEnemyResourceNames[0], &rect, 80, 33, COMBAT_PREVIEW_Z);
	xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[0], Dos94_combat_user_TieFighter);
	g_combatEnemyPreviewActors[1] = xactanim_Res_Anim_Actor(
		g_combatResourceFile, g_combatEnemyResourceNames[1], &rect, 80, 33, COMBAT_PREVIEW_Z);
	xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[1], Dos94_combat_user_TieInterceptor);
	g_combatEnemyPreviewActors[2] = xactanim_Res_Anim_Actor(
		g_combatResourceFile, g_combatEnemyResourceNames[2], &rect, 80, 33, COMBAT_PREVIEW_Z);
	xactor_Set_Actor_User_Function(g_combatEnemyPreviewActors[2], Dos94_combat_user_TieBomber);
	for (enemyIndex = 0; enemyIndex < COMBAT_ENEMY_PREVIEW_COUNT; ++enemyIndex) {
		g_combatEnemyPreviewActors[enemyIndex]->id = enemyIndex;
		g_combatEnemyPreviewActors[enemyIndex]->var1 = COMBAT_PREVIEW_ROLE;
	}
	xio_Set_Key_Buttons();
	combat_OpenMusic(g_combatResourceFile, g_combatFilm, &g_combatMonitorTime);
	combat_LoadSoundEffects();
	xview_Set_View_Update_Function(Dos94_combat_end_Combat_View);
	XwCombat_RunView();
}

/* DOS94 0x2c1154. */
static void Dos94_combat_end_Combat_View(int time) {
	int16_t key;
	if (time == 0 && !xcursor_Is_Cursor_Visible())
		xcursor_Show_Cursor();
	if (time == 4)
		combat_HandleSoundAction(COMBAT_SOUND_HYDRAULICS_START);
	if (time == 30)
		combat_HandleSoundAction(COMBAT_SOUND_HYDRAULICS_STOP);
	combat_HandleSoundAction(COMBAT_SOUND_TYPING_UPDATE);
	key = xio_Get_Free_Key();
	if (key && shellext_MoveGridFocus(&g_combatKeyboardFocusIndex, g_combatKeyboardFocusX,
									  g_combatKeyboardFocusY, COMBAT_FOCUS_ROWS, COMBAT_FOCUS_COLUMNS, key)) {
		xio_Set_Mouse_Position(g_combatKeyboardFocusX[g_combatKeyboardFocusIndex],
							   g_combatKeyboardFocusY[g_combatKeyboardFocusIndex]);
		xio_Get_Key();
	}
	if (time < g_combatFilm->cels) {
		xactor_Refresh_Actor(g_combatScreenActor);
		xactor_Refresh_Actor(g_combatFilmTag3Actor);
		xactor_Refresh_Actor(g_combatNavigationRevealActor);
		xactor_Refresh_Actor(g_combatShipIconFrameActor);
		if (xinpattr_Is_Input_Visible(g_combatNavigationInput))
			xinpattr_Refresh_Input(g_combatNavigationInput);
		if (xinpattr_Is_Input_Visible(g_combatHoverLabelInput))
			xinpattr_Refresh_Input(g_combatHoverLabelInput);
	}
	if (time >= (uint16_t)(g_combatFilm->cels - 24))
		g_combatMonitorTime = (g_combatMonitorTime + 1) % COMBAT_MONITOR_PERIOD;
}

/* DOS94 0x2c138e. */
static int16_t Dos94_combat_film_Combat_Callback(Film* film, FilmObject* filmObject) {
	if (filmObject->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, filmObject, filmObject + 1);
		actor = filmObject->object;
		switch (actor->var1) {
			case COMBAT_ROLE_TAG_1:
				g_combatFilmTag1Actor = actor;
				xactor_Non_Refreshable_Actor(actor);
				break;
			case COMBAT_ROLE_SCREEN:
				g_combatScreenActor = actor;
				xactor_Set_Actor_User_Function(actor, Dos94_combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatScreenActor);
				g_combatScreenActor->id = COMBAT_DOOR_SCREEN;
				break;
			case COMBAT_ROLE_TAG_3:
				g_combatFilmTag3Actor = actor;
				xactor_Set_Actor_User_Function(actor, Dos94_combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatFilmTag3Actor);
				g_combatFilmTag3Actor->id = COMBAT_DOOR_SCREEN;
				break;
			case COMBAT_ROLE_LEFT_DOOR:
				g_combatLeftDoorActor = actor;
				xactor_Set_Actor_User_Function(actor, Dos94_combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatLeftDoorActor);
				g_combatLeftDoorActor->id = COMBAT_DOOR_LEFT;
				break;
			case COMBAT_ROLE_RIGHT_DOOR:
				g_combatRightDoorActor = actor;
				xactor_Set_Actor_User_Function(actor, Dos94_combat_user_Door);
				xactor_Non_Refreshable_Actor(g_combatRightDoorActor);
				g_combatRightDoorActor->id = COMBAT_DOOR_RIGHT;
				break;
			case COMBAT_ROLE_NAVIGATION_REVEAL:
				g_combatNavigationRevealActor = actor;
				xactor_Set_Actor_User_Function(actor, combat_user_NavigationReveal);
				xactor_Non_Refreshable_Actor(g_combatNavigationRevealActor);
				break;
			case COMBAT_ROLE_SHIP_ICON_FRAME:
				g_combatShipIconFrameActor = actor;
				xactor_Set_Actor_User_Function(actor, Dos94_combat_user_ShipIconFrame);
				xactor_Set_Actor_Draw_Function(g_combatShipIconFrameActor, combat_draw_ShipIconFrame);
				xactor_Non_Refreshable_Actor(g_combatShipIconFrameActor);
				break;
			case COMBAT_ROLE_FIRST_DECOR:
			case COMBAT_ROLE_SECOND_DECOR:
				xactor_Set_Actor_User_Function(actor, combat_user_DecorState);
				break;
			case COMBAT_ROLE_RANDOM_ANIMATION:
				if (rand() & 1)
					xactor_Set_Actor_Time(actor, 0, 0);
				break;
		}
	}
	return 0;
}

/* DOS94 0x2c1a8a. */
static void Dos94_combat_idraw_DoorLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		if (input->var1 != 0) {
			xrect_Offset_Rect(frame, COMBAT_DOOR_LABEL_SHADOW_OFFSET, COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Combat Briefing", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_SHADOW_COLOR);
			xrect_Offset_Rect(frame, -COMBAT_DOOR_LABEL_SHADOW_OFFSET, -COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Combat Briefing", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_TEXT_COLOR);
		} else {
			xrect_Offset_Rect(frame, COMBAT_DOOR_LABEL_SHADOW_OFFSET, COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_SHADOW_COLOR);
			xrect_Offset_Rect(frame, -COMBAT_DOOR_LABEL_SHADOW_OFFSET, -COMBAT_DOOR_LABEL_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, COMBAT_DOOR_LABEL_FONT,
									  COMBAT_DOOR_LABEL_TEXT_COLOR);
		}
	}
}

/* DOS94 0x2c1f02. */
static void Dos94_combat_idraw_Combat(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	int16_t selectedShip = shipext_Get_Combat_Ship();
	int16_t selectedMission = shipext_Get_Combat_Mission();
	char missionNumber[COMBAT_MISSION_NUMBER_CAPACITY];
	char labelText[COMBAT_SELECTION_LABEL_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		xstyle_Style_Paint_TextField(frame);
		if (input->id == COMBAT_SELECTION_SHIP_LABEL) {
			if (selectedShip >= g_combatAvailableShipCount)
				xparagrp_Get_Paragraph_String(g_combatShipListText, labelText, COMBAT_TOUR_NAMES_PARAGRAPH,
											  g_combatSelectionSourceIndices[selectedShip] -
												  SHIPEXT_SHIP_COUNT);
			else
				xparagrp_Get_Paragraph_String(g_combatShipListText, labelText, COMBAT_SHIP_NAMES_PARAGRAPH,
											  selectedShip);
		} else if (selectedShip >= g_combatAvailableShipCount) {
			int16_t selectedBaseMission = 0;
			int16_t baseMissionCount = 0;
			int16_t secondVariant = 0;
			int16_t selectedSecondVariant = 0;
			int16_t variantIndex;
			for (variantIndex = 0; variantIndex < g_combatMissionCounts[selectedShip]; ++variantIndex) {
				if (variantIndex == selectedMission) {
					selectedBaseMission = baseMissionCount;
					selectedSecondVariant = secondVariant;
				}
				if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT]
										 [baseMissionCount]
											 .missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
					if (secondVariant != 0) {
						secondVariant = 0;
						++baseMissionCount;
					} else {
						secondVariant = 1;
					}
				} else {
					++baseMissionCount;
				}
			}
			strcpy(labelText, "Mission ");
			if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT]
									 [selectedBaseMission]
										 .missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
				if (selectedSecondVariant != 0)
					sprintf(missionNumber, "%db/%d", selectedBaseMission + 1, baseMissionCount);
				else
					sprintf(missionNumber, "%da/%d", selectedBaseMission + 1, baseMissionCount);
			} else {
				sprintf(missionNumber, "%d/%d", selectedBaseMission + 1, baseMissionCount);
			}
			strcat(labelText, missionNumber);
		} else {
			sprintf(labelText, "Mission %d/%d", selectedMission + 1, g_combatMissionCounts[selectedShip]);
		}
		xfont_Print_Centered_Text(labelText, frame, COMBAT_SELECTION_LABEL_FONT,
								  COMBAT_SELECTION_LABEL_TEXT_COLOR);
	}
}

/* DOS94 0x2c21e8. */
static void Dos94_combat_user_Stars(Actor* actor, int time) {
	(void)time;
	if (!xactor_Is_Actor_Visible(actor)) {
		xactor_Show_Actor(actor);
	}
	actor->x += COMBAT_STARFIELD_STEP;
	if (actor->x >= 320) {
		actor->x -= 640;
	}
}

/* DOS94 0x2c223e. */
static void Dos94_combat_user_Door(Actor* actor, int time) {
	int refresh = 1;
	(void)time;
	if (actor->var1 == 0) {
		if (actor->state == 0)
			combat_HandleSoundAction(2);
		if (actor->state < actor->arraySize - 1)
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		else
			refresh = 0;
	} else if (actor->var1 <= 1) {
		if (actor->state == actor->arraySize - 1)
			combat_HandleSoundAction(1);
		if (actor->state != 0)
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		else
			refresh = 0;
		actor->var1 = 0;
	} else if (actor->xv || actor->xvf)
		xactor_Refresh_Actor(actor);
	if (refresh)
		xactor_Refresh_Actor(actor);
}

/* DOS94 0x2c23e2. */
static void Dos94_combat_user_ShipIconFrame(Actor* actor, int time) {
	if (time < (uint16_t)(g_combatFilm->cels - 1)) {
		xactor_Refresh_Actor(actor);
	}
}

/* DOS94 0x2c2532. */
static void Dos94_combat_user_TieFighter(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime >= COMBAT_TIE_FIGHTER_START &&
		g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_3_END) {
		switch (g_combatMonitorTime) {
			case COMBAT_TIE_FIGHTER_START:
				xactor_Show_Actor(actor);
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				actor->state = 0;
				break;
			case COMBAT_TIE_FIGHTER_INFO:
				xactor_Show_Actor(g_combatShipInfoActor);
				xactor_Set_Actor_State_Speed(actor, 0, 0);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_PHASE_1;
				g_combatShipInfoActor->var2 = 0;
				g_combatCurrentEnemyPreview = actor;
				break;
			case COMBAT_TIE_FIGHTER_RESUME:
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				xactor_Hide_Actor(g_combatShipInfoActor);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_IDLE;
				break;
		}
	} else if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, 82, 35, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Hide_Actor(actor);
	}
}

/* DOS94 0x2c26f8. */
static void Dos94_combat_user_TieInterceptor(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime >= COMBAT_TIE_INTERCEPTOR_START &&
		g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_4_END) {
		switch (g_combatMonitorTime) {
			case COMBAT_TIE_INTERCEPTOR_START:
				xactor_Show_Actor(actor);
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				actor->state = 0;
				break;
			case COMBAT_TIE_INTERCEPTOR_INFO:
				xactor_Show_Actor(g_combatShipInfoActor);
				xactor_Set_Actor_State_Speed(actor, 0, 0);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_PHASE_1;
				g_combatShipInfoActor->var2 = 0;
				g_combatCurrentEnemyPreview = actor;
				break;
			case COMBAT_TIE_INTERCEPTOR_RESUME:
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				xactor_Hide_Actor(g_combatShipInfoActor);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_IDLE;
				break;
		}
	} else if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, 82, 35, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Hide_Actor(actor);
	}
}

/* DOS94 0x2c28be. */
static void Dos94_combat_user_TieBomber(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime >= COMBAT_TIE_BOMBER_START &&
		g_combatMonitorTime < COMBAT_MONITOR_INTERVAL_5_END) {
		switch (g_combatMonitorTime) {
			case COMBAT_TIE_BOMBER_START:
				xactor_Show_Actor(actor);
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				actor->state = 0;
				break;
			case COMBAT_TIE_BOMBER_INFO:
				xactor_Show_Actor(g_combatShipInfoActor);
				xactor_Set_Actor_State_Speed(actor, 0, 0);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_PHASE_1;
				g_combatShipInfoActor->var2 = 0;
				g_combatCurrentEnemyPreview = actor;
				break;
			case COMBAT_TIE_BOMBER_RESUME:
				xactor_Set_Actor_State_Speed(actor, 1, 0);
				xactor_Hide_Actor(g_combatShipInfoActor);
				g_combatShipInfoActor->var1 = COMBAT_SHIP_INFO_IDLE;
				break;
		}
	} else if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, 138, 69, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Hide_Actor(actor);
	}
}

/* DOS94 0x2c2b40. */
static int16_t Dos94_combat_Draw_Combat_Screen_Flyby(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													 int16_t y, int16_t refresh) {
	Rect enemyRect;
	Rect titleRect;
	(void)clip;
	(void)x;
	(void)y;
	(void)refresh;
	if (actor->var1 == COMBAT_SHIP_INFO_PHASE_1 && actor->var2 == 1)
		combat_HandleSoundAction(COMBAT_SOUND_TARGET);
	switch (actor->var1) {
		case COMBAT_SHIP_INFO_PHASE_1: {
			int16_t tick;
			int16_t borderCount;
			int16_t borderColor;
			uint32_t borderIndex;
			xactor_Get_Actor_Rect(g_combatCurrentEnemyPreview, &enemyRect);
			tick = actor->var2 - 1;
			if (tick < COMBAT_ENEMY_BORDER_GROW_END) {
				xrect_Inset_Rect(&enemyRect, -COMBAT_ENEMY_BORDER_INITIAL_EXPANSION,
								 -COMBAT_ENEMY_BORDER_INITIAL_EXPANSION);
				borderColor = COMBAT_ENEMY_COLOR_MAX - COMBAT_ENEMY_COLOR_STEP * tick;
				borderCount = tick + 1;
			} else if (tick < COMBAT_ENEMY_BORDER_SHRINK_START) {
				int16_t inset = COMBAT_ENEMY_BORDER_STEP * (tick - COMBAT_ENEMY_BORDER_END);
				xrect_Inset_Rect(&enemyRect, inset, inset);
				borderColor = COMBAT_ENEMY_COLOR_MIN;
				borderCount = COMBAT_ENEMY_BORDER_MAX_COUNT;
			} else {
				int16_t inset;
				borderCount = COMBAT_ENEMY_BORDER_END - tick;
				inset = COMBAT_ENEMY_BORDER_STEP * borderCount;
				xrect_Inset_Rect(&enemyRect, -inset, -inset);
				borderColor = COMBAT_ENEMY_COLOR_MIN;
			}
			if (borderCount != 0) {
				borderIndex = borderCount;
				do {
					xpaint_Frame_Clipped_Rect(&enemyRect, borderColor);
					xrect_Inset_Rect(&enemyRect, COMBAT_ENEMY_BORDER_STEP, COMBAT_ENEMY_BORDER_STEP);
					borderColor += COMBAT_ENEMY_COLOR_STEP;
				} while (--borderIndex != 0);
			}
			break;
		}
		case COMBAT_SHIP_INFO_PHASE_2: {
			int16_t titleColor =
				COMBAT_ENEMY_TITLE_COLOR_STEP * (actor->var2 + COMBAT_ENEMY_TITLE_COLOR_START);
			int16_t enemyIndex;
			int16_t specX;
			int16_t specY;
			int16_t firstLine;
			int16_t endLine;
			int16_t revealChars;
			int16_t lineDelay;
			int16_t lineIndex;
			if (titleColor > COMBAT_ENEMY_COLOR_MAX)
				titleColor = COMBAT_ENEMY_COLOR_MAX;
			xrect_Copy_Rect(&titleRect, frame);
			titleRect.bottom = titleRect.top + 8;
			xrect_Offset_Rect(&titleRect, 0, COMBAT_ENEMY_TITLE_TOP);
			enemyIndex = g_combatCurrentEnemyPreview->id;
			xfont_Print_Centered_Text(g_combatEnemyText[0], &titleRect, 0, titleColor);
			xrect_Offset_Rect(&titleRect, 0, 10);
			xfont_Print_Centered_Text(g_combatEnemyText[enemyIndex + 1], &titleRect, 0, titleColor);
			specX = enemy_spec_x[enemyIndex];
			firstLine = g_combatEnemySpecLineBounds[enemyIndex];
			endLine = g_combatEnemySpecLineBounds[enemyIndex + 1];
			specY = enemy_spec_y[enemyIndex];
			revealChars = actor->var2;
			lineDelay = revealChars;
			if (firstLine < endLine) {
				for (lineIndex = firstLine; lineIndex < endLine; ++lineIndex) {
					int16_t textY = frame->top;
					int16_t textX = specX;
					textX += frame->left;
					textY += specY;
					Dos94_DrawTypewriterLine(g_combatEnemyText[lineIndex], 1, textX, textY, revealChars);
					revealChars = actor->var2;
					/* The original updates this delay but passes var2 to every line. */
					if (lineDelay == revealChars)
						lineDelay -= COMBAT_ENEMY_LINE_DELAY;
					specY += 8;
				}
			}
			xactor_Get_Actor_Rect(g_combatCurrentEnemyPreview, &enemyRect);
			xrect_Inset_Rect(&enemyRect, -COMBAT_ENEMY_BORDER_STEP, -COMBAT_ENEMY_BORDER_STEP);
			xpaint_Paint_Clipped_Rect(&enemyRect, COMBAT_ENEMY_BACKGROUND_COLOR);
			xpaint_Frame_Clipped_Rect(&enemyRect, COMBAT_ENEMY_FRAME_COLOR);
			return 1;
		}
		default:
			break;
	}
	return 1;
}

/* DOS94 0x2c2e86. */
static void Dos94_combat_user_Score(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime != 0 && g_combatMonitorTime >= COMBAT_SCORE_START &&
		g_combatMonitorTime < COMBAT_SCORE_END) {
		if (actor->var1 != 0) {
			actor->var2 += 1;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

/* DOS94 0x2c2f7c. */
static int16_t Dos94_combat_Draw_Combat_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													 int16_t y, int16_t refresh) {
	char pilotName[COMBAT_SCORE_PILOT_TEXT_CAPACITY];
	char scoreText[COMBAT_SCORE_TEXT_CAPACITY];
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		XwMissionHighScores* missionScores;
		int16_t missionIndex;
		int16_t mission;
		combat_Load_Combat_High_Scores();
		mission = shipext_Get_Combat_Mission();
		xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipext_Get_Combat_Ship()],
									  g_shellMissionName, 0, mission);
		missionIndex = 0;
		missionScores = xmemhdl_Lock_Handle(g_combatScoreHandle);
		while (missionIndex < g_combatScoreMissionCount && missionScores[missionIndex].missionName[0] &&
			   strcmp(missionScores[missionIndex].missionName, g_shellMissionName) != 0)
			++missionIndex;
		if (missionIndex < g_combatScoreMissionCount && missionScores[missionIndex].missionName[0] != 0) {
			int16_t revealPosition = actor->var2;
			int16_t textX = frame->left + COMBAT_SCORE_MARGIN;
			int16_t textY, detailY, entryIndex;
			if (revealPosition > 84)
				textY = frame->top + COMBAT_SCORE_MARGIN;
			else
				textY = frame->top - revealPosition + 84 + COMBAT_SCORE_MARGIN;
			detailY = textY + COMBAT_SCORE_DETAIL_Y_OFFSET;
			for (entryIndex = 0; entryIndex < COMBAT_SCORE_ENTRY_COUNT; ++entryIndex) {
				int16_t color;
				if (revealPosition < 0)
					break;
				color = revealPosition + COMBAT_SCORE_COLOR_BASE;
				if (color > COMBAT_SCORE_COLOR_MAX)
					color = COMBAT_SCORE_COLOR_MAX;
				strcpy(pilotName, missionScores[missionIndex].entries[entryIndex].pilotName);
				if (pilotName[0] != 0) {
					int score;
					int16_t kills, nameWidth;
					xfont_Print_Clipped_Text(pilotName, textX, textY, 0, color);
					score = missionScores[missionIndex].entries[entryIndex].score;
					kills = missionScores[missionIndex].entries[entryIndex].kills;
					nameWidth = 60;
					sprintf(scoreText, "Score %6ld    Kills %u", (long)score, (unsigned int)(int)kills);
					Dos94_DrawTypewriterLine(scoreText, 1, textX + nameWidth, detailY, revealPosition);
				}
				revealPosition -= COMBAT_SCORE_ENTRY_REVEAL_STEP;
				textY += 12;
				detailY += 12;
			}
		} else
			xfont_Print_Clipped_Text("No High Scores", frame->left + 44, frame->top + 6, 0,
									 COMBAT_SCORE_COLOR_MAX);
		xmemhdl_Unlock_Handle(g_combatScoreHandle);
	}
	return 1;
}

/* DOS94 0x2c3222. */
static void Dos94_combat_user_Welcome(Actor* actor, int time) {
	(void)time;
	if (g_combatMonitorTime != 0 && g_combatMonitorTime >= COMBAT_WELCOME_START &&
		g_combatMonitorTime < COMBAT_WELCOME_END) {
		if (actor->var1 != 0) {
			actor->var2 += 1;
		} else {
			xactor_Show_Actor(actor);
			actor->var1 = 1;
			actor->var2 = 0;
		}
	} else if (xactor_Is_Actor_Visible(actor)) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
}

/* DOS94 0x2c331a. */
static int16_t Dos94_combat_draw_Welcome(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										 int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		int16_t revealBudget = actor->var2;
		int16_t drawX = frame->left + COMBAT_WELCOME_LEFT_INSET;
		int16_t drawY;
		int16_t lineIndex;
		int16_t lineLength;
		if (revealBudget > 84)
			drawY = frame->top + COMBAT_WELCOME_TOP_INSET;
		else
			drawY = frame->top - revealBudget + 88;
		for (lineIndex = 0;
			 (lineLength = (int16_t)strlen(g_combatWelcomeLines[lineIndex])) != 0 && revealBudget >= 0;
			 ++lineIndex, revealBudget -= lineLength >> COMBAT_WELCOME_LENGTH_SHIFT) {
			Dos94_combat_DrawSoundTypewriterLine(g_combatWelcomeLines[lineIndex], 0, drawX, drawY,
												 revealBudget);
			drawY += 12;
		}
	}
	return 1;
}

/* DOS94 0x2c34fc. */
static int16_t Dos94_combat_Draw_Combat_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													   int16_t y, int16_t refresh) {
	int16_t selectedShip = shipext_Get_Combat_Ship();
	int16_t shipIndex = selectedShip;
	int16_t selectedMission = shipext_Get_Combat_Mission();
	Rect titleRect;
	int16_t paragraphWidth;
	int16_t paragraphHeight;
	char text[COMBAT_MISSION_SCREEN_TEXT_CAPACITY];
	char missionNumber[COMBAT_MISSION_SCREEN_TEXT_CAPACITY];
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		if (selectedShip >= g_combatAvailableShipCount)
			xparagrp_Get_Paragraph_String(g_combatShipListText, text, COMBAT_TOUR_NAMES_PARAGRAPH,
										  g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT);
		else
			xparagrp_Get_Paragraph_String(g_combatShipListText, text, COMBAT_SHIP_NAMES_PARAGRAPH,
										  selectedShip);
		strcat(text, " Mission ");
		if (selectedShip >= g_combatAvailableShipCount) {
			int16_t secondVariant = 0;
			int16_t operationIndex = 0;
			int16_t variantsRemaining;
			if (selectedMission > 0)
				for (variantsRemaining = selectedMission; variantsRemaining != 0; --variantsRemaining) {
					if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] -
											  SHIPEXT_SHIP_COUNT][operationIndex]
							.missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
						if (secondVariant != 0) {
							secondVariant = 0;
							++operationIndex;
						} else {
							secondVariant = 1;
						}
					} else {
						++operationIndex;
					}
				}
			if (g_tourOperationTables[g_combatSelectionSourceIndices[selectedShip] - SHIPEXT_SHIP_COUNT]
									 [operationIndex]
										 .missionChoiceB != SHIPEXT_TOUR_NO_ENTRY) {
				if (secondVariant != 0)
					sprintf(missionNumber, g_combatMissionFormatB, operationIndex + 1);
				else
					sprintf(missionNumber, g_combatMissionFormatA, operationIndex + 1);
			} else {
				sprintf(missionNumber, "%d", operationIndex + 1);
			}
		} else {
			sprintf(missionNumber, "%d", selectedMission + 1);
		}
		strcat(text, missionNumber);
		xrect_Copy_Rect(&titleRect, frame);
		titleRect.top += COMBAT_MISSION_SCREEN_TOP_INSET;
		titleRect.bottom = titleRect.top + 12;
		xfont_Print_Centered_Text(text, &titleRect, 0, COMBAT_MISSION_SCREEN_COLOR);
		xrect_Offset_Rect(&titleRect, 0, 10);
		xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipIndex], text,
									  COMBAT_MISSION_TITLES_PARAGRAPH, selectedMission);
		xfont_Print_Centered_Text(text, &titleRect, 0, COMBAT_MISSION_SCREEN_COLOR);
		{
			int16_t revealBudget = actor->var2 - COMBAT_MISSION_SCREEN_REVEAL_DELAY;
			if (revealBudget > 0) {
				int16_t paragraph = selectedMission + COMBAT_MISSION_DESCRIPTION_PARAGRAPH_BASE;
				int16_t drawX;
				int16_t drawY;
				int16_t lineCount;
				int16_t lineIndex;
				xparagrp_Get_Paragraph_Size(g_combatMissionParagraphs[shipIndex], 0, paragraph,
											&paragraphWidth, &paragraphHeight);
				drawX = frame->left + ((frame->right - paragraphWidth - frame->left) >> 1);
				drawY = frame->top + 30;
				lineCount = xparagrp_Count_Paragraph_Strings(g_combatMissionParagraphs[shipIndex], paragraph);
				for (lineIndex = 0; revealBudget > 0;) {
					if (lineIndex >= lineCount)
						break;
					xparagrp_Get_Paragraph_String(g_combatMissionParagraphs[shipIndex], text, paragraph,
												  lineIndex);
					Dos94_combat_DrawSoundTypewriterLine(text, 0, drawX, drawY, revealBudget);
					++lineIndex;
					revealBudget -= strlen(text) >> COMBAT_MISSION_SCREEN_LENGTH_SHIFT;
					drawY += 10;
				}
			}
		}
	}
	return 1;
}

/* DOS94 0x2c3890. */
static void Dos94_combat_DrawSoundTypewriterLine(const char* text, uint16_t fontId, int16_t x, int16_t y,
												 int16_t revealSteps) {
	if (revealSteps >= 0) {
		if (strlen(text) > (size_t)(TEXTEXT_CHARACTERS_PER_STEP * revealSteps)) {
			if (text[TEXTEXT_CHARACTERS_PER_STEP * revealSteps] == ' ' ||
				text[TEXTEXT_CHARACTERS_PER_STEP * revealSteps + 1] == ' ')
				combat_HandleSoundAction(COMBAT_SOUND_TYPING_STOP);
			else
				combat_HandleSoundAction(COMBAT_SOUND_TYPING_START);
		}
		Dos94_DrawTypewriterLine(text, fontId, x, y, TEXTEXT_CHARACTERS_PER_STEP * revealSteps);
	}
}

/* DOS94 0x2c1e02. */
static void draw_arrow(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (refresh) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(input->id == 0 || input->id == 2 ? 1 : 3, frame, clip,
										button->pressed);
	}
}

static void create_navigation(void) {
	Rect frame;
	xrect_Set_Rect(&frame, 96, 148, 224, 196);
	g_combatHoverLabelInput = xinput_Alloc_Input(g_combatRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_combatHoverLabelInput, Dos94_combat_idraw_DoorLabel);
	xinpattr_Hide_Input(g_combatHoverLabelInput);
	xrect_Set_Rect(&frame, 48, 148, 225, 199);
	g_combatNavigationInput = xinput_Alloc_Input(g_combatRootInput, &frame, 0, 0);
	xinpattr_Hide_Input(g_combatNavigationInput);
	for (int row = 0; row < 2; ++row) {
		xrect_Set_Rect(&frame, 54, 6 + row * 24, 70, 21 + row * 24);
		PushButton* button =
			xbtnpush_Alloc_Button(g_combatNavigationInput, &frame, 0, combat_iuser_Combat, NULL, row * 2);
		xinpattr_Set_Input_Draw_Function(&button->header, draw_arrow);
		xrect_Set_Rect(&frame, 6, 6 + row * 24, 22, 21 + row * 24);
		button =
			xbtnpush_Alloc_Button(g_combatNavigationInput, &frame, 0, combat_iuser_Combat, NULL, row * 2 + 1);
		xinpattr_Set_Input_Draw_Function(&button->header, draw_arrow);
		xinpattr_Set_Input_Allign(&button->header, 2, 0);
		xrect_Set_Rect(&frame, 72, 6 + row * 24, 153, 20 + row * 24);
		Input* label = xinput_Alloc_Input(g_combatNavigationInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(label, combat_iupdate_SelectionLabel);
		xinpattr_Set_Input_Draw_Function(label, Dos94_combat_idraw_Combat);
		label->id = row;
	}
}
