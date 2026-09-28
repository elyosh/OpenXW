#include "xw_dos94/frontend/train.h"
#include "xw/frontend/train.h"
#include "xw_dos94/frontend/textext.h"
#include "xw_runtime/integration/landru_adapter.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/textext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/combat_callbacks.h"
#include "xw_runtime/integration/train_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/train_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <landru/canvas.h>
#include <landru/style.h>

static Actor* panel_actor;
static Actor* ship_icon;
static int32_t r2_sound_ticks;
static const char welcome_lines[][40] = {
	"Welcome to the Rebel Alliance", "Pilot Proving Ground.  Here you", "will learn to fly the main three",
	"Alliance starfighters.  The",   "'maze' is designed to test your", "skill in maneuvering and firing",
	"at fixed targets.  Scoring is", "based on speed and accuracy.",    ""
};

static void Dos94_train_end_Train_View(int time);
static void Dos94_train_iuser_Train_Screen(Input* input, int time);
static int16_t Dos94_train_iupdate_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										int rightEvent, int16_t x, int16_t y);
static void Dos94_train_iuser_Door(Input* input, int time);
static void Dos94_train_idraw_DoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_train_iuser_Train(Input* input, int time);
static int16_t Dos94_train_iupdate_SelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key,
												  int leftEvent, int rightEvent, int16_t x, int16_t y);
static void Dos94_train_idraw_Train(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_train_user_XWingPresentation(Actor* actor, int time);
static void Dos94_train_user_AWingPresentation(Actor* actor, int time);
static void Dos94_train_user_YWingPresentation(Actor* actor, int time);
static void Dos94_train_user_Stars(Actor* actor, int time);
static void Dos94_train_user_Door(Actor* actor, int time);
static void Dos94_train_UpdateShipInfoAnimation(Actor* actor);
static void Dos94_train_user_ShipInfo(Actor* actor, int time);
static int16_t Dos94_train_DrawShipInfoPanel(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
											 int16_t refresh);
static void Dos94_train_DrawAWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x,
												int16_t y);
static void Dos94_train_DrawXWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x,
												int16_t y);
static void Dos94_train_DrawYWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x,
												int16_t y);
static void Dos94_train_user_MissionText(Actor* actor, int time);
static int16_t Dos94_train_Draw_Train_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													 int16_t y, int16_t refresh);
static void Dos94_train_user_HighScores(Actor* actor, int time);
static int16_t Dos94_train_Draw_Train_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x,
												   int16_t y, int16_t refresh);
static void Dos94_train_user_Welcome(Actor* actor, int time);
static int16_t Dos94_train_DrawWelcomeText(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										   int16_t refresh);
static void Dos94_train_DrawTypewriterLineWithSound(const char* text, uint16_t fontId, int16_t x, int16_t y,
													int16_t revealTicks);
static void Dos94_train_RestartMissionText(void);
static void Dos94_train_OpenMusic(ResFile* unusedResourceFile, Film* film, int* presentationFrame);
void Dos94_train_CloseMusic(void);
static void Dos94_train_user_Music(Sound* sound, int time);

/* DOS94 0x3010b4. */
static void Dos94_train_end_Train_View(int time) {
	int16_t key;
	int cels;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0 && shellext_MoveGridFocus(&g_trainingFocusIndex, g_trainingFocusX, g_trainingFocusY,
										   TRAIN_FOCUS_ROWS, TRAIN_FOCUS_COLUMNS, key) != 0) {
		xio_Set_Mouse_Position(g_trainingFocusX[g_trainingFocusIndex],
							   g_trainingFocusY[g_trainingFocusIndex]);
		xio_Get_Key();
	}
	cels = g_trainingFilm->cels;
	if (time >= (uint16_t)(cels - 16)) {
		if (time == cels)
			xinpattr_Show_Input(g_trainingMonitorInput);
		g_trainingPresentationFrame = (g_trainingPresentationFrame + 1) % (947 + 1);
	}
	if (time == 2)
		train_HandleSoundAction(TRAIN_SOUND_HYDRAULIC_START);
	if (time == 35)
		train_HandleSoundAction(TRAIN_SOUND_HYDRAULIC_STOP);
	train_HandleSoundAction(TRAIN_SOUND_TYPING_UPDATE);
}

/* DOS94 0x30120c. */
static void Dos94_train_iuser_Train_Screen(Input* input, int time) {
	(void)time;
	if (input->var1 != 0) {
		if ((uint16_t)xactor_Is_Actor_Visible(g_trainingShipInfoActor) != 0) {
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = 0;
			g_trainingShipInfoActor->var2 = 0;
		}
		if (g_trainingPresentationFrame < TRAIN_MISSION_TEXT_END) {
			g_trainingPresentationFrame = TRAIN_MISSION_TEXT_END - 1;
		} else if (g_trainingPresentationFrame < TRAIN_HIGH_SCORES_END) {
			g_trainingPresentationFrame = TRAIN_HIGH_SCORES_END - 1;
		} else if (g_trainingPresentationFrame < 449) {
			g_trainingPresentationFrame = 449 - 1;
		} else if (g_trainingPresentationFrame < 629) {
			g_trainingPresentationFrame = 629 - 1;
		} else if (g_trainingPresentationFrame < 803) {
			g_trainingPresentationFrame = 803 - 1;
		} else {
			g_trainingPresentationFrame = 947 - 1;
		}
		input->var1 = 0;
	}
}

/* DOS94 0x3012fa. */
static int16_t Dos94_train_iupdate_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0)
		return 0;
	if (leftEvent == TRAIN_DOOR_ACTIVATE_EVENT || rightEvent == TRAIN_DOOR_ACTIVATE_EVENT) {
		input->var1 = TRAIN_DOOR_ACTION_EXIT;
		if (input->id != TRAIN_DOOR_EXIT) {
			switch (shipext_Get_Train_Ship()) {
				case SHIPEXT_SHIP_AWING:
					input->var2 = 262;
					break;
				case SHIPEXT_SHIP_XWING:
					input->var2 = 260;
					break;
				case SHIPEXT_SHIP_YWING:
					input->var2 = 261;
					break;
				case SHIPEXT_SHIP_BWING:
					input->var2 = 266;
					break;
			}
		} else {
			input->var2 = XW_SCENE_TRAINING_RETURN_SHUTTLE;
		}
	} else {
		input->var1 = TRAIN_DOOR_ACTION_HINT;
	}
	if (input->id == TRAIN_DOOR_EXIT)
		g_trainingExitDoorActor->var1 = 1;
	else
		g_trainingLaunchDoorActor->var1 = 1;
	return 1;
}

