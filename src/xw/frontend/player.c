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

// GLOBAL: XW 0x4D4D38
char g_inflightSubsystemNames[XW_PLAYER_SUBSYSTEM_COUNT][PLAYER_REPAIR_TITLE_CAPACITY] = {
	"Auto-Ejection System", "Hyperdrive",         "Cannons",          "Flight Control System", "Engine Power",
	"Targeting System",     "Shields (Fwd/Rear)", "Missile Launchers"
};

// GLOBAL: XW 0x4D4DF8
char g_inflightRepairColumnTitles[PLAYER_REPAIR_COLUMN_COUNT][PLAYER_REPAIR_TITLE_CAPACITY] = {
	"Repair List", "Status", "Repair Time"
};

// GLOBAL: XW 0x4D4E40
int16_t g_inflightRepairColumnX[PLAYER_REPAIR_COLUMN_COUNT] = { 7, 260, 320 };

// GLOBAL: XW 0x4D4E88
uint16_t g_inflightPanelBoldColors[PLAYER_REPAIR_COLUMN_COUNT] = { 61, 55, 51 };

// GLOBAL: XW 0x4D4E8E
uint16_t g_inflightPanelTextColors[PLAYER_REPAIR_COLUMN_COUNT] = { 62, 54, 50 };

// GLOBAL: XW 0x4D5050
const char g_repairCompleteLabel[] = "Done";

// GLOBAL: XW 0x4D4A30
const int16_t g_inflightMapCommandParameterCounts[PLAYER_COMMAND_COUNT] = { 0, 0, 1, 0, 4, 0, 4, 4, 4, 4, 0,
																			1, 1, 1, 1, 2, 2, 1, 1, 0, 0, 0,
																			1, 1, 1, 1, 0, 3, 3, 3, 3, 0, 3,
																			0, 0, 0, 0, 0, 0, 0, 0, 0 };

// GLOBAL: XW 0x4D4C90
uint8_t g_inflightCargoHudShipIds[PLAYER_CARGO_HUD_SHIP_COUNT] = { 8, 9, 11, 12, 15 };

// GLOBAL: XW 0x4F7700
XwMissionHeader g_inflightMissionHeader = { 0 };

// GLOBAL: XW 0x4F77D0
Actor* g_inflightMapStarsActor = NULL;

// GLOBAL: XW 0x4F77D4
int16_t g_inflightMapShipCount = 0;

// GLOBAL: XW 0x4F7810
LandruHandle g_inflightMapFlightGroupHandles[PLAYER_MAP_RECORD_CAPACITY] = { LANDRU_NULL_HANDLE };

