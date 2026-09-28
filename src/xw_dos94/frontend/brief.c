#include "xw_dos94/frontend/brief.h"
#include "xw_dos94/frontend/brief_officers.h"
#include "xw_runtime/integration/landru_adapter.h"
#include "xw_runtime/storage/storage.h"
#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#endif
#include "xw/frontend/brief.h"

#include "xw/audio/lolevel.h"
#include "xw/audio/sound.h"
#include "xw/audio/soundext.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/shell_flight.h"
#include "xw/frontend/player.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/brief_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/brief_music_task.h"
#include "xw_runtime/runtime/brief_view_task.h"
#endif

#include <ctype.h>
#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
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
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void Dos94_brief_end_View(int time);
static void Dos94_brief_iuser_Brief(Input* input, int time);
static void Dos94_brief_DrawDoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_brief_HandlePlaybackButton(Input* input, int time);
static int16_t Dos94_brief_UpdatePageLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										   int rightEvent, int16_t x, int16_t y);
static void Dos94_brief_DrawPageLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh);
static void Dos94_brief_user_Door(Actor* actor, int time);
static void Dos94_brief_AdvanceOrResetAtEnd(Actor* actor, int time);
static int16_t Dos94_brief_DrawMapViewport(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										   int16_t refresh);
static void Dos94_brief_SelectPage(int16_t pageIndex);
static void Dos94_brief_Rewind_Page(int16_t scriptIndex);
static void Dos94_brief_Step_Page(int16_t scriptIndex, int16_t initializeState);
static int16_t Dos94_brief_Seek_Page(int16_t scriptIndex, int16_t targetTime, int16_t initializeState);
static void Dos94_brief_Seek_Page_Section(int16_t scriptIndex);
static void Dos94_brief_Reseek_Page(int16_t scriptIndex);
static int16_t Dos94_brief_DrawViewportAndSelection(const Rect* viewportRect, const Rect* clipRect,
													int16_t redrawText);
static int16_t Dos94_brief_DrawOverlays(const Rect* viewportRect, const Rect* clipRect);
static void Dos94_brief_Draw_Map_Paragraph(const Rect* rect, LandruHandle textHandle,
										   LandruHandle attributeHandle, int16_t suppressCenteredHeadings,
										   int16_t textSlotIndex);
static void Dos94_brief_InitRuntime(int16_t reuseTextBuffers);
static void Dos94_brief_InitDefaultScript(int16_t scriptIndex);
static void Dos94_brief_ApplyLayout(int16_t layoutIndex);
static void Dos94_brief_LoadMissionChoice(int16_t choiceIndex, int16_t reuseTextBuffers);
static int Dos94_brief_LoadBriefingFile(const char* missionName);
static void Dos94_brief_ReadScripts(XwFile* stream);

/* DOS94 0x180fb4. */
static void Dos94_brief_end_View(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0) {
		if (g_briefingHasMissionChoice != 0) {
			if (shellext_MoveGridFocus(&g_briefingFocusIndex, g_briefingMissionChoiceFocusX,
									   g_briefingMissionChoiceFocusY, XW_BRIEFING_CHOICE_FOCUS_ROWS,
									   XW_BRIEFING_FOCUS_COLUMNS, key) != 0) {
				xio_Set_Mouse_Position(g_briefingMissionChoiceFocusX[g_briefingFocusIndex],
									   g_briefingMissionChoiceFocusY[g_briefingFocusIndex]);
				xio_Get_Key();
			}
		} else {
			if (shellext_MoveGridFocus(&g_briefingFocusIndex, g_briefingFocusX, g_briefingFocusY,
									   XW_BRIEFING_FOCUS_ROWS, XW_BRIEFING_FOCUS_COLUMNS, key) != 0) {
				xio_Set_Mouse_Position(g_briefingFocusX[g_briefingFocusIndex],
									   g_briefingFocusY[g_briefingFocusIndex]);
				xio_Get_Key();
			}
		}
	}
	brief_HandleSoundAction(XW_BRIEF_SOUND_TICK);
	if (g_briefingFilm->cur_cel != g_briefingFilm->cels)
		xactor_Refresh_Actor(g_briefingTextActor);
	if (g_briefingRequestedNarrationPage != g_briefingLoadedNarrationPage) {
		if (g_briefingNarrationSound != NULL) {
			xsound_Free_Sound(g_briefingNarrationSound);
			g_briefingNarrationSound = NULL;
		}
		g_briefingLoadedNarrationPage = g_briefingRequestedNarrationPage;
		brief_LoadNarration();
	}
}

/* DOS94 0x18127a. */
static void Dos94_brief_iuser_Brief(Input* input, int time) {
	(void)time;
	switch (input->var1) {
		case 0:
			if (xinpattr_Is_Input_Visible(g_briefingDoorHintInput) &&
				input->id == g_briefingDoorHintInput->var1) {
				xinpattr_Show_Input(g_briefingHintCompanionInput);
				xinpattr_Hide_Input(g_briefingDoorHintInput);
				xactor_Refresh_Actor(g_briefingInteriorActor);
				xinpattr_Refresh_Input(g_briefingHintCompanionInput);
			}
			break;
		case XW_BRIEF_DOOR_ACTION_EXIT:
			xerror_Set_Landru_Exit(input->var2);
			if (input->var2 == 122) {
				brief_CommitMissionChoice();
				brief_WritePilotRecord();
			}
			break;
		case XW_BRIEF_DOOR_ACTION_HOVER:
			if (xinpattr_Is_Input_Visible(g_briefingHintCompanionInput)) {
				xinpattr_Hide_Input(g_briefingHintCompanionInput);
				xinpattr_Show_Input(g_briefingDoorHintInput);
				xactor_Refresh_Actor(g_briefingInteriorActor);
				xinpattr_Refresh_Input(g_briefingDoorHintInput);
				g_briefingDoorHintInput->var1 = input->id;
			}
			input->var1 = 0;
			break;
	}
}

/* DOS94 0x1813f6. */
static void Dos94_brief_DrawDoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char text[XW_BRIEF_DOOR_HINT_CAPACITY];
	(void)clip;
	if (refresh == 0) {
		return;
	}
	strcpy(text, input->var1 == 0 ? "Abort Mission" : "Enter Mission");
	xrect_Offset_Rect(frame, 1, 1);
	xfont_Print_Centered_Text(text, frame, 0, 16);
	xrect_Offset_Rect(frame, -1, -1);
	xfont_Print_Centered_Text(text, frame, 0, 15);
}

/* DOS94 0x181482. */
static void Dos94_brief_HandlePlaybackButton(Input* input, int time) {
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		int16_t id = input->id;
		switch (id) {
			case XW_BRIEF_BUTTON_PREVIOUS_PAGE:
			case XW_BRIEF_BUTTON_NEXT_PAGE:
				if (g_briefingRuntime->scriptCount > 1) {
					if (id == XW_BRIEF_BUTTON_NEXT_PAGE)
						Dos94_brief_SelectPage(g_briefingRuntime->activeScriptIndex + 1);
					else
						Dos94_brief_SelectPage(g_briefingRuntime->activeScriptIndex - 1);
					Dos94_brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
					xinput_Refresh_System_Inputs();
					g_briefingSelectedIconPlusOne = 0;
				}
				break;
			case XW_BRIEF_BUTTON_REWIND:
				Dos94_brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
				break;
			case XW_BRIEF_BUTTON_STOP:
				xinpattr_Refresh_Input(g_briefingWorldInput);
				g_briefingRuntime->playbackActive = 0;
				break;
			case XW_BRIEF_BUTTON_PLAY:
				xinpattr_Refresh_Input(g_briefingWorldInput);
				g_briefingRuntime->playbackActive = 1;
				g_briefingSelectedIconPlusOne = 0;
				break;
			case XW_BRIEF_BUTTON_MISSION_A:
			case XW_BRIEF_BUTTON_MISSION_B:
				if (id - XW_BRIEF_BUTTON_MISSION_A != g_briefingMissionChoice) {
					Dos94_brief_LoadMissionChoice(id - XW_BRIEF_BUTTON_MISSION_A, 1);
					xview_Refresh_View();
				}
				break;
			default:
				break;
		}
	}
}