/* DOS94 0x3013b2. */
static void Dos94_train_iuser_Door(Input* input, int time) {
	(void)time;
	switch (input->var1) {
		case TRAIN_DOOR_ACTION_IDLE:
			if (xinpattr_Is_Input_Visible(g_trainingDoorHintInput) != 0 &&
				input->id == g_trainingDoorHintInput->var1) {
				xinpattr_Show_Input(g_trainingNavigationInput);
				xinpattr_Hide_Input(g_trainingDoorHintInput);
				xactor_Refresh_Actor(panel_actor);
				xinpattr_Refresh_Input(g_trainingNavigationInput);
			}
			break;
		case TRAIN_DOOR_ACTION_EXIT:
			xerror_Set_Landru_Exit(input->var2);
			break;
		case TRAIN_DOOR_ACTION_HINT:
			if (xinpattr_Is_Input_Visible(g_trainingNavigationInput) != 0) {
				xinpattr_Hide_Input(g_trainingNavigationInput);
				xinpattr_Show_Input(g_trainingDoorHintInput);
				xactor_Refresh_Actor(panel_actor);
				xinpattr_Refresh_Input(g_trainingDoorHintInput);
				g_trainingDoorHintInput->var1 = input->id;
			}
			input->var1 = TRAIN_DOOR_ACTION_IDLE;
			break;
	}
}

/* DOS94 0x301510. */
static void Dos94_train_idraw_DoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		if (input->var1 != 0) {
			xrect_Offset_Rect(frame, TRAIN_HINT_SHADOW_OFFSET, TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Maze", frame, TRAIN_LABEL_FONT, TRAIN_LABEL_BACKGROUND_COLOR);
			xrect_Offset_Rect(frame, -TRAIN_HINT_SHADOW_OFFSET, -TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Maze", frame, TRAIN_LABEL_FONT, TRAIN_LABEL_TEXT_COLOR);
		} else {
			xrect_Offset_Rect(frame, TRAIN_HINT_SHADOW_OFFSET, TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, TRAIN_LABEL_FONT,
									  TRAIN_LABEL_BACKGROUND_COLOR);
			xrect_Offset_Rect(frame, -TRAIN_HINT_SHADOW_OFFSET, -TRAIN_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Return To Spaceport", frame, TRAIN_LABEL_FONT, TRAIN_LABEL_TEXT_COLOR);
		}
	}
}

/* DOS94 0x30161e. */
static void Dos94_train_iuser_Train(Input* input, int time) {
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		int16_t selectionChanged = 0;
		int16_t selectedShip = shipext_Get_Train_Ship();
		int16_t selectedLevel = shipext_Get_Train_Level();
		int buttonId = input->id;
		switch (buttonId) {
			case TRAIN_BUTTON_PREVIOUS_SHIP:
			case TRAIN_BUTTON_NEXT_SHIP: {
				int16_t availableLevels;
				if (buttonId == TRAIN_BUTTON_PREVIOUS_SHIP) {
					do {
						if (selectedShip != 0)
							--selectedShip;
						else
							selectedShip = SHIPEXT_SHIP_COUNT - 1;
						selectionChanged =
							shipext_IsShipAvailable(selectedShip) & (selectedShip != TRAIN_SKIPPED_SHIP_SLOT);
					} while (selectionChanged == 0);
				} else {
					do {
						if (selectedShip < SHIPEXT_SHIP_COUNT - 1)
							++selectedShip;
						else
							selectedShip = 0;
						selectionChanged =
							shipext_IsShipAvailable(selectedShip) & (selectedShip != TRAIN_SKIPPED_SHIP_SLOT);
					} while (selectionChanged == 0);
				}
				shipext_Set_Train_Ship(selectedShip);
				availableLevels = g_trainingPilot.trainingLevelProgress[shipext_Get_Train_Ship()] + 1;
				g_trainingAvailableLevels = availableLevels;
				if (availableLevels > g_trainingTotalLevels) {
					availableLevels = g_trainingTotalLevels;
					g_trainingAvailableLevels = g_trainingTotalLevels;
				}
				if (selectedLevel >= availableLevels)
					shipext_Set_Train_Level(g_trainingAvailableLevels - 1);
				break;
			}
			case TRAIN_BUTTON_PREVIOUS_LEVEL:
				if (selectedLevel != 0)
					shipext_Set_Train_Level(selectedLevel - 1);
				else
					shipext_Set_Train_Level(g_trainingAvailableLevels - 1);
				selectionChanged = 1;
				break;
			case TRAIN_BUTTON_NEXT_LEVEL:
				if (selectedLevel < g_trainingAvailableLevels - 1)
					shipext_Set_Train_Level(selectedLevel + 1);
				else
					shipext_Set_Train_Level(0);
				selectionChanged = 1;
				break;
		}
		if (selectionChanged != 0) {
			xactor_Refresh_Actor(panel_actor);
			xinpattr_Refresh_Input(g_trainingNavigationInput);
			Dos94_train_RestartMissionText();
		}
	}
}

/* DOS94 0x301812. */
static int16_t Dos94_train_iupdate_SelectionLabel(Input* input, Rect* frame, Rect* clip, int16_t key,
												  int leftEvent, int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key == 0 && (leftEvent == TRAIN_MOUSE_PRESS || rightEvent == TRAIN_MOUSE_PRESS)) {
		if (xactor_Is_Actor_Visible(g_trainingMissionTextActor) != 0) {
			g_trainingPresentationFrame = TRAIN_MISSION_TEXT_START;
			g_trainingMissionTextActor->var2 = TRAIN_MISSION_TEXT_REVEAL_ALL;
		} else {
			Dos94_train_RestartMissionText();
		}
	}
	return 0;
}

