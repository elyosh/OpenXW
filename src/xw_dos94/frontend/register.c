#include "xw_dos94/frontend/register.h"
#include "xw_runtime/storage/storage.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/port.h"
#endif
#include "xw/frontend/register.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/input_callbacks.h"
#include "xw_runtime/integration/register_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/register_view_task.h"
#endif

#include "xw_runtime/runtime/register_task.h"

#include <ctype.h>
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/dialog.h>
#include <landru/dlgjoy.h>
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

static Actor* soldiers[3];
static Actor* message_background;
static int16_t robot_layers[3];
static const int16_t dos_navigation_x[55] = { 12,  118, 153, 153, 153, 12,  118, 153, 153, 153, 12,
											  118, 153, 153, 153, 12,  118, 153, 153, 153, 12,  118,
											  153, 153, 153, 12,  118, 153, 153, 153, 12,  118, 153,
											  153, 153, 12,  118, 153, 153, 153, 12,  118, 153, 153,
											  153, 12,  118, 153, 219, 256, 12,  115, 175, 243, 243 };
static const int16_t dos_navigation_y[55] = { 160, 98,  98,  98,  98,  160, 105, 105, 105, 105, 160,
											  112, 112, 112, 112, 160, 119, 119, 119, 119, 160, 126,
											  126, 126, 126, 160, 133, 133, 133, 133, 160, 140, 140,
											  140, 140, 160, 147, 147, 147, 147, 160, 154, 154, 154,
											  154, 160, 161, 161, 179, 179, 160, 175, 175, 191, 191 };
static const int16_t dos_delete_x[3] = { 112, 163, 214 };
static const int16_t dos_delete_y[3] = { 110, 110, 110 };

/* DOS94 0x580c82. Native pilot records retain the shared storage layout. */
void Dos94_register_end_View(int time) {
	char typedName[REGISTER_STRING_WORK_CAPACITY];
	int16_t key;
	if (time == 0) {
		if ((uint16_t)xcursor_Is_Cursor_Visible() == 0)
			xcursor_Show_Cursor();
	}
	if (time == 0 || time == XW_REGISTER_VIEW_RESUME_PROTECTION) {
		xfiledir_Read_Directory(&g_RegisterDirectory);
		g_RegisterLoadedPilotCount = g_RegisterDirectory.count;
		xcursor_Set_Cursor(REGISTER_CURSOR_BUSY);
		register_Build_Fast_Pilot_Record();
		xcursor_Set_Cursor(REGISTER_CURSOR_NORMAL);
		g_RegisterPageCount = (g_RegisterPilotCount + 20 - 1) / 20;
		if (g_RegisterPageCount == 0)
			g_RegisterPageCount = 1;
		g_RegisterCurrentPage = 0;
		register_Set_Your_Reg_Pilot();
	}
	register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, typedName);
	Dos94_register_xuser_Pilot_Name(typedName);
	if (typedName[0] != 0) {
		if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterLogButton) == 0) {
			xinpattr_Show_Input(g_RegisterLogButton);
			xinpattr_Show_Input(g_RegisterMeritsButton);
			xview_Refresh_View();
		}
		if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterDeleteButton) != 0) {
			if (g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE) {
				xinpattr_Hide_Input(g_RegisterDeleteButton);
				xview_Refresh_View();
			}
		} else if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE) {
			xinpattr_Show_Input(g_RegisterDeleteButton);
			g_RegisterDeleteLabelState = REGISTER_DELETE_LABEL_UNKNOWN;
			xview_Refresh_View();
		}
		if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterDeleteButton) != 0) {
			register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
			if (g_RegisterShellPilot.lost_status != 0) {
				if (g_RegisterDeleteLabelState != REGISTER_DELETE_LABEL_MODIFY) {
					xbtnpush_Set_Button_Name((PushButton*)g_RegisterDeleteButton,
											 g_RegisterResourceAndLabelText[REGISTER_TEXT_MODIFY_PILOT]);
					g_RegisterDeleteLabelState = REGISTER_DELETE_LABEL_MODIFY;
				}
			} else if (g_RegisterDeleteLabelState != REGISTER_DELETE_LABEL_DELETE) {
				xbtnpush_Set_Button_Name((PushButton*)g_RegisterDeleteButton,
										 g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE_PILOT]);
				g_RegisterDeleteLabelState = REGISTER_DELETE_LABEL_DELETE;
			}
			xview_Refresh_View();
		}
	} else if ((uint16_t)xinpattr_Is_Input_Visible(g_RegisterLogButton) != 0) {
		xinpattr_Hide_Input(g_RegisterLogButton);
		xinpattr_Hide_Input(g_RegisterMeritsButton);
		xinpattr_Hide_Input(g_RegisterDeleteButton);
		xview_Refresh_View();
	}
	xinpattr_Refresh_Input(g_RegisterPageInput);
	if (time == REGISTER_INITIAL_SPEECH_FRAME && shellext_Get_Cur_Scene() == XW_SCENE_REGISTER_INITIAL)
		register_PlaySpeech(REGISTER_SPEECH_FIRST, time);
	key = xio_Get_Free_Key();
	if (key != 0 && shellext_MoveGridFocus(&g_RegisterNavigationIndex, dos_navigation_x, dos_navigation_y,
										   REGISTER_NAVIGATION_ROWS, REGISTER_NAVIGATION_COLUMNS, key) != 0)
		xio_Set_Mouse_Position(dos_navigation_x[g_RegisterNavigationIndex],
							   dos_navigation_y[g_RegisterNavigationIndex]);
}