/* DOS94 0x181752. */
static int16_t Dos94_brief_UpdatePageLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
										   int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0)
		return 0;
	if (leftEvent == XW_BRIEF_PAGE_CLICK_EVENT || rightEvent == XW_BRIEF_PAGE_CLICK_EVENT)
		Dos94_brief_Seek_Page_Section(g_briefingRuntime->activeScriptIndex);
	return 0;
}

/* DOS94 0x181780. */
static void Dos94_brief_DrawPageLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char pageLabel[XW_BRIEFING_PAGE_LABEL_CAPACITY];

	(void)input;
	(void)clip;
	if (refresh != 0) {
		xstyle_Style_Paint_TextField(frame);
		sprintf(pageLabel, "Page %d of %d", g_briefingRuntime->activeScriptIndex + 1,
				g_briefingRuntime->scriptCount);
		xfont_Print_Centered_Text(pageLabel, frame, XW_BRIEFING_PAGE_FONT, XW_BRIEFING_PAGE_TEXT_COLOR);
	}
}

/* DOS94 0x1817e2. */
static void Dos94_brief_user_Door(Actor* actor, int time) {
	int refresh = 1;
	(void)time;
	if (actor->var1) {
		if (actor->state == actor->arraySize - 1)
			brief_HandleSoundAction(1);
		if (actor->state)
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		else
			refresh = 0;
		actor->var1 = 0;
	} else {
		if (actor->state == 0)
			brief_HandleSoundAction(2);
		if (actor->state < actor->arraySize - 1)
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		else
			refresh = 0;
	}
	if (refresh)
		xactor_Refresh_Actor(actor);
}

/* DOS94 0x18188e. */
static void Dos94_brief_AdvanceOrResetAtEnd(Actor* actor, int time) {
	(void)actor;
	if (g_briefingRuntime->playbackActive != 0 && time != 0) {
		int16_t script = g_briefingRuntime->activeScriptIndex;
		if (g_briefingRuntime->scriptCurrentTime[script] < g_briefingRuntime->scriptEndTime[script])
			Dos94_brief_Step_Page(script, 0);
		else
			Dos94_brief_Rewind_Page(script);
		brief_Move_Display_Map();
	}
}

/* DOS94 0x1818f2. */
static int16_t Dos94_brief_DrawMapViewport(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
										   int16_t refresh) {
	(void)actor;
	(void)x;
	(void)y;
	Dos94_brief_DrawViewportAndSelection(frame, clip, refresh);
	xcanvas_Max_Drawing_Canvas_Clip();
	brief_SetOfficerRegionRestored(0);
	return 1;
}

/* DOS94 0x1820ea. */
static void Dos94_brief_SelectPage(int16_t pageIndex) {
	int16_t scriptCount = g_briefingRuntime->scriptCount;
	if (pageIndex >= scriptCount)
		pageIndex -= scriptCount;
	if (pageIndex < 0)
		pageIndex += scriptCount;
	g_briefingRuntime->activeScriptIndex = pageIndex;
	Dos94_brief_Reseek_Page(pageIndex);
	Dos94_brief_ApplyLayout(g_briefingRuntime->scriptLayoutIndex[pageIndex]);
}

/* DOS94 0x182158. */
static void Dos94_brief_Rewind_Page(int16_t scriptIndex) {
	int16_t slot;
	g_briefingRuntime->mapCenter[0] = 0;
	g_briefingRuntime->mapCenter[1] = 0;
	g_briefingRuntime->mapTargetCenter[0] = 0;
	g_briefingRuntime->mapTargetCenter[1] = 0;
	g_briefingRuntime->mapScale[0] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->mapScale[1] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[0] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[1] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->field_37DC = 0;
	for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->textSlotActive) /
								sizeof(g_briefingRuntime->textSlotActive[0]));
		 ++slot)
		g_briefingRuntime->textSlotActive[slot] = 0;
	for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->fgMarkerActive) /
								sizeof(g_briefingRuntime->fgMarkerActive[0]));
		 ++slot)
		g_briefingRuntime->fgMarkerActive[slot] = 0;
	for (slot = 0;
		 slot < (int)(sizeof(g_briefingRuntime->labelActive) / sizeof(g_briefingRuntime->labelActive[0]));
		 ++slot)
		g_briefingRuntime->labelActive[slot] = 0;
	g_briefingRuntime->scriptCurrentTime[scriptIndex] = 0;
	g_briefingRuntime->scriptCursorWordIndex[scriptIndex] = 0;
	Dos94_brief_Step_Page(scriptIndex, 1);
}

