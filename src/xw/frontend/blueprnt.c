#include "xw/frontend/blueprnt.h"

#include "xw/landru_config.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/blueprint_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/blueprint_task.h"
#endif
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include <landru/actanim.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memptr.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/res.h>
#include <landru/view.h>
#include <landru/viewadd.h>

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D0538
char g_blueprintShipAnimationNames[BP_SHIP_ANIMATION_COUNT][BP_SHIP_ANIMATION_NAME_CAPACITY] = {
	"bp_xwing", "bp_ywing", "bp_awing", "bp_bwing", "bp_intr",  "bp_fight", "bp_bomb", "bp_gboat", "calspin",
	"bp_nebl",  "bp_corv",  "bp_cont",  "bp_frght", "bp_sdest", "bp_tyd",   "bp_tug",  "bp_idict", "bp_tuna"
};

// GLOBAL: XW 0x4D0760
int16_t g_blueprintFocusX[BP_FOCUS_COUNT] = { 32, 102, 214, 310, 32, 102, 214, 310 };

// GLOBAL: XW 0x4D0770
int16_t g_blueprintFocusY[BP_FOCUS_COUNT] = { 120, 170, 170, 120, 120, 188, 188, 120 };

// GLOBAL: XW 0x4D07E0
const char g_blueprintUnavailableLabel[BP_UNAVAILABLE_LABEL_CAPACITY] = "--";

// GLOBAL: XW 0x4D0A58
const char* g_blueprintWaitingMusicName = "waiting";

// GLOBAL: XW 0x4D0A5C
const char* g_blueprintMusicResourceName = "bpmusic.lfd";

// GLOBAL: XW 0x4D0A60
const char* g_blueprintHallMarchMusicName = "halmarch";

// GLOBAL: XW 0x4F4BE0
LandruHandle g_blueprintText = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F4BE4
Actor* g_blueprintPreviousShipButtonActor;

// GLOBAL: XW 0x4F4BE8
Actor* g_blueprintNextShipButtonActor;

// GLOBAL: XW 0x4F4BEC
ResFile* g_blueprintResourceFile = NULL;

// GLOBAL: XW 0x4F4BF0
Actor* g_blueprintNextComponentButtonActor;

// GLOBAL: XW 0x4F4BF4
Actor* g_blueprintRightDoorActor;

// GLOBAL: XW 0x4F4BF8
ResFile* g_blueprintShipResourceFile = NULL;

// GLOBAL: XW 0x4F4BFC
Input* g_blueprintRightDoorInput = NULL;

// GLOBAL: XW 0x4F4C00
int16_t g_blueprintLoadedShipIndex = 0;

// GLOBAL: XW 0x4F4C04
int16_t g_blueprintSelectedComponent = 0;

// GLOBAL: XW 0x4F4C08
Actor* g_blueprintShipActor = NULL;

// GLOBAL: XW 0x4F4C0C
Actor* g_blueprintPreviousComponentButtonActor;

// GLOBAL: XW 0x4F4C10
Input* g_blueprintWorldInput = NULL;

// GLOBAL: XW 0x4F4C14
int16_t g_blueprintPhaseTick = 0;

// GLOBAL: XW 0x4F4C18
Input* g_blueprintDoorHintInput;

// GLOBAL: XW 0x4F4C1C
XwBlueprintDisplayPhase g_blueprintDisplayPhase = 0;

// GLOBAL: XW 0x4F4C20
int g_blueprintTextRevealTick = 0;

// GLOBAL: XW 0x4F4C24
Actor* g_blueprintBaseHologramActor = NULL;

// GLOBAL: XW 0x4F4C28
Actor* g_blueprintLeftDoorActor;

// GLOBAL: XW 0x4F4C2C
Film* g_blueprintFilm = NULL;

// GLOBAL: XW 0x4F4C30
Input* g_blueprintControlsInput;

// GLOBAL: XW 0x4F4C34
Input* g_blueprintLeftDoorInput = NULL;

// GLOBAL: XW 0x4F4C38
int16_t g_blueprintDetailedShipCount = 0;

// GLOBAL: XW 0x4F4C3C
int16_t g_blueprintDisplayedComponent = 0;

// GLOBAL: XW 0x4F4C40
int16_t g_blueprintSelectedShip = 0;

// GLOBAL: XW 0x4F4C44
int16_t g_blueprintDisplayedShip = 0;

// GLOBAL: XW 0x4F4C48
int16_t g_blueprintLegacyWord = 0;

// GLOBAL: XW 0x4F4C4C
int16_t g_blueprintFocusIndex = 0;

// GLOBAL: XW 0x4F5380
Sound* g_blueprintWaitingMusic = NULL;

// GLOBAL: XW 0x4F5384
Sound* g_blueprintHallMarchMusic = NULL;

// GLOBAL: XW 0x4F5388
Film* g_blueprintMusicFilm = NULL;

// GLOBAL: XW 0x4F5390
uint8_t g_blueprintColorTable[BP_COLOR_TABLE_SIZE] = { 0 };