/* DOS94 0x5816a6. Native pilot records retain the shared storage layout. */
int16_t Dos94_register_iupdate_Register(Input* input, Rect* frame, Rect* clip, int16_t phase, int leftEvent,
										int rightEvent, int16_t x, int16_t y) {
	char pilotName[REGISTER_PILOT_PATH_CAPACITY];
	REGISTER_FastPilotRecord pilotRecord;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (phase != 0) {
		return 0;
	}
	register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
	if (strlen(pilotName) == 0) {
		input->var1 = REGISTER_DOOR_EMPTY_NAME;
		for (int i = 0; i < 3; ++i)
			soldiers[i]->var1 = 1;
	} else {
		register_Index_To_Pilot_Record(g_RegisterActivePilot, &pilotRecord);
		if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE &&
			pilotRecord.lost_status != REGISTER_PILOT_AVAILABLE) {
			if (pilotRecord.lost_status == REGISTER_PILOT_LOST_ALTERNATE) {
				input->var1 = REGISTER_DOOR_LOST_PILOT_ALTERNATE;
			} else {
				input->var1 = REGISTER_DOOR_LOST_PILOT;
			}
			for (int i = 0; i < 3; ++i)
				soldiers[i]->var1 = 1;
		} else {
			if (leftEvent == REGISTER_MOUSE_RELEASE || rightEvent == REGISTER_MOUSE_RELEASE) {
				input->var1 = REGISTER_DOOR_ACCEPT;
				input->var2 = XW_SCENE_CONCOURSE;
			} else {
				input->var1 = REGISTER_DOOR_HOVER;
			}
			g_RegisterDoorActor->var1 = 1;
		}
	}
	return 1;
}

/* DOS94 0x58207c. Native pilot records retain the shared storage layout. */
void Dos94_register_iuser_Pilot_Button(Input* input, int unusedContext) {
	char pilotFilename[REGISTER_PILOT_BUTTON_PATH_CAPACITY];
	int16_t id;
	(void)unusedContext;
	if (xinpattr_Get_Input_Selected(input) == 0)
		return;
	id = input->id;
	if (id < REGISTER_PILOT_BUTTON_LOG) {
		if (id == REGISTER_PILOT_BUTTON_LEFT) {
			if (g_RegisterCurrentPage > 0) {
				--g_RegisterCurrentPage;
				xinput_Refresh_System_Inputs();
			} else {
				g_RegisterCurrentPage = g_RegisterPageCount - 1;
			}
		} else if (g_RegisterCurrentPage < g_RegisterPageCount - 1) {
			++g_RegisterCurrentPage;
		} else {
			g_RegisterCurrentPage = 0;
		}
		if (g_RegisterPageCount > 1) {
			g_RegisterPilotOffset = 20 * g_RegisterCurrentPage;
			xinpattr_Refresh_Input(g_RegisterPageInput);
		}
	} else if (id == REGISTER_PILOT_BUTTON_DELETE) {
		Dos94_register_Do_Delete_Dialog(Dos94_Register_CompleteDeletion, NULL);
	} else {
		if (id == REGISTER_PILOT_BUTTON_LOG)
			xerror_Set_Landru_Exit(XW_SCENE_PILOT_LOG_FROM_REGISTER);
		else
			xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_REGISTER);
		if (g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE) {
			register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotFilename);
			strcpy(g_RegisterShellPilot.name, pilotFilename);
			g_RegisterShellPilot.field_18 = 0;
			g_RegisterShellPilot.deleted = 0;
			g_RegisterShellPilot.lost_status = 0;
			g_RegisterShellPilot.rank = 0;
			g_RegisterShellPilot.current_tour = 0;
			g_RegisterShellPilot.field_1D = 0;
			g_RegisterShellPilot.score = 0;
			strcat(pilotFilename, ".PLT");
			shipext_Revive_Pilot(pilotFilename, SHIPEXT_CREATE_PILOT);
		} else {
			register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
		}
	}
}

/* DOS94 0x5823be. Native pilot records retain the shared storage layout. */
void Dos94_register_iuser_Pilot_Name(Input* input, int unusedContext) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)input;
	(void)unusedContext;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		Dos94_register_xuser_Pilot_Name(button->name);
		if (strlen(button->name) != 0) {
			g_RegisterPilotInfoInput->var1 = 1;
		} else {
			g_RegisterPilotInfoInput->var1 = 0;
			g_RegisterPilotInfoInput->var2 = 0;
			xinpattr_Refresh_Input(g_RegisterPilotInfoInput);
		}
	}
}

/* DOS94 0x58246c. Native pilot records retain the shared storage layout. */
void Dos94_register_xuser_Pilot_Name(const char* searchName) {
	int16_t logicalIndex;
	int16_t matchedIndex = REGISTER_PILOT_SLOT_NONE;
	int16_t pilotSlot;
	char pilotName[REGISTER_PILOT_PATH_CAPACITY];
	for (logicalIndex = 0; logicalIndex < g_RegisterPilotCount && matchedIndex == REGISTER_PILOT_SLOT_NONE;
		 ++logicalIndex) {
		if (register_Index_To_Pilot(logicalIndex, &pilotSlot) != 0 &&
			register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotName, pilotSlot) != 0 &&
			strcmp(pilotName, searchName) == 0)
			matchedIndex = logicalIndex;
	}
	if (matchedIndex != REGISTER_PILOT_SLOT_NONE) {
		if (g_RegisterActivePilot != matchedIndex) {
			int matchingPage;
			xinpattr_Refresh_Input(g_RegisterPilotListInput);
			g_RegisterActivePilot = matchedIndex;
			matchingPage = matchedIndex / 20;
			if (matchingPage != g_RegisterCurrentPage) {
				g_RegisterCurrentPage = matchedIndex / 20;
				g_RegisterPilotOffset = 20 * g_RegisterCurrentPage;
				xinpattr_Refresh_Input(g_RegisterPageInput);
			}
		}
	} else if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE) {
		xinpattr_Refresh_Input(g_RegisterPilotListInput);
		g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
	}
}

/* DOS94 0x58341c. Native pilot records retain the shared storage layout. */
void Dos94_register_Delete_Pilot_Record(void) {
	if (g_RegisterFastPilotHandle != LANDRU_NULL_HANDLE) {
		REGISTER_FastPilotRecord* records = xmemhdl_Lock_Handle(g_RegisterFastPilotHandle);
		int16_t slotIndex;
		int16_t visibleIndex = 0;
		int16_t deleted = 0;
		for (slotIndex = 0; slotIndex < g_RegisterLoadedPilotCount; ++slotIndex) {
			if (records[slotIndex].deleted == 0) {
				if (visibleIndex == g_RegisterActivePilot) {
					records[slotIndex].name[0] = 0;
					records[slotIndex].deleted = -1;
					deleted = 1;
				}
				++visibleIndex;
			}
		}
		xmemhdl_Unlock_Handle(g_RegisterFastPilotHandle);
		if (deleted != 0) {
			g_RegisterPageCount = (--g_RegisterPilotCount + 20 - 1) / 20;
			if (g_RegisterPageCount == 0) {
				g_RegisterPageCount = 1;
			}
			if (g_RegisterCurrentPage >= g_RegisterPageCount) {
				g_RegisterCurrentPage = g_RegisterPageCount - 1;
			}
		}
		g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
		memset(&g_RegisterPilotData, 0, sizeof(g_RegisterPilotData));
	}
}