// GLOBAL: XW 0x4F7868
Actor* g_inflightMapIconActors[PLAYER_MAP_ICON_ACTOR_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F7890
LandruHandle g_inflightMapSecondaryRecordHandles[PLAYER_MAP_RECORD_CAPACITY] = { LANDRU_NULL_HANDLE };

// FUNCTION: XW 0x451DC0
void player_Rewind_Page(int16_t scriptIndex) {
	int16_t slotIndex;
	for (slotIndex = 0; slotIndex < (int)(sizeof(g_inflightMapState->textSlotActive) /
										  sizeof(g_inflightMapState->textSlotActive[0]));
		 ++slotIndex) {
		g_inflightMapState->textSlotActive[slotIndex] = 0;
	}
	g_inflightMapState->scriptCurrentTime[scriptIndex] = 0;
	g_inflightMapState->scriptCursorWordIndex[scriptIndex] = 0;
	player_Step_Page(scriptIndex);
}

// FUNCTION: XW 0x451E10
void player_Step_Page(int16_t scriptIndex) {
	int16_t cursor = g_inflightMapState->scriptCursorWordIndex[scriptIndex];
	int16_t commandStartIndex = cursor;
	int16_t refreshRequired = 0;
	int16_t commandTime = g_inflightMapState->scriptWords[scriptIndex][cursor];
	int16_t commandParameters[PLAYER_COMMAND_PARAMETER_CAPACITY];
	g_inflightMapState->textSlotsChanged = 0;
	if (commandTime <= g_inflightMapState->scriptCurrentTime[scriptIndex]) {
		do {
			int16_t opcode;
			int16_t parameterIndex;
			int16_t parameterCount;
			commandStartIndex = cursor;
			commandTime = g_inflightMapState->scriptWords[scriptIndex][cursor++];
			opcode = g_inflightMapState->scriptWords[scriptIndex][cursor++];
			parameterCount = g_inflightMapCommandParameterCounts[opcode];
			for (parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex) {
				commandParameters[parameterIndex] = g_inflightMapState->scriptWords[scriptIndex][cursor++];
			}
			if (commandTime == g_inflightMapState->scriptCurrentTime[scriptIndex]) {
				switch (opcode) {
					case PLAYER_COMMAND_CLEAR_TEXT: {
						int16_t slotIndex;
						for (slotIndex = 0; slotIndex < (int)(sizeof(g_inflightMapState->textSlotActive) /
															  sizeof(g_inflightMapState->textSlotActive[0]));
							 ++slotIndex) {
							if (g_inflightMapState->textSlotActive[slotIndex] != 0)
								g_inflightMapState->textSlotActive[slotIndex] = 0;
						}
						g_inflightMapState->textSlotsChanged = 1;
						refreshRequired = 1;
						break;
					}
					case PLAYER_COMMAND_FIRST_TEXT_SLOT:
					case PLAYER_COMMAND_FIRST_TEXT_SLOT + 1:
					case PLAYER_COMMAND_FIRST_TEXT_SLOT + 2:
					case PLAYER_COMMAND_LAST_TEXT_SLOT: {
						int16_t slotIndex = opcode - PLAYER_COMMAND_FIRST_TEXT_SLOT;
						if (g_inflightMapState->textSlotActive[slotIndex] == 0) {
							g_inflightMapState->textSlotActive[slotIndex] = 1;
							g_inflightMapState->textSlotBlockIndex[slotIndex] = commandParameters[0];
						}
						refreshRequired = 1;
						break;
					}
				}
			}
		} while (commandTime <= g_inflightMapState->scriptCurrentTime[scriptIndex]);
	}
	++g_inflightMapState->scriptCurrentTime[scriptIndex];
	g_inflightMapState->scriptCursorWordIndex[scriptIndex] = commandStartIndex;
	if (refreshRequired != 0)
		xview_Refresh_View();
}

// FUNCTION: XW 0x451FA0
int16_t player_Reseek_Page(int16_t scriptIndex) {
	int16_t preservedTime = g_inflightMapState->scriptCurrentTime[scriptIndex];
	if (preservedTime == 0)
		preservedTime = 1;
	player_Rewind_Page(scriptIndex);
	return player_Seek_Page(scriptIndex, preservedTime - 1);
}

// FUNCTION: XW 0x451FE0
int16_t player_Seek_Page(int16_t scriptIndex, int16_t targetTime) {
	int16_t currentTime = g_inflightMapState->scriptCurrentTime[scriptIndex];
	if (targetTime != currentTime - 1) {
		if (targetTime < currentTime)
			player_Rewind_Page(scriptIndex);
		while (g_inflightMapState->scriptCurrentTime[scriptIndex] <= targetTime)
			player_Step_Page(scriptIndex);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x452050
int16_t player_DrawInflightMap(Rect* bounds, Rect* clip, int16_t refresh) {
	(void)refresh;
	player_Draw_Display_Grid(bounds, clip);
	player_Draw_Display_Ship(bounds, clip);
	return 1;
}

// FUNCTION: XW 0x452080
int16_t player_Draw_Display_Grid(Rect* bounds, Rect* unusedClip) {
	Rect gridBounds;
	int16_t majorColor;
	int16_t minorColor;
	int16_t startColumn;
	int16_t startRow;
	int16_t startX;
	int16_t startY;
	int16_t gridX;
	int16_t gridY;
	int16_t column;
	int16_t row;
	int16_t showAllMinorLines;
	(void)unusedClip;
	if (g_inflightMapState->mapColorMode != 0) {
		majorColor = PLAYER_GRID_DARK_MAJOR_COLOR;
		minorColor = PLAYER_GRID_DARK_MINOR_COLOR;
	} else {
		majorColor = PLAYER_GRID_LIGHT_MAJOR_COLOR;
		minorColor = PLAYER_GRID_LIGHT_MINOR_COLOR;
	}
	player_Stars_To_Back();
	xrect_Copy_Rect(&gridBounds, bounds);
	xpaint_Frame_Clipped_Rect(&gridBounds, minorColor);
	startColumn = g_inflightMapCenterX / PLAYER_MAP_WORLD_COORDINATE_SCALE;
	if (g_inflightMapCenterX > 0 && (g_inflightMapCenterX & PLAYER_GRID_CELL_FRACTION_MASK) != 0)
		++startColumn;
	startX = gridBounds.left + ((gridBounds.right - gridBounds.left) >> 1) +
			 ((g_inflightMapScaleX * ((-g_inflightMapCenterX) & PLAYER_GRID_CELL_FRACTION_MASK)) >>
			  PLAYER_GRID_CELL_FRACTION_BITS);
	for (; startX > gridBounds.left; --startColumn)
		startX -= g_inflightMapScaleX;
	startRow = g_inflightMapCenterY / PLAYER_MAP_WORLD_COORDINATE_SCALE;
	if (g_inflightMapCenterY > 0 && (g_inflightMapCenterY & PLAYER_GRID_CELL_FRACTION_MASK) != 0)
		++startRow;
	startY = gridBounds.top + ((gridBounds.bottom - gridBounds.top) >> 1) +
			 ((g_inflightMapScaleY * ((-g_inflightMapCenterY) & PLAYER_GRID_CELL_FRACTION_MASK)) >>
			  PLAYER_GRID_CELL_FRACTION_BITS);
	for (; startY > gridBounds.top; --startRow)
		startY -= g_inflightMapScaleY;
	if (g_inflightMapScaleX >= PLAYER_GRID_HALF_DETAIL_SCALE) {
		showAllMinorLines = g_inflightMapScaleX >= PLAYER_GRID_FULL_DETAIL_SCALE;
		for (gridX = startX, column = startColumn; gridX < gridBounds.right;
			 gridX += g_inflightMapScaleX, ++column) {
			if ((column & (PLAYER_GRID_MAJOR_INTERVAL - 1)) != 0 &&
				((column & (PLAYER_GRID_MAJOR_INTERVAL - 1)) == PLAYER_GRID_MAJOR_INTERVAL / 2 ||
				 showAllMinorLines))
				xpaint_Vert_Clipped_Line(gridX, gridBounds.top, gridBounds.bottom - gridBounds.top,
										 minorColor);
		}
		for (gridY = startY, row = startRow; gridY < gridBounds.bottom; gridY += g_inflightMapScaleY, ++row) {
			if ((row & (PLAYER_GRID_MAJOR_INTERVAL - 1)) != 0 &&
				((row & (PLAYER_GRID_MAJOR_INTERVAL - 1)) == PLAYER_GRID_MAJOR_INTERVAL / 2 ||
				 showAllMinorLines))
				xpaint_Horiz_Clipped_Line(gridBounds.left, gridY, gridBounds.right - gridBounds.left,
										  minorColor);
		}
	}
	for (gridX = startX, column = startColumn; gridX < gridBounds.right;
		 gridX += g_inflightMapScaleX, ++column) {
		if ((column & (PLAYER_GRID_MAJOR_INTERVAL - 1)) == 0)
			xpaint_Vert_Clipped_Line(gridX, gridBounds.top, gridBounds.bottom - gridBounds.top, majorColor);
	}
	for (gridY = startY, row = startRow; gridY < gridBounds.bottom; gridY += g_inflightMapScaleY, ++row) {
		if ((row & (PLAYER_GRID_MAJOR_INTERVAL - 1)) == 0)
			xpaint_Horiz_Clipped_Line(gridBounds.left, gridY, gridBounds.right - gridBounds.left, majorColor);
	}
	return 1;
}

// FUNCTION: XW 0x452340
int16_t player_Draw_Display_Ship(Rect* frame, Rect* clip) {
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
		xrect_Set_Rect(&selectionBounds, screenX - PLAYER_MAP_SELECTION_HALF_SIZE,
					   screenY - PLAYER_MAP_SELECTION_HALF_SIZE, screenX + PLAYER_MAP_SELECTION_HALF_SIZE + 1,
					   screenY + PLAYER_MAP_SELECTION_HALF_SIZE + 1);
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

// FUNCTION: XW 0x4525B0
int16_t player_DrawBriefingText(Rect* frame, Rect* clip, int16_t refresh) {
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
				xrect_Scale_Rect(&textFrame, PLAYER_TEXT_VIEWPORT_SCALE, PLAYER_TEXT_VIEWPORT_SCALE);
				if (viewportIndex + 1 == PLAYER_TEXT_VIEWPORT_COUNT ||
					g_inflightMapState->textViewportActive[viewportIndex + 1] == 0)
					textFrame.bottom = PLAYER_TEXT_VIEWPORT_BOTTOM;
				xrect_Offset_Rect(&textFrame, frame->left, frame->top);
				xrect_Copy_Rect(&textClip, clip);
				xrect_Clip_Rect(&textClip, &textFrame);
				xcanvas_Set_Drawing_Canvas_Clip(&textClip);
				xpaint_Paint_Clipped_Rect(&textFrame, PLAYER_TEXT_VIEWPORT_COLOR);
				if (g_inflightMapState->textSlotActive[viewportIndex] != 0) {
					int textBlockIndex = g_inflightMapState->textSlotBlockIndex[viewportIndex];
					LandruHandle textHandle = g_inflightMapState->textBlockHandles[textBlockIndex];
					LandruHandle attributeHandle = g_inflightMapState->textAttributeHandles[textBlockIndex];
					xrect_Inset_Rect(&textFrame, PLAYER_TEXT_INSET_X, PLAYER_TEXT_INSET_Y);
					player_Draw_Map_Paragraph(&textFrame, textHandle, attributeHandle, 0);
				}
			}
		}
		xcanvas_Set_Drawing_Canvas_Clip(clip);
	}
	return 1;
}

// FUNCTION: XW 0x452720
void player_Draw_Map_Paragraph(Rect* bounds, LandruHandle textHandle, LandruHandle attributeHandle,
							   int16_t literalMode) {
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
	lineBounds.bottom = lineBounds.top + PLAYER_PARAGRAPH_LINE_HEIGHT;
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
	xfont_Enable_FontID_Shadow(PLAYER_PARAGRAPH_FONT);
	xfont_Set_FontID_Bold_Color(PLAYER_PARAGRAPH_FONT, PLAYER_PARAGRAPH_BOLD_COLOR);
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
			if (xfont_Get_String_Width_0(PLAYER_PARAGRAPH_FONT, lineBuffer) >= (int16_t)availableWidth) {
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
				xfont_Print_Centered_Text(&lineBuffer[1], &lineBounds, PLAYER_PARAGRAPH_FONT,
										  PLAYER_PARAGRAPH_CENTER_COLOR);
			else
				xfont_Print_Clipped_Text(lineBuffer, lineBounds.left + PLAYER_PARAGRAPH_INSET,
										 lineBounds.top + PLAYER_PARAGRAPH_INSET, PLAYER_PARAGRAPH_FONT,
										 PLAYER_PARAGRAPH_TEXT_COLOR);
			lineEnd = PLAYER_PARAGRAPH_NO_LINE;
			lineStart = PLAYER_PARAGRAPH_NO_LINE;
			xrect_Offset_Rect(&lineBounds, 0, PLAYER_PARAGRAPH_LINE_HEIGHT);
		}
	} while (finished == 0);
	xfont_Disable_FontID_Shadow(PLAYER_PARAGRAPH_FONT);
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x4529E0
int16_t player_DrawDamageControl(Rect* frame, Rect* clip, int16_t refresh) {
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
		panelRect.bottom = panelRect.top + PLAYER_REPAIR_HEADER_HEIGHT;
		xpaint_Horiz_Clipped_Line(panelRect.left, panelRect.bottom - 1, panelRect.right - panelRect.left,
								  PLAYER_REPAIR_SEPARATOR_COLOR);
		xfont_Enable_FontID_Shadow(PLAYER_REPAIR_ROW_FONT);
		xfont_Enable_FontID_Shadow(PLAYER_REPAIR_HEADER_FONT);
		for (columnIndex = 0, columnsRemaining = PLAYER_REPAIR_COLUMN_COUNT; columnsRemaining != 0;
			 ++columnIndex, --columnsRemaining) {
			xfont_Print_Clipped_Text(g_inflightRepairColumnTitles[columnIndex],
									 frame->left + g_inflightRepairColumnX[columnIndex],
									 frame->top + PLAYER_REPAIR_HEADER_TOP, PLAYER_REPAIR_HEADER_FONT,
									 g_inflightPanelTextColors[columnIndex]);
		}
		rowOffsetY = PLAYER_REPAIR_INITIAL_ROW_OFFSET;
		for (priorityRow = 0; priorityRow < XW_PLAYER_SUBSYSTEM_COUNT; ++priorityRow) {
			int16_t subsystemId;
			uint16_t healthPercent;
			int16_t healthColor;
			if (priorityRow == g_inflightRepairSelectionIndex) {
				int16_t rowY = rowOffsetY + frame->top;
				panelRect.top = rowY + PLAYER_REPAIR_HIGHLIGHT_TOP;
				panelRect.bottom = rowY + PLAYER_REPAIR_HIGHLIGHT_TOP + INFLIGHT_REPAIR_ROW_HEIGHT;
				if (g_inflightRepairDragActive != 0)
					xpaint_Paint_Clipped_Rect(&panelRect, PLAYER_REPAIR_DRAG_COLOR);
				else
					xpaint_Paint_Clipped_Rect(&panelRect, PLAYER_REPAIR_SELECTION_COLOR);
			}
			subsystemId = g_playerSubsystemRepairPriority[priorityRow];
			xfont_Print_Clipped_Text(g_inflightSubsystemNames[subsystemId],
									 frame->left + g_inflightRepairColumnX[0],
									 rowOffsetY + frame->top + PLAYER_REPAIR_TEXT_TOP, PLAYER_REPAIR_ROW_FONT,
									 g_inflightPanelBoldColors[0]);
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
			xfont_Print_Clipped_Text(valueText, frame->left + g_inflightRepairColumnX[1],
									 rowOffsetY + frame->top + PLAYER_REPAIR_TEXT_TOP, PLAYER_REPAIR_ROW_FONT,
									 healthColor);
			if (g_playerFlightState.subsystemRepairTimers[subsystemId] != 0) {
				uint16_t minutes =
					g_playerFlightState.subsystemRepairTimers[subsystemId] / PLAYER_REPAIR_SECONDS_PER_MINUTE;
				uint16_t seconds =
					g_playerFlightState.subsystemRepairTimers[subsystemId] % PLAYER_REPAIR_SECONDS_PER_MINUTE;
				if (seconds < PLAYER_REPAIR_TWO_DIGIT_SECONDS)
					sprintf(valueText, "%u:0%u", minutes, seconds);
				else
					sprintf(valueText, "%u:%u", minutes, seconds);
			} else
				memcpy(valueText, g_repairCompleteLabel, sizeof(g_repairCompleteLabel));
			xfont_Print_Clipped_Text(valueText,
									 frame->left + g_inflightRepairColumnX[2] + PLAYER_REPAIR_TIMER_INSET,
									 rowOffsetY + frame->top + PLAYER_REPAIR_TEXT_TOP, PLAYER_REPAIR_ROW_FONT,
									 g_inflightPanelBoldColors[2]);
			rowOffsetY += INFLIGHT_REPAIR_ROW_HEIGHT;
		}
		xfont_Disable_FontID_Shadow(PLAYER_REPAIR_ROW_FONT);
		xfont_Disable_FontID_Shadow(PLAYER_REPAIR_HEADER_FONT);
	}
	return 1;
}

