#include "xw_dos94/frontend/inflight.h"
#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#endif
#include "xw/frontend/inflight_ui.h"

#include "xw/assets/model_mesh.h"
#include "xw/audio/soundext.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/mission/spec.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/player.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/shared.h"
#include "xw_runtime/integration/inflight_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/inflight_music_task.h"
#include "xw_runtime/runtime/inflight_view_task.h"
#endif

#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/actor.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/style.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xw/frontend/player.h"

#include "xw/assets/model_mesh.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/frontend/brief.h"
#include "xw/frontend/inflight_ui.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shipext.h"
#include "xw/util/shared.h"

#include <ctype.h>
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/paint.h>
#include <landru/res.h>
#include <landru/view.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Actor* fallback_actor;
static const int16_t repair_column_x[] = { 7, 130, 160 };
static void create_side_panels(void);
XwShellSceneResult Dos94_InflightUI_Show(struct XwShellContext* context);
static int16_t Dos94_InflightUI_UpdateViewport(Input* input, Rect* frame, Rect* clip, int16_t key,
											   int leftEvent, int rightEvent, int16_t x, int16_t y);
static void Dos94_InflightUI_DrawViewport(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_InflightUI_DrawLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_InflightUI_DrawShipDetails(Rect* frame, Rect* clip);
static int16_t Dos94_player_DrawInflightMap(Rect* bounds, Rect* clip, int16_t refresh);
static int16_t Dos94_player_Draw_Display_Ship(Rect* frame, Rect* clip);
static int16_t Dos94_player_DrawBriefingText(Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_player_Draw_Map_Paragraph(Rect* bounds, LandruHandle textHandle,
											LandruHandle attributeHandle, int16_t literalMode);
static int16_t Dos94_player_DrawDamageControl(Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_player_Init_Display_Map(void);

/* DOS94 0x200000. */
XwShellSceneResult Dos94_InflightUI_Show(struct XwShellContext* context) {
	ResFile* resource;
	Film* film;
	int16_t objectIndex;
	int iconIndex;
	int panelIndex;
	Input* titleLabel;
	Input* selectionLabel;
	PushButton* button;
	Rect controlRect;
	Rect viewportRect;
	g_inflightMapSelectedShipIndex = INFLIGHT_SELECTION_CLEAR;
	g_inflightShipDetailsIndexPlusOne = 0;
	g_inflightRepairDragActive = 0;
	g_inflightViewMode = shellext_Get_Cur_Scene() - XW_SCENE_INFLIGHT_MAP;
	if (g_inflightMapPreferencesInitialized == 0) {
		g_inflightMapPreferencesInitialized = 1;
		g_inflightRepairSelectionIndex = 0;
		g_inflightMapZoomLevel = INFLIGHT_INITIAL_ZOOM_LEVEL;
		g_inflightMapScaleX = INFLIGHT_INITIAL_MAP_SCALE;
		g_inflightMapScaleY = INFLIGHT_INITIAL_MAP_SCALE;
		g_inflightMapCenterX = 0;
		g_inflightMapCenterY = 0;
	}
	g_inflightFocusIndex = INFLIGHT_INITIAL_FOCUS;
	xio_Set_Mouse_Position(210, 185);
	g_inflightMapStateHandle =
		xmemhdl_Alloc_Clear_Handle(sizeof(*g_inflightMapState), LANDRU_MEMORY_RESOURCE);
	g_inflightMapState = xmemhdl_Lock_Handle(g_inflightMapStateHandle);
	xmemhdl_Unlock_Handle(g_inflightMapStateHandle);
	Dos94_player_Init_Display_Map();
	player_LoadMissionRecords(g_shellMissionName);
	player_Load_Display_Map(g_shellMissionName);
	player_Rewind_Page(g_inflightMapState->activeScriptIndex);
	g_inflightMapShipCount = 0;
	for (objectIndex = 0; objectIndex < PLAYER_MAP_CRAFT_LIMIT; ++objectIndex) {
		if (player_IsObjectVisible(objectIndex) != 0)
			++g_inflightMapShipCount;
	}
	player_SelectPlayerShip();
	player_OrderPendingRepairsFirst();
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\inflight.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource("inflight.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&controlRect, 0, 0, 320, 200);
	film = xfilm_Res_Film(resource, "inflight", &controlRect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(film, context->standardPalette);
	g_inflightBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "inflight");
	xactor_Non_Refreshable_Actor(g_inflightBackgroundActor);
	static const char* const icons[] = { "iconsgrn", "iconsred", "iconsblue" };
	ResFile* icon_resource = shipext_IsTourAvailable(4) ? xres_Open_Resource("bwing.lfd") : resource;
	for (iconIndex = 0; iconIndex < 3; ++iconIndex) {
		g_inflightMapIconActors[iconIndex] =
			xactanim_Res_Anim_Actor(icon_resource, icons[iconIndex], &controlRect, 0, 0, 0);
		xactor_Set_Actor_Time(g_inflightMapIconActors[iconIndex], 0, 0);
	}
	fallback_actor = NULL;
	if (icon_resource != resource) {
		fallback_actor = xactdelt_Res_Delta_Actor(icon_resource, "bradar", &controlRect, 0, 0, 0);
		xactor_Set_Actor_Time(fallback_actor, 0, 0);
		xres_Close_Resource(icon_resource);
	}
	g_inflightShipDetailsActor = xactanim_Res_Anim_Actor(resource, "radar", &controlRect, 0, 0, 0);
	xactor_Set_Actor_Time(g_inflightShipDetailsActor, 0, 0);
	g_inflightMapStarsActor =
		xactdelt_Res_Delta_Actor(context->resourceFile, "stars-4", &controlRect, 0, 0, 0);
	xactor_Set_Actor_Time(g_inflightMapStarsActor, 0, 0);
	xrect_Set_Rect(&controlRect, 0, 0, 320, 200);
	g_inflightRootInput = xinput_Alloc_Input(NULL, &controlRect, 0, 0);
	xrect_Set_Rect(&viewportRect, 12, 29, 224, 167);
	g_inflightViewportInput = xinput_Alloc_Input(g_inflightRootInput, &viewportRect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_inflightViewportInput, Dos94_InflightUI_UpdateViewport);
	xinpattr_Set_Input_User_Function(g_inflightViewportInput, XwInflight_ApplyShipSelection);
	xinpattr_Set_Input_Draw_Function(g_inflightViewportInput, Dos94_InflightUI_DrawViewport);
	g_inflightViewportInput->mouseUsage = downMoveUpInput;
	xrect_Set_Rect(&controlRect, 63, 1, 193, 17);
	titleLabel = xinput_Alloc_Input(g_inflightRootInput, &controlRect, 0, 0);
	xinpattr_Set_Input_Draw_Function(titleLabel, Dos94_InflightUI_DrawLabel);
	titleLabel->id = INFLIGHT_LABEL_TITLE;
	xrect_Set_Rect(&controlRect, 45, 1, 61, 17);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_PREVIOUS_VIEW);
	xinpattr_Set_Input_Draw_Function(&button->header, Dos94_InflightUI_DrawLabel);
	xrect_Set_Rect(&controlRect, 195, 1, 211, 17);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_NEXT_VIEW);
	xinpattr_Set_Input_Draw_Function(&button->header, Dos94_InflightUI_DrawLabel);
	xrect_Set_Rect(&controlRect, 257, 186, 317, 198);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, "Exit",
								   INFLIGHT_BUTTON_RESUME);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	xrect_Set_Rect(&controlRect, 244, 29, 317, 167);
	for (panelIndex = 0; panelIndex != INFLIGHT_VIEW_COUNT; ++panelIndex) {
		g_inflightPanelInputs[panelIndex] = xinput_Alloc_Input(g_inflightRootInput, &controlRect, 0, 0);
		xinpattr_Hide_Input(g_inflightPanelInputs[panelIndex]);
	}
	create_side_panels();
	xrect_Set_Rect(&controlRect, 63, 3, 193, 19);
	selectionLabel = xinput_Alloc_Input(g_inflightRootInput, &controlRect, 0, 0);
	xinpattr_Set_Input_Update_Function(selectionLabel, InflightUI_UpdateSelectionLabel);
	xinpattr_Set_Input_Draw_Function(selectionLabel, Dos94_InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(selectionLabel, INFLIGHT_ALIGN_NEAR, INFLIGHT_ALIGN_FAR);
	selectionLabel->id = INFLIGHT_LABEL_CENTER_SELECTED;
	xrect_Set_Rect(&controlRect, 45, 3, 61, 19);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_PREVIOUS_ITEM);
	xinpattr_Set_Input_Draw_Function(&button->header, Dos94_InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(&button->header, INFLIGHT_ALIGN_NEAR, INFLIGHT_ALIGN_FAR);
	xrect_Set_Rect(&controlRect, 195, 3, 211, 19);
	button = xbtnpush_Alloc_Button(g_inflightRootInput, &controlRect, 0, XwInflight_HandleButton, NULL,
								   INFLIGHT_BUTTON_NEXT_ITEM);
	xinpattr_Set_Input_Draw_Function(&button->header, Dos94_InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(&button->header, INFLIGHT_ALIGN_NEAR, INFLIGHT_ALIGN_FAR);
	xinpattr_Show_Input(g_inflightPanelInputs[g_inflightViewMode]);
	if (g_inflightViewMode < INFLIGHT_VIEW_DAMAGE_CONTROL)
		InflightUI_SelectBriefingPage(g_inflightViewMode);
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(InflightUI_UpdateView);
	XwInflight_RunView(resource);
}

/* DOS94 0x200dee. */
static int16_t Dos94_InflightUI_UpdateViewport(Input* input, Rect* frame, Rect* clip, int16_t key,
											   int leftEvent, int rightEvent, int16_t x, int16_t y) {
	Rect localViewport;
	int16_t selectedIndex;
	int16_t handled;
	int16_t mouseY;
	(void)clip;
	if (key != 0)
		return 0;
	mouseY = y;
	if (g_inflightViewMode == INFLIGHT_VIEW_MAP) {
		if (leftEvent == INFLIGHT_MOUSE_PRESS) {
			xrect_Copy_Rect(&localViewport, frame);
			xrect_Origin_Rect(&localViewport);
			if ((uint16_t)player_Find_Ship_On_Screen(&localViewport, x, mouseY, &selectedIndex))
				input->var1 = selectedIndex + 1;
			else
				input->var1 = INFLIGHT_SELECTION_CLEAR;
		}
		if (rightEvent != 0) {
			switch (rightEvent) {
				case INFLIGHT_MOUSE_PRESS: {
					int16_t mapX, mapY;
					player_Screen_To_Map_Pos(frame, (int16_t)(x + frame->left),
											 (int16_t)(frame->top + mouseY), &mapX, &mapY);
					g_inflightMapCenterX = mapX;
					g_inflightMapCenterY = mapY;
					xinpattr_Refresh_Input(input);
					xcursor_Disable_Cursor();
					xio_Set_Mouse_Position(118, 98);
					break;
				}
				case INFLIGHT_MOUSE_HOLD: {
					int16_t mapX, mapY;
					player_Screen_To_Map_Pos(frame, (int16_t)(x + frame->left),
											 (int16_t)(frame->top + mouseY), &mapX, &mapY);
					g_inflightMapCenterX = mapX;
					g_inflightMapCenterY = mapY;
					xio_Set_Mouse_Position(118, 98);
					xinpattr_Refresh_Input(input);
					break;
				}
				case INFLIGHT_MOUSE_RELEASE:
					if (!xcursor_Is_Cursor_Visible())
						xcursor_Enable_Cursor();
					break;
			}
		}
		handled = 1;
	} else
		handled = key;
	if (g_inflightViewMode == INFLIGHT_VIEW_DAMAGE_CONTROL) {
		int mouseEvent = leftEvent;
		handled = 1;
		if (mouseEvent == 0)
			mouseEvent = rightEvent;
		switch (mouseEvent) {
			case INFLIGHT_MOUSE_PRESS:
				if (mouseY >= 16 && mouseY < 112) {
					g_inflightRepairDragActive = 1;
					g_inflightRepairSelectionIndex = (mouseY - 16) / 12;
					xinput_Refresh_System_Inputs();
				} else
					handled = 0;
				break;
			case INFLIGHT_MOUSE_HOLD: {
				int16_t targetRow = (mouseY - 16) / 12;
				int16_t previousRow;
				if (targetRow < 0)
					targetRow = 0;
				if (targetRow >= XW_PLAYER_SUBSYSTEM_COUNT)
					targetRow = XW_PLAYER_SUBSYSTEM_COUNT - 1;
				previousRow = g_inflightRepairSelectionIndex;
				if (targetRow < previousRow) {
					int16_t movedSubsystem = g_playerSubsystemRepairPriority[previousRow];
					int16_t row;
					for (row = g_inflightRepairSelectionIndex - 1; row >= targetRow; --row)
						g_playerSubsystemRepairPriority[row + 1] = g_playerSubsystemRepairPriority[row];
					g_playerSubsystemRepairPriority[targetRow] = movedSubsystem;
				} else if (targetRow > previousRow) {
					int16_t movedSubsystem = g_playerSubsystemRepairPriority[previousRow];
					int16_t row;
					for (row = g_inflightRepairSelectionIndex + 1; row <= targetRow; ++row)
						g_playerSubsystemRepairPriority[row - 1] = g_playerSubsystemRepairPriority[row];
					g_playerSubsystemRepairPriority[targetRow] = movedSubsystem;
				}
				if (targetRow != previousRow) {
					g_inflightRepairSelectionIndex = targetRow;
					xinput_Refresh_System_Inputs();
				}
				break;
			}
			case INFLIGHT_MOUSE_RELEASE:
				g_inflightRepairDragActive = 0;
				xinput_Refresh_System_Inputs();
				break;
		}
	}
	return handled;
}

/* DOS94 0x201084. */
static void Dos94_InflightUI_DrawViewport(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)input;
	if (refresh != 0) {
		switch (g_inflightViewMode) {
			case INFLIGHT_VIEW_MAP:
				Dos94_player_DrawInflightMap(frame, clip, refresh);
				break;
			case INFLIGHT_VIEW_BRIEFING:
				Dos94_player_DrawBriefingText(frame, clip, refresh);
				break;
			case INFLIGHT_VIEW_DAMAGE_CONTROL:
				Dos94_player_DrawDamageControl(frame, clip, refresh);
				break;
		}
	}
}

/* DOS94 0x2014be. */
static void Dos94_InflightUI_DrawLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char label[INFLIGHT_DETAIL_TEXT_CAPACITY];
	const char* labelText;
	if (refresh == 0)
		return;
	switch (input->id) {
		case INFLIGHT_BUTTON_PREVIOUS_VIEW:
		case INFLIGHT_BUTTON_NEXT_VIEW:
		case INFLIGHT_BUTTON_PREVIOUS_ITEM:
		case INFLIGHT_BUTTON_NEXT_ITEM: {
			int16_t arrowIcon;
			xstyle_Style_Paint_Border(frame, ((PushButton*)input)->pressed);
			if (input->id == INFLIGHT_BUTTON_PREVIOUS_VIEW || input->id == INFLIGHT_BUTTON_PREVIOUS_ITEM)
				arrowIcon = INFLIGHT_LABEL_ARROW_LEFT;
			else
				arrowIcon = INFLIGHT_LABEL_ARROW_RIGHT;
			xstyle_Style_Draw_Centered_Icon(arrowIcon, frame, clip, ((PushButton*)input)->pressed);
			return;
		}
		case INFLIGHT_LABEL_TITLE:
			xstyle_Style_Paint_TextField(frame);
			xfont_Print_Centered_Text(g_inflightViewTitles[g_inflightViewMode], frame, 0,
									  INFLIGHT_LABEL_COLOR);
			return;
		case INFLIGHT_LABEL_CENTER_SELECTED:
			xstyle_Style_Paint_TextField(frame);
			label[0] = 0;
			labelText = "";
			switch (g_inflightViewMode) {
				case INFLIGHT_VIEW_MAP:
					if (g_inflightMapSelectedShipIndex != INFLIGHT_SELECTION_CLEAR)
						sprintf(label, "Spacecraft %d of %d", g_inflightMapSelectedShipIndex + 1,
								(int16_t)g_inflightMapShipCount);
					else
						labelText = "No Ship Selected";
					break;
				case INFLIGHT_VIEW_BRIEFING:
					if (g_inflightMapState->scriptCount > 1)
						sprintf(label, "Page %d of %d", g_inflightMapState->activeScriptIndex,
								g_inflightMapState->scriptCount - 1);
					else
						labelText = "No Briefing";
					break;
				case INFLIGHT_VIEW_DAMAGE_CONTROL:
					labelText = g_inflightSubsystemNames
						[g_playerSubsystemRepairPriority[g_inflightRepairSelectionIndex]];
					break;
			}
			if (labelText[0] != 0)
				strcpy(label, labelText);
			xfont_Print_Centered_Text(label, frame, 0, INFLIGHT_LABEL_COLOR);
			break;
		case INFLIGHT_LABEL_SHIP_DETAILS:
			if (g_inflightShipDetailsIndexPlusOne != 0)
				Dos94_InflightUI_DrawShipDetails(frame, clip);
			else
				xpaint_Paint_Clipped_Rect(frame, INFLIGHT_DETAIL_BACKGROUND);
			return;
	}
}

/* DOS94 0x2017c6. */
static void Dos94_InflightUI_DrawShipDetails(Rect* frame, Rect* clip) {
	Rect innerFrame;
	Rect nameFrame;
	int16_t objectIndex;
	uint8_t hudShipId;
	int16_t fallbackImage;
	int16_t iff;
	CraftData* craft;
	int16_t identityVisible;
	LandruHandle flightGroupHandle;
	int distanceHundredths;
	int16_t distanceWhole;
	int16_t distanceFraction;
	int textWidth;
	char statusText[INFLIGHT_DETAIL_STATUS_CAPACITY];
	char cargoText[INFLIGHT_DETAIL_TEXT_CAPACITY];
	char displayName[INFLIGHT_DETAIL_TEXT_CAPACITY];
	char distanceText[INFLIGHT_DETAIL_TEXT_CAPACITY];
	xrect_Copy_Rect(&innerFrame, frame);
	xrect_Inset_Rect(&innerFrame, 1, 1);
	xpaint_Paint_Clipped_Rect(&innerFrame, INFLIGHT_DETAIL_BACKGROUND);
	objectIndex = player_VisibleIndexToObject(g_inflightShipDetailsIndexPlusOne - 1);
	hudShipId = g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType];
	if (hudShipId > INFLIGHT_DETAIL_LAST_SHIP_IMAGE) {
		fallbackImage = 1;
	} else {
		xactor_Set_Actor_State(g_inflightShipDetailsActor, hudShipId - 1, 0);
		fallbackImage = 0;
	}
	iff = g_objectTable[objectIndex].iff;
	craft = (CraftData*)g_objectTable[objectIndex].instanceData;
	if (iff == 0 ||
		g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] < INFLIGHT_DETAIL_HIDDEN_ID_FIRST ||
		g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] > INFLIGHT_DETAIL_HIDDEN_ID_LAST)
		identityVisible = 1;
	else
		identityVisible = 0;
	identityVisible |= craft->isInspected;
	/* Windows flight uses different indices for craft definitions and display categories. */
	uint16_t category = spec_getstatisticscategory(g_objectTable[objectIndex].objectType);
	const char* abbreviation = "???";
	if (category < INFLIGHT_CRAFT_ABBREVIATION_COUNT)
		abbreviation = g_inflightCraftAbbreviations[category];
	else if (category == SPEC_STATISTICS_INTERDICTOR)
		abbreviation = "INT";
	strcpy(displayName, abbreviation);
	strcat(displayName, ": ");
	flightGroupHandle = g_inflightMapFlightGroupHandles[craft->flightGroupIndex];
	if (flightGroupHandle != LANDRU_NULL_HANDLE && identityVisible != 0) {
		XwMissionFlightGroup* flightGroup = (XwMissionFlightGroup*)xmemhdl_Lock_Handle(flightGroupHandle);
		strcat(displayName, flightGroup->name);
		if ((int16_t)flightGroup->numberOfCraft > 1) {
			sprintf(cargoText, " %d", craft->craftIndexInFlightGroup + 1);
			strcat(displayName, cargoText);
		}
		xmemhdl_Unlock_Handle(flightGroupHandle);
	}
	if (identityVisible == 0)
		strcat(displayName, "UNKNOWN");
	if (player_HasCargoReadout(objectIndex) != 0) {
		if (craft->isInspected != 0) {
			if (craft->cargoName[0] != 0)
				strcpy(cargoText, craft->cargoName);
			else
				strcpy(cargoText, "NONE");
		} else {
			strcpy(cargoText, "UNKNOWN");
		}
	} else {
		cargoText[0] = 0;
	}
	distanceHundredths = g_missionRuntimeState.snapshotCraftDisplayDistances[objectIndex];
	distanceWhole = distanceHundredths / INFLIGHT_DISTANCE_FRACTION_SCALE;
	distanceFraction = distanceHundredths % INFLIGHT_DISTANCE_FRACTION_SCALE;
	if (distanceFraction < INFLIGHT_DISTANCE_LEADING_ZERO_LIMIT)
		sprintf(distanceText, "DIS:\002%d\001.\0020%d", distanceWhole, distanceFraction);
	else
		sprintf(distanceText, "DIS:\002%d\001.\002%d", distanceWhole, distanceFraction);
	strcpy(statusText,
		   g_inflightCraftStatusText[g_missionRuntimeState.snapshotCraftStatusCodes[objectIndex]]);
	if (fallbackImage)
		xactdelt_Draw_Delta_Actor(fallback_actor, frame, clip, innerFrame.left + 2, innerFrame.top + 7, 1);
	else
		xactanim_Draw_Anim_Actor(g_inflightShipDetailsActor, frame, clip, innerFrame.left + 2,
								 innerFrame.top + 7, 1);
	xrect_Copy_Rect(&nameFrame, &innerFrame);
	nameFrame.bottom = nameFrame.top + 8;
	xfont_Set_FontID_Bold_Color(1, g_inflightPanelBoldColors[iff]);
	xfont_Print_Centered_Text(displayName, &nameFrame, 1, g_inflightPanelTextColors[iff]);
	if (cargoText[0] != 0) {
		textWidth = xfont_Get_String_Width_0(1, cargoText) + INFLIGHT_DETAIL_TEXT_INSET;
		xfont_Print_Clipped_Text(cargoText, innerFrame.right - textWidth, innerFrame.bottom - 12, 1,
								 g_inflightPanelBoldColors[INFLIGHT_DETAIL_CARGO_COLOR_INDEX]);
	}
	xfont_Set_FontID_Bold_Color(1, g_inflightPanelBoldColors[INFLIGHT_DETAIL_DISTANCE_COLOR_INDEX]);
	xfont_Print_Clipped_Text(distanceText, innerFrame.left + INFLIGHT_DETAIL_TEXT_INSET,
							 innerFrame.bottom - 6, 1,
							 g_inflightPanelTextColors[INFLIGHT_DETAIL_DISTANCE_COLOR_INDEX]);
	textWidth = xfont_Get_String_Width_0(1, statusText) + INFLIGHT_DETAIL_TEXT_INSET;
	xfont_Print_Clipped_Text(statusText, innerFrame.right - textWidth, innerFrame.bottom - 6, 1,
							 INFLIGHT_DETAIL_STATUS_COLOR);
}