/* DOS94 0x30186e. */
static void Dos94_train_idraw_Train(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	enum { LABEL_CAPACITY = 32 };

	char label[LABEL_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		xstyle_Style_Paint_TextField(frame);
		if (input->id == TRAIN_LABEL_SHIP) {
			xfont_Print_Centered_Text(g_trainingShipNames[shipext_Get_Train_Ship()], frame, TRAIN_LABEL_FONT,
									  TRAIN_LABEL_TEXT_COLOR);
		} else {
			sprintf(label, "Level %d/%d", shipext_Get_Train_Level() + 1, g_trainingAvailableLevels);
			xfont_Print_Centered_Text(label, frame, TRAIN_LABEL_FONT, TRAIN_LABEL_TEXT_COLOR);
		}
	}
}

/* DOS94 0x3018ec. */
static void Dos94_train_user_XWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < 453 || g_trainingPresentationFrame >= 629) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_PRESENTATION_RESET_X, TRAIN_PRESENTATION_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case 453:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case 506:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case 606:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

/* DOS94 0x301a7e. */
static void Dos94_train_user_AWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < TRAIN_AWING_START || g_trainingPresentationFrame >= 449) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_PRESENTATION_RESET_X, TRAIN_AWING_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case TRAIN_AWING_START:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case 327:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case 427:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

/* DOS94 0x301c10. */
static void Dos94_train_user_YWingPresentation(Actor* actor, int time) {
	(void)time;
	if ((g_trainingPresentationFrame < 633 || g_trainingPresentationFrame >= 803) &&
		(uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Set_Actor_Pos(actor, TRAIN_YWING_RESET_X, TRAIN_YWING_RESET_Y, 0, 0);
		xactor_Set_Actor_Speed(actor, 0, 0, 0, 0);
		xactor_Set_Actor_State(actor, 0, 0);
		xactor_Set_Actor_State_Speed(actor, 0, 0);
		xactor_Set_Actor_Scale(actor, TRAIN_PRESENTATION_FULL_SCALE, TRAIN_PRESENTATION_FULL_SCALE);
		xactor_Hide_Actor(actor);
	}
	switch (g_trainingPresentationFrame) {
		case 633:
			xactor_Show_Actor(actor);
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			actor->state = 0;
			break;
		case 680:
			xactor_Show_Actor(g_trainingShipInfoActor);
			xactor_Set_Actor_State_Speed(actor, 0, 0);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_SHRINK;
			g_trainingShipInfoActor->var2 = 0;
			g_trainingDisplayedShipActor = actor;
			break;
		case 780:
			xactor_Set_Actor_State_Speed(actor, 1, 0);
			xactor_Hide_Actor(g_trainingShipInfoActor);
			g_trainingShipInfoActor->var1 = TRAIN_SHIP_INFO_IDLE;
			break;
	}
}

/* DOS94 0x301f28. */
static void Dos94_train_user_Stars(Actor* actor, int time) {
	(void)time;
	if (!xactor_Is_Actor_Visible(actor))
		xactor_Show_Actor(actor);
	actor->x += TRAIN_STAR_SCROLL_STEP;
	if (actor->x >= 320) {
		actor->x -= 640;
	}
}

/* DOS94 0x301f6a. */
static void Dos94_train_user_Door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	if (time < 8) {
		xactor_Refresh_Actor(actor);
		return;
	}
	if (actor->var1 == 0) {
		if (actor->state == actor->arraySize - 1) {
			train_HandleSoundAction(TRAIN_SOUND_DOOR_CLOSE);
		}
		if (actor->state != 0) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		} else {
			needsRefresh = 0;
		}
	} else {
		if (actor->state == 0) {
			train_HandleSoundAction(TRAIN_SOUND_DOOR_OPEN);
		}
		if (actor->state != actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		} else {
			needsRefresh = 0;
		}
		actor->var1 = 0;
	}
	if (needsRefresh != 0) {
		xactor_Refresh_Actor(actor);
	}
}

/* DOS94 0x302028. */
static void Dos94_train_UpdateShipInfoAnimation(Actor* actor) {
	switch (actor->var1) {
		case TRAIN_SHIP_INFO_SHRINK:
			if (actor->var2 < 54) {
				actor->var2 += 9;
				g_trainingDisplayedShipActor->xscale -= TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->yscale -= TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->x -= g_trainingDisplayedShipActor->var1;
			} else {
				actor->var1 = TRAIN_SHIP_INFO_TEXT;
				actor->var2 = 0;
			}
			break;
		case TRAIN_SHIP_INFO_TEXT:
			if (actor->var2 < 84) {
				actor->var2 += 1;
			} else {
				actor->var1 = TRAIN_SHIP_INFO_EXPAND;
				actor->var2 = 0;
			}
			break;
		case TRAIN_SHIP_INFO_EXPAND:
			if (actor->var2 < 54) {
				actor->var2 += 9;
				g_trainingDisplayedShipActor->xscale += TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->yscale += TRAIN_SHIP_INFO_SCALE_STEP;
				g_trainingDisplayedShipActor->x += g_trainingDisplayedShipActor->var1;
			} else {
				actor->var1 = TRAIN_SHIP_INFO_COMPLETE;
				actor->var2 = 0;
			}
			break;
	}
}

/* DOS94 0x3020f2. */
static void Dos94_train_user_ShipInfo(Actor* actor, int time) {
	Rect viewFrame;
	if (g_trainingPresentationFrame == 0) {
		xactor_Hide_Actor(actor);
		actor->var1 = 0;
	}
	if (time > 0 && time <= 34) {
		xview_Get_View_Frame(TRAIN_SCREEN_VIEW, &viewFrame);
		xrect_Offset_Rect(&viewFrame, 0, 4);
		xview_Set_View_Frame(TRAIN_SCREEN_VIEW, &viewFrame);
		xactor_Refresh_Actor(g_trainingBackgroundActor);
		xactor_Refresh_Actor(g_trainingScreenActor);
	}
}