// FUNCTION: XW 0x452CE0
void player_Stars_To_Back(void) {
	Rect canvasBounds;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	if (g_inflightMapState->mapColorMode != 0) {
		xpaint_Paint_Clipped_Rect(&canvasBounds, PLAYER_MAP_BACKGROUND_COLOR);
	} else {
		xpaint_Paint_Clipped_Rect(&canvasBounds, PLAYER_STARS_BACKGROUND_COLOR);
		if (g_inflightMapStarsActor != NULL) {
			xactdelt_Draw_Delta_Actor(g_inflightMapStarsActor, &canvasBounds, &canvasBounds, 0, 0, 1);
		}
	}
}

// FUNCTION: XW 0x452D50
void player_Init_Display_Map(void) {
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
							   PLAYER_MAP_INITIAL_RIGHT, PLAYER_MAP_INITIAL_BOTTOM);
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

// FUNCTION: XW 0x452FC0
void player_Free_Display_Map(void) {
	uint16_t index;
	for (index = 0; index < sizeof(g_inflightMapState->labelTextHandles) / sizeof(LandruHandle); ++index) {
		LandruHandle label = g_inflightMapState->labelTextHandles[index];
		if (label != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(label);
		}
	}
	for (index = 0; index < sizeof(g_inflightMapState->textBlockHandles) / sizeof(LandruHandle); ++index) {
		LandruHandle text = g_inflightMapState->textBlockHandles[index];
		LandruHandle attributes;
		if (text != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(text);
		}
		attributes = g_inflightMapState->textAttributeHandles[index];
		if (attributes != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(attributes);
		}
	}
	for (index = 0; index < PLAYER_MAP_RECORD_CAPACITY; ++index) {
		LandruHandle flightGroup = g_inflightMapFlightGroupHandles[index];
		LandruHandle secondaryRecord;
		if (flightGroup != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(flightGroup);
			g_inflightMapFlightGroupHandles[index] = LANDRU_NULL_HANDLE;
		}
		secondaryRecord = g_inflightMapSecondaryRecordHandles[index];
		if (secondaryRecord != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(secondaryRecord);
			g_inflightMapSecondaryRecordHandles[index] = LANDRU_NULL_HANDLE;
		}
	}
}