/* DOS94 0x201f4a. */
static int16_t Dos94_player_DrawInflightMap(Rect* bounds, Rect* clip, int16_t refresh) {
	(void)refresh;
	player_Draw_Display_Grid(bounds, clip);
	Dos94_player_Draw_Display_Ship(bounds, clip);
	return 1;
}

/* DOS94 0x2021c4. */
static int16_t Dos94_player_Draw_Display_Ship(Rect* frame, Rect* clip) {
	int16_t screenY;
	int16_t screenX;
	int16_t actorOffsetX;
	int16_t actorOffsetY;
	Rect selectionBounds;
	Rect mapBounds;
	int16_t objectIndex;
	xrect_Copy_Rect(&mapBounds, frame);
	if (g_inflightMapSelectedShipIndex != INFLIGHT_SELECTION_CLEAR) {
		int16_t selectedObjectIndex = player_VisibleIndexToObject(g_inflightMapSelectedShipIndex);
		int16_t mapX = g_objectTable[selectedObjectIndex].worldX / PLAYER_MAP_WORLD_COORDINATE_SCALE;
		int16_t mapY = g_objectTable[selectedObjectIndex].worldY / -PLAYER_MAP_WORLD_COORDINATE_SCALE;
		player_Map_To_Screen_Pos(&mapBounds, mapX, mapY, &screenX, &screenY);
		xrect_Set_Rect(&selectionBounds, screenX - 4, screenY - 4, screenX + 4 + 1, screenY + 4 + 1);
		xrect_Inset_Rect(&selectionBounds, -PLAYER_MAP_SELECTION_EXPANSION, -PLAYER_MAP_SELECTION_EXPANSION);
		xpaint_Paint_Clipped_Rect(&selectionBounds, PLAYER_MAP_SELECTION_BACKGROUND);
		xpaint_Frame_Clipped_Rect(&selectionBounds, PLAYER_MAP_SELECTION_FRAME_COLOR);
	}
	for (objectIndex = 0; objectIndex < PLAYER_MAP_CRAFT_LIMIT; ++objectIndex) {
		if (player_IsObjectVisible(objectIndex) != 0) {
			int16_t mapX = g_objectTable[objectIndex].worldX / PLAYER_MAP_WORLD_COORDINATE_SCALE;
			int16_t mapY = g_objectTable[objectIndex].worldY / -PLAYER_MAP_WORLD_COORDINATE_SCALE;
			int16_t iconState = g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] - 1;
			int16_t iffIndex;
			if (iconState >= PLAYER_MAP_ICON_BASE_COUNT)
				iconState = PLAYER_MAP_ICON_BASE_COUNT - 1;
			iffIndex = g_objectTable[objectIndex].iff;
			if (g_inflightMapScaleX < PLAYER_MAP_ICON_LOW_ZOOM_THRESHOLD) {
				if (iffIndex != 0)
					iconState += PLAYER_MAP_ICON_BASE_COUNT;
				else
					iconState += PLAYER_MAP_ICON_IFF0_LOW_ZOOM_OFFSET;
			}
			if (iconState >= 0) {
				Actor* iconActor;
				player_Map_To_Screen_Pos(&mapBounds, mapX, mapY, &screenX, &screenY);
				xactor_Set_Actor_State(g_inflightMapIconActors[iffIndex], iconState, 0);
				xactanim_Get_Anim_Actor_Offset(g_inflightMapIconActors[iffIndex], &actorOffsetX,
											   &actorOffsetY);
				iconActor = g_inflightMapIconActors[iffIndex];
				screenX -= actorOffsetX + (iconActor->w >> 1);
				screenY -= actorOffsetY + (iconActor->h >> 1);
				if (objectIndex == g_playerFlightState.objectIndex) {
					xactor_Set_Actor_Remap_Color(iconActor, PLAYER_MAP_PLAYER_ICON_COLOR);
					xactanim_Draw_Anim_Actor(g_inflightMapIconActors[iffIndex], frame, clip, screenX, screenY,
											 1);
					xactor_Clear_Actor_Remap_Color(g_inflightMapIconActors[iffIndex]);
				} else {
					xactanim_Draw_Anim_Actor(iconActor, frame, clip, screenX, screenY, 1);
				}
			}
		}
	}
	return 1;
}