/* DOS94 0x3021a2. */
static int16_t Dos94_train_DrawShipInfoPanel(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
											 int16_t refresh) {
	Rect panelRect;
	(void)refresh;
	switch (actor->var1) {
		case TRAIN_SHIP_INFO_SHRINK:
			xrect_Set_Rect(&panelRect, 246 - actor->var2, 19, 246, 127);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			xrect_Set_Rect(&panelRect, 74, 127 - (actor->var2 >> TRAIN_INFO_PANEL_VERTICAL_SHIFT), 246, 127);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			break;
		case TRAIN_SHIP_INFO_TEXT:
			xrect_Set_Rect(&panelRect, 192, 19, 246, 127);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			xrect_Set_Rect(&panelRect, 74, 100, 246, 127);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			if (g_trainingDisplayedShipActor == g_trainingAWingActor)
				Dos94_train_DrawAWingSpecifications(actor, frame, clip, x, y);
			else if (g_trainingDisplayedShipActor == g_trainingYWingActor)
				Dos94_train_DrawYWingSpecifications(actor, frame, clip, x, y);
			else
				Dos94_train_DrawXWingSpecifications(actor, frame, clip, x, y);
			break;
		case TRAIN_SHIP_INFO_EXPAND:
			xrect_Set_Rect(&panelRect, actor->var2 + 192, 19, 246, 127);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			xrect_Set_Rect(&panelRect, 74, (actor->var2 >> TRAIN_INFO_PANEL_VERTICAL_SHIFT) + 100, 246, 127);
			xpaint_Paint_Clipped_Rect(&panelRect, TRAIN_INFO_PANEL_COLOR);
			break;
	}
	return 1;
}