// FUNCTION: XW 0x453080
void player_Clear_Page_Commands(int16_t scriptIndex) {
	g_inflightMapState->scriptWords[scriptIndex][0] = PLAYER_SCRIPT_SENTINEL_TIME;
	g_inflightMapState->scriptWords[scriptIndex][1] = PLAYER_COMMAND_END;
	g_inflightMapState->scriptEndTime[scriptIndex] = PLAYER_SCRIPT_DEFAULT_END_TIME;
	g_inflightMapState->scriptCurrentTime[scriptIndex] = 0;
	g_inflightMapState->scriptCursorWordIndex[scriptIndex] = 0;
	g_inflightMapState->scriptWordCount[scriptIndex] = PLAYER_COMMAND_HEADER_WORD_COUNT;
	g_inflightMapState->scriptPositionSetIndex[scriptIndex] = 0;
	g_inflightMapState->scriptLayoutIndex[scriptIndex] = 0;
	player_Rewind_Page(scriptIndex);
}

// FUNCTION: XW 0x453120
void player_ApplyBriefingLayout(int16_t layoutIndex) {
	int16_t viewportIndex;
	for (viewportIndex = 0; viewportIndex < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewportIndex) {
		if (viewportIndex != XW_BRIEFING_LAYOUT_MAP_VIEWPORT) {
			xrect_Copy_Rect(&g_inflightMapState->textViewportRects[viewportIndex],
							&g_inflightMapState->layoutRects[layoutIndex][viewportIndex]);
			g_inflightMapState->textViewportActive[viewportIndex] =
				g_inflightMapState->layoutActive[layoutIndex][viewportIndex];
		} else {
			xrect_Copy_Rect(&g_inflightMapState->mapViewportRect,
							&g_inflightMapState->layoutRects[layoutIndex][XW_BRIEFING_LAYOUT_MAP_VIEWPORT]);
			g_inflightMapState->mapViewportActive =
				g_inflightMapState->layoutActive[layoutIndex][XW_BRIEFING_LAYOUT_MAP_VIEWPORT];
		}
	}
}