// FUNCTION: XW 0x435370
XwShellSceneResult blueprnt_Blueprint(struct XwShellContext* context) {
	ResFile* resource;
	PushButton* previousShipButton;
	PushButton* nextShipButton;
	PushButton* previousComponentButton;
	PushButton* nextComponentButton;
	Input* shipLabel;
	Input* componentLabel;
	Rect frame;
	g_blueprintFocusIndex = BP_INITIAL_FOCUS;
	xio_Set_Mouse_Position(BP_INITIAL_MOUSE_X, BP_INITIAL_MOUSE_Y);
	g_blueprintDetailedShipCount = BP_DETAILED_SHIP_COUNT;
	g_blueprintShipResourceFile = xres_Open_Resource(":X-Wing Data\\RESOURCE\\ships640.lfd");
	if (g_blueprintShipResourceFile == NULL)
		g_blueprintShipResourceFile = xres_Open_Resource("ships640.lfd");
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\bp640.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource("bp640.lfd");
	g_blueprintResourceFile = resource;
	g_blueprintText = xparagrp_Res_Paragraph(resource, "bptxt640");
	blueprnt_LoadColorTable(resource);
	xrect_Set_Rect(&frame, 0, 0, BP_ANIMATION_WIDTH, BP_ANIMATION_HEIGHT);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	g_blueprintFilm = xfilm_Res_Film(resource, "bprint_5", &frame, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_blueprintFilm, context->standardPalette);
	g_blueprintBaseHologramActor = xactor_Alloc_Actor(0);
	xactor_Set_Actor_Frame(g_blueprintBaseHologramActor, &frame);
	xactor_Set_Actor_Bounds(g_blueprintBaseHologramActor, &frame);
	xactor_Set_Actor_User_Function(g_blueprintBaseHologramActor, blueprnt_UpdateHologramActor);
	xactor_Set_Actor_Draw_Function(g_blueprintBaseHologramActor, blueprnt_DrawHologramActor);
	xactor_Show_Actor(g_blueprintBaseHologramActor);
	g_blueprintBaseHologramActor->id = 0;
	xactor_Add_Actor_To_System(g_blueprintBaseHologramActor);
	g_blueprintLeftDoorActor = xactor_Find_Actor(FOURCC_ANIM, "bp_doorl");
	xactor_Set_Actor_User_Function(g_blueprintLeftDoorActor, blueprnt_user_Blueprint_Door);
	g_blueprintLeftDoorActor->id = BP_LEFT_DOOR;
	g_blueprintRightDoorActor = xactor_Find_Actor(FOURCC_ANIM, "bp_doorr");
	xactor_Set_Actor_User_Function(g_blueprintRightDoorActor, blueprnt_user_Blueprint_Door);
	g_blueprintRightDoorActor->id = BP_RIGHT_DOOR;
	g_blueprintLoadedShipIndex = BP_NO_LOADED_SHIP;
	g_blueprintDisplayedShip = 0;
	g_blueprintSelectedShip = 0;
	g_blueprintDisplayedComponent = 0;
	g_blueprintSelectedComponent = 0;
	g_blueprintLegacyWord = 0;
	g_blueprintDisplayPhase = BP_INITIAL_DELAY;
	g_blueprintPhaseTick = 0;
	g_blueprintTextRevealTick = 0;
	xrect_Set_Rect(&frame, 0, 0, BP_ANIMATION_WIDTH, BP_ANIMATION_HEIGHT);
	g_blueprintWorldInput = xinput_Alloc_Input(NULL, &frame, 0, 0);
	xinpattr_Refreshable_Input(g_blueprintWorldInput);
	g_blueprintControlsInput = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
	xrect_Set_Rect(&frame, 0, BP_DOOR_TOP, BP_LEFT_DOOR_RIGHT, BP_DOOR_BOTTOM);
	g_blueprintLeftDoorInput = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_blueprintLeftDoorInput, blueprnt_iupdate_Blueprint_Door);
	xinpattr_Set_Input_User_Function(g_blueprintLeftDoorInput, blueprnt_iuser_Blueprint_Door);
	g_blueprintLeftDoorInput->mouseUsage = allInput;
	g_blueprintLeftDoorInput->id = BP_LEFT_DOOR;
	xrect_Set_Rect(&frame, BP_RIGHT_DOOR_LEFT, BP_DOOR_TOP, BP_ANIMATION_WIDTH, BP_DOOR_BOTTOM);
	g_blueprintRightDoorInput = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_blueprintRightDoorInput, blueprnt_iupdate_Blueprint_Door);
	xinpattr_Set_Input_User_Function(g_blueprintRightDoorInput, blueprnt_iuser_Blueprint_Door);
	g_blueprintRightDoorInput->mouseUsage = allInput;
	g_blueprintRightDoorInput->id = BP_RIGHT_DOOR;
	xrect_Set_Rect(&frame, BP_LABEL_LEFT, BP_SHIP_LABEL_TOP, BP_SHIP_LABEL_RIGHT, BP_SHIP_LABEL_BOTTOM);
	g_blueprintDoorHintInput = xinput_Alloc_Input(g_blueprintWorldInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_blueprintDoorHintInput, blueprnt_DrawDoorHint);
	xinpattr_Hide_Input(g_blueprintDoorHintInput);
	xrect_Set_Rect(&frame, BP_PREVIOUS_LEFT, BP_SHIP_BUTTON_TOP, BP_PREVIOUS_RIGHT, BP_SHIP_BUTTON_BOTTOM);
	previousShipButton = xbtnpush_Alloc_Button(g_blueprintControlsInput, &frame, 0, blueprnt_iuser_Blueprint,
											   NULL, BP_PREVIOUS_SHIP);
	xinpattr_Set_Input_Draw_Function(&previousShipButton->header, XwBlueprint_DrawNavigationButton);
	g_blueprintPreviousShipButtonActor = xactor_Find_Actor(FOURCC_ANIM, "bp_but00");
	xrect_Set_Rect(&frame, BP_NEXT_LEFT, BP_SHIP_BUTTON_TOP, BP_NEXT_SHIP_RIGHT, BP_SHIP_BUTTON_BOTTOM);
	nextShipButton = xbtnpush_Alloc_Button(g_blueprintControlsInput, &frame, 0, blueprnt_iuser_Blueprint,
										   NULL, BP_NEXT_SHIP);
	xinpattr_Set_Input_Draw_Function(&nextShipButton->header, XwBlueprint_DrawNavigationButton);
	g_blueprintNextShipButtonActor = xactor_Find_Actor(FOURCC_ANIM, "bp_but02");
	xrect_Set_Rect(&frame, BP_LABEL_LEFT, BP_SHIP_LABEL_TOP, BP_SHIP_LABEL_RIGHT, BP_SHIP_LABEL_BOTTOM);
	shipLabel = xinput_Alloc_Input(g_blueprintControlsInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(shipLabel, blueprnt_draw_Blueprint_Text);
	shipLabel->id = BP_SHIP_NAME_INPUT;
	xrect_Set_Rect(&frame, BP_PREVIOUS_LEFT, BP_SHIP_BUTTON_BOTTOM, BP_PREVIOUS_RIGHT,
				   BP_COMPONENT_BUTTON_BOTTOM);
	previousComponentButton = xbtnpush_Alloc_Button(g_blueprintControlsInput, &frame, 0,
													blueprnt_iuser_Blueprint, NULL, BP_PREVIOUS_COMPONENT);
	xinpattr_Set_Input_Draw_Function(&previousComponentButton->header, XwBlueprint_DrawNavigationButton);
	g_blueprintPreviousComponentButtonActor = xactor_Find_Actor(FOURCC_ANIM, "bp_but01");
	xrect_Set_Rect(&frame, BP_NEXT_LEFT, BP_SHIP_BUTTON_BOTTOM, BP_NEXT_COMPONENT_RIGHT,
				   BP_COMPONENT_BUTTON_BOTTOM);
	nextComponentButton = xbtnpush_Alloc_Button(g_blueprintControlsInput, &frame, 0, blueprnt_iuser_Blueprint,
												NULL, BP_NEXT_COMPONENT);
	xinpattr_Set_Input_Draw_Function(&nextComponentButton->header, XwBlueprint_DrawNavigationButton);
	g_blueprintNextComponentButtonActor = xactor_Find_Actor(FOURCC_ANIM, "bp_but03");
	xrect_Set_Rect(&frame, BP_LABEL_LEFT, BP_COMPONENT_LABEL_TOP, BP_COMPONENT_LABEL_RIGHT,
				   BP_COMPONENT_LABEL_BOTTOM);
	componentLabel = xinput_Alloc_Input(g_blueprintControlsInput, &frame, 0, 0);
	xinpattr_Set_Input_Draw_Function(componentLabel, blueprnt_draw_Blueprint_Text);
	componentLabel->id = BP_COMPONENT_NAME_INPUT;
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(blueprnt_end_Blueprint_View);
	blueprnt_OpenMusic(resource, g_blueprintFilm);
	blueprnt_LoadUiSounds();
	FrontendAudio_PlayFile("XwingCD\\music\\filmtech.wav", 1);
#ifdef XW_MODERN
	XwBlueprint_RunView(resource);
#else
	j_xviewadd_Handle_View();
	blueprnt_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xio_Clear_Key_Buttons();
	if ((uint16_t)xcursor_Is_Cursor_Visible())
		xcursor_Hide_Cursor();
	xparagrp_Free_Paragraph(g_blueprintText);
	if (g_blueprintShipActor != NULL) {
		xactor_Free_Actor_From_System(g_blueprintShipActor);
		xactor_Free_Actor_Data(g_blueprintShipActor);
		g_blueprintShipActor = NULL;
	}
	xres_Close_Resource(g_blueprintShipResourceFile);
	xres_Close_Resource(resource);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4359F0
void blueprnt_end_Blueprint_View(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0 && shellext_MoveGridFocus(&g_blueprintFocusIndex, g_blueprintFocusX, g_blueprintFocusY,
										   BP_FOCUS_ROWS, BP_FOCUS_COLUMNS, key) != 0) {
		xio_Set_Mouse_Position(g_blueprintFocusX[g_blueprintFocusIndex],
							   g_blueprintFocusY[g_blueprintFocusIndex]);
		xio_Get_Key();
	}
}