/* DOS94 0x18227e. */
static void Dos94_brief_Step_Page(int16_t scriptIndex, int16_t initializeState) {
	int16_t cursor = g_briefingRuntime->scriptCursorWordIndex[scriptIndex];
	int16_t commandStart = cursor;
	int16_t commandTime = g_briefingRuntime->scriptWords[scriptIndex][cursor];
	int16_t refreshText = 0;
	int16_t args[XW_BRIEF_COMMAND_ARGUMENT_CAPACITY];
	g_briefingRuntime->field_37E6 = 0;
	g_briefingRuntime->textSlotsChanged = 0;
	g_briefingRuntime->fgMarkersChanged = 0;
	g_briefingRuntime->labelsChanged = 0;
	g_briefingRuntime->mapCenterDirty = 0;
	g_briefingRuntime->mapScaleDirty = 0;
	g_briefingRuntime->pauseMarkerReached = 0;
	if (commandTime <= g_briefingRuntime->scriptCurrentTime[scriptIndex]) {
		do {
			int16_t opcode, argCount;
			int argIndex;
			commandStart = cursor;
			commandTime = g_briefingRuntime->scriptWords[scriptIndex][cursor++];
			opcode = g_briefingRuntime->scriptWords[scriptIndex][cursor++];
			argCount = g_briefingScriptOpcodeArgCounts[opcode];
			for (argIndex = 0; argCount > 0; ++argIndex, --argCount)
				args[argIndex] = g_briefingRuntime->scriptWords[scriptIndex][cursor++];
			if (commandTime == g_briefingRuntime->scriptCurrentTime[scriptIndex]) {
				switch (opcode) {
					case XW_BRIEF_COMMAND_PAUSE:
						g_briefingRuntime->pauseMarkerReached = 1;
						break;
					case XW_BRIEF_COMMAND_CLEAR_TEXT: {
						int16_t slot;
						for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->textSlotActive) /
													sizeof(g_briefingRuntime->textSlotActive[0]));
							 ++slot) {
							if (g_briefingRuntime->textSlotActive[slot] != 0)
								g_briefingRuntime->textSlotActive[slot] = 0;
						}
						g_briefingRuntime->textSlotsChanged = 1;
						refreshText = 1;
						break;
					}
					case XW_BRIEF_COMMAND_TEXT_FIRST:
					case XW_BRIEF_COMMAND_TEXT_FIRST + 1:
					case XW_BRIEF_COMMAND_TEXT_FIRST + 2:
					case XW_BRIEF_COMMAND_TEXT_LAST: {
						int16_t slot = opcode - XW_BRIEF_COMMAND_TEXT_FIRST;
						if (g_briefingRuntime->textSlotActive[slot] == 0) {
							g_briefingRuntime->textSlotActive[slot] = 1;
							g_briefingRuntime->textSlotBlockIndex[slot] = args[0];
							if (slot == XW_BRIEF_NARRATION_TEXT_SLOT)
								g_briefingRequestedNarrationPage = args[0];
						}
						refreshText = 1;
						break;
					}
					case XW_BRIEF_COMMAND_CENTER:
						if (commandTime == 0 || initializeState != 0) {
							g_briefingRuntime->mapTargetCenter[0] = args[0];
							g_briefingRuntime->mapCenter[0] = args[0];
							g_briefingRuntime->mapTargetCenter[1] = args[1];
							g_briefingRuntime->mapCenter[1] = args[1];
						} else if (g_briefingRuntime->mapTargetCenter[0] != args[0] ||
								   g_briefingRuntime->mapTargetCenter[1] != args[1]) {
							g_briefingRuntime->mapTargetCenter[0] = args[0];
							g_briefingRuntime->mapTargetCenter[1] = args[1];
						}
						g_briefingRuntime->mapCenterDirty = 1;
						break;
					case XW_BRIEF_COMMAND_SCALE:
						if (commandTime == 0 || initializeState != 0) {
							g_briefingRuntime->mapTargetScale[0] = args[0];
							g_briefingRuntime->mapScale[0] = g_briefingRuntime->mapTargetScale[0];
							g_briefingRuntime->mapTargetScale[1] = args[1];
							g_briefingRuntime->mapScale[1] = g_briefingRuntime->mapTargetScale[1];
						} else if (g_briefingRuntime->mapTargetScale[0] != args[0] ||
								   g_briefingRuntime->mapTargetScale[1] != args[1]) {
							g_briefingRuntime->mapTargetScale[0] = args[0];
							g_briefingRuntime->mapTargetScale[1] = args[1];
						}
						g_briefingRuntime->mapScaleDirty = 1;
						break;
					case XW_BRIEF_COMMAND_CLEAR_MARKERS: {
						int16_t slot;
						for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->fgMarkerActive) /
													sizeof(g_briefingRuntime->fgMarkerActive[0]));
							 ++slot) {
							if (g_briefingRuntime->fgMarkerActive[slot] != 0)
								g_briefingRuntime->fgMarkerActive[slot] = 0;
						}
						g_briefingRuntime->fgMarkersChanged = 1;
						break;
					}
					case XW_BRIEF_COMMAND_MARKER_FIRST:
					case XW_BRIEF_COMMAND_MARKER_FIRST + 1:
					case XW_BRIEF_COMMAND_MARKER_FIRST + 2:
					case XW_BRIEF_COMMAND_MARKER_LAST: {
						int16_t slot = opcode - XW_BRIEF_COMMAND_MARKER_FIRST;
						if (g_briefingRuntime->fgMarkerActive[slot] == 0) {
							g_briefingRuntime->fgMarkerActive[slot] = 1;
							g_briefingRuntime->fgMarkerAge[slot] =
								initializeState == 1 ? XW_BRIEF_INITIALIZED_AGE : 0;
							g_briefingRuntime->fgMarkerIconIndex[slot] = args[0];
						}
						break;
					}
					case XW_BRIEF_COMMAND_CLEAR_LABELS: {
						int16_t slot;
						for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->labelActive) /
													sizeof(g_briefingRuntime->labelActive[0]));
							 ++slot) {
							if (g_briefingRuntime->labelActive[slot] != 0)
								g_briefingRuntime->labelActive[slot] = 0;
						}
						g_briefingRuntime->labelsChanged = 1;
						break;
					}
					case XW_BRIEF_COMMAND_LABEL_FIRST:
					case XW_BRIEF_COMMAND_LABEL_FIRST + 1:
					case XW_BRIEF_COMMAND_LABEL_FIRST + 2:
					case XW_BRIEF_COMMAND_LABEL_LAST: {
						int16_t slot = opcode - XW_BRIEF_COMMAND_LABEL_FIRST;
						if (g_briefingRuntime->labelActive[slot] == 0) {
							g_briefingRuntime->labelActive[slot] = 1;
							g_briefingRuntime->labelAge[slot] =
								initializeState == 1 ? XW_BRIEF_INITIALIZED_AGE : 0;
							g_briefingRuntime->labelTextIndex[slot] = args[0];
							g_briefingRuntime->labelX[slot] = args[1];
							g_briefingRuntime->labelY[slot] = args[2];
						}
						break;
					}
					case XW_BRIEF_COMMAND_CLEAR_FIELD_37DC:
						if (g_briefingRuntime->field_37DC != 0) {
							g_briefingRuntime->field_37DC = 0;
							g_briefingRuntime->field_37E6 = 1;
						}
						break;
					case XW_BRIEF_COMMAND_SET_FIELD_37DC:
						if (g_briefingRuntime->field_37DC == 0) {
							g_briefingRuntime->field_37DC = 1;
							g_briefingRuntime->field_37E4 =
								initializeState == 1 ? XW_BRIEF_INITIALIZED_AGE : 0;
							g_briefingRuntime->field_37DE = args[0];
							g_briefingRuntime->field_37E0 = args[1];
							g_briefingRuntime->field_37E2 = args[2];
						}
						break;
					default:
						break;
				}
			}
		} while (commandTime <= g_briefingRuntime->scriptCurrentTime[scriptIndex]);
	}
	++g_briefingRuntime->scriptCurrentTime[scriptIndex];
	g_briefingRuntime->scriptCursorWordIndex[scriptIndex] = commandStart;
	if (refreshText != 0 && g_briefingTextActor != NULL)
		xactor_Refresh_Actor(g_briefingTextActor);
}

/* DOS94 0x1827a4. */
static int16_t Dos94_brief_Seek_Page(int16_t scriptIndex, int16_t targetTime, int16_t initializeState) {
	if (targetTime != g_briefingRuntime->scriptCurrentTime[scriptIndex] - 1) {
		if (targetTime < g_briefingRuntime->scriptCurrentTime[scriptIndex])
			Dos94_brief_Rewind_Page(scriptIndex);
		while (g_briefingRuntime->scriptCurrentTime[scriptIndex] <= targetTime)
			Dos94_brief_Step_Page(scriptIndex, initializeState);
		return 1;
	}
	return 0;
}