// FUNCTION: XW 0x4531D0
int player_Find_Ship_On_Screen(Rect* bounds, int16_t screenX, int16_t screenY, int16_t* selectedIndex) {
	int16_t objectIndex = 0;
	int16_t visibleIndex;
	int16_t nearestDistance = PLAYER_MAP_INITIAL_PICK_DISTANCE;
	int16_t originalScreenX = screenX;
	int16_t iconY;
	*selectedIndex = 0;
	for (visibleIndex = 0; visibleIndex < (int16_t)g_inflightMapShipCount; ++visibleIndex) {
		if (player_IsObjectVisible(objectIndex) == 0) {
			while (objectIndex < PLAYER_MAP_CRAFT_LIMIT) {
				++objectIndex;
				if (player_IsObjectVisible(objectIndex) != 0)
					break;
			}
		}
		if (objectIndex < PLAYER_MAP_CRAFT_LIMIT) {
			int distanceX;
			int16_t mapX = g_objectTable[objectIndex].worldX / PLAYER_MAP_WORLD_COORDINATE_SCALE;
			int16_t mapY = g_objectTable[objectIndex].worldY / -PLAYER_MAP_WORLD_COORDINATE_SCALE;
			player_Map_To_Screen_Pos(bounds, mapX, mapY, &screenX, &iconY);
			distanceX = abs(originalScreenX - screenX);
			if (distanceX < nearestDistance) {
				int distanceY = abs(screenY - iconY);
				if (distanceY < nearestDistance) {
					nearestDistance = distanceY;
					if (distanceX >= distanceY)
						nearestDistance = distanceX;
					*selectedIndex = visibleIndex;
				}
			}
			++objectIndex;
		}
	}
	return nearestDistance <= PLAYER_MAP_PICK_RADIUS;
}

// FUNCTION: XW 0x4532F0
int16_t player_VisibleIndexToObject(int16_t visibleShipIndex) {
	int16_t objectIndex = 0;
	int16_t eligibleShipIndex = 0;
	for (; objectIndex < PLAYER_MAP_CRAFT_LIMIT; ++objectIndex) {
		if (player_IsObjectVisible(objectIndex)) {
			if (eligibleShipIndex == visibleShipIndex)
				return objectIndex;
			++eligibleShipIndex;
		}
	}
	return visibleShipIndex;
}

// FUNCTION: XW 0x453330
int16_t player_HasCargoReadout(int16_t objectIndex) {
	int16_t cargoTypeIndex;
	for (cargoTypeIndex = 0; cargoTypeIndex < PLAYER_CARGO_HUD_SHIP_COUNT; ++cargoTypeIndex) {
		if (g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] ==
			(int8_t)g_inflightCargoHudShipIds[cargoTypeIndex])
			return 1;
	}
	return 0;
}

// FUNCTION: XW 0x453370
void player_SelectPlayerShip(void) {
	int16_t objectSlot = 0;
	int16_t visibleShipIndex = 0;
	for (; objectSlot < PLAYER_MAP_CRAFT_LIMIT; ++objectSlot) {
		if (player_IsObjectVisible(objectSlot) != 0) {
			if (objectSlot == g_playerFlightState.objectIndex) {
				g_inflightMapSelectedShipIndex = visibleShipIndex;
				g_inflightMapCenterX = g_objectTable[objectSlot].worldX / PLAYER_MAP_WORLD_COORDINATE_SCALE;
				g_inflightMapCenterY = g_objectTable[objectSlot].worldY / -PLAYER_MAP_WORLD_COORDINATE_SCALE;
				g_inflightShipDetailsIndexPlusOne = visibleShipIndex + 1;
				return;
			}
			++visibleShipIndex;
		}
	}
}

// FUNCTION: XW 0x453400
void player_OrderPendingRepairsFirst(void) {
	int16_t noSwaps;
	do {
		int index;
		noSwaps = 1;
		for (index = 1; index != XW_PLAYER_SUBSYSTEM_COUNT; ++index) {
			int16_t precedingSubsystem = g_playerSubsystemRepairPriority[index - 1];
			int16_t currentSubsystem = g_playerSubsystemRepairPriority[index];
			if (g_playerFlightState.subsystemRepairTimers[precedingSubsystem] == 0 &&
				g_playerFlightState.subsystemRepairTimers[currentSubsystem] != 0) {
				g_playerSubsystemRepairPriority[index - 1] = (uint8_t)currentSubsystem;
				g_playerSubsystemRepairPriority[index] = (uint8_t)precedingSubsystem;
				noSwaps = 0;
			}
		}
	} while (noSwaps == 0);
}