/* DOS94 0x20242a. */
static int16_t Dos94_player_DrawBriefingText(Rect* frame, Rect* clip, int16_t refresh) {
	Rect textFrame;
	Rect textClip;
	if (refresh != 0) {
		int viewportIndex;
		int viewportsRemaining;
		xpaint_Paint_Clipped_Rect(frame, PLAYER_TEXT_BACKGROUND_COLOR);
		for (viewportIndex = 0, viewportsRemaining = PLAYER_TEXT_VIEWPORT_COUNT; viewportsRemaining != 0;
			 ++viewportIndex, --viewportsRemaining) {
			if (g_inflightMapState->textViewportActive[viewportIndex] != 0) {
				xrect_Copy_Rect(&textFrame, &g_inflightMapState->textViewportRects[viewportIndex]);
				xrect_Offset_Rect(&textFrame, frame->left, frame->top);
				xrect_Copy_Rect(&textClip, clip);
				xrect_Clip_Rect(&textClip, &textFrame);
				xcanvas_Set_Drawing_Canvas_Clip(&textClip);
				xpaint_Paint_Clipped_Rect(&textFrame, PLAYER_TEXT_VIEWPORT_COLOR);
				if (g_inflightMapState->textSlotActive[viewportIndex] != 0) {
					int textBlockIndex = g_inflightMapState->textSlotBlockIndex[viewportIndex];
					LandruHandle textHandle = g_inflightMapState->textBlockHandles[textBlockIndex];
					LandruHandle attributeHandle = g_inflightMapState->textAttributeHandles[textBlockIndex];
					xrect_Inset_Rect(&textFrame, 2, 1);
					Dos94_player_Draw_Map_Paragraph(&textFrame, textHandle, attributeHandle, 0);
				}
			}
		}
		xcanvas_Set_Drawing_Canvas_Clip(clip);
	}
	return 1;
}