/* DOS94 0x30236a. */
static void Dos94_train_DrawAWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x,
												int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	Dos94_DrawTypewriterLine("Speed: 120 MGLT", 1, 78, 102, infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		Dos94_DrawTypewriterLine("Shields/Hull: 50 SBD/15 RU", 1, 78, 110,
								 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		Dos94_DrawTypewriterLine("Lasers and Concussion Missiles", 1, 78, 118,
								 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		Dos94_DrawTypewriterLine("A-WING", 0, 202, 59, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		Dos94_DrawTypewriterLine("Fighter", 0, 202, 67, infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingRebelLogoActor, frame, clip, 246 - logoOffset, 27, 1);
}

/* DOS94 0x30249e. */
static void Dos94_train_DrawXWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x,
												int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	Dos94_DrawTypewriterLine("Speed: 100 MGLT", 1, 78, 102, infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		Dos94_DrawTypewriterLine("Shields/Hull: 50 SBD/20 RU", 1, 78, 110,
								 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		Dos94_DrawTypewriterLine("Lasers and Proton Torpedoes", 1, 78, 118,
								 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		Dos94_DrawTypewriterLine("X-WING", 0, 202, 59, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		Dos94_DrawTypewriterLine("Fighter", 0, 202, 67, infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingIncomLogoActor, frame, clip, 242 - logoOffset, 27, 1);
}

/* DOS94 0x3025d2. */
static void Dos94_train_DrawYWingSpecifications(Actor* infoActor, Rect* frame, Rect* clip, int16_t x,
												int16_t y) {
	int16_t logoReveal;
	int16_t logoOffset;
	(void)x;
	(void)y;
	Dos94_DrawTypewriterLine("Speed: 80 MGLT", 1, 78, 102, infoActor->var2);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		Dos94_DrawTypewriterLine("Shields/Hull: 75 SBD/40 RU", 1, 78, 110,
								 infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		Dos94_DrawTypewriterLine("Lasers and Proton Torpedoes", 1, 78, 118,
								 infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_FIRST_DELAY)
		Dos94_DrawTypewriterLine("Y-WING", 0, 202, 59, infoActor->var2 - TRAIN_SPEC_FIRST_DELAY);
	if (infoActor->var2 >= TRAIN_SPEC_SECOND_DELAY)
		Dos94_DrawTypewriterLine("Bomber", 0, 202, 67, infoActor->var2 - TRAIN_SPEC_SECOND_DELAY);
	logoReveal = infoActor->var2;
	logoOffset = TRAIN_SPEC_LOGO_STEP * logoReveal;
	if (logoReveal >= TRAIN_SPEC_SECOND_DELAY)
		logoOffset = TRAIN_SPEC_LOGO_MAX_OFFSET;
	xactdelt_Draw_Delta_Actor(g_trainingIncomLogoActor, frame, clip, 242 - logoOffset, 27, 1);
}

/* DOS94 0x302706. */
static void Dos94_train_user_MissionText(Actor* actor, int time) {
	(void)time;
	if (g_trainingPresentationFrame != 0 && g_trainingPresentationFrame >= TRAIN_MISSION_TEXT_START &&
		g_trainingPresentationFrame < TRAIN_MISSION_TEXT_END) {
		if (actor->var1 != 0) {
			++actor->var2;
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

/* DOS94 0x3027be. */
static int16_t Dos94_train_Draw_Train_Screen_Mission(Actor* actor, Rect* frame, Rect* clip, int16_t x,
													 int16_t y, int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		Rect headingRect;
		char lineText[TRAIN_MISSION_TEXT_CAPACITY];
		char levelLabel[TRAIN_MISSION_TEXT_CAPACITY];
		int16_t revealTicks;
		int16_t boldColor;
		xrect_Copy_Rect(&headingRect, frame);
		headingRect.top += TRAIN_MISSION_HEADING_INSET;
		headingRect.bottom = headingRect.top + 12;
		strcpy(lineText, "Rebel Proving Ground");
		xfont_Print_Centered_Text(lineText, &headingRect, 0, TRAIN_MISSION_TEXT_COLOR);
		xrect_Offset_Rect(&headingRect, 0, 12);
		strcpy(lineText, g_trainingShipNames[shipext_Get_Train_Ship()]);
		xparagrp_Get_Paragraph_String(g_trainingLevelParagraph, levelLabel, TRAIN_MISSION_LEVEL_PARAGRAPH,
									  shipext_Get_Train_Level());
		strcat(lineText, " ");
		strcat(lineText, levelLabel);
		xfont_Print_Centered_Text(lineText, &headingRect, 0, TRAIN_MISSION_TEXT_COLOR);
		revealTicks = (int16_t)(actor->var2 - TRAIN_MISSION_REVEAL_DELAY);
		xfont_Set_FontID_Bold_Color(0, TRAIN_MISSION_BOLD_COLOR);
		boldColor = xfont_Get_FontID_Bold_Color(0);
		if (revealTicks > 0) {
			int16_t paragraphIndex = shipext_Get_Train_Level() + TRAIN_MISSION_BODY_PARAGRAPH_BASE;
			int16_t paragraphWidth;
			int16_t paragraphHeight;
			int16_t lineX;
			int16_t lineY;
			int16_t lineCount;
			int16_t lineIndex;
			xparagrp_Get_Paragraph_Size(g_trainingLevelParagraph, 0, paragraphIndex, &paragraphWidth,
										&paragraphHeight);
			lineX = (int16_t)(frame->left + ((frame->right - paragraphWidth - frame->left) >> 1));
			lineY = frame->top + 30;
			lineCount = xparagrp_Count_Paragraph_Strings(g_trainingLevelParagraph, paragraphIndex);
			for (lineIndex = 0; lineIndex < lineCount;) {
				xparagrp_Get_Paragraph_String(g_trainingLevelParagraph, lineText, paragraphIndex, lineIndex);
				Dos94_train_DrawTypewriterLineWithSound(lineText, 0, lineX, lineY, revealTicks);
				++lineIndex;
				lineY += 10;
				revealTicks = (int16_t)(revealTicks - (strlen(lineText) / TEXTEXT_CHARACTERS_PER_STEP));
				if (revealTicks <= 0) {
					break;
				}
			}
		}
		/* Original reads the bold color after setting it. */
		xfont_Set_FontID_Bold_Color(0, boldColor);
	}
	return 1;
}

/* DOS94 0x3029ea. */
static void Dos94_train_user_HighScores(Actor* actor, int time) {
	(void)time;
	if (g_trainingPresentationFrame != 0 && g_trainingPresentationFrame >= TRAIN_HIGH_SCORES_START &&
		g_trainingPresentationFrame < TRAIN_HIGH_SCORES_END) {
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

/* DOS94 0x302aa4. */
static int16_t Dos94_train_Draw_Train_Screen_Score(Actor* actor, Rect* frame, Rect* clip, int16_t x,
												   int16_t y, int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		int16_t revealTicks = actor->var2;
		int16_t nameX = frame->left + TRAIN_SCORE_LEFT_INSET;
		int16_t rowY;
		int16_t scoreY;
		int16_t index;
		char scoreText[TRAIN_SCORE_TEXT_CAPACITY];
		if (revealTicks > 84)
			rowY = frame->top + TRAIN_SCORE_TOP_INSET;
		else
			rowY = frame->top - revealTicks + 88;
		for (index = 0, scoreY = rowY + TRAIN_SCORE_TEXT_Y_OFFSET; index < TRAIN_SCORE_COUNT;
			 revealTicks -= TRAIN_SCORE_ROW_DELAY, rowY += 12, scoreY += 12, ++index) {
			int16_t nameColor;
			if (revealTicks < 0)
				break;
			nameColor = revealTicks + TRAIN_SCORE_COLOR_START;
			if (nameColor > TRAIN_SCORE_COLOR_END)
				nameColor = TRAIN_SCORE_COLOR_END;
			if (g_trainingScoreNames[index][0] != '\0') {
				int16_t nameColumnWidth;
				xfont_Print_Clipped_Text(g_trainingScoreNames[index], nameX, rowY, 0, nameColor);
				nameColumnWidth = 60;
				sprintf(scoreText, "Score %6ld    Level %u", (long)g_trainingScorePoints[index],
						(unsigned int)g_trainingScoreLevels[index]);
				Dos94_DrawTypewriterLine(scoreText, 1, nameX + nameColumnWidth, scoreY, revealTicks);
			}
		}
	}
	return 1;
}

/* DOS94 0x302b9c. */
static void Dos94_train_user_Welcome(Actor* actor, int time) {
	(void)time;
	if (g_trainingPresentationFrame != 0 && g_trainingPresentationFrame >= 807 &&
		g_trainingPresentationFrame < 947) {
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

/* DOS94 0x302c56. */
static int16_t Dos94_train_DrawWelcomeText(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										   int16_t refresh) {
	(void)clip;
	(void)x;
	(void)y;
	if (refresh != 0) {
		int16_t revealBudget = actor->var2;
		int16_t drawX = frame->left + TRAIN_WELCOME_LEFT_INSET;
		int16_t drawY;
		int16_t lineIndex;
		int16_t lineLength;
		if (revealBudget > 84)
			drawY = frame->top + TRAIN_WELCOME_TOP_INSET;
		else
			drawY = frame->top - revealBudget + 88;
		for (lineIndex = 0;
			 (lineLength = (int16_t)strlen(welcome_lines[lineIndex])) != 0 && revealBudget >= 0;
			 ++lineIndex, revealBudget -= lineLength >> TRAIN_WELCOME_LENGTH_SHIFT) {
			Dos94_train_DrawTypewriterLineWithSound(welcome_lines[lineIndex], 0, drawX, drawY, revealBudget);
			drawY += 12;
		}
	}
	return 1;
}

/* DOS94 0x302d0a. */
static void Dos94_train_DrawTypewriterLineWithSound(const char* text, uint16_t fontId, int16_t x, int16_t y,
													int16_t revealTicks) {
	if (revealTicks >= 0) {
		if (strlen(text) > (size_t)(TEXTEXT_CHARACTERS_PER_STEP * revealTicks)) {
			if (text[TEXTEXT_CHARACTERS_PER_STEP * revealTicks] == ' ' ||
				text[TEXTEXT_CHARACTERS_PER_STEP * revealTicks + 1] == ' ')
				train_HandleSoundAction(TRAIN_SOUND_TYPING_STOP);
			else
				train_HandleSoundAction(TRAIN_SOUND_TYPING_START);
		}
		Dos94_DrawTypewriterLine(text, fontId, x, y, TEXTEXT_CHARACTERS_PER_STEP * revealTicks);
	}
}

/* DOS94 0x302ea6. */
static void Dos94_train_RestartMissionText(void) {
	g_trainingPresentationFrame = TRAIN_MISSION_TEXT_START;
	if (xactor_Is_Actor_Visible(g_trainingMissionTextActor) != 0) {
		g_trainingMissionTextActor->var1 = 1;
		g_trainingMissionTextActor->var2 = TRAIN_MISSION_TEXT_REVEAL_ALL;
	}
	if (xactor_Is_Actor_Visible(g_trainingShipInfoActor) != 0) {
		xactor_Hide_Actor(g_trainingShipInfoActor);
		g_trainingShipInfoActor->var1 = 0;
		g_trainingShipInfoActor->var2 = 0;
	}
}

/* DOS94 0x30327e. */
static void Dos94_train_OpenMusic(ResFile* unusedResourceFile, Film* film, int* presentationFrame) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_trainingMusic.sound = xsound_Find_Gmid("rebels");
		g_trainingMusic.film = film;
		g_trainingMusic.presentationFrame = presentationFrame;
		if (g_trainingMusic.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\trmusic.lfd");
			if (musicResource == NULL)
				musicResource = xres_Open_Resource("trmusic.lfd");
			g_trainingMusic.sound = xsound_Res_Music(musicResource, "rebels");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_trainingMusic.sound);
			if (g_trainingMusic.openCount > TRAIN_MUSIC_INITIAL_LOAD_LIMIT)
				soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
								 TRAIN_MUSIC_REPEAT_LOAD_CONTROL, 0);
			else
				soundext_ScanMidi(g_trainingMusic.sound, 0, TRAIN_MUSIC_INITIAL_LOAD_BEAT, 0);
		}
		soundext_FadeVolume(g_trainingMusic.sound, TRAIN_MUSIC_OPEN_VOLUME, TRAIN_MUSIC_CLOSE_FADE_DURATION);
		xsound_Set_Sound_Keep(g_trainingMusic.sound);
		xsound_Set_Sound_User_Function(g_trainingMusic.sound, Dos94_train_user_Music);
		++g_trainingMusic.openCount;
	}
}

/* DOS94 0x3033b2. */
void Dos94_train_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		switch (xerror_Get_Landru_Exit()) {
			case XW_SCENE_TRAINING_RETURN_SHUTTLE: {
				int tick = soundext_GetMusicParam(g_trainingMusic.sound, XW_SOUND_QUERY_TICK, 0);
				soundext_JumpMidi(g_trainingMusic.sound, TRAIN_MUSIC_RETURN_GROUP, TRAIN_MUSIC_RETURN_BEAT,
								  tick);
				soundext_FadeVolume(g_trainingMusic.sound, 0, TRAIN_MUSIC_RETURN_FADE_OUT_DURATION);
				soundext_FadeVolume(g_trainingMusic.sound, TRAIN_MUSIC_RETURN_VOLUME,
									TRAIN_MUSIC_RETURN_FADE_IN_DURATION);
				break;
			}
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_260:
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_261:
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_262:
			case XW_SCENE_LEGACY_TRAINING_TRANSITION_266:
				break;
			default:
				soundext_SetPriority((intptr_t)g_trainingMusic.sound, 0);
				soundext_FadeVolume(g_trainingMusic.sound, 0, TRAIN_MUSIC_CLOSE_FADE_DURATION);
				break;
		}
		g_trainingMusic.phase = TRAIN_MUSIC_WAIT_FILM;
	}
}

/* DOS94 0x303484. */
static void Dos94_train_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	switch (g_trainingMusic.phase) {
		case TRAIN_MUSIC_WAIT_FILM:
			if (g_trainingMusic.film->cur_cel == TRAIN_MUSIC_START_CEL) {
				soundext_ClearTriggers();
				if (g_trainingMusic.openCount > TRAIN_MUSIC_INITIAL_OPEN_LIMIT)
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_REPEAT_CONTROL, 0);
				else
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_INITIAL_CONTROL, 0);
				g_trainingMusic.phase = TRAIN_MUSIC_WAIT_FIRST_FRAME;
			}
			break;
		case TRAIN_MUSIC_WAIT_FIRST_FRAME:
			if (*g_trainingMusic.presentationFrame >= TRAIN_MUSIC_FIRST_FRAME) {
				soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
								 TRAIN_MUSIC_PRESENTATION_CONTROL, 0);
				g_trainingMusic.phase = TRAIN_MUSIC_WAIT_LAST_FRAME;
			}
			break;
		case TRAIN_MUSIC_WAIT_LAST_FRAME:
			if (*g_trainingMusic.presentationFrame >= 753) {
				int beat = soundext_GetMusicParam(g_trainingMusic.sound, XW_SOUND_QUERY_BEAT, 0);
				if (beat > TRAIN_MUSIC_BEAT_THRESHOLD)
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_LATE_BEAT_CONTROL, 0);
				else
					soundext_SetHook(g_trainingMusic.sound, XW_SOUND_CONTROL_DIRECT,
									 TRAIN_MUSIC_REPEAT_CONTROL, 0);
				g_trainingMusic.phase = TRAIN_MUSIC_COMPLETE;
			}
			break;
	}
}

/* DOS94 0x301da2. Sound countdown is independent of the animation countdown. */
static void r2_update(Actor* actor, int time) {
	if (time == 1)
		train_HandleSoundAction(1);
	if (time != 0 && r2_sound_ticks != 0) {
		if (--r2_sound_ticks == 0) {
			train_HandleSoundAction((rand() & 3) + 2);
			r2_sound_ticks = (rand() & 15) + 64;
		}
	} else
		r2_sound_ticks = (rand() & 15) + 48;
	if (!xactor_Is_Actor_Visible(actor))
		return;
	if (!xactor_Is_Actor_Active(actor))
		xactor_Activate_Actor(actor);
	if (xio_Any_Button()) {
		actor->var1 = 1;
		actor->var2 = 8;
		xactor_Set_Actor_State_Speed(actor, 1, 0);
	} else if (actor->var2 != 0) {
		--actor->var2;
	} else if (actor->var1 != 0) {
		actor->var1 = 0;
		actor->var2 = rand() & 15;
		xactor_Set_Actor_State_Speed(actor, 0, 0);
	} else {
		static const int16_t speeds[8][2] = { { 0, -128 }, { 0, -128 }, { 0, -64 }, { 0, 64 },
											  { 0, 128 },  { 0, 128 },  { 1, 0 },   { -1, 0 } };
		actor->var1 = 1;
		actor->var2 = (rand() & 15) + 15;
		int speed = rand() & 7;
		xactor_Set_Actor_State_Speed(actor, speeds[speed][0], speeds[speed][1]);
	}
}

/* DOS94 0x3015d4 and 0x3017a8. */
static void draw_ship_icon(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)input;
	if (refresh) {
		xpaint_Paint_Clipped_Rect(frame, 0);
		xstyle_Style_Draw_Centered_Actor(ship_icon, frame, clip, shipext_Get_Train_Ship());
	}
}