// FUNCTION: XW 0x453450
int16_t player_IsObjectVisible(int16_t objectIndex) {
	if (objectIndex < PLAYER_MAP_CRAFT_LIMIT &&
		g_objectTypeHudShipIds[g_objectTable[objectIndex].objectType] != 0 &&
		((CraftData*)g_objectTable[objectIndex].instanceData)->objectKind == 0)
		return 1;
	return 0;
}

// FUNCTION: XW 0x453490
void player_Map_To_Screen_Pos(Rect* bounds, int16_t mapX, int16_t mapY, int16_t* screenX, int16_t* screenY) {
	*screenX = (mapX - g_inflightMapCenterX) * g_inflightMapScaleX / PLAYER_MAP_WORLD_COORDINATE_SCALE;
	*screenX += bounds->left + ((bounds->right - bounds->left) >> 1);
	*screenY = (mapY - g_inflightMapCenterY) * g_inflightMapScaleY / PLAYER_MAP_WORLD_COORDINATE_SCALE;
	*screenY += bounds->top + ((bounds->bottom - bounds->top) >> 1);
}

// FUNCTION: XW 0x453520
void player_Screen_To_Map_Pos(Rect* bounds, int16_t screenX, int16_t screenY, int16_t* mapX, int16_t* mapY) {
	int zoomLevel = g_inflightMapZoomLevel;
	int coordinateMultiplier = PLAYER_MAP_WORLD_COORDINATE_SCALE;
	if (zoomLevel >= PLAYER_MAP_BASE_ZOOM_LEVEL + 1) {
#ifdef XW_MODERN
		coordinateMultiplier =
			(int32_t)((uint32_t)coordinateMultiplier
					  << (((uint32_t)zoomLevel - PLAYER_MAP_BASE_ZOOM_LEVEL) & PLAYER_MAP_SHIFT_COUNT_MASK));
#else
		coordinateMultiplier <<= zoomLevel - PLAYER_MAP_BASE_ZOOM_LEVEL;
#endif
	}
#ifdef XW_MODERN
	*mapX = (int16_t)((int64_t)g_inflightMapCenterX +
					  (int32_t)((uint32_t)coordinateMultiplier *
								(uint32_t)(int16_t)(screenX - ((bounds->right - bounds->left) >> 1) -
													bounds->left)) /
						  g_inflightMapScaleX);
	*mapY = (int16_t)((int64_t)g_inflightMapCenterY +
					  (int32_t)((uint32_t)coordinateMultiplier *
								(uint32_t)(int16_t)(screenY - ((bounds->bottom - bounds->top) >> 1) -
													bounds->top)) /
						  g_inflightMapScaleY);
#else
	*mapX = coordinateMultiplier * (int16_t)(screenX - ((bounds->right - bounds->left) >> 1) - bounds->left) /
				g_inflightMapScaleX +
			g_inflightMapCenterX;
	*mapY = coordinateMultiplier * (int16_t)(screenY - ((bounds->bottom - bounds->top) >> 1) - bounds->top) /
				g_inflightMapScaleY +
			g_inflightMapCenterY;
#endif
}

// FUNCTION: XW 0x4535B0
void player_LoadMissionRecords(const char* missionName) {
	char resolvedPath[PLAYER_MISSION_PATH_CAPACITY];
	char logicalPath[PLAYER_MISSION_PATH_CAPACITY];
	int16_t tryFallback;
	LandruFile* missionFile;
	shipext_Get_Mission_Path(logicalPath, missionName, 1);
	if (logicalPath[0] == ':') {
		strcpy(resolvedPath, "c:\\XwingCD\\");
		resolvedPath[0] = (char)g_installDriveLetter;
		strcat(resolvedPath, logicalPath);
		tryFallback = 0;
	} else if (logicalPath[0] == ';' || logicalPath[0] == '+') {
		strcpy(resolvedPath, "X-Wing Data\\");
		strcat(resolvedPath, &logicalPath[1]);
		tryFallback = 1;
	} else {
		strcpy(resolvedPath, logicalPath);
		/* The original fallback flag is uninitialized on this path. */
		tryFallback = 0;
	}
	missionFile = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
	if (missionFile == NULL && tryFallback != 0) {
		if (logicalPath[0] == ';') {
			strcpy(resolvedPath, "c:\\XwingCD\\");
			strcat(resolvedPath, &logicalPath[1]);
			resolvedPath[0] = (char)g_installDriveLetter;
		} else {
			strcpy(resolvedPath, &logicalPath[1]);
		}
		missionFile = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
	}
	memset(g_inflightMapSecondaryRecordHandles, 0, sizeof(g_inflightMapSecondaryRecordHandles));
	memset(g_inflightMapFlightGroupHandles, 0, sizeof(g_inflightMapFlightGroupHandles));
	if (missionFile != NULL) {
		int16_t flightGroupIndex;
		int16_t secondaryIndex;
		xfile_Read_Data_From_File(missionFile, &g_inflightMissionHeader, sizeof(g_inflightMissionHeader));
		for (flightGroupIndex = 0; flightGroupIndex < (int16_t)g_inflightMissionHeader.flightGroupCount;
			 ++flightGroupIndex) {
			LandruHandle handle = xres_Resource_Data_To_Handle(missionFile, sizeof(XwMissionFlightGroup),
															   LANDRU_MEMORY_RESOURCE);
			g_inflightMapFlightGroupHandles[flightGroupIndex] = handle;
			xmemhdl_Lock_Handle(handle);
			nullsub_SharedNoOp();
		}
		/* Preserve the original flight-group-sized reads for the secondary list. */
		for (secondaryIndex = 0; secondaryIndex < (int16_t)g_inflightMissionHeader.objectRecordCount;
			 ++secondaryIndex) {
			LandruHandle handle = xres_Resource_Data_To_Handle(missionFile, sizeof(XwMissionFlightGroup),
															   LANDRU_MEMORY_RESOURCE);
			g_inflightMapSecondaryRecordHandles[secondaryIndex] = handle;
			xmemhdl_Lock_Handle(handle);
			nullsub_SharedNoOp();
		}
		xfile_Close_File(missionFile);
	}
}