/* DOS94 0x583748. Native pilot records retain the shared storage layout. */
struct REGISTER_RegStringButton* Dos94_register_Alloc_Input_Reg_String_Button(
	Input* parent, Rect* frame, int16_t zinput, InputUserFunc user, const char* initialName,
	int16_t isFilenameMode, int16_t id) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)xinput_Alloc_Dialog_Input(
		parent, frame, zinput, sizeof(REGISTER_RegStringButton) - sizeof(Input));
	int16_t index;
	xinpattr_Set_Input_Draw_Function(&button->input, register_idraw_Reg_String_Button);
	xinpattr_Set_Input_Update_Function(&button->input, Dos94_register_iupdate_Reg_String_Button);
	xinpattr_Set_Input_User_Function(&button->input, user);
	button->input.id = id;
	for (index = 0; initialName[index]; index++)
		button->name[index] = initialName[index];
	button->name[index] = 0;
	button->is_filename_mode = isFilenameMode;
	return button;
}

/* DOS94 0x58394c. Native pilot records retain the shared storage layout. */
int16_t Dos94_register_iupdate_Reg_String_Button(Input* input, Rect* frame, Rect* clip, int16_t key,
												 int leftEvent, int rightEvent, int16_t x, int16_t y) {
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)input;
	char workingText[REGISTER_STRING_WORK_CAPACITY];
	int16_t index;
	int16_t length;
	int16_t accepted;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	for (index = 0; button->name[index] != '\0'; ++index)
		workingText[index] = button->name[index];
	workingText[index] = '\0';
	length = (int16_t)strlen(workingText);
	accepted = 1;
	if (key != 0) {
		if (key == REGISTER_KEY_DELETE || key == '\b') {
			if (length != 0)
				workingText[length - 1] = '\0';
		} else if (key > ' ' && key < REGISTER_KEY_ASCII_DELETE) {
			if ((isalpha)(key)) {
				accepted = Dos94_register_Add_Key_To_Reg_String(button, workingText, (char)toupper(key));
			} else if ((isdigit)(key) && button->is_filename_mode != 0) {
				accepted = Dos94_register_Add_Key_To_Reg_String(button, workingText, (char)key);
			} else if ((key == '_' || key == '-') && button->is_filename_mode != 0) {
				accepted = Dos94_register_Add_Key_To_Reg_String(button, workingText, (char)key);
			} else {
				accepted = 0;
			}
		} else {
			accepted = 0;
		}
	}
	for (index = 0; workingText[index] != '\0'; ++index)
		button->name[index] = workingText[index];
	button->name[index] = '\0';
	if (accepted != 0) {
		xinpattr_Refresh_Input(input);
		xinpattr_Selected_Input(input);
	}
	return accepted;
}

/* DOS94 0x583b5e. Native pilot records retain the shared storage layout. */
int16_t Dos94_register_Add_Key_To_Reg_String(struct REGISTER_RegStringButton* input, char* text,
											 char character) {
	int16_t length = (int16_t)strlen(text);

	if (length < (input->is_filename_mode ? 8 : 20)) {
		text[length] = character;
		text[length + 1] = '\0';
		return 1;
	}
	return 0;
}

/* DOS94 0x583cf0. Native pilot records retain the shared storage layout. */
void Dos94_register_Do_Delete_Dialog(DialogSubResultHandler complete, void* context) {
	Input* dialog;
	int16_t hadKeyButtons = xio_Is_Key_Buttons();
	if (hadKeyButtons == 0)
		xio_Set_Key_Buttons();
	g_RegisterDeleteFocus = REGISTER_DELETE_INITIAL_FOCUS;
	xio_Set_Mouse_Position(dos_delete_x[REGISTER_DELETE_INITIAL_FOCUS],
						   dos_delete_y[REGISTER_DELETE_INITIAL_FOCUS]);
	dialog = Dos94_register_Build_Delete_Dialog();
	XwRegister_ScheduleDeleteDialog(dialog, hadKeyButtons, complete, context);
}

/* DOS94 0x583d98. Native pilot records retain the shared storage layout. */
Input* Dos94_register_Build_Delete_Dialog(void) {
	Rect rect;
	Input* dialog;
	PushButton* button;
	xrect_Set_Rect(&rect, 0, 0, REGISTER_DELETE_DIALOG_WIDTH, REGISTER_DELETE_DIALOG_HEIGHT);
	dialog = xinput_Alloc_Dialog_Input(NULL, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(dialog, Dos94_register_iupdate_Delete_Input);
	xinpattr_Set_Input_Draw_Function(dialog, Dos94_register_idraw_Delete_Input);
	xinpattr_Set_Input_Allign(dialog, REGISTER_DIALOG_ALIGN_CENTER, REGISTER_DIALOG_ALIGN_CENTER);
	xinpattr_Show_Input(dialog);
	dialog->id = 0;
	xrect_Set_Rect(&rect, REGISTER_DELETE_BUTTON_INSET, REGISTER_DELETE_BUTTON_INSET,
				   REGISTER_DELETE_BUTTON_INSET + REGISTER_DELETE_BUTTON_WIDTH,
				   REGISTER_DELETE_BUTTON_BOTTOM);
	button =
		xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Delete_Input,
							  g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE], REGISTER_DELETE_CONFIRM);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_NEAR, REGISTER_DIALOG_ALIGN_FAR);
	register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
	if (g_RegisterShellPilot.lost_status != REGISTER_PILOT_AVAILABLE) {
		xrect_Set_Rect(&rect, 0, REGISTER_DELETE_BUTTON_INSET, REGISTER_DELETE_BUTTON_WIDTH,
					   REGISTER_DELETE_BUTTON_BOTTOM);
		button = xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Delete_Input,
									   g_RegisterResourceAndLabelText[REGISTER_TEXT_REVIVE],
									   REGISTER_DELETE_REVIVE);
		xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_CENTER, REGISTER_DIALOG_ALIGN_FAR);
	}
	xrect_Set_Rect(&rect, REGISTER_DELETE_BUTTON_INSET, REGISTER_DELETE_BUTTON_INSET,
				   REGISTER_DELETE_BUTTON_INSET + REGISTER_DELETE_BUTTON_WIDTH,
				   REGISTER_DELETE_BUTTON_BOTTOM);
	button =
		xbtnpush_Alloc_Button(dialog, &rect, 0, register_iuser_Delete_Input,
							  g_RegisterResourceAndLabelText[REGISTER_TEXT_CANCEL], REGISTER_DELETE_CANCEL);
	xinpattr_Set_Input_Allign(&button->header, REGISTER_DIALOG_ALIGN_FAR, REGISTER_DIALOG_ALIGN_FAR);
	return dialog;
}