/* DOS94 0x202570. */
static void Dos94_player_Draw_Map_Paragraph(Rect* bounds, LandruHandle textHandle,
											LandruHandle attributeHandle, int16_t literalMode) {
	Rect lineBounds;
	char lineBuffer[PLAYER_PARAGRAPH_BUFFER_SIZE];
	const unsigned char* textBytes;
	const unsigned char* attributeBytes;
	int16_t availableWidth;
	int16_t scanLineStart;
	int16_t lineStart;
	int16_t lineEnd;
	int16_t nextLineStart;
	int scanIndex;
	int lastSpaceIndex;
	int16_t finished;
	xrect_Copy_Rect(&lineBounds, bounds);
	lineBounds.bottom = lineBounds.top + 10;
	lineBuffer[0] = 0;
	availableWidth = lineBounds.right - lineBounds.left;
	textBytes = xmemhdl_Lock_Handle(textHandle);
	attributeBytes = xmemhdl_Lock_Handle(attributeHandle);
	scanLineStart = 0;
	lineEnd = PLAYER_PARAGRAPH_NO_LINE;
	lineStart = PLAYER_PARAGRAPH_NO_LINE;
	scanIndex = 0;
	nextLineStart = 0;
	lastSpaceIndex = 0;
	finished = 0;
	xfont_Enable_FontID_Shadow(0);
	xfont_Set_FontID_Bold_Color(0, PLAYER_PARAGRAPH_BOLD_COLOR);
	do {
		int characterIndex = (int16_t)scanIndex;
		unsigned char character = textBytes[characterIndex];
		if (character == '$' || character == 0) {
			lineStart = scanLineStart;
			if (literalMode == 0 && character == '$')
				lineEnd = (int16_t)(scanIndex - 1);
			else
				lineEnd = (int16_t)scanIndex;
			++scanIndex;
			nextLineStart = (int16_t)scanIndex;
			if (textBytes[(int16_t)scanIndex] == 0)
				finished = 1;
			scanLineStart = nextLineStart;
		} else {
			if (character == ' ')
				lastSpaceIndex = scanIndex;
			if ((isspace)(textBytes[characterIndex]) == 0) {
				for (; (isspace)(textBytes[(int16_t)scanIndex]) == 0; characterIndex = (int16_t)scanIndex) {
					character = textBytes[characterIndex];
					if (character == '$' || character == 0)
						break;
					++scanIndex;
					lineBuffer[characterIndex - scanLineStart] = (char)character;
				}
			} else {
				++scanIndex;
				lineBuffer[characterIndex - scanLineStart] = (char)textBytes[characterIndex];
			}
			lineBuffer[(int16_t)scanIndex - scanLineStart] = 0;
			if (xfont_Get_String_Width_0(0, lineBuffer) >= (int16_t)availableWidth) {
				lineStart = scanLineStart;
				lineEnd = (int16_t)lastSpaceIndex;
				scanIndex = lastSpaceIndex + 1;
				nextLineStart = (int16_t)scanIndex;
				scanLineStart = nextLineStart;
			}
		}
		if (lineStart != PLAYER_PARAGRAPH_NO_LINE) {
			int outputIndex = 0;
			int inputIndex;
			unsigned char currentAttribute = 0;
			lineBuffer[0] = 0;
			for (inputIndex = lineStart; inputIndex <= lineEnd; ++inputIndex) {
				if (attributeBytes[inputIndex] != currentAttribute) {
					currentAttribute = attributeBytes[inputIndex];
					if (currentAttribute != 0)
						lineBuffer[(int16_t)outputIndex] = PLAYER_PARAGRAPH_BOLD_CONTROL;
					else
						lineBuffer[(int16_t)outputIndex] = PLAYER_PARAGRAPH_NORMAL_CONTROL;
					++outputIndex;
				}
				lineBuffer[(int16_t)outputIndex] = (char)textBytes[inputIndex];
				++outputIndex;
			}
			lineBuffer[(int16_t)outputIndex] = 0;
			if (literalMode == 0 && lineBuffer[0] == '>')
				xfont_Print_Centered_Text(&lineBuffer[1], &lineBounds, 0, PLAYER_PARAGRAPH_CENTER_COLOR);
			else
				xfont_Print_Clipped_Text(lineBuffer, lineBounds.left + PLAYER_PARAGRAPH_INSET,
										 lineBounds.top + PLAYER_PARAGRAPH_INSET, 0,
										 PLAYER_PARAGRAPH_TEXT_COLOR);
			lineEnd = PLAYER_PARAGRAPH_NO_LINE;
			lineStart = PLAYER_PARAGRAPH_NO_LINE;
			xrect_Offset_Rect(&lineBounds, 0, 10);
		}
	} while (finished == 0);
	xfont_Disable_FontID_Shadow(0);
	xmemhdl_Unlock_Handle(attributeHandle);
	xmemhdl_Unlock_Handle(textHandle);
}