// FUNCTION: XW 0x453870
int player_Load_Display_Map(const char* missionName) {
	char resolvedPath[PLAYER_MISSION_PATH_CAPACITY];
	char logicalPath[PLAYER_MISSION_PATH_CAPACITY];
	int16_t tryFallback;
	LandruFile* briefingFile;
	uint16_t headerWord;
	uint16_t layoutCount;
	int16_t layoutIndex;
	shipext_Get_Mission_Path(logicalPath, missionName, 0);
	if (logicalPath[0] == ':') {
		strcpy(resolvedPath, "c:\\XwingCD\\");
		resolvedPath[0] = (char)g_installDriveLetter;
		strcat(resolvedPath, logicalPath);
		tryFallback = 0;
	} else if (logicalPath[0] == ';' || logicalPath[0] == '+') {
		strcpy(resolvedPath, "X-Wing Data\\");
		strcat(resolvedPath, &logicalPath[1]);
		tryFallback = 1;
	} else {
		strcpy(resolvedPath, logicalPath);
		/* The original fallback flag is uninitialized on this path. */
		tryFallback = 0;
	}
	briefingFile = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
	if (briefingFile == NULL && tryFallback != 0) {
		if (logicalPath[0] == ';') {
			strcpy(resolvedPath, "X-Wing CD:");
			strcat(resolvedPath, &logicalPath[1]);
		} else {
			strcpy(resolvedPath, &logicalPath[1]);
		}
		briefingFile = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
	}
	if (briefingFile != NULL) {
		xfile_Read_Word_From_File(briefingFile, &headerWord);
		player_ReadIconData(briefingFile);
		xfile_Read_Word_From_File(briefingFile, &layoutCount);
		g_inflightMapState->layoutCount = layoutCount;
		for (layoutIndex = 0; layoutIndex < g_inflightMapState->layoutCount; ++layoutIndex)
			player_ReadLayout(briefingFile, layoutIndex);
		player_ReadScripts(briefingFile);
		player_ReadExtendedIconData(briefingFile);
		player_ReadTextBuffers(briefingFile);
		xfile_Close_File(briefingFile);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x453B00
void player_ReadScripts(XwFile* stream) {
	uint16_t scriptCount;
	uint16_t wordCount;
	uint16_t endTime;
	uint16_t positionSetIndex;
	uint16_t layoutIndex;
	int16_t scriptIndex;

	xfile_Read_Word_From_File(stream, &scriptCount);
	g_inflightMapState->activeScriptIndex = 0;
	g_inflightMapState->scriptCount = scriptCount;
	for (scriptIndex = 0; scriptIndex < (int16_t)scriptCount; ++scriptIndex) {
		xfile_Read_Word_From_File(stream, &endTime);
		xfile_Read_Word_From_File(stream, &wordCount);
		xfile_Read_Word_From_File(stream, &positionSetIndex);
		xfile_Read_Word_From_File(stream, &layoutIndex);
		g_inflightMapState->scriptEndTime[scriptIndex] = endTime;
		g_inflightMapState->scriptWordCount[scriptIndex] = wordCount;
		g_inflightMapState->scriptPositionSetIndex[scriptIndex] = positionSetIndex;
		g_inflightMapState->scriptLayoutIndex[scriptIndex] = layoutIndex;
		xfile_Read_Data_From_File(stream, g_inflightMapState->scriptWords[scriptIndex],
								  (int16_t)wordCount *
									  (int)sizeof(g_inflightMapState->scriptWords[scriptIndex][0]));
	}
	player_ApplyBriefingLayout(g_inflightMapState->scriptLayoutIndex[g_inflightMapState->activeScriptIndex]);
}

// FUNCTION: XW 0x453C20
void player_ReadIconData(XwFile* stream) {
	uint16_t iconCount, positionSetCount, value;
	int16_t positionSetIndex, iconIndex;
	xfile_Read_Word_From_File(stream, &iconCount);
	xfile_Read_Word_From_File(stream, &positionSetCount);
	g_inflightMapState->field_00C2 = 0;
	g_inflightMapState->mapIconCount = iconCount;
	g_inflightMapState->field_00C6 = 0;
	g_inflightMapState->mapPositionSetCount = positionSetCount;
	for (positionSetIndex = 0; positionSetIndex < (int16_t)positionSetCount; ++positionSetIndex) {
		for (iconIndex = 0; iconIndex < (int16_t)iconCount; ++iconIndex) {
			xfile_Read_Word_From_File(stream, &value);
			g_inflightMapState->mapIconX[positionSetIndex][iconIndex] = value;
			xfile_Read_Word_From_File(stream, &value);
			g_inflightMapState->mapIconY[positionSetIndex][iconIndex] = value;
			xfile_Read_Word_From_File(stream, &value);
			g_inflightMapState->mapIconZ[positionSetIndex][iconIndex] = value;
		}
	}
	for (iconIndex = 0; iconIndex < (int16_t)iconCount; ++iconIndex) {
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->mapIconType[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->mapIconColorOverride[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0232[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0282[iconIndex] = value;
		xfile_Read_Data_From_File(stream, g_inflightMapState->mapIconName[iconIndex],
								  sizeof(g_inflightMapState->mapIconName[iconIndex]));
		xfile_Read_Data_From_File(stream, g_inflightMapState->field_0552[iconIndex],
								  sizeof(g_inflightMapState->field_0552[iconIndex]));
		xfile_Read_Data_From_File(stream, g_inflightMapState->field_07D2[iconIndex],
								  sizeof(g_inflightMapState->field_07D2[iconIndex]));
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0A52[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0E62[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0EB2[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0F02[iconIndex] = value;
	}
}

// FUNCTION: XW 0x453EB0
void player_ReadLayout(XwFile* stream, int16_t layoutIndex) {
	Rect viewportRect;
	uint16_t active;
	int viewportIndex;
	for (viewportIndex = 0; viewportIndex < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewportIndex) {
		xfile_Read_Data_From_File(stream, &viewportRect, sizeof(viewportRect));
		xrect_Copy_Rect(&g_inflightMapState->layoutRects[layoutIndex][viewportIndex], &viewportRect);
		xfile_Read_Word_From_File(stream, &active);
		g_inflightMapState->layoutActive[layoutIndex][viewportIndex] = active;
	}
}

// FUNCTION: XW 0x453F30
void player_ReadExtendedIconData(XwFile* stream) {
	uint16_t value;
	int16_t iconIndex = 0;
	int blockIndex;
	g_inflightMapState->field_10E2 = 0;
	g_inflightMapState->field_10E4 = 0;
	xfile_Read_Word_From_File(stream, &value);
	g_inflightMapState->field_00CA = value;
	xfile_Read_Word_From_File(stream, &value);
	g_inflightMapState->field_00CC = value;
	xfile_Read_Word_From_File(stream, &value);
	g_inflightMapState->field_00D0 = value;
	xfile_Read_Word_From_File(stream, &value);
	g_inflightMapState->mapColorMode = value;
	for (blockIndex = 0; blockIndex < XW_BRIEFING_EXTENDED_HEADER_BLOCK_COUNT; ++blockIndex)
		xfile_Read_Data_From_File(stream, g_inflightMapState->extendedHeaderBlocks[blockIndex],
								  sizeof(g_inflightMapState->extendedHeaderBlocks[blockIndex]));
	for (; iconIndex < g_inflightMapState->mapIconCount; ++iconIndex) {
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0F52[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0FA2[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_0FF2[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1042[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1092[iconIndex] = value;
		xfile_Read_Data_From_File(stream, g_inflightMapState->field_10E6[iconIndex],
								  sizeof(g_inflightMapState->field_10E6[iconIndex]));
		xfile_Read_Data_From_File(stream, g_inflightMapState->field_1316[iconIndex],
								  sizeof(g_inflightMapState->field_1316[iconIndex]));
		xfile_Read_Data_From_File(stream, g_inflightMapState->field_1546[iconIndex],
								  sizeof(g_inflightMapState->field_1546[iconIndex]));
		xfile_Read_Data_From_File(stream, g_inflightMapState->field_1776[iconIndex],
								  sizeof(g_inflightMapState->field_1776[iconIndex]));
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_19A6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->mapIconPlayerShipFlag[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1A46[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1A96[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1AE6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1B36[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1B86[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1BD6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1C26[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1C76[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1CC6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_inflightMapState->field_1D16[iconIndex] = value;
	}
}

// FUNCTION: XW 0x4542E0
void player_ReadTextBuffers(XwFile* stream) {
	int16_t labelCount;
	int16_t textCount;
	int16_t labelIndex;
	int16_t textIndex;
	xfile_Read_Word_From_File(stream, (uint16_t*)&labelCount);
	for (labelIndex = 0; labelIndex < XW_BRIEFING_PAGE_LABEL_CAPACITY; ++labelIndex) {
		if (labelIndex < labelCount) {
			int16_t length;
			char* text;
			xfile_Read_Word_From_File(stream, (uint16_t*)&length);
			text = (char*)xmemhdl_Lock_Handle(g_inflightMapState->labelTextHandles[labelIndex]);
			xfile_Read_Data_From_File(stream, text, length);
			text[length] = '\0';
			xmemhdl_Unlock_Handle(g_inflightMapState->labelTextHandles[labelIndex]);
		} else {
			xmemhdl_Handle_nSet(g_inflightMapState->labelTextHandles[labelIndex], 0, sizeof(char));
		}
	}
	xfile_Read_Word_From_File(stream, (uint16_t*)&textCount);
	for (textIndex = 0; textIndex < XW_BRIEFING_TEXT_BUFFER_CAPACITY; ++textIndex) {
		xmemhdl_Handle_nSet(g_inflightMapState->textBlockHandles[textIndex], 0, XW_BRIEFING_TEXT_BUFFER_SIZE);
		xmemhdl_Handle_nSet(g_inflightMapState->textAttributeHandles[textIndex], 0,
							XW_BRIEFING_TEXT_BUFFER_SIZE);
		if (textIndex < textCount) {
			int16_t length;
			char* text;
			uint8_t* attributes;
			xfile_Read_Word_From_File(stream, (uint16_t*)&length);
			text = (char*)xmemhdl_Lock_Handle(g_inflightMapState->textBlockHandles[textIndex]);
			xfile_Read_Data_From_File(stream, text, length);
			text[length] = '\0';
			xmemhdl_Unlock_Handle(g_inflightMapState->textBlockHandles[textIndex]);
			attributes = (uint8_t*)xmemhdl_Lock_Handle(g_inflightMapState->textAttributeHandles[textIndex]);
			xfile_Read_Data_From_File(stream, attributes, length);
			xmemhdl_Unlock_Handle(g_inflightMapState->textAttributeHandles[textIndex]);
		}
	}
}