/* DOS94 0x583fb0. Native pilot records retain the shared storage layout. */
int16_t Dos94_register_iupdate_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
											int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	if (key != 0) {
		if (key == REGISTER_KEY_LEFT || key == REGISTER_KEY_UP) {
			g_RegisterDeleteFocus =
				(g_RegisterDeleteFocus + REGISTER_DELETE_TARGET_COUNT - 1) % REGISTER_DELETE_TARGET_COUNT;
			xio_Set_Mouse_Position(dos_delete_x[g_RegisterDeleteFocus], dos_delete_y[g_RegisterDeleteFocus]);
			return 1;
		} else if (key == REGISTER_KEY_RIGHT || key == REGISTER_KEY_DOWN) {
			g_RegisterDeleteFocus = (g_RegisterDeleteFocus + 1) % REGISTER_DELETE_TARGET_COUNT;
			xio_Set_Mouse_Position(dos_delete_x[g_RegisterDeleteFocus], dos_delete_y[g_RegisterDeleteFocus]);
			return 1;
		}
	}
	return 0;
}

/* DOS94 0x584132. Native pilot records retain the shared storage layout. */
void Dos94_register_idraw_Delete_Input(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	Rect titleRect;
	char pilotName[REGISTER_STRING_WORK_CAPACITY];
	char title[REGISTER_DIALOG_TITLE_CAPACITY];
	(void)clip;
	if (refresh != 0) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
		register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
		if (g_RegisterShellPilot.lost_status != REGISTER_PILOT_AVAILABLE)
			strcpy(title, g_RegisterResourceAndLabelText[REGISTER_TEXT_MODIFY_PILOT]);
		else
			strcpy(title, g_RegisterResourceAndLabelText[REGISTER_TEXT_DELETE_PILOT]);
		strcat(title, " ");
		strcat(title, pilotName);
		strcat(title, "?");
		if (input->id == 0) {
			xrect_Copy_Rect(&titleRect, frame);
			xpaint_Frame_Clipped_Rect(&titleRect, REGISTER_DIALOG_FRAME_COLOR);
			xrect_Inset_Rect(&titleRect, REGISTER_DIALOG_FRAME_INSET, REGISTER_DIALOG_FRAME_INSET);
			xstyle_Style_Paint_Border(&titleRect, 0);
			titleRect.bottom = titleRect.top + REGISTER_DIALOG_TITLE_HEIGHT;
			xfont_Enable_FontID_Shadow(REGISTER_DIALOG_FONT);
			xfont_Print_Centered_Text(title, &titleRect, REGISTER_DIALOG_FONT, REGISTER_DIALOG_TEXT_COLOR);
			xfont_Disable_FontID_Shadow(REGISTER_DIALOG_FONT);
		}
	}
}