/* DOS94 0x18282a. */
static void Dos94_brief_Seek_Page_Section(int16_t scriptIndex) {
	int16_t foundStop = 0;
	int16_t nextOpcode = 0;
	int16_t textVisible = 0;
	int16_t visibleTextFrames = 0;
	int16_t originalTime = g_briefingRuntime->scriptCurrentTime[scriptIndex];
	int16_t seekTime, seekOpcode;
	Dos94_brief_Rewind_Page(scriptIndex);
	while (foundStop == 0 && nextOpcode != XW_BRIEF_COMMAND_END) {
		int16_t slot;
		nextOpcode =
			g_briefingRuntime
				->scriptWords[scriptIndex][g_briefingRuntime->scriptCursorWordIndex[scriptIndex] + 1];
		if (g_briefingRuntime->textSlotsChanged != 0) {
			visibleTextFrames = 0;
			textVisible = 0;
		}
		for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->textSlotActive) /
									sizeof(g_briefingRuntime->textSlotActive[0]));
			 ++slot) {
			if (g_briefingRuntime->textSlotActive[slot] != 0)
				textVisible = 1;
		}
		if (textVisible != 0)
			++visibleTextFrames;
		if ((g_briefingRuntime->mapScaleDirty != 0 || g_briefingRuntime->mapCenterDirty != 0 ||
			 g_briefingRuntime->pauseMarkerReached != 0 || visibleTextFrames == 1) &&
			g_briefingRuntime->scriptCurrentTime[scriptIndex] >= originalTime)
			foundStop = 1;
		else
			Dos94_brief_Step_Page(scriptIndex, 1);
	}
	if ((g_briefingRuntime->pauseMarkerReached != 0 || visibleTextFrames == 1) && foundStop == 1) {
		seekTime = g_briefingRuntime->scriptCurrentTime[scriptIndex];
		seekOpcode = 0;
	} else {
		int cursor = g_briefingRuntime->scriptCursorWordIndex[scriptIndex];
		seekTime = g_briefingRuntime->scriptWords[scriptIndex][cursor];
		seekOpcode = g_briefingRuntime->scriptWords[scriptIndex][cursor + 1];
	}
	if (seekOpcode == XW_BRIEF_COMMAND_END)
		Dos94_brief_Rewind_Page(scriptIndex);
	else
		Dos94_brief_Seek_Page(scriptIndex, seekTime, 0);
}

/* DOS94 0x1829ec. */
static void Dos94_brief_Reseek_Page(int16_t scriptIndex) {
	int16_t savedTime = g_briefingRuntime->scriptCurrentTime[scriptIndex];
	if (savedTime == 0)
		savedTime = 1;
	Dos94_brief_Rewind_Page(scriptIndex);
	Dos94_brief_Seek_Page(scriptIndex, savedTime - 1, 1);
}

/* DOS94 0x182bea. */
static int16_t Dos94_brief_DrawViewportAndSelection(const Rect* viewportRect, const Rect* clipRect,
													int16_t redrawText) {
	Rect contentRect;
	Rect selectionRect;
	Rect clippedRect;
	char text[XW_BRIEFING_SELECTION_LABEL_SIZE];
	int16_t textSlot;
	if (redrawText != 0) {
		xpaint_Paint_Clipped_Rect((Rect*)viewportRect, 0);
		for (textSlot = 0; textSlot < (int)(sizeof(g_briefingRuntime->textViewportActive) /
											sizeof(g_briefingRuntime->textViewportActive[0]));
			 ++textSlot) {
			if (g_briefingRuntime->textViewportActive[textSlot] != 0) {
				xrect_Copy_Rect(&contentRect, &g_briefingRuntime->textViewportRects[textSlot]);
				xrect_Offset_Rect(&contentRect, viewportRect->left, viewportRect->top);
				xrect_Copy_Rect(&clippedRect, (Rect*)clipRect);
				xrect_Clip_Rect(&clippedRect, &contentRect);
				xcanvas_Set_Drawing_Canvas_Clip(&clippedRect);
				xpaint_Paint_Clipped_Rect(&contentRect, XW_BRIEFING_TEXT_BACKGROUND_COLOR);
				if (g_briefingRuntime->textSlotActive[textSlot] != 0) {
					int textBlockIndex = g_briefingRuntime->textSlotBlockIndex[textSlot];
					LandruHandle textHandle = g_briefingRuntime->textBlockHandles[textBlockIndex];
					LandruHandle attributeHandle = g_briefingRuntime->textAttributeHandles[textBlockIndex];
					xrect_Inset_Rect(&contentRect, 2, 1);
					Dos94_brief_Draw_Map_Paragraph(&contentRect, textHandle, attributeHandle, 0, textSlot);
				}
			}
		}
		xcanvas_Set_Drawing_Canvas_Clip((Rect*)clipRect);
	}
	if (g_briefingRuntime->mapViewportActive != 0) {
		xrect_Copy_Rect(&contentRect, &g_briefingRuntime->mapViewportRect);
		xrect_Offset_Rect(&contentRect, viewportRect->left, viewportRect->top);
		xrect_Copy_Rect(&clippedRect, (Rect*)clipRect);
		xrect_Clip_Rect(&clippedRect, &contentRect);
		xcanvas_Set_Drawing_Canvas_Clip(&clippedRect);
		brief_Draw_Display_Grid(&contentRect, &clippedRect);
		Dos94_brief_DrawOverlays(&contentRect, &clippedRect);
		if (g_briefingSelectedIconPlusOne != 0) {
			int16_t previewType;
			int16_t colorOverride;
			int16_t colorGroup;
			int16_t characterIndex;
			int16_t showLabel;
			const char* labelText;
			xrect_Copy_Rect(&selectionRect, &contentRect);
			selectionRect.right = selectionRect.left + 70;
			selectionRect.bottom = selectionRect.top + 40;
			if (xio_Mouse_X() < XW_BRIEFING_SELECTION_MOUSE_LIMIT) {
				xrect_Offset_Rect(&selectionRect, XW_BRIEFING_SELECTION_SHIFT, 0);
			}
			xpaint_Frame_Clipped_Rect(&selectionRect, XW_BRIEF_HIGHLIGHT_FRAME_COLOR);
			xrect_Inset_Rect(&selectionRect, 1, 1);
			xpaint_Paint_Clipped_Rect(&selectionRect, XW_BRIEF_HIGHLIGHT_FILL_COLOR);
			previewType = g_briefingRuntime->mapIconType[g_briefingSelectedIconPlusOne - 1];
			colorOverride = g_briefingRuntime->mapIconColorOverride[g_briefingSelectedIconPlusOne - 1];
			if (previewType <= XW_BRIEFING_LAST_SHIP_ICON_TYPE) {
				if (colorOverride == 0) {
					colorGroup = g_briefingMapDefaultIconColorGroups[previewType - 1];
				} else {
					colorGroup = colorOverride - 1;
				}
			} else {
				colorGroup = 0;
			}
			if (colorGroup < 0)
				colorGroup = 0;
			if (colorGroup > XW_BRIEFING_SELECTION_COLOR_COUNT - 1)
				colorGroup = XW_BRIEFING_SELECTION_COLOR_COUNT - 1;
			if (previewType >= XW_BRIEFING_ICON_MINE_FIRST && previewType <= XW_BRIEFING_ICON_MINE_LAST)
				previewType = XW_BRIEFING_ICON_MINE_FIRST;
			if (previewType >= XW_BRIEFING_ICON_COMMUNICATION && previewType < XW_BRIEFING_ICON_BWING)
				previewType -= XW_BRIEFING_MINE_VARIANT_COUNT;
			if (previewType == XW_BRIEFING_ICON_BWING) {
				previewType = XW_BRIEFING_PREVIEW_BWING;
				xactdelt_Draw_Delta_Actor(g_briefingBwingRadarActor, &contentRect, &clippedRect,
										  selectionRect.left + 1, selectionRect.top + 7, 1);
			} else {
				xactor_Set_Actor_State(g_briefingSelectionPreviewActor, previewType - 1, 0);
				xactanim_Draw_Anim_Actor(g_briefingSelectionPreviewActor, &contentRect, &clippedRect,
										 selectionRect.left + 1,
										 selectionRect.top + XW_BRIEFING_SELECTION_PREVIEW_Y, 1);
			}
			strcpy(text, g_briefingSelectionTypeLabels[previewType - 1]);
			strcat(text, ": ");
			showLabel = colorGroup == 0;
			showLabel |= (previewType > XW_BRIEFING_CONCEALED_TYPE_LAST) |
						 (previewType < XW_BRIEFING_CONCEALED_TYPE_FIRST);
			if (showLabel != 0) {
				labelText = g_briefingRuntime->mapIconName[g_briefingSelectedIconPlusOne - 1];
				strcat(text, labelText);
			} else {
				labelText = "UNKNOWN";
				strcat(text, labelText);
			}
			for (characterIndex = 0; characterIndex < (int)sizeof(text); ++characterIndex) {
				if (text[characterIndex] == 0)
					break;
				text[characterIndex] = toupper(text[characterIndex]);
			}
			xfont_Print_Clipped_Text(text, selectionRect.left + XW_BRIEFING_SELECTION_LABEL_X,
									 selectionRect.top + 1, 1, g_briefingSelectionTextColors[colorGroup]);
			return 1;
		}
	}
	return 1;
}