// FUNCTION: XW 0x435A60
int16_t blueprnt_iupdate_Blueprint_Door(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										int rightEvent, int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0) {
		return 0;
	}
	if (leftEvent == 3 || rightEvent == 3) {
		input->var1 = 1;
		if (input->id != 0) {
			input->var2 = XW_SCENE_FILM_ROOM;
			shipext_Set_Mission_Outcome(16);
		} else {
			input->var2 = XW_SCENE_CONCOURSE;
		}
	} else {
		input->var1 = 2;
	}
	if (input->id != 0) {
		g_blueprintRightDoorActor->var1 = 1;
	} else {
		g_blueprintLeftDoorActor->var1 = 1;
	}
	return 1;
}

// FUNCTION: XW 0x435AE0
void blueprnt_iuser_Blueprint_Door(Input* input, int time) {
	(void)time;
	switch (input->var1) {
		case 0:
			if (xinpattr_Is_Input_Visible(g_blueprintDoorHintInput) &&
				input->id == g_blueprintDoorHintInput->var1) {
				xinpattr_Show_Input(g_blueprintControlsInput);
				xinpattr_Hide_Input(g_blueprintDoorHintInput);
				xinpattr_Refresh_Input(g_blueprintControlsInput);
			}
			break;
		case 1:
			xerror_Set_Landru_Exit(input->var2);
			break;
		case 2:
			if (xinpattr_Is_Input_Visible(g_blueprintControlsInput)) {
				xinpattr_Hide_Input(g_blueprintControlsInput);
				xinpattr_Show_Input(g_blueprintDoorHintInput);
				g_blueprintDoorHintInput->var1 = input->id;
				xinpattr_Refresh_Input(g_blueprintDoorHintInput);
			}
			input->var1 = 0;
			break;
	}
}