/* DOS94 0x582766. Native pilot records retain the shared storage layout. */
void Dos94_register_idraw_Pilot_Info(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	unsigned int drawMode = REGISTER_INFO_NONE;
	int16_t outSlot[2];
	char tourName[REGISTER_INFO_SHORT_TEXT_SIZE];
	Rect rect;
	char rankName[REGISTER_INFO_SHORT_TEXT_SIZE];
	char scoreText[REGISTER_INFO_SHORT_TEXT_SIZE];
	char pilotName[REGISTER_DIRECTORY_NAME_CAPACITY];
	char pilotFileName[REGISTER_DIRECTORY_NAME_CAPACITY];
	if (refresh != 0) {
		xactdelt_Draw_Delta_Actor(g_RegisterLeftDeskActor, frame, clip, 0, 0, refresh);
		drawMode = REGISTER_INFO_CACHED;
	}
	if (input->var1 != 0) {
		if (input->var1 == REGISTER_INFO_START) {
			input->var1 = REGISTER_INFO_RUNNING;
			input->var2 = 0;
			drawMode = REGISTER_INFO_NONE;
		} else {
			xactdelt_Draw_Delta_Actor(g_RegisterLeftDeskActor, frame, clip, 0, 0, 1);
			if (input->var2 < REGISTER_INFO_DELAY) {
				drawMode = ((unsigned int)input->var2 & REGISTER_INFO_FLASH_BIT) >> 1;
			} else {
				drawMode = REGISTER_INFO_LOAD;
				input->var1 = 0;
				input->var2 = 0;
			}
		}
	}
	if ((uint16_t)drawMode == REGISTER_INFO_LOAD || (uint16_t)drawMode == REGISTER_INFO_CACHED) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
		if ((uint16_t)drawMode == REGISTER_INFO_LOAD)
			memset(&g_RegisterPilotData, 0, sizeof(g_RegisterPilotData));
		if (g_RegisterActivePilot != REGISTER_PILOT_SLOT_NONE && (uint16_t)drawMode == REGISTER_INFO_LOAD &&
			register_Index_To_Pilot(g_RegisterActivePilot, outSlot) != 0 &&
			register_Find_Reg_Dir_Name(&g_RegisterDirectory, pilotFileName, outSlot[0]) != 0) {
			strcat(pilotFileName, ".PLT");
			if (shipext_Load_Pilot(pilotFileName, 0) == 0) {
				pilotName[0] = 0;
				g_RegisterActivePilot = REGISTER_PILOT_SLOT_NONE;
				register_Set_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
				xinpattr_Refresh_Input(&g_RegisterPilotNameInput->input);
			}
		}
		if (strlen(pilotName) != 0) {
			strcpy(rankName, g_RegisterRankNames[g_RegisterPilotData.rank]);
			sprintf(scoreText, "%lu", (unsigned long)g_RegisterPilotData.score);
			if (g_RegisterPilotData.tour_status[g_RegisterPilotData.current_tour] == 1)
				xparagrp_Get_Paragraph_String(g_RegisterTourParagraph, tourName, 0,
											  g_RegisterPilotData.current_tour);
			else
				tourName[0] = 0;
			xfont_Enable_FontID_Shadow(0);
			xfont_Enable_FontID_Shadow(1);
			xfont_Print_Clipped_Text("Name", frame->left + REGISTER_INFO_TEXT_X,
									 frame->top + REGISTER_INFO_NAME_Y, 1, 40);
			xpaint_Horiz_Clipped_Line(frame->left + REGISTER_INFO_RULE_X, frame->top + 10,
									  frame->right - frame->left - REGISTER_INFO_RULE_MARGIN,
									  REGISTER_INFO_RULE_COLOR);
			if (g_RegisterPilotData.lost_status != 0) {

				xfont_Print_Clipped_Text(g_RegisterLostStatusLabels[g_RegisterPilotData.lost_status],
										 frame->left + 26, frame->top + REGISTER_INFO_NAME_Y, 1,
										 REGISTER_INFO_STATUS_COLOR);
			}
			xfont_Print_Clipped_Text("Rank", frame->left + REGISTER_INFO_TEXT_X, frame->top + 22, 1, 62);
			xpaint_Horiz_Clipped_Line(frame->left + REGISTER_INFO_RULE_X, frame->top + 28,
									  frame->right - frame->left - REGISTER_INFO_RULE_MARGIN,
									  REGISTER_INFO_RULE_COLOR);
			xfont_Print_Clipped_Text("TOD Score", frame->left + REGISTER_INFO_TEXT_X, frame->top + 40, 1, 54);
			xpaint_Horiz_Clipped_Line(frame->left + REGISTER_INFO_RULE_X, frame->top + 46,
									  frame->right - frame->left - REGISTER_INFO_RULE_MARGIN,
									  REGISTER_INFO_RULE_COLOR);
			if (tourName[0] != 0) {
				xfont_Print_Clipped_Text(g_RegisterLostStatusLabels[0], frame->left + REGISTER_INFO_TEXT_X,
										 frame->top + 58, 1, 49);
				xpaint_Horiz_Clipped_Line(frame->left + REGISTER_INFO_RULE_X, frame->top + 64,
										  frame->right - frame->left - REGISTER_INFO_RULE_MARGIN,
										  REGISTER_INFO_RULE_COLOR);
			}
			xfont_Print_Clipped_Text(pilotName, frame->left + REGISTER_INFO_TEXT_X, frame->top + 12, 0, 47);
			xfont_Print_Clipped_Text(rankName, frame->left + REGISTER_INFO_TEXT_X, frame->top + 30, 0, 61);
			xfont_Print_Clipped_Text(scoreText, frame->left + REGISTER_INFO_TEXT_X, frame->top + 48, 0, 55);
			if (tourName[0] != 0) {
				xrect_Set_Rect(&rect, frame->left, frame->top + 66, frame->right, frame->top + 74);
				xfont_Print_Centered_Text(tourName, &rect, 0, 50);
			}
			xfont_Disable_FontID_Shadow(0);
			xfont_Disable_FontID_Shadow(1);
		}
	}
	if ((uint16_t)drawMode == REGISTER_INFO_ACCESS) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotName);
		xrect_Offset_Rect(frame, 1, REGISTER_INFO_ACCESS_Y);
		xfont_Print_Centered_Text(g_RegisterResourceAndLabelText[REGISTER_INFO_ACCESS_TEXT], frame, 0,
								  REGISTER_INFO_ACCESS_SHADOW_COLOR);
		xrect_Offset_Rect(frame, -1, -1);
		xfont_Print_Centered_Text(g_RegisterResourceAndLabelText[REGISTER_INFO_ACCESS_TEXT], frame, 0,
								  REGISTER_INFO_ACCESS_COLOR);
		xrect_Offset_Rect(frame, 1, REGISTER_INFO_ACCESS_NAME_Y);
		xfont_Print_Centered_Text(pilotName, frame, 0, REGISTER_INFO_ACCESS_SHADOW_COLOR);
		xrect_Offset_Rect(frame, -1, -1);
		xfont_Print_Centered_Text(pilotName, frame, 0, REGISTER_INFO_ACCESS_COLOR);
	}
}