/* DOS94 0x1832de. */
static int16_t Dos94_brief_DrawOverlays(const Rect* viewportRect, const Rect* clipRect) {
	int16_t positionSetIndex =
		g_briefingRuntime->scriptPositionSetIndex[g_briefingRuntime->activeScriptIndex];
	Rect viewport;
	Rect highlightRect;
	char text[XW_BRIEF_READOUT_CAPACITY];
	int16_t x, y, offsetX, offsetY;
	int markerIndex;
	int16_t iconIndex;
	int labelIndex;
	int markersRemaining;
	int labelsRemaining;
	xrect_Copy_Rect(&viewport, (Rect*)viewportRect);
	for (markerIndex = 0, markersRemaining = sizeof(g_briefingRuntime->fgMarkerActive) /
											 sizeof(g_briefingRuntime->fgMarkerActive[0]);
		 markersRemaining != 0; ++markerIndex, --markersRemaining) {
		if (g_briefingRuntime->fgMarkerActive[markerIndex] != 0) {
			int16_t markerIcon = g_briefingRuntime->fgMarkerIconIndex[markerIndex];
			brief_Map_To_Screen_Pos(&viewport, g_briefingRuntime->mapIconX[positionSetIndex][markerIcon],
									g_briefingRuntime->mapIconY[positionSetIndex][markerIcon], &x, &y);
			xrect_Set_Rect(&highlightRect, x - 4, y - 4, x + 4 + 1, y + 4 + 1);
			brief_DrawCraftIconHighlight(&highlightRect, g_briefingRuntime->fgMarkerAge[markerIndex]);
		}
	}
	for (iconIndex = 0; iconIndex < g_briefingRuntime->mapIconCount; ++iconIndex) {
		int16_t state = g_briefingRuntime->mapIconType[iconIndex];
		int16_t colorOverride = g_briefingRuntime->mapIconColorOverride[iconIndex];
		int16_t mapX = g_briefingRuntime->mapIconX[positionSetIndex][iconIndex];
		int16_t mapY = g_briefingRuntime->mapIconY[positionSetIndex][iconIndex];
		int16_t group;
		--state;
		if (state <= XW_BRIEFING_MAP_ICON_BASE_COUNT - 1) {
			if (colorOverride == 0)
				group = g_briefingMapDefaultIconColorGroups[state];
			else
				group = colorOverride - 1;
		} else
			group = 0;
		if (group < 0)
			group = 0;
		if (group > XW_BRIEFING_MAP_ICON_ACTOR_COUNT - 1)
			group = XW_BRIEFING_MAP_ICON_ACTOR_COUNT - 1;
		if (g_briefingRuntime->mapScale[0] < XW_BRIEFING_MAP_ICON_LOW_ZOOM_THRESHOLD) {
			if (group != 0)
				state += XW_BRIEFING_MAP_ICON_BASE_COUNT;
			else
				state += XW_BRIEFING_MAP_ICON_GROUP0_LOW_ZOOM_OFFSET;
		}
		if (state >= 0) {
			Actor* actor;
			brief_Map_To_Screen_Pos(&viewport, mapX, mapY, &x, &y);
			xactor_Set_Actor_State(g_briefingMapIconActors[group], state, 0);
			xactanim_Get_Anim_Actor_Offset(g_briefingMapIconActors[group], &offsetX, &offsetY);
			actor = g_briefingMapIconActors[group];
			x -= offsetX + (actor->w >> 1);
			y -= offsetY + (actor->h >> 1);
			xactanim_Draw_Anim_Actor(actor, (Rect*)viewportRect, (Rect*)clipRect, x, y, 1);
		}
	}
	for (labelIndex = 0,
		labelsRemaining = sizeof(g_briefingRuntime->labelActive) / sizeof(g_briefingRuntime->labelActive[0]);
		 labelsRemaining != 0; ++labelIndex, --labelsRemaining) {
		if (g_briefingRuntime->labelActive[labelIndex] != 0) {
			int16_t textIndex = g_briefingRuntime->labelTextIndex[labelIndex];
			brief_Map_To_Screen_Pos(&viewport, g_briefingRuntime->labelX[labelIndex],
									g_briefingRuntime->labelY[labelIndex], &x, &y);
			strcpy(text, (const char*)xmemhdl_Lock_Handle(g_briefingRuntime->labelTextHandles[textIndex]));
			xmemhdl_Unlock_Handle(g_briefingRuntime->labelTextHandles[textIndex]);
			brief_Draw_Double_Readout_Text(text, 1, x, y, g_briefingRuntime->labelAge[labelIndex]);
		}
	}
	return 1;
}