// FUNCTION: XW 0x435BB0
void blueprnt_DrawDoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	if (refresh != 0) {
		if (input->var1 != 0) {
			xrect_Offset_Rect(frame, BP_HINT_SHADOW_OFFSET, BP_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Film Room", frame, BP_HINT_FONT, BP_HINT_SHADOW_COLOR);
			xrect_Offset_Rect(frame, -BP_HINT_SHADOW_OFFSET, -BP_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Enter Film Room", frame, BP_HINT_FONT, BP_HINT_TEXT_COLOR);
		} else {
			xrect_Offset_Rect(frame, BP_HINT_SHADOW_OFFSET, BP_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Exit Tech Room", frame, BP_HINT_FONT, BP_HINT_SHADOW_COLOR);
			xrect_Offset_Rect(frame, -BP_HINT_SHADOW_OFFSET, -BP_HINT_SHADOW_OFFSET);
			xfont_Print_Centered_Text("Exit Tech Room", frame, BP_HINT_FONT, BP_HINT_TEXT_COLOR);
		}
	}
}

// FUNCTION: XW 0x435C50
void blueprnt_iuser_Blueprint(Input* input, int time) {
	int16_t shipChanged = 0;
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		switch (input->id) {
			case BP_PREVIOUS_SHIP:
				if (g_blueprintSelectedShip != 0)
					--g_blueprintSelectedShip;
				else
					g_blueprintSelectedShip =
						xparagrp_Count_Paragraph_Strings(g_blueprintText, BP_SHIP_NAME_PARAGRAPH) - 1;
				shipChanged = 1;
				g_blueprintSelectedComponent = 0;
				break;
			case BP_NEXT_SHIP:
				if (g_blueprintSelectedShip ==
					xparagrp_Count_Paragraph_Strings(g_blueprintText, BP_SHIP_NAME_PARAGRAPH) - 1)
					g_blueprintSelectedShip = 0;
				else
					++g_blueprintSelectedShip;
				shipChanged = 1;
				g_blueprintSelectedComponent = 0;
				break;
			case BP_PREVIOUS_COMPONENT:
				if (g_blueprintSelectedShip < g_blueprintDetailedShipCount) {
					if (g_blueprintSelectedComponent != 0)
						--g_blueprintSelectedComponent;
					else
						g_blueprintSelectedComponent =
							xparagrp_Count_Paragraph_Strings(
								g_blueprintText, g_blueprintSelectedShip + BP_COMPONENT_PARAGRAPH_BASE) /
								BP_COMPONENT_STRING_COUNT -
							1;
				}
				break;
			case BP_NEXT_COMPONENT:
				if (g_blueprintSelectedShip < g_blueprintDetailedShipCount) {
					if (g_blueprintSelectedComponent ==
						xparagrp_Count_Paragraph_Strings(g_blueprintText, g_blueprintSelectedShip +
																			  BP_COMPONENT_PARAGRAPH_BASE) /
								BP_COMPONENT_STRING_COUNT -
							1)
						g_blueprintSelectedComponent = 0;
					else
						++g_blueprintSelectedComponent;
				}
				break;
		}
		xinpattr_Refresh_Input(g_blueprintControlsInput);
		if (shipChanged != 0) {
			XwBlueprintDisplayPhase phase = g_blueprintDisplayPhase;
			int reuseLoadedShip = 1;
			if (g_blueprintDisplayedShip != g_blueprintSelectedShip &&
				(phase == BP_DISPLAY || phase == BP_REVEAL)) {
				phase = BP_LOADING;
				reuseLoadedShip = 0;
				g_blueprintDisplayPhase = BP_LOADING;
				g_blueprintPhaseTick = 0;
			}
			if (reuseLoadedShip != 0) {
				if (phase == BP_DISPLAY) {
					phase = BP_REVEAL;
					g_blueprintPhaseTick = 0;
					g_blueprintDisplayPhase = BP_REVEAL;
				}
				if (phase == BP_REVEAL)
					g_blueprintPhaseTick = 0;
			}
		}
		g_blueprintTextRevealTick = 0;
	}
}

// FUNCTION: XW 0x435E70
void blueprnt_DrawNavigationButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	Actor* buttonActor;

	if (refresh != 0) {
		switch (button->header.id) {
			case 0:
				buttonActor = g_blueprintPreviousShipButtonActor;
				break;
			case 1:
				buttonActor = g_blueprintNextShipButtonActor;
				break;
			case 2:
				buttonActor = g_blueprintPreviousComponentButtonActor;
				break;
			case 3:
				buttonActor = g_blueprintNextComponentButtonActor;
				break;
			default:
				buttonActor = NULL;
				break;
		}
		if (buttonActor != NULL) {
			xactor_Set_Actor_State(buttonActor, button->pressed, 0);
			xactanim_Draw_Anim_Actor(buttonActor, frame, clip, 0, 0, refresh);
		}
	}
}