/* DOS94 0x581eb0. Native pilot records retain the shared storage layout. */
void Dos94_register_idraw_ListMessage(Input* input, Rect* frame, Rect* unusedClip, int16_t refresh) {
	Rect textRect;
	(void)unusedClip;
	if (refresh != 0) {
		xactdelt_Draw_Delta_Actor(message_background, frame, unusedClip, 0, 0, refresh);
		int16_t firstLineIndex;
		int16_t secondLineIndex;
		int16_t textColor;
		const char* line;
		xrect_Copy_Rect(&textRect, frame);
		switch (input->var1) {
			case REGISTER_MESSAGE_ENTER:
				firstLineIndex = REGISTER_TEXT_ENTER;
				secondLineIndex = REGISTER_TEXT_SPACEPORT;
				textColor = REGISTER_MESSAGE_NORMAL_COLOR;
				break;
			case REGISTER_MESSAGE_REGISTER:
				firstLineIndex = REGISTER_TEXT_YOU_MUST;
				secondLineIndex = REGISTER_TEXT_REGISTER;
				textColor = REGISTER_MESSAGE_WARNING_COLOR;
				break;
			case REGISTER_MESSAGE_CAPTURED:
				firstLineIndex = REGISTER_TEXT_THIS_PILOT_IS;
				secondLineIndex = REGISTER_TEXT_CAPTURED;
				textColor = REGISTER_MESSAGE_WARNING_COLOR;
				break;
			case REGISTER_MESSAGE_DEAD:
				firstLineIndex = REGISTER_TEXT_THIS_PILOT_IS;
				secondLineIndex = REGISTER_TEXT_DEAD;
				textColor = REGISTER_MESSAGE_WARNING_COLOR;
				break;
			default:
				textColor = refresh;
				secondLineIndex = refresh;
				firstLineIndex = refresh;
				break;
		}
		xrect_Offset_Rect(&textRect, REGISTER_MESSAGE_SHADOW_OFFSET,
						  REGISTER_MESSAGE_FIRST_LINE_Y + REGISTER_MESSAGE_SHADOW_OFFSET);
		line = g_RegisterResourceAndLabelText[firstLineIndex];
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, REGISTER_MESSAGE_SHADOW_COLOR);
		xrect_Offset_Rect(&textRect, -REGISTER_MESSAGE_SHADOW_OFFSET, -REGISTER_MESSAGE_SHADOW_OFFSET);
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, textColor);
		xrect_Offset_Rect(&textRect, REGISTER_MESSAGE_SHADOW_OFFSET,
						  REGISTER_MESSAGE_LINE_SPACING + REGISTER_MESSAGE_SHADOW_OFFSET);
		line = g_RegisterResourceAndLabelText[secondLineIndex];
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, REGISTER_MESSAGE_SHADOW_COLOR);
		xrect_Offset_Rect(&textRect, -REGISTER_MESSAGE_SHADOW_OFFSET, -REGISTER_MESSAGE_SHADOW_OFFSET);
		xfont_Print_Centered_Text(line, &textRect, REGISTER_MESSAGE_FONT, textColor);
	}
}

/* DOS94 0x584e26. Native pilot records retain the shared storage layout. */
void Dos94_register_user_Music(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	if (g_RegisterMusicState.film->cur_cel == REGISTER_MUSIC_START_CEL) {
		soundext_Start_Resource_Sound(g_RegisterMusicState.sound);
		xsound_Set_Sound_Keep(g_RegisterMusicState.sound);
		soundext_FadeVolume(g_RegisterMusicState.sound, REGISTER_MUSIC_VOLUME, REGISTER_MUSIC_FADE_DURATION);
	}
}

/* DOS94 0x581032. */
static int16_t draw_background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	if (refresh) {
		Rect screen;
		xrect_Set_Rect(&screen, 0, 0, 320, 200);
		xpaint_Paint_Clipped_Rect(&screen, 0);
	}
	return xactdelt_Draw_Delta_Actor(actor, frame, clip, x, y, refresh);
}

/* DOS94 0x581094. Each pattern changes only the indicated robot layers. */
static void update_robot(Actor* actor, int32_t time) {
	if (!time || actor->var1 <= 0) {
		actor->var1 = 40 + (rand() & 15);
		actor->var2 = rand() & 3;
	}
	if (actor->var1 > 31) {
		robot_layers[0] = robot_layers[1] = robot_layers[2] = 0;
	} else {
		int step = actor->var1 - 1;
		switch (actor->var2) {
			case 0:
				if (step == 8)
					robot_layers[0] = 1;
				else if (step == 16)
					robot_layers[0] = 0;
				else if (step == 24)
					robot_layers[0] = 4;
				break;
			case 1:
				if (step == 8)
					robot_layers[0] = robot_layers[1] = 1;
				else if (step == 16)
					robot_layers[1] = 2;
				else if (step == 24)
					robot_layers[1] = 1;
				break;
			case 2:
				if (step == 8) {
					robot_layers[2] = 1;
					robot_layers[0] = 2;
				} else if (step == 12)
					robot_layers[0] = 4;
				else if (step == 16)
					robot_layers[2] = 2;
				else if (step == 24)
					robot_layers[2] = 1;
				break;
			case 3:
				if (step == 4)
					robot_layers[2] = 1;
				else if (step == 8) {
					robot_layers[0] = 4;
					robot_layers[1] = 1;
				} else if (step == 12) {
					robot_layers[2] = 0;
					robot_layers[0] = 2;
				} else if (step == 16) {
					robot_layers[0] = 1;
					robot_layers[1] = 2;
				} else if (step == 24)
					robot_layers[1] = 1;
				break;
		}
	}
	--actor->var1;
}

/* DOS94 0x581318. */
static int16_t draw_robot(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	if (!refresh)
		return 0;
	xactor_Set_Actor_State(actor, 0, 0);
	xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	xactor_Set_Actor_State(actor, robot_layers[0] + 1, 0);
	xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	xactor_Set_Actor_State(actor, robot_layers[1] + 6, 0);
	xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
	xactor_Set_Actor_State(actor, robot_layers[2] + 10, 0);
	return xactanim_Draw_Anim_Actor(actor, frame, clip, x, y, refresh);
}

/* DOS94 0x581430. */
static void update_door(Actor* actor, int32_t time) {
	(void)time;
	int refresh = 1;
	if (actor->var1) {
		if (!actor->state)
			register_PlaySoundCue(2);
		if (actor->state < actor->arraySize - 1)
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		else
			refresh = 0;
		actor->var1 = 0;
	} else {
		if (actor->state == actor->arraySize - 1)
			register_PlaySoundCue(3);
		if (actor->state)
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		else
			refresh = 0;
	}
	if (refresh)
		xactor_Refresh_Actor(actor);
}

/* DOS94 0x581520. The three ANIMs already contain their authored screen offsets. */
static void update_soldier(Actor* actor, int32_t time) {
	if (actor->var1) {
		if (actor->var2 == 2)
			register_PlaySoundCue(1);
		if (actor->var2 < 3 && (time & 1))
			++actor->var2;
		actor->var1 = 0;
	} else if (actor->var2 > 0 && (time & 1))
		--actor->var2;
	static const int16_t states[12] = { 1, 1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 3 };
	int index = 4 * actor->id + actor->var2;
	if ((unsigned)index >= 12)
		return;
	xactor_Set_Actor_Pos(actor, 0, actor->id ? 1 : 0, 0, 0);
	xactor_Set_Actor_State(actor, states[index], 0);
}