/* DOS94 0x202828. */
static int16_t Dos94_player_DrawDamageControl(Rect* frame, Rect* clip, int16_t refresh) {
	Rect panelRect;
	char valueText[PLAYER_REPAIR_VALUE_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		int columnIndex;
		int columnsRemaining;
		int rowOffsetY;
		int16_t priorityRow;
		xpaint_Paint_Clipped_Rect(frame, PLAYER_REPAIR_BACKGROUND);
		xrect_Copy_Rect(&panelRect, frame);
		xrect_Inset_Rect(&panelRect, PLAYER_REPAIR_BORDER_X, PLAYER_REPAIR_BORDER_Y);
		xpaint_Paint_Clipped_Bevel(&panelRect, PLAYER_REPAIR_BEVEL_TOP, PLAYER_REPAIR_BEVEL_BOTTOM,
								   PLAYER_REPAIR_BEVEL_FILL, 0);
		xrect_Inset_Rect(&panelRect, PLAYER_REPAIR_CONTENT_INSET_X, PLAYER_REPAIR_BORDER_Y);
		panelRect.bottom = panelRect.top + 10;
		xpaint_Horiz_Clipped_Line(panelRect.left, panelRect.bottom - 1, panelRect.right - panelRect.left,
								  PLAYER_REPAIR_SEPARATOR_COLOR);
		xfont_Enable_FontID_Shadow(0);
		xfont_Enable_FontID_Shadow(1);
		for (columnIndex = 0, columnsRemaining = PLAYER_REPAIR_COLUMN_COUNT; columnsRemaining != 0;
			 ++columnIndex, --columnsRemaining) {
			xfont_Print_Clipped_Text(
				g_inflightRepairColumnTitles[columnIndex], frame->left + repair_column_x[columnIndex],
				frame->top + PLAYER_REPAIR_HEADER_TOP, 1, g_inflightPanelTextColors[columnIndex]);
		}
		rowOffsetY = 12;
		for (priorityRow = 0; priorityRow < XW_PLAYER_SUBSYSTEM_COUNT; ++priorityRow) {
			int16_t subsystemId;
			uint16_t healthPercent;
			int16_t healthColor;
			if (priorityRow == g_inflightRepairSelectionIndex) {
				int16_t rowY = rowOffsetY + frame->top;
				panelRect.top = rowY + 4;
				panelRect.bottom = rowY + 4 + 12;
				if (g_inflightRepairDragActive != 0)
					xpaint_Paint_Clipped_Rect(&panelRect, PLAYER_REPAIR_DRAG_COLOR);
				else
					xpaint_Paint_Clipped_Rect(&panelRect, PLAYER_REPAIR_SELECTION_COLOR);
			}
			subsystemId = g_playerSubsystemRepairPriority[priorityRow];
			xfont_Print_Clipped_Text(g_inflightSubsystemNames[subsystemId], frame->left + repair_column_x[0],
									 rowOffsetY + frame->top + 6, 0, g_inflightPanelBoldColors[0]);
			healthPercent = g_playerFlightState.subsystemHealth[subsystemId];
			if (healthPercent == PLAYER_REPAIR_FULL_HEALTH) {
				strcpy(valueText, "OK");
				healthColor = PLAYER_REPAIR_HEALTHY_COLOR;
			} else {
				healthColor = g_inflightPanelBoldColors[1];
				if (healthPercent == 0)
					strcpy(valueText, "Out");
				else
					sprintf(valueText, "%d%%", healthPercent);
			}
			xfont_Print_Clipped_Text(valueText, frame->left + repair_column_x[1], rowOffsetY + frame->top + 6,
									 0, healthColor);
			if (g_playerFlightState.subsystemRepairTimers[subsystemId] != 0) {
				uint16_t whole = g_playerFlightState.subsystemRepairTimers[subsystemId] / 100;
				uint16_t fraction = g_playerFlightState.subsystemRepairTimers[subsystemId] % 100;
				if (fraction < PLAYER_REPAIR_TWO_DIGIT_SECONDS)
					sprintf(valueText, "%u:0%u", whole, fraction);
				else
					sprintf(valueText, "%u:%u", whole, fraction);
			} else
				memcpy(valueText, g_repairCompleteLabel, sizeof(g_repairCompleteLabel));
			xfont_Print_Clipped_Text(valueText, frame->left + repair_column_x[2] + PLAYER_REPAIR_TIMER_INSET,
									 rowOffsetY + frame->top + 6, 0, g_inflightPanelBoldColors[2]);
			rowOffsetY += 12;
		}
		xfont_Disable_FontID_Shadow(0);
		xfont_Disable_FontID_Shadow(1);
	}
	return 1;
}