static void draw_button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (refresh) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(input->id == 0 || input->id == 2 ? 1 : 3, frame, clip,
										button->pressed);
	}
}

static void create_navigation(void) {
	Rect frame;
	Input* input;
	xrect_Set_Rect(&frame, 38, 152, 206, 198);
	g_trainingDoorHintInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_trainingDoorHintInput, Dos94_train_idraw_DoorHint);
	xinpattr_Hide_Input(g_trainingDoorHintInput);
	g_trainingNavigationInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xrect_Set_Rect(&frame, 3, 1, 44, 45);
	input = xinput_Alloc_Input(g_trainingNavigationInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(input, draw_ship_icon);
	input->id = 0;
	for (int row = 0; row < 2; ++row) {
		xrect_Set_Rect(&frame, 48, 6, 64, 22);
		PushButton* button = xbtnpush_Alloc_Button(g_trainingNavigationInput, &frame, 0,
												   Dos94_train_iuser_Train, NULL, row * 2);
		xinpattr_Set_Input_Draw_Function(&button->header, draw_button);
		if (row)
			xinpattr_Set_Input_Allign(&button->header, 0, 2);
		xrect_Set_Rect(&frame, 4, 6, 20, 22);
		button = xbtnpush_Alloc_Button(g_trainingNavigationInput, &frame, 0, Dos94_train_iuser_Train, NULL,
									   row * 2 + 1);
		xinpattr_Set_Input_Draw_Function(&button->header, draw_button);
		xinpattr_Set_Input_Allign(&button->header, 2, row ? 2 : 0);
		xrect_Set_Rect(&frame, 66, 7, 146, 21);
		input = xinput_Alloc_Input(g_trainingNavigationInput, &frame, 0, 0);
		xinpattr_Set_Input_Update_Function(input, Dos94_train_iupdate_SelectionLabel);
		xinpattr_Set_Input_Draw_Function(input, Dos94_train_idraw_Train);
		input->id = row;
		if (row)
			xinpattr_Set_Input_Allign(input, 0, 2);
	}
}