// FUNCTION: XW 0x435EF0
void blueprnt_draw_Blueprint_Text(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char label[BP_LABEL_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, BP_LABEL_BACKGROUND_COLOR);
		if (input->id == BP_SHIP_NAME_INPUT) {
			xparagrp_Get_Paragraph_String(g_blueprintText, label, BP_SHIP_NAME_PARAGRAPH,
										  g_blueprintSelectedShip);
		} else if (g_blueprintSelectedShip >= g_blueprintDetailedShipCount) {
			memcpy(label, g_blueprintUnavailableLabel, sizeof(g_blueprintUnavailableLabel));
		} else {
			xparagrp_Get_Paragraph_String(g_blueprintText, label,
										  g_blueprintSelectedShip + BP_COMPONENT_PARAGRAPH_BASE,
										  g_blueprintSelectedComponent * BP_COMPONENT_STRING_COUNT);
		}
		xfont_Print_Centered_Text(label, frame, BP_LABEL_FONT, BP_LABEL_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x435FA0
void blueprnt_user_Blueprint_Door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	if (time == 0)
		actor->var1 = 0;
	if (time >= BP_DOOR_ANIMATION_START_TIME) {
		if (actor->var1 == 0) {
			if (actor->state == 1)
				blueprnt_HandleSoundAction(BP_SOUND_DOOR_CLOSE);
			if (actor->state != 0)
				xactor_Set_Actor_State(actor, actor->state - 1, 0);
			else
				needsRefresh = 0;
		} else {
			if (actor->state == 0)
				blueprnt_HandleSoundAction(BP_SOUND_DOOR_OPEN);
			if (actor->state != actor->arraySize - 1)
				xactor_Set_Actor_State(actor, actor->state + 1, 0);
			else
				needsRefresh = 0;
			actor->var1 = 0;
		}
	}
	if (needsRefresh != 0)
		xactor_Refresh_Actor(actor);
}

// FUNCTION: XW 0x436040
void blueprnt_UpdateHologramActor(Actor* actor, int time) {
	(void)time;
	switch (g_blueprintDisplayPhase) {
		case BP_IDLE:
			break;
		case BP_INITIAL_DELAY:
			if (g_blueprintPhaseTick == BP_LOADING_TICKS) {
				g_blueprintDisplayPhase = BP_LOADING;
				g_blueprintPhaseTick = 0;
			} else {
				++g_blueprintPhaseTick;
			}
			break;
		case BP_LOADING:
			if (++g_blueprintPhaseTick == BP_LOADING_TICKS) {
				g_blueprintDisplayPhase = BP_REVEAL;
				g_blueprintPhaseTick = 0;
			} else if (g_blueprintLoadedShipIndex != g_blueprintSelectedShip) {
				blueprnt_LoadSelectedShipAnimation();
				g_blueprintLoadedShipIndex = g_blueprintSelectedShip;
			}
			break;
		case BP_REVEAL:
			if (++g_blueprintPhaseTick == BP_REVEAL_TICKS) {
				g_blueprintDisplayPhase = BP_DISPLAY;
				g_blueprintPhaseTick = 0;
			}
			g_blueprintDisplayedShip = g_blueprintSelectedShip;
			g_blueprintDisplayedComponent = g_blueprintSelectedComponent;
			break;
		case BP_DISPLAY:
			g_blueprintPhaseTick = g_blueprintTextRevealTick >> 1;
			if (g_blueprintPhaseTick > BP_DISPLAY_PHASE_TICK_LIMIT)
				g_blueprintPhaseTick = BP_DISPLAY_PHASE_TICK_LIMIT;
			actor->var2 = 1;
			g_blueprintDisplayedShip = g_blueprintSelectedShip;
			++g_blueprintTextRevealTick;
			g_blueprintDisplayedComponent = g_blueprintSelectedComponent;
			break;
		default:
			break;
	}
	if (actor->var2 == 1) {
		int16_t state = actor->state;
		if (state < actor->arraySize - 1)
			xactor_Set_Actor_State(actor, state + 1, 0);
		else
			actor->state = 0;
	}
}

// FUNCTION: XW 0x4361B0
void blueprnt_LoadSelectedShipAnimation(void) {
	Rect rect;
	Actor* temporaryActor;
	if (g_blueprintBaseHologramActor != NULL) {
		xactor_Set_Actor_User_Function(g_blueprintBaseHologramActor, NULL);
		xactor_Set_Actor_Draw_Function(g_blueprintBaseHologramActor, NULL);
	}
	xrect_Set_Rect(&rect, 0, 0, BP_ANIMATION_WIDTH, BP_ANIMATION_HEIGHT);
	temporaryActor = xactanim_Res_Anim_Actor(
		g_blueprintShipResourceFile, g_blueprintShipAnimationNames[g_blueprintSelectedShip], &rect, 0, 0, 0);
	if (temporaryActor != NULL) {
		if (g_blueprintShipActor == NULL) {
			g_blueprintShipActor = temporaryActor;
		} else {
			if (g_blueprintShipActor->data != LANDRU_NULL_HANDLE) {
				xmemhdl_Free_Handle(g_blueprintShipActor->data);
				g_blueprintShipActor->data = LANDRU_NULL_HANDLE;
			}
			if (g_blueprintShipActor->array != LANDRU_NULL_HANDLE) {
				xmemhdl_Free_Handle(g_blueprintShipActor->array);
				g_blueprintShipActor->array = LANDRU_NULL_HANDLE;
			}
			xactor_Free_Actor_From_System(temporaryActor);
			memcpy(g_blueprintShipActor->res_name, temporaryActor->res_name,
				   sizeof(g_blueprintShipActor->res_name));
			g_blueprintShipActor->start = temporaryActor->start;
			g_blueprintShipActor->stop = temporaryActor->stop;
			g_blueprintShipActor->frame = temporaryActor->frame;
			g_blueprintShipActor->frame_v = temporaryActor->frame_v;
			g_blueprintShipActor->bounds = temporaryActor->bounds;
			g_blueprintShipActor->x = temporaryActor->x;
			g_blueprintShipActor->y = temporaryActor->y;
			g_blueprintShipActor->xf = temporaryActor->xf;
			g_blueprintShipActor->yf = temporaryActor->yf;
			g_blueprintShipActor->xv = temporaryActor->xv;
			g_blueprintShipActor->yv = temporaryActor->yv;
			g_blueprintShipActor->xvf = temporaryActor->xvf;
			g_blueprintShipActor->yvf = temporaryActor->yvf;
			g_blueprintShipActor->w = temporaryActor->w;
			g_blueprintShipActor->h = temporaryActor->h;
			g_blueprintShipActor->zplane = temporaryActor->zplane;
			g_blueprintShipActor->flags = temporaryActor->flags;
			g_blueprintShipActor->id = temporaryActor->id;
			g_blueprintShipActor->state = temporaryActor->state;
			g_blueprintShipActor->state_f = temporaryActor->state_f;
			g_blueprintShipActor->state_v = temporaryActor->state_v;
			g_blueprintShipActor->state_vf = temporaryActor->state_vf;
			g_blueprintShipActor->foreColor = temporaryActor->foreColor;
			g_blueprintShipActor->backColor = temporaryActor->backColor;
			g_blueprintShipActor->xscale = temporaryActor->xscale;
			g_blueprintShipActor->yscale = temporaryActor->yscale;
			g_blueprintShipActor->var1 = temporaryActor->var1;
			g_blueprintShipActor->var2 = temporaryActor->var2;
			g_blueprintShipActor->varptr = temporaryActor->varptr;
			g_blueprintShipActor->varhdl = temporaryActor->varhdl;
			g_blueprintShipActor->data = temporaryActor->data;
			g_blueprintShipActor->array = temporaryActor->array;
			g_blueprintShipActor->arraySize = temporaryActor->arraySize;
			xmemptr_Free_System_Pointer(temporaryActor);
		}
		xactor_Set_Actor_User_Function(g_blueprintShipActor, blueprnt_UpdateHologramActor);
		xactor_Set_Actor_Draw_Function(g_blueprintShipActor, blueprnt_DrawHologramActor);
		xactor_Show_Actor(g_blueprintShipActor);
		g_blueprintShipActor->id = 0;
	} else {
		xactor_Set_Actor_User_Function(g_blueprintBaseHologramActor, blueprnt_UpdateHologramActor);
		xactor_Set_Actor_Draw_Function(g_blueprintBaseHologramActor, blueprnt_DrawHologramActor);
	}
}

// FUNCTION: XW 0x436500
int16_t blueprnt_DrawHologramActor(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh) {
	int16_t drawHologram;
	int16_t drawRevealEdges;
	int16_t drawComponentText;
	Rect apertureClip;
	Rect drawRect;
	char description[BP_DESCRIPTION_CAPACITY];
	if (refresh != 0) {
		drawHologram = 0;
		drawRevealEdges = 0;
		drawComponentText = 0;
		switch (g_blueprintDisplayPhase) {
			case BP_IDLE:
				break;
			case BP_INITIAL_DELAY:
				if (g_blueprintPhaseTick == BP_LOADING_TICKS) {
					g_blueprintDisplayPhase = BP_LOADING;
					g_blueprintPhaseTick = 0;
				} else
					++g_blueprintPhaseTick;
				break;
			case BP_LOADING:
				xrect_Copy_Rect(&drawRect, frame);
				xrect_Inset_Rect(&drawRect, BP_LOADING_INSET_X, BP_LOADING_INSET_Y);
				xpaint_Frame_Clipped_Rect(&drawRect, BP_DESCRIPTION_HEADING_COLOR);
				xfont_Enable_FontID_Shadow(BP_LABEL_FONT);
				xfont_Print_Centered_Text("Accessing Holograms", frame, BP_LABEL_FONT,
										  BP_DESCRIPTION_TEXT_COLOR);
				xfont_Disable_FontID_Shadow(BP_LABEL_FONT);
				break;
			case BP_REVEAL:
				xrect_Set_Rect(&drawRect, BP_APERTURE_LEFT, BP_APERTURE_REVEAL_TOP, BP_APERTURE_RIGHT,
							   BP_APERTURE_REVEAL_BOTTOM);
				if (g_blueprintPhaseTick > BP_APERTURE_DELAY_TICKS)
					xrect_Inset_Rect(&drawRect, 0,
									 BP_APERTURE_GROWTH * BP_APERTURE_DELAY_TICKS -
										 (g_blueprintPhaseTick << BP_APERTURE_GROWTH_SHIFT));
				if (drawRect.top < BP_APERTURE_TOP)
					drawRect.top = BP_APERTURE_TOP;
				if (drawRect.bottom > BP_APERTURE_BOTTOM)
					drawRect.bottom = BP_APERTURE_BOTTOM;
				drawRevealEdges = 1;
				drawHologram = 1;
				break;
			case BP_DISPLAY:
				xrect_Set_Rect(&drawRect, BP_APERTURE_LEFT, BP_APERTURE_TOP, BP_APERTURE_RIGHT,
							   BP_APERTURE_BOTTOM);
				drawHologram = 1;
				drawComponentText = 1;
				break;
		}
		if (drawRevealEdges != 0) {
			xpaint_Horiz_Clipped_Line(drawRect.left, drawRect.top,
									  drawRect.right - drawRect.left - BP_APERTURE_EDGE_MARGIN,
									  BP_APERTURE_COLOR);
			xpaint_Horiz_Clipped_Line(drawRect.left, drawRect.bottom - 1,
									  drawRect.right - drawRect.left - BP_APERTURE_EDGE_MARGIN,
									  BP_APERTURE_COLOR);
		}
		if (drawHologram != 0) {
			xrect_Copy_Rect(&apertureClip, &drawRect);
			xrect_Inset_Rect(&apertureClip, 0, 1);
			xrect_Clip_Rect(&apertureClip, clip);
			if (xrect_Empty_Rect(&apertureClip) == 0) {
				xcanvas_Set_Drawing_Canvas_Clip(&apertureClip);
				if (g_blueprintDisplayedShip >= g_blueprintDetailedShipCount) {
					int16_t shipTextY;
					int16_t shipTextHeight;
					int16_t textColor;
					int16_t shipLine;
					xfont_Enable_FontID_Shadow(BP_DESCRIPTION_SHADOW_FONT);
					shipTextY = BP_DESCRIPTION_Y;
					shipTextHeight =
						BP_DESCRIPTION_GROWTH * (g_blueprintTextRevealTick + BP_DESCRIPTION_REVEAL_LEAD);
					if (shipTextHeight > BP_DESCRIPTION_MAX_HEIGHT)
						shipTextHeight = BP_DESCRIPTION_MAX_HEIGHT;
					xrect_Set_Rect(&drawRect, BP_DESCRIPTION_LEFT, BP_DESCRIPTION_TOP,
								   BP_SHIP_DESCRIPTION_RIGHT, shipTextHeight + BP_DESCRIPTION_BOTTOM_BASE);
					xpaint_Frame_Clipped_Rect(&drawRect, BP_APERTURE_COLOR);
					xrect_Inset_Rect(&drawRect, 1, 1);
					for (shipLine = 1; shipLine < BP_COMPONENT_STRING_COUNT; ++shipLine) {
						xparagrp_Get_Paragraph_String(
							g_blueprintText, description, BP_SHIP_DESCRIPTION_PARAGRAPH,
							shipLine + BP_COMPONENT_STRING_COUNT *
										   (g_blueprintDisplayedShip - g_blueprintDetailedShipCount));
						textColor = shipLine != 1 ? BP_DESCRIPTION_TEXT_COLOR : BP_DESCRIPTION_HEADING_COLOR;
						xfont_Print_Clipped_Text(description, BP_DESCRIPTION_X, shipTextY, BP_LABEL_FONT,
												 textColor);
						shipTextY += BP_DESCRIPTION_LINE_HEIGHT;
					}
					xfont_Disable_FontID_Shadow(BP_DESCRIPTION_SHADOW_FONT);
					if (actor->var2 != 0)
						xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
				} else {
					if (actor->var2 != 0)
						xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
					if (drawComponentText != 0) {
						int16_t descriptionLineCount = 0;
						int16_t countLine;
						int16_t componentTextY;
						int16_t componentTextHeight;
						int16_t componentLine;
						int16_t textColor;
						for (countLine = 1; countLine < BP_COMPONENT_STRING_COUNT; ++countLine) {
							xparagrp_Get_Paragraph_String(
								g_blueprintText, description,
								g_blueprintDisplayedShip + BP_COMPONENT_PARAGRAPH_BASE,
								countLine + BP_COMPONENT_STRING_COUNT * g_blueprintDisplayedComponent);
							if (strlen(description) > 1)
								++descriptionLineCount;
						}
						componentTextY = BP_DESCRIPTION_Y;
						componentTextHeight =
							BP_DESCRIPTION_GROWTH * (g_blueprintTextRevealTick + BP_DESCRIPTION_REVEAL_LEAD);
						if (componentTextHeight >
							BP_DESCRIPTION_LINE_HEIGHT * descriptionLineCount - BP_DESCRIPTION_BOTTOM_TRIM)
							componentTextHeight = BP_DESCRIPTION_LINE_HEIGHT * descriptionLineCount -
												  BP_DESCRIPTION_BOTTOM_TRIM;
						xrect_Set_Rect(&drawRect, BP_DESCRIPTION_LEFT, BP_DESCRIPTION_TOP,
									   BP_COMPONENT_DESCRIPTION_RIGHT,
									   componentTextHeight + BP_DESCRIPTION_BOTTOM_BASE);
						xpaint_Frame_Clipped_Rect(&drawRect, BP_APERTURE_COLOR);
						xrect_Inset_Rect(&drawRect, 1, 1);
						xfont_Enable_FontID_Shadow(BP_DESCRIPTION_SHADOW_FONT);
						for (componentLine = 1; componentLine <= descriptionLineCount; ++componentLine) {
							xparagrp_Get_Paragraph_String(
								g_blueprintText, description,
								g_blueprintDisplayedShip + BP_COMPONENT_PARAGRAPH_BASE,
								componentLine + BP_COMPONENT_STRING_COUNT * g_blueprintDisplayedComponent);
							if (strlen(description) > 1) {
								textColor = componentLine != 1 ? BP_DESCRIPTION_TEXT_COLOR
															   : BP_DESCRIPTION_HEADING_COLOR;
								xfont_Print_Clipped_Text(description, BP_DESCRIPTION_X, componentTextY,
														 BP_LABEL_FONT, textColor);
							}
							componentTextY += BP_DESCRIPTION_LINE_HEIGHT;
						}
						xfont_Disable_FontID_Shadow(BP_DESCRIPTION_SHADOW_FONT);
					}
				}
			}
		}
	}
	return 0;
}

// FUNCTION: XW 0x4381E0
void blueprnt_OpenMusic(void* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled()) {
		g_blueprintMusicFilm = film;
		g_blueprintWaitingMusic = xsound_Find_Gmid(g_blueprintWaitingMusicName);
		if (!g_blueprintWaitingMusic) {
			ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\bpmusic.lfd");
			if (!musicResource) {
				musicResource = xres_Open_Resource(g_blueprintMusicResourceName);
			}
			g_blueprintWaitingMusic = xsound_Res_Music(musicResource, g_blueprintWaitingMusicName);
			soundext_Start_Resource_Sound(g_blueprintWaitingMusic);
			xres_Close_Resource(musicResource);
		}
		xsound_Set_Sound_Keep(g_blueprintWaitingMusic);
		xsound_Set_Sound_User_Function(g_blueprintWaitingMusic, Cutscene_IgnoreSoundEvent);
		g_blueprintHallMarchMusic = xsound_Find_Gmid(g_blueprintHallMarchMusicName);
		if (g_blueprintHallMarchMusic && soundext_Count_Resource_Instances(g_blueprintHallMarchMusic) == 1) {
			xsound_Set_Sound_Keep(g_blueprintHallMarchMusic);
		}
	}
}