/* DOS94 0x581aa8. Coordinates passed by Landru are relative to this input. */
static int16_t update_list(Input* input, Rect* frame, Rect* clip, int16_t key, int left, int right, int16_t x,
						   int16_t y) {
	(void)clip;
	if (key)
		return 0;
	if ((left ? left : right) == 3) {
		int row = ((int16_t)(y + 6) / 7) - 1;
		if (row < 0)
			row = 0;
		if (row > 9)
			row = 9;
		if (x > (frame->right - frame->left) / 2)
			row += 10;
		int16_t slot;
		char name[REGISTER_DIRECTORY_NAME_CAPACITY];
		if (register_Index_To_Pilot(g_RegisterPilotOffset + row, &slot) &&
			register_Find_Reg_Dir_Name(&g_RegisterDirectory, name, slot)) {
			register_Set_Reg_String_Button_Name(g_RegisterPilotNameInput, name);
			xinpattr_Refresh_Input(input);
			g_RegisterActivePilot = g_RegisterPilotOffset + row;
			g_RegisterPilotInfoInput->var1 = 1;
		}
	}
	return 1;
}

/* DOS94 0x581c00. Left-column missing slots do not advance the text row. */
static void draw_list(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)input;
	if (!refresh)
		return;
	xactdelt_Draw_Delta_Actor(g_RegisterRightDeskActor, frame, clip, 0, 0, refresh);
	int midpoint = frame->left + (frame->right - frame->left) / 2;
	xfont_Enable_FontID_Shadow(1);
	for (int column = 0; column < 2; ++column) {
		Rect row = *frame;
		if (column)
			row.left = midpoint;
		else
			row.right = midpoint - 3;
		xcanvas_Set_Drawing_Canvas_Clip(&row);
		row.top += 1;
		for (int i = 0; i < 10; ++i) {
			int16_t slot;
			int index = g_RegisterPilotOffset + 10 * column + i;
			if (register_Index_To_Pilot(index, &slot)) {
				char name[REGISTER_DIRECTORY_NAME_CAPACITY];
				if (register_Find_Reg_Dir_Name(&g_RegisterDirectory, name, slot)) {
					REGISTER_FastPilotRecord record;
					register_Index_To_Pilot_Record(index, &record);
					int color = record.lost_status ? 4 : 15;
					if (g_RegisterActivePilot == index)
						color = 14;
					xfont_Print_Clipped_Text(name, row.left + (column ? 2 : 1), row.top, 1, color);
				}
				row.top += 7;
			} else if (column)
				row.top += 7;
		}
	}
	xfont_Disable_FontID_Shadow(1);
}

/* DOS94 0x5822ac. */
static void draw_button(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	PushButton* button = (PushButton*)input;
	if (!refresh)
		return;
	xstyle_Style_Paint_Border(frame, button->pressed);
	if (input->id < 2)
		xstyle_Style_Draw_Centered_Icon(input->id == 0 ? 1 : 3, frame, clip, button->pressed);
	else
		xstyle_Style_Small_Button_Text(button->labels, frame, button->pressed);
}

/* DOS94 0x582352. */
static void draw_page(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)input;
	(void)clip;
	if (!refresh)
		return;
	char text[32];
	xpaint_Paint_Clipped_Bevel(frame, 38, 26, 0, 1);
	snprintf(text, sizeof text, "Page %d/%d", g_RegisterCurrentPage + 1, g_RegisterPageCount);
	xfont_Print_Centered_Text(text, frame, 1, 15);
}

/* DOS94 0x5825c8. */
static void draw_name(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)clip;
	REGISTER_RegStringButton* button = (REGISTER_RegStringButton*)input;
	int width = xfont_Get_String_Width_0(0, button->name);
	if (refresh) {
		xpaint_Paint_Clipped_Rect(frame, 0);
		xfont_Print_Clipped_Text(button->name, frame->left + 2, frame->top + 1, 0, 38);
	}
	int color = xinpattr_Is_Input_Active(input) && strlen(button->name) < 8 && xio_Blink() ? 38 : 0;
	xpaint_Horiz_Clipped_Line(frame->left + width + 4, frame->top + 7, 5, color);
}

void Dos94_Register_CompleteDeletion(int16_t result, void* context) {
	(void)context;
	if (result == REGISTER_DELETE_CANCEL)
		return;
	if (result == REGISTER_DELETE_CONFIRM) {
		char filename[REGISTER_PILOT_BUTTON_PATH_CAPACITY];
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, filename);
		strcat(filename, ".PLT");
		XwStorage_Remove(filename);
		Dos94_register_Delete_Pilot_Record();
	} else
		register_Revive_Pilot_Record();
	xview_Refresh_View();
}

/* DOS94 0x584e82. */
static void load_sounds(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_GUARD, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_MENU_DOOR_CLOSE_1, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		g_RegisterSpeech[REGISTER_SPEECH_FIRST] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_WELCOME, REGISTER_SPEECH_FIRST, NULL, 0);
		g_RegisterSpeech[REGISTER_SPEECH_WAIT] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_YOU_MAY, REGISTER_SPEECH_WAIT, NULL, 1);
		g_RegisterSpeech[REGISTER_SPEECH_REGISTER] =
			soundext_LoadSpeech(XW_SHELL_SPEECH_REGISTER, REGISTER_SPEECH_REGISTER, NULL, 0);
	}
}