/* DOS94 0x183660. */
static void Dos94_brief_Draw_Map_Paragraph(const Rect* rect, LandruHandle textHandle,
										   LandruHandle attributeHandle, int16_t suppressCenteredHeadings,
										   int16_t textSlotIndex) {
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
	xrect_Copy_Rect(&lineBounds, (Rect*)rect);
	scanLineStart = 0;
	(void)textSlotIndex;
	lineBounds.bottom = lineBounds.top + 10;
	lineBuffer[0] = 0;
	availableWidth = lineBounds.right - lineBounds.left;
	textBytes = xmemhdl_Lock_Handle(textHandle);
	attributeBytes = xmemhdl_Lock_Handle(attributeHandle);
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
			if (suppressCenteredHeadings == 0 && character == '$')
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
			if (isspace(textBytes[characterIndex]) == 0) {
				for (;;) {
					characterIndex = (int16_t)scanIndex;
					if (isspace(textBytes[characterIndex]))
						break;
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
			uint16_t currentAttribute = 0;
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
			if (suppressCenteredHeadings == 0 && lineBuffer[0] == '>')
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

/* DOS94 0x183bae. */
static void Dos94_brief_InitRuntime(int16_t reuseTextBuffers) {
	int16_t index, layout, viewport;
	g_briefingLoadedNarrationPage = -1;
	if (g_briefingNarrationSound != NULL) {
		xsound_Free_Sound(g_briefingNarrationSound);
		g_briefingNarrationSound = NULL;
	}
	for (index = 0; index < XW_BRIEFING_PAGE_LABEL_CAPACITY; ++index) {
		if (reuseTextBuffers != 0) {
			LandruHandle handle = g_briefingRuntime->labelTextHandles[index];
			if (handle != LANDRU_NULL_HANDLE)
				xmemhdl_Handle_nSet(handle, 0, XW_BRIEF_LABEL_BUFFER_SIZE);
		} else {
			g_briefingRuntime->labelTextHandles[index] =
				xmemhdl_Alloc_Clear_Handle(XW_BRIEF_LABEL_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
		}
	}
	for (index = 0; index < XW_BRIEFING_TEXT_BUFFER_CAPACITY; ++index) {
		if (reuseTextBuffers != 0) {
			LandruHandle handle = g_briefingRuntime->textBlockHandles[index];
			if (handle != LANDRU_NULL_HANDLE)
				xmemhdl_Handle_nSet(handle, 0, XW_BRIEFING_TEXT_BUFFER_SIZE);
			handle = g_briefingRuntime->textAttributeHandles[index];
			if (handle != LANDRU_NULL_HANDLE)
				xmemhdl_Handle_nSet(handle, 0, XW_BRIEFING_TEXT_BUFFER_SIZE);
		} else {
			g_briefingRuntime->textBlockHandles[index] =
				xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_TEXT_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
			g_briefingRuntime->textAttributeHandles[index] =
				xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_TEXT_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
		}
	}
	g_briefingRuntime->playbackActive = 1;
	g_briefingRuntime->field_00C2 = 0;
	g_briefingRuntime->mapIconCount = 0;
	g_briefingRuntime->field_00C6 = 0;
	g_briefingRuntime->mapPositionSetCount = 1;
	g_briefingRuntime->field_1D66 = 0;
	g_briefingRuntime->layoutCount = 1;
	for (layout = 0; layout < XW_BRIEFING_LAYOUT_COUNT; ++layout) {
		for (viewport = 0; viewport < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewport) {
			if (viewport != XW_BRIEFING_LAYOUT_MAP_VIEWPORT || layout != 0) {
				xrect_Clear_Rect(&g_briefingRuntime->layoutRects[layout][viewport]);
				g_briefingRuntime->layoutActive[layout][viewport] = 0;
			} else {
				xrect_Set_Rect(&g_briefingRuntime->layoutRects[0][XW_BRIEFING_LAYOUT_MAP_VIEWPORT], 0, 0, 212,
							   138);
				g_briefingRuntime->layoutActive[0][XW_BRIEFING_LAYOUT_MAP_VIEWPORT] = 1;
			}
		}
	}
	g_briefingRuntime->activeScriptIndex = 0;
	g_briefingRuntime->scriptCount = 1;
	Dos94_brief_InitDefaultScript(0);
	g_briefingRuntime->scriptPositionSetIndex[0] = 0;
	g_briefingRuntime->scriptLayoutIndex[0] = 0;
	g_briefingRuntime->mapCenter[0] = 0;
	g_briefingRuntime->mapCenter[1] = 0;
	g_briefingRuntime->mapTargetCenter[0] = 0;
	g_briefingRuntime->mapTargetCenter[1] = 0;
	g_briefingRuntime->mapScale[0] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapScale[1] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[0] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[1] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapViewportActive = 0;
	g_briefingRuntime->field_37DC = 0;
	for (index = 0; index < (int)(sizeof(g_briefingRuntime->textViewportActive) /
								  sizeof(g_briefingRuntime->textViewportActive[0]));
		 ++index)
		g_briefingRuntime->textViewportActive[index] = 0;
	for (index = 0; index < (int)(sizeof(g_briefingRuntime->textSlotActive) /
								  sizeof(g_briefingRuntime->textSlotActive[0]));
		 ++index)
		g_briefingRuntime->textSlotActive[index] = 0;
	for (index = 0; index < (int)(sizeof(g_briefingRuntime->fgMarkerActive) /
								  sizeof(g_briefingRuntime->fgMarkerActive[0]));
		 ++index)
		g_briefingRuntime->fgMarkerActive[index] = 0;
	for (index = 0;
		 index < (int)(sizeof(g_briefingRuntime->labelActive) / sizeof(g_briefingRuntime->labelActive[0]));
		 ++index)
		g_briefingRuntime->labelActive[index] = 0;
	g_briefingRuntime->textSlotActive[0] = 1;
	g_briefingRuntime->textSlotBlockIndex[0] = 1;
	Dos94_brief_ApplyLayout(g_briefingRuntime->scriptLayoutIndex[g_briefingRuntime->activeScriptIndex]);
}

/* DOS94 0x183fb4. */
static void Dos94_brief_InitDefaultScript(int16_t scriptIndex) {
	g_briefingRuntime->scriptWords[scriptIndex][0] = XW_BRIEF_SENTINEL_TIME;
	g_briefingRuntime->scriptWords[scriptIndex][1] = XW_BRIEF_COMMAND_END;
	g_briefingRuntime->scriptEndTime[scriptIndex] = XW_BRIEF_DEFAULT_END_TIME;
	g_briefingRuntime->scriptCurrentTime[scriptIndex] = 0;
	g_briefingRuntime->scriptCursorWordIndex[scriptIndex] = 0;
	g_briefingRuntime->scriptWordCount[scriptIndex] = XW_BRIEF_COMMAND_HEADER_WORDS;
	g_briefingRuntime->scriptPositionSetIndex[scriptIndex] = 0;
	g_briefingRuntime->scriptLayoutIndex[scriptIndex] = 0;
	Dos94_brief_Rewind_Page(scriptIndex);
}

/* DOS94 0x184062. */
static void Dos94_brief_ApplyLayout(int16_t layoutIndex) {
	int16_t viewportIndex;
	for (viewportIndex = 0; viewportIndex < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewportIndex) {
		if (viewportIndex != XW_BRIEFING_LAYOUT_MAP_VIEWPORT) {
			xrect_Copy_Rect(&g_briefingRuntime->textViewportRects[viewportIndex],
							&g_briefingRuntime->layoutRects[layoutIndex][viewportIndex]);
			g_briefingRuntime->textViewportActive[viewportIndex] =
				g_briefingRuntime->layoutActive[layoutIndex][viewportIndex];
		} else {
			xrect_Copy_Rect(&g_briefingRuntime->mapViewportRect,
							&g_briefingRuntime->layoutRects[layoutIndex][XW_BRIEFING_LAYOUT_MAP_VIEWPORT]);
			g_briefingRuntime->mapViewportActive =
				g_briefingRuntime->layoutActive[layoutIndex][XW_BRIEFING_LAYOUT_MAP_VIEWPORT];
		}
	}
}

/* DOS94 0x18477e. */
static void Dos94_brief_LoadMissionChoice(int16_t choiceIndex, int16_t reuseTextBuffers) {
	ResFile* resource;
	LandruHandle paragraph;
	int16_t selectedMission;
	char tourName[XW_BRIEF_TOUR_RESOURCE_NAME_CAPACITY];
	g_briefingMissionChoice = choiceIndex;
	resource = XwLandru_OpenMissionResource(":X-Wing Data\\Resource\\missions.lfd");
	if (resource == NULL)
		resource = XwLandru_OpenMissionResource("missions.lfd");
	sprintf(tourName, "tour%d", g_briefingPilotRecord.current_tour + 1);
	selectedMission = g_briefingPilotRecord.briefingMissionChoices[choiceIndex];
	g_briefingPilotRecord.selectedTourMission = selectedMission;
	paragraph = xparagrp_Res_Paragraph(resource, tourName);
	xparagrp_Get_Paragraph_String(paragraph, g_shellMissionName, 0, selectedMission);
	xparagrp_Free_Paragraph(paragraph);
	xres_Close_Resource(resource);
	Dos94_brief_InitRuntime(reuseTextBuffers);
	Dos94_brief_LoadBriefingFile(g_shellMissionName);
	Dos94_brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
}

/* DOS94 0x184848. */
static int Dos94_brief_LoadBriefingFile(const char* missionName) {
	uint16_t allowFallback;
	LandruFile* stream;
	char resolvedPath[XW_BRIEFING_MISSION_PATH_CAPACITY];
	char logicalPath[XW_BRIEFING_MISSION_PATH_CAPACITY];
	shipext_Get_Mission_Path(logicalPath, missionName, 0);
	if (logicalPath[0] == ';' || logicalPath[0] == '+') {
		strcpy(resolvedPath, "X-Wing Data\\");
		strcat(resolvedPath, &logicalPath[1]);
		allowFallback = 1;
	} else {
		strcpy(resolvedPath, logicalPath);
		/* The original fallback flag is uninitialized for an unprefixed path. */
		allowFallback = 0;
	}
	stream = XwStorage_OpenMission(resolvedPath);
	if (stream == NULL) {
		if (allowFallback != 0) {
			if (logicalPath[0] == ';') {
				strcpy(resolvedPath, "c:\\XwingCD\\");
				strcat(resolvedPath, &logicalPath[1]);
				resolvedPath[0] = (char)g_installDriveLetter;
			} else {
				strcpy(resolvedPath, &logicalPath[1]);
			}
			stream = XwStorage_OpenMission(resolvedPath);
		}
	}
	if (stream != NULL) {
		int16_t layoutIndex;
		uint16_t layoutCount;
		uint16_t discardedHeaderWord;
		xfile_Read_Word_From_File(stream, &discardedHeaderWord);
		brief_ReadIconData(stream);
		xfile_Read_Word_From_File(stream, &layoutCount);
		g_briefingRuntime->layoutCount = layoutCount;
		for (layoutIndex = 0; layoutIndex < g_briefingRuntime->layoutCount; ++layoutIndex) {
			brief_ReadLayout(stream, layoutIndex);
		}
		Dos94_brief_ReadScripts(stream);
		brief_ReadExtendedIconData(stream);
		brief_ReadTextBuffers(stream);
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

/* DOS94 0x184916. */
static void Dos94_brief_ReadScripts(XwFile* stream) {
	uint16_t scriptCount;
	uint16_t wordCount;
	uint16_t endTime;
	uint16_t positionSetIndex;
	uint16_t layoutIndex;
	int16_t scriptIndex;

	xfile_Read_Word_From_File(stream, &scriptCount);
	g_briefingRuntime->activeScriptIndex = 0;
	g_briefingRuntime->scriptCount = scriptCount;
	for (scriptIndex = 0; scriptIndex < (int16_t)scriptCount; ++scriptIndex) {
		xfile_Read_Word_From_File(stream, &endTime);
		xfile_Read_Word_From_File(stream, &wordCount);
		xfile_Read_Word_From_File(stream, &positionSetIndex);
		xfile_Read_Word_From_File(stream, &layoutIndex);
		g_briefingRuntime->scriptEndTime[scriptIndex] = endTime;
		g_briefingRuntime->scriptWordCount[scriptIndex] = wordCount;
		g_briefingRuntime->scriptPositionSetIndex[scriptIndex] = positionSetIndex;
		g_briefingRuntime->scriptLayoutIndex[scriptIndex] = layoutIndex;
		xfile_Read_Data_From_File(stream, g_briefingRuntime->scriptWords[scriptIndex],
								  (int16_t)wordCount *
									  (int)sizeof(g_briefingRuntime->scriptWords[scriptIndex][0]));
	}
	Dos94_brief_ApplyLayout(g_briefingRuntime->scriptLayoutIndex[g_briefingRuntime->activeScriptIndex]);
}

/* DOS94 0x1815d6. */
static void draw_playback_button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (!refresh)
		return;
	if (input->id < 2) {
		xstyle_Style_Paint_Border(frame, button->pressed);
		xstyle_Style_Draw_Centered_Icon(input->id == 0 ? 1 : 3, frame, clip, button->pressed);
		return;
	}
	xpaint_Frame_Clipped_Rect(frame, 16);
	xrect_Inset_Rect(frame, 1, 1);
	int highlighted;
	if (input->id >= 5) {
		xpaint_Paint_Clipped_Bevel(frame, 51, 49, 50, button->pressed);
		highlighted = input->id - g_briefingMissionChoice == 5;
	} else {
		xstyle_Style_Paint_Border(frame, button->pressed);
		highlighted = input->id - g_briefingRuntime->playbackActive == 3 && !button->pressed;
	}
	xrect_Inset_Rect(frame, -1, -1);
	if (highlighted && !button->pressed)
		xfont_Print_Centered_Text(button->labels, frame, 0, 14);
	else
		xstyle_Style_Small_Button_Text(button->labels, frame, button->pressed);
}

static void playback_button(const char* label, int id, int left, int top, int right, int bottom, int ax,
							int ay) {
	Rect rect;
	xrect_Set_Rect(&rect, left, top, right, bottom);
	PushButton* button = xbtnpush_Alloc_Button(g_briefingHintCompanionInput, &rect, 0,
											   Dos94_brief_HandlePlaybackButton, label, id);
	xinpattr_Set_Input_Draw_Function(&button->header, draw_playback_button);
	xinpattr_Set_Input_Allign(&button->header, ax, ay);
}

static void create_controls(int scene) {
	Rect rect;
	xrect_Set_Rect(&rect, scene == 111 ? 97 : 92, 150, scene == 111 ? 228 : 223, 196);
	g_briefingDoorHintInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_briefingDoorHintInput, Dos94_brief_DrawDoorHint);
	xinpattr_Hide_Input(g_briefingDoorHintInput);
	g_briefingHintCompanionInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	playback_button("Rewind", 2, 4, 0, 42, 14, 0, 0);
	playback_button("Stop", 3, 0, 0, 38, 14, 1, 0);
	playback_button("Play", 4, 4, 0, 42, 14, 2, 0);
	if (g_briefingHasMissionChoice) {
		playback_button("Mission A", 5, 4, 15, 62, 29, 0, 0);
		playback_button("Mission B", 6, 4, 15, 62, 29, 2, 0);
	}
	playback_button(NULL, 0, 5, 0, 21, 16, 0, 2);
	playback_button(NULL, 1, 5, 0, 21, 16, 2, 2);
	xrect_Set_Rect(&rect, 23, 1, 108, 15);
	Input* label = xinput_Alloc_Input(g_briefingHintCompanionInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(label, Dos94_brief_UpdatePageLabel);
	xinpattr_Set_Input_Draw_Function(label, Dos94_brief_DrawPageLabel);
	xinpattr_Set_Input_Allign(label, 0, 2);
}

/* DOS94 0x1857bc retains music across pilot assignment and launch. */
void Dos94_brief_CloseMusic(void) {
	if (!ShellPreferences_GetMusicEnabled())
		return;
	soundext_ClearTriggers();
	int scene = xerror_Get_Landru_Exit();
	Sound* previous = g_briefMusicState.previousMusic;
	Sound* music = g_briefMusicState.music;
	if (scene == 60 && previous == xsound_Find_Gmid("mission") &&
		(uint8_t)soundext_Count_Resource_Instances(previous) == 1 &&
		(uint8_t)soundext_Count_Resource_Instances(music) != 1)
		return;
	if (scene == 120) {
		if (previous == xsound_Find_Gmid("mission"))
			soundext_FadeVolume(previous, 0, 300);
		return;
	}
	if (scene == 122) {
		if (previous)
			soundext_FadeVolume(previous, 0, 300);
		soundext_FadeVolume(music, 0, 200);
		return;
	}
	if (scene == 60 || scene == 83) {
		soundext_SetPriority((intptr_t)music, 0);
		soundext_FadeVolume(music, 0, 300);
	}
	if (previous) {
		xsound_Clear_Sound_Keep(previous);
		xsound_Free_Sound(previous);
	}
}

/* DOS94 0x180000. Runtime arrays and pilot records use the port's shared native types. */
void Dos94_Brief(XwShellContext* context) {
	Rect rect, map_frame;
	int scene = shellext_Get_Cur_Scene();
	g_briefingRequestedNarrationPage = -1;
	g_briefingLoadedNarrationPage = -1;
	g_briefingNarrationSound = NULL;
	g_briefingSelectedIconPlusOne = 0;
	g_briefingMissionChoice = 0;
	g_briefingFocusIndex = XW_BRIEFING_INITIAL_FOCUS;
	xio_Set_Mouse_Position(298, 104);
	g_briefingRuntimeHandle = xmemhdl_Alloc_Clear_Handle(sizeof(*g_briefingRuntime), LANDRU_MEMORY_RESOURCE);
	g_briefingRuntime = xmemhdl_Lock_Handle(g_briefingRuntimeHandle);
	xmemhdl_Unlock_Handle(g_briefingRuntimeHandle);
	brief_LoadPilotRecord();
	g_briefingTextActor = NULL;
	if (scene == 112) {
		Dos94_brief_LoadMissionChoice(0, 0);
		g_briefingHasMissionChoice = g_briefingPilotRecord.briefingMissionChoices[0] != 255 &&
									 g_briefingPilotRecord.briefingMissionChoices[1] != 255;
		shipext_Set_Last_Briefed_Tour_Operation(g_briefingPilotRecord.currentTourOperation);
	} else {
		Dos94_brief_InitRuntime(0);
		Dos94_brief_LoadBriefingFile(g_shellMissionName);
		Dos94_brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
		g_briefingHasMissionChoice = 0;
	}
	ResFile* resource = xres_Open_Resource("brief.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	g_briefingFilm = xfilm_Res_Film(resource, scene == 111 ? "cbrief_f" : "brief_f", &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_briefingFilm, context->standardPalette);
	g_briefingBackgroundActor = xactor_Find_Actor(FOURCC_DELT, scene == 111 ? "brief-2" : "brief");
	xactor_Non_Refreshable_Actor(g_briefingBackgroundActor);
	g_briefingInteriorActor = xactor_Find_Actor(FOURCC_DELT, "panel");
	xactor_Non_Refreshable_Actor(g_briefingInteriorActor);
	g_briefingLeftDoorActor = xactanim_Res_Anim_Actor(resource, "ldoor", &rect, 0, 0, 40);
	xactor_Set_Actor_User_Function(g_briefingLeftDoorActor, Dos94_brief_user_Door);
	xactor_Set_Actor_State(g_briefingLeftDoorActor, g_briefingLeftDoorActor->arraySize - 1, 0);
	xactor_Non_Refreshable_Actor(g_briefingLeftDoorActor);
	g_briefingLeftDoorActor->id = 0;
	g_briefingRightDoorActor = xactanim_Alloc_Anim_Actor(0, &rect, 301, 0, 40);
	xactor_Copy_Actor_Data(g_briefingRightDoorActor, g_briefingLeftDoorActor);
	xactor_Set_Actor_Name(g_briefingRightDoorActor, FOURCC_ANIM, "rdoor");
	xactor_Set_Actor_User_Function(g_briefingRightDoorActor, Dos94_brief_user_Door);
	xactor_Set_Actor_State(g_briefingRightDoorActor, g_briefingRightDoorActor->arraySize - 1, 0);
	xactor_Set_Actor_Flip(g_briefingRightDoorActor, 1, 0);
	xactor_Non_Refreshable_Actor(g_briefingRightDoorActor);
	g_briefingRightDoorActor->id = 1;
	static const char* const icons[] = { "iconsgrn", "iconsred", "iconsblue" };
	ResFile* icon_resource = shipext_IsTourAvailable(4) ? xres_Open_Resource("bwing.lfd") : resource;
	for (int i = 0; i < 3; ++i) {
		g_briefingMapIconActors[i] = xactanim_Res_Anim_Actor(icon_resource, icons[i], &rect, 0, 0, 0);
		xactor_Set_Actor_Time(g_briefingMapIconActors[i], 0, 0);
	}
	g_briefingBwingRadarActor = NULL;
	if (icon_resource != resource) {
		g_briefingBwingRadarActor = xactdelt_Res_Delta_Actor(icon_resource, "bradar", &rect, 0, 0, 0);
		xactor_Set_Actor_Time(g_briefingBwingRadarActor, 0, 0);
		xres_Close_Resource(icon_resource);
	}
	g_briefingSelectionPreviewActor = xactanim_Res_Anim_Actor(resource, "radar", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_briefingSelectionPreviewActor, 0, 0);
	g_briefingStarsActor = xactdelt_Res_Delta_Actor(context->resourceFile, "stars-4", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_briefingStarsActor, 0, 0);
	g_briefingOfficerRegionBuffer = xmemhdl_Alloc_Clear_Handle(6000, 0);
	g_briefingOfficerRegionRestored = 1;
	Dos94_Brief_CreateOfficer(resource, &rect, scene);
	xrect_Set_Rect(&map_frame, scene == 111 ? 73 : 35, 4, scene == 111 ? 285 : 247, 142);
	g_briefingTextActor = xactcust_Alloc_Custom_Actor(0, &map_frame, 0, 0, 90);
	xactor_Set_Actor_User_Function(g_briefingTextActor, Dos94_brief_AdvanceOrResetAtEnd);
	xactor_Set_Actor_Draw_Function(g_briefingTextActor, Dos94_brief_DrawMapViewport);
	xactor_Non_Refreshable_Actor(g_briefingTextActor);
	g_briefingTextActor->var1 = 0;
	g_briefingOfficerRestoreActor = xactcust_Alloc_Custom_Actor(0, &map_frame, 0, 0, 200);
	xactor_Set_Actor_Draw_Function(g_briefingOfficerRestoreActor, brief_RestoreOfficerRegion);
	g_briefingWorldInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	g_briefingMapInput = xinput_Alloc_Input(g_briefingWorldInput, &map_frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_briefingMapInput, brief_UpdateMapSelection);
	xinpattr_Set_Input_User_Function(g_briefingMapInput, brief_ApplyMapSelection);
	g_briefingMapInput->mouseUsage = allInput;
	xrect_Set_Rect(&rect, 0, 24, 20, 146);
	g_briefingLeftDoorInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_briefingLeftDoorInput, brief_iupdate_Brief);
	xinpattr_Set_Input_User_Function(g_briefingLeftDoorInput, Dos94_brief_iuser_Brief);
	g_briefingLeftDoorInput->mouseUsage = allInput;
	g_briefingLeftDoorInput->id = 0;
	xrect_Set_Rect(&rect, 300, 24, 320, 146);
	g_briefingRightDoorInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_briefingRightDoorInput, brief_iupdate_Brief);
	xinpattr_Set_Input_User_Function(g_briefingRightDoorInput, Dos94_brief_iuser_Brief);
	g_briefingRightDoorInput->mouseUsage = allInput;
	g_briefingRightDoorInput->id = 1;
	create_controls(scene);
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(Dos94_brief_end_View);
	XwBrief_RunView(resource);
}