// FUNCTION: XW 0x4382C0
void blueprnt_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		if (g_blueprintWaitingMusic != NULL &&
			soundext_Count_Resource_Instances(g_blueprintWaitingMusic) != 1) {
			if (g_blueprintHallMarchMusic != NULL) {
				soundext_SetHook(g_blueprintHallMarchMusic, XW_SOUND_CONTROL_DIRECT,
								 BLUEPRINT_HALL_MUSIC_CONTROL_VALUE, 0);
			}
		} else {
			if (g_blueprintWaitingMusic != NULL) {
				Sound* music;
				/* Resource pointers are outside the numeric flight-sound ID range. */
				soundext_SetPriority(0, 0);
				music = g_blueprintWaitingMusic;
				soundext_FadeVolume(music, 0, BLUEPRINT_MUSIC_FADE_DURATION);
			}
			if (g_blueprintHallMarchMusic != NULL) {
				xsound_Clear_Sound_Keep(g_blueprintHallMarchMusic);
				xsound_Free_Sound(g_blueprintHallMarchMusic);
			}
		}
		soundext_ClearTriggers();
	}
}

// FUNCTION: XW 0x438350
void blueprnt_LoadColorTable(void* resourceFile) {
	int offset;
	uint32_t size;
	if (xres_Get_Resource_Offset(resourceFile, BP_COLOR_RESOURCE_TYPE, "colors", &offset, &size) != 0 &&
		xres_Open_Resource_Data(resourceFile, offset) != 0) {
		xres_Read_Resource_Buffer_Data(resourceFile, g_blueprintColorTable, sizeof(g_blueprintColorTable));
		xres_Close_Resource_Data(resourceFile);
	}
}

// FUNCTION: XW 0x4383B0
void blueprnt_LoadUiSounds(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1A, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_HYDROL_1, 0, NULL, 0, 1);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x438400
void blueprnt_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case BP_SOUND_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_1A);
				break;
			case BP_SOUND_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_TOUR_DOOR_CLOSE_1);
				break;
			case BP_SOUND_HYDRAULICS:
				soundext_Play_SFX(XW_SHELL_SFX_HYDROL_1);
				break;
			case BP_SOUND_FADE_HYDRAULICS:
				soundext_Fade_SFX(XW_SHELL_SFX_HYDROL_1, 0, BP_SOUND_FADE_TICKS);
				break;
		}
	}
}