/* DOS94 0x202b92. */
static void Dos94_player_Init_Display_Map(void) {
	int16_t bufferIndex;
	int16_t layoutIndex;
	int16_t viewportIndex;
	int16_t slotIndex;
	for (bufferIndex = 0; bufferIndex < XW_BRIEFING_PAGE_LABEL_CAPACITY; ++bufferIndex) {
		g_inflightMapState->labelTextHandles[bufferIndex] =
			xmemhdl_Alloc_Clear_Handle(PLAYER_MAP_LABEL_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
	}
	for (bufferIndex = 0; bufferIndex < XW_BRIEFING_TEXT_BUFFER_CAPACITY; ++bufferIndex) {
		g_inflightMapState->textBlockHandles[bufferIndex] =
			xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_TEXT_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
		g_inflightMapState->textAttributeHandles[bufferIndex] =
			xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_TEXT_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
	}
	g_inflightMapState->playbackActive = 1;
	g_inflightMapState->field_00C2 = 0;
	g_inflightMapState->mapIconCount = 0;
	g_inflightMapState->field_00C6 = 0;
	g_inflightMapState->mapPositionSetCount = 1;
	g_inflightMapState->field_1D66 = 0;
	g_inflightMapState->layoutCount = 1;
	for (layoutIndex = 0; layoutIndex < XW_BRIEFING_LAYOUT_COUNT; ++layoutIndex) {
		for (viewportIndex = 0; viewportIndex < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewportIndex) {
			if (viewportIndex == XW_BRIEFING_LAYOUT_MAP_VIEWPORT && layoutIndex == 0) {
				xrect_Set_Rect(&g_inflightMapState->layoutRects[0][XW_BRIEFING_LAYOUT_MAP_VIEWPORT], 0, 0,
							   212, 138);
				g_inflightMapState->layoutActive[0][XW_BRIEFING_LAYOUT_MAP_VIEWPORT] = 1;
			} else {
				xrect_Clear_Rect(&g_inflightMapState->layoutRects[layoutIndex][viewportIndex]);
				g_inflightMapState->layoutActive[layoutIndex][viewportIndex] = 0;
			}
		}
	}
	g_inflightMapState->activeScriptIndex = 0;
	g_inflightMapState->scriptCount = 1;
	player_Clear_Page_Commands(0);
	g_inflightMapState->scriptPositionSetIndex[0] = 0;
	g_inflightMapState->scriptLayoutIndex[0] = 0;
	g_inflightMapState->mapViewportActive = 0;
	g_inflightMapState->field_37DC = 0;
	for (slotIndex = 0; slotIndex < (int)(sizeof(g_inflightMapState->textViewportActive) /
										  sizeof(g_inflightMapState->textViewportActive[0]));
		 ++slotIndex)
		g_inflightMapState->textViewportActive[slotIndex] = 0;
	for (slotIndex = 0; slotIndex < (int)(sizeof(g_inflightMapState->textSlotActive) /
										  sizeof(g_inflightMapState->textSlotActive[0]));
		 ++slotIndex)
		g_inflightMapState->textSlotActive[slotIndex] = 0;
	for (slotIndex = 0; slotIndex < (int)(sizeof(g_inflightMapState->fgMarkerActive) /
										  sizeof(g_inflightMapState->fgMarkerActive[0]));
		 ++slotIndex)
		g_inflightMapState->fgMarkerActive[slotIndex] = 0;
	for (slotIndex = 0; slotIndex < (int)(sizeof(g_inflightMapState->labelActive) /
										  sizeof(g_inflightMapState->labelActive[0]));
		 ++slotIndex)
		g_inflightMapState->labelActive[slotIndex] = 0;
	g_inflightMapState->textSlotActive[0] = 1;
	g_inflightMapState->textSlotBlockIndex[0] = 1;
	player_ApplyBriefingLayout(g_inflightMapState->scriptLayoutIndex[g_inflightMapState->activeScriptIndex]);
}

static void side_button(int panel, const char* label, int id, int left, int top, int right, int bottom,
						int align) {
	Rect rect;
	xrect_Set_Rect(&rect, left, top, right, bottom);
	PushButton* button =
		xbtnpush_Alloc_Button(g_inflightPanelInputs[panel], &rect, 0, XwInflight_HandleButton, label, id);
	if (panel == INFLIGHT_VIEW_MAP)
		xinpattr_Set_Input_Update_Function(&button->header, XwInflight_UpdateRepeatingButton);
	xinpattr_Set_Input_Draw_Function(&button->header, XwInflight_DrawTextButton);
	if (align >= 0)
		xinpattr_Set_Input_Allign(&button->header, align, 0);
}

static void create_side_panels(void) {
	Rect rect;
	xrect_Set_Rect(&rect, -1, 10, 71, 57);
	g_inflightShipDetailsInput = xinput_Alloc_Input(g_inflightPanelInputs[INFLIGHT_VIEW_MAP], &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_inflightShipDetailsInput, Dos94_InflightUI_DrawLabel);
	xinpattr_Set_Input_Allign(g_inflightShipDetailsInput, 1, 0);
	g_inflightShipDetailsInput->id = INFLIGHT_LABEL_SHIP_DETAILS;
	side_button(0, "Up", 6, 2, 70, 36, 82, 1);
	side_button(0, "Left", 7, 3, 84, 37, 96, 0);
	side_button(0, "Right", 8, 0, 84, 34, 96, 2);
	side_button(0, "Down", 9, 2, 98, 36, 110, 1);
	side_button(0, "Zoom", 10, 3, 126, 73, 138, -1);
	side_button(2, "Sooner", 14, 3, 112, 73, 124, -1);
	side_button(2, "Later", 15, 3, 126, 73, 138, -1);
}