/* DOS94 0x300000. Pilot and high-score storage use the port's shared record formats. */
void Dos94_Train(XwShellContext* shell) {
	Rect frame;
	g_trainingFocusIndex = 9;
	xio_Set_Mouse_Position(270, 98);
	g_trainingPresentationFrame = 0;
	LandruFile* scores = xfile_Open_File(LANDRU_FILE_ROOT_USER, "X-Wing Data\\train.hgh", "rb");
	if (scores) {
		for (int i = 0; i < TRAIN_SCORE_COUNT; ++i) {
			uint32_t points;
			xfile_Read_Data_From_File(scores, g_trainingScoreNames[i], TRAIN_SCORE_DISK_NAME_CAPACITY);
			xfile_Read_Long_From_File(scores, &points);
			g_trainingScorePoints[i] = points;
			xfile_Read_Word_From_File(scores, &g_trainingScoreLevels[i]);
		}
		xfile_Close_File(scores);
	}
	ResFile* missions = XwLandru_OpenMissionResource("missions.lfd");
	g_trainingLevelParagraph = xparagrp_Res_Paragraph(missions, "level");
	g_trainingTotalLevels = xparagrp_Count_Paragraph_Strings(g_trainingLevelParagraph, 0);
	xres_Close_Resource(missions);
	g_trainingResourceFile = xres_Open_Resource("train.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xview_Set_View_ZPlane(0, 60, 100);
	xrect_Set_Rect(&frame, 74, -114, 246, -9);
	xview_Set_View_Frame(1, &frame);
	xview_Set_View_ZPlane(1, 0, 50);
	xview_Disable_View_Copy(1);
	xview_Enable_View_Erase(1);
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_trainingFilm = xfilm_Res_Film(g_trainingResourceFile, "train", &frame, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_trainingFilm, shell->standardPalette);
	Actor* backdrop = xactor_Find_Actor(FOURCC_DELT, "TRAIN-A");
	if (backdrop->draw != NULL) {
		xcanvas_Set_Drawing_Canvas_Clip(&frame);
		backdrop->draw(backdrop, &frame, &frame, backdrop->x, backdrop->y, 1);
	}
	/* Retire the film-owned entry as well as the one-shot backdrop actor. */
	xfilm_Free_Film_Data_Object(g_trainingFilm, backdrop->film_entry_index);
	g_trainingBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "TRAIN-B");
	xactor_Non_Refreshable_Actor(g_trainingBackgroundActor);
	g_trainingExitDoorActor = xactor_Find_Actor(FOURCC_ANIM, "LDOOR");
	xactor_Set_Actor_User_Function(g_trainingExitDoorActor, Dos94_train_user_Door);
	xactor_Non_Refreshable_Actor(g_trainingExitDoorActor);
	g_trainingExitDoorActor->id = 0;
	g_trainingLaunchDoorActor = xactor_Find_Actor(FOURCC_ANIM, "RDOOR");
	xactor_Set_Actor_User_Function(g_trainingLaunchDoorActor, Dos94_train_user_Door);
	xactor_Non_Refreshable_Actor(g_trainingLaunchDoorActor);
	g_trainingLaunchDoorActor->id = 1;
	panel_actor = xactor_Find_Actor(FOURCC_DELT, "PANEL");
	xactor_Non_Refreshable_Actor(panel_actor);
	g_trainingScreenActor = xactor_Find_Actor(FOURCC_DELT, "SCREEN");
	xactor_Non_Refreshable_Actor(g_trainingScreenActor);
	Actor* r2 = xactor_Find_Actor(FOURCC_ANIM, "r2Plug");
	xactor_Set_Actor_User_Function(r2, r2_update);
	ResFile* icons = shipext_IsTourAvailable(4) ? xres_Open_Resource("bwing.lfd") : g_trainingResourceFile;
	ship_icon = xactanim_Res_Anim_Actor(icons, "ships", &frame, 0, 0, 60);
	if (icons != g_trainingResourceFile)
		xres_Close_Resource(icons);
	xactor_Set_Actor_Time(ship_icon, 0, 0);
	g_trainingStarsLeftActor = xactdelt_Res_Delta_Actor(shell->resourceFile, "stars-4", &frame, 0, 0, 50);
	g_trainingStarsRightActor = xactdelt_Res_Delta_Actor(shell->resourceFile, "stars-4", &frame, 320, 0, 50);
	xactor_Set_Actor_User_Function(g_trainingStarsLeftActor, Dos94_train_user_Stars);
	xactor_Set_Actor_User_Function(g_trainingStarsRightActor, Dos94_train_user_Stars);
	g_trainingIncomLogoActor = xactdelt_Res_Delta_Actor(g_trainingResourceFile, "incom", &frame, 0, 0, 10);
	g_trainingRebelLogoActor = xactdelt_Res_Delta_Actor(g_trainingResourceFile, "rebel", &frame, 0, 0, 10);
	xactor_Set_Actor_Time(g_trainingIncomLogoActor, 0, 0);
	xactor_Set_Actor_Time(g_trainingRebelLogoActor, 0, 0);
	g_trainingShipInfoActor = xactcust_Alloc_Custom_Actor(0, &frame, 74, 19, 0);
	xactor_Set_Actor_Update_Function(g_trainingShipInfoActor, Dos94_train_UpdateShipInfoAnimation);
	xactor_Set_Actor_User_Function(g_trainingShipInfoActor, Dos94_train_user_ShipInfo);
	xactor_Set_Actor_Draw_Function(g_trainingShipInfoActor, Dos94_train_DrawShipInfoPanel);
	g_trainingShipInfoActor->var1 = 0;
	xrect_Set_Rect(&frame, 0, 0, 173, 109);
	g_trainingHighScoresActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_trainingHighScoresActor, Dos94_train_user_HighScores);
	xactor_Set_Actor_Draw_Function(g_trainingHighScoresActor, Dos94_train_Draw_Train_Screen_Score);
	g_trainingHighScoresActor->var1 = 0;
	g_trainingWelcomeActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_trainingWelcomeActor, Dos94_train_user_Welcome);
	xactor_Set_Actor_Draw_Function(g_trainingWelcomeActor, Dos94_train_DrawWelcomeText);
	g_trainingWelcomeActor->var1 = 0;
	g_trainingMissionTextActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, 0);
	xactor_Set_Actor_User_Function(g_trainingMissionTextActor, Dos94_train_user_MissionText);
	xactor_Set_Actor_Draw_Function(g_trainingMissionTextActor, Dos94_train_Draw_Train_Screen_Mission);
	g_trainingMissionTextActor->var1 = 0;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_trainingRootInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xrect_Set_Rect(&frame, 74, 19, 246, 127);
	g_trainingMonitorInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_trainingMonitorInput, XwCombat_UpdateMonitor);
	xinpattr_Set_Input_User_Function(g_trainingMonitorInput, Dos94_train_iuser_Train_Screen);
	xinpattr_Hide_Input(g_trainingMonitorInput);
	xrect_Set_Rect(&frame, 0, 24, 46, 146);
	g_trainingExitDoorInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_trainingExitDoorInput, Dos94_train_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_trainingExitDoorInput, Dos94_train_iuser_Door);
	g_trainingExitDoorInput->mouseUsage = allInput;
	g_trainingExitDoorInput->id = 0;
	xrect_Set_Rect(&frame, 274, 24, 320, 146);
	g_trainingLaunchDoorInput = xinput_Alloc_Input(g_trainingRootInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_trainingLaunchDoorInput, Dos94_train_iupdate_Door);
	xinpattr_Set_Input_User_Function(g_trainingLaunchDoorInput, Dos94_train_iuser_Door);
	g_trainingLaunchDoorInput->mouseUsage = allInput;
	g_trainingLaunchDoorInput->id = 1;
	create_navigation();
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_trainingAWingActor = xactanim_Res_Anim_Actor(g_trainingResourceFile, "awing", &frame, 1, 8, 10);
	xactor_Set_Actor_User_Function(g_trainingAWingActor, Dos94_train_user_AWingPresentation);
	g_trainingAWingActor->var1 = 3;
	g_trainingXWingActor = xactanim_Res_Anim_Actor(g_trainingResourceFile, "xwing", &frame, 1, 2, 10);
	xactor_Set_Actor_User_Function(g_trainingXWingActor, Dos94_train_user_XWingPresentation);
	g_trainingXWingActor->var1 = 4;
	g_trainingYWingActor = xactanim_Res_Anim_Actor(g_trainingResourceFile, "ywing", &frame, 19, 14, 10);
	xactor_Set_Actor_User_Function(g_trainingYWingActor, Dos94_train_user_YWingPresentation);
	g_trainingYWingActor->var1 = 7;
	train_LoadPilotProgress(g_RegisterShellPilot.name);
	g_trainingAvailableLevels = g_trainingPilot.trainingLevelProgress[shipext_Get_Train_Ship()] + 1;
	if (g_trainingAvailableLevels > g_trainingTotalLevels)
		g_trainingAvailableLevels = g_trainingTotalLevels;
	xio_Set_Key_Buttons();
	Dos94_train_OpenMusic(g_trainingResourceFile, g_trainingFilm, &g_trainingPresentationFrame);
	train_LoadSoundEffects();
	xview_Set_View_Update_Function(Dos94_train_end_Train_View);
	XwTrain_RunView();
}