/* DOS94 0x580000. File records are supplied by the shared native pilot store. */
void Dos94_Register(XwShellContext* shell) {
	g_RegisterNavigationIndex = 54;
	xio_Set_Mouse_Position(243, 191);
	g_RegisterTourParagraph = xparagrp_Res_Paragraph(shell->resourceFile, "tours");
	ResFile* resource = xres_Open_Resource("register.lfd");
	g_RegisterActivePilot = -1;
	g_RegisterPilotOffset = g_RegisterPilotCount = g_RegisterCurrentPage = 0;
	g_RegisterPageCount = 1;
	g_RegisterFastPilotHandle = 0;
	Rect rect;
	xrect_Set_Rect(&rect, 0, 0, 320, 200);
	g_RegisterFilm = xfilm_Res_Film(resource, "pilot", &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_RegisterFilm, shell->standardPalette);
	g_RegisterBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "PILOT");
	xactor_Set_Actor_Draw_Function(g_RegisterBackgroundActor, draw_background);
	xactor_Non_Refreshable_Actor(g_RegisterBackgroundActor);
	g_RegisterDoorActor = xactor_Find_Actor(FOURCC_ANIM, "DOOR");
	xactor_Set_Actor_User_Function(g_RegisterDoorActor, update_door);
	xactor_Non_Refreshable_Actor(g_RegisterDoorActor);
	static const char* const parts[] = { "grdhed01", "grdbdy03", "grdarm02" };
	for (int i = 0; i < 3; ++i) {
		soldiers[i] = xactor_Find_Actor(FOURCC_ANIM, parts[i]);
		xactor_Set_Actor_User_Function(soldiers[i], update_soldier);
		soldiers[i]->id = i;
	}
	g_RegisterRobotActor = xactanim_Res_Anim_Actor(resource, "robot", &rect, 0, 0, 50);
	xactor_Set_Actor_User_Function(g_RegisterRobotActor, update_robot);
	xactor_Set_Actor_Draw_Function(g_RegisterRobotActor, draw_robot);
	g_RegisterRightDeskActor = xactdelt_Res_Delta_Actor(resource, "nameback", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_RegisterRightDeskActor, 0, 0);
	message_background = xactdelt_Res_Delta_Actor(resource, "namebck2", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(message_background, 0, 0);
	g_RegisterLeftDeskActor = xactdelt_Res_Delta_Actor(resource, "infoback", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_RegisterLeftDeskActor, 0, 0);
	g_RegisterProtectSymbolActor = xactanim_Res_Anim_Actor(resource, "symbols", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_RegisterProtectSymbolActor, 0, 0);
	g_RegisterPageInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	xinpattr_Refresh_Input(g_RegisterPageInput);
	xrect_Set_Rect(&rect, 0, 42, 34, 182);
	g_RegisterDoorInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_RegisterDoorInput, Dos94_register_iupdate_Register);
	xinpattr_Set_Input_User_Function(g_RegisterDoorInput, register_iuser_Register);
	g_RegisterDoorInput->mouseUsage = allInput;
	xrect_Set_Rect(&rect, 111, 94, 185, 166);
	g_RegisterPilotListInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_RegisterPilotListInput, update_list);
	xinpattr_Set_Input_User_Function(g_RegisterPilotListInput, XwInput_UserNoOp);
	xinpattr_Set_Input_Draw_Function(g_RegisterPilotListInput, draw_list);
	g_RegisterListOverlayInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_RegisterListOverlayInput, Dos94_register_idraw_ListMessage);
	xinpattr_Hide_Input(g_RegisterListOverlayInput);
	xrect_Set_Rect(&rect, 109, 168, 182, 184);
	Input* navigation = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	for (int i = 0; i < 2; ++i) {
		xrect_Set_Rect(&rect, 0, 0, 14, 16);
		PushButton* button =
			xbtnpush_Alloc_Button(navigation, &rect, 0, Dos94_register_iuser_Pilot_Button, NULL, i);
		xinpattr_Set_Input_Draw_Function(&button->header, draw_button);
		xinpattr_Set_Input_Allign(&button->header, i ? 2 : 0, 1);
	}
	xrect_Set_Rect(&rect, 0, 0, 43, 14);
	Input* page = xinput_Alloc_Input(navigation, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(page, draw_page);
	xinpattr_Set_Input_Allign(page, 1, 1);
	page->id = 1;
	xrect_Set_Rect(&rect, 118, 185, 178, 194);
	g_RegisterPilotNameInput = Dos94_register_Alloc_Input_Reg_String_Button(
		g_RegisterPageInput, &rect, 0, Dos94_register_iuser_Pilot_Name, "", 1, 0);
	xinpattr_Set_Input_Draw_Function(&g_RegisterPilotNameInput->input, draw_name);
	xrect_Set_Rect(&rect, 207, 93, 279, 194);
	g_RegisterPilotInfoInput = xinput_Alloc_Input(g_RegisterPageInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_RegisterPilotInfoInput, XwInput_UpdateNoOp);
	xinpattr_Set_Input_User_Function(g_RegisterPilotInfoInput, register_iuser_Pilot_Info);
	xinpattr_Set_Input_Draw_Function(g_RegisterPilotInfoInput, Dos94_register_idraw_Pilot_Info);
	static const int bounds[3][4] = { { 207, 174, 232, 184 },
									  { 234, 174, 279, 184 },
									  { 207, 186, 279, 196 } };
	static const char* const labels[] = { "Log", "Merits", "Delete Pilot" };
	Input** buttons[] = { &g_RegisterLogButton, &g_RegisterMeritsButton, &g_RegisterDeleteButton };
	for (int i = 0; i < 3; ++i) {
		xrect_Set_Rect(&rect, bounds[i][0], bounds[i][1], bounds[i][2], bounds[i][3]);
		*buttons[i] = &xbtnpush_Alloc_Button(g_RegisterPageInput, &rect, 0, Dos94_register_iuser_Pilot_Button,
											 labels[i], i + 2)
						   ->header;
		xinpattr_Set_Input_Draw_Function(*buttons[i], draw_button);
		xinpattr_Hide_Input(*buttons[i]);
	}
	xfiledir_Init_Directory(&g_RegisterDirectory, ".PLT", 0);
	register_OpenMusic(resource, g_RegisterFilm);
	if (ShellPreferences_GetMusicEnabled())
		xsound_Set_Sound_User_Function(g_RegisterMusicState.sound, Dos94_register_user_Music);
	load_sounds(resource, g_RegisterFilm);
	xview_Set_View_Update_Function(Dos94_register_end_View);
	xview_Disable_All_View_Erase();
	xio_Set_Key_Buttons();
	XwRegister_RunView(resource);
}
