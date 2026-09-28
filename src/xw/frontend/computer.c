#include "xw/frontend/computer.h"

#include "xw_runtime/runtime/computer_task.h"

#include <landru/dialog.h>
#include <landru/font.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/style.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DD8EC
const char g_computerConfirmPrompt[] = "Are you sure?";

// GLOBAL: XW 0x5667AC
Input* g_computerConfirmYesButton = NULL;

// GLOBAL: XW 0x5667B0
char g_computerConfirmYesLabel[COMPUTER_CONFIRM_LABEL_CAPACITY] = { 0 };

// GLOBAL: XW 0x5667B6
char g_computerConfirmNoLabel[COMPUTER_CONFIRM_LABEL_CAPACITY] = { 0 };

// GLOBAL: XW 0x5667E4
Input* g_computerConfirmNoButton = NULL;

// FUNCTION: XW 0x4A94E0
void computer_ConfirmJoystickBindingReset(DialogSubResultHandler complete, void* context) {
	Input* dialog = computer_Build_Exit(g_computerConfirmPrompt);
	xio_Set_Mouse_Position(COMPUTER_CONFIRM_MOUSE_X, COMPUTER_CONFIRM_MOUSE_Y);
	XwComputer_ScheduleBindingResetDialog(dialog, complete, context);
}

// FUNCTION: XW 0x4A9530
Input* computer_Build_Exit(const char* prompt) {
	Rect bounds;
	Input* dialog;
	(void)prompt;
	xrect_Set_Rect(&bounds, 0, 0, COMPUTER_CONFIRM_WIDTH, COMPUTER_CONFIRM_HEIGHT);
	dialog = xinput_Alloc_Dialog_Input(NULL, &bounds, 0, 0);
	xinpattr_Set_Input_Draw_Function(dialog, computer_idraw_Exit);
	xinpattr_Set_Input_Allign(dialog, COMPUTER_CONFIRM_ALIGN_CENTER, COMPUTER_CONFIRM_ALIGN_CENTER);
	xinpattr_Start_Input(dialog);
	strcpy(g_computerConfirmYesLabel, "Yes");
	strcpy(g_computerConfirmNoLabel, "No");
	xrect_Set_Rect(&bounds, COMPUTER_CONFIRM_YES_LEFT, COMPUTER_CONFIRM_BUTTON_TOP,
				   COMPUTER_CONFIRM_YES_RIGHT, COMPUTER_CONFIRM_BUTTON_BOTTOM);
	g_computerConfirmYesButton = &xbtnpush_Alloc_Button(dialog, &bounds, 0, computer_iuser_Exit,
														g_computerConfirmYesLabel, COMPUTER_EXIT_YES)
									  ->header;
	xinpattr_Set_Input_Update_Function(g_computerConfirmYesButton, computer_iupdate_ConfirmYes);
	xrect_Set_Rect(&bounds, COMPUTER_CONFIRM_NO_LEFT, COMPUTER_CONFIRM_BUTTON_TOP, COMPUTER_CONFIRM_NO_RIGHT,
				   COMPUTER_CONFIRM_BUTTON_BOTTOM);
	g_computerConfirmNoButton = &xbtnpush_Alloc_Button(dialog, &bounds, 0, computer_iuser_Exit,
													   g_computerConfirmNoLabel, COMPUTER_EXIT_NO)
									 ->header;
	xinpattr_Set_Input_Update_Function(g_computerConfirmNoButton, computer_iupdate_ConfirmNo);
	return dialog;
}

// FUNCTION: XW 0x4A9650
int16_t computer_iupdate_ConfirmYes(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									int rightEvent, int16_t x, int16_t y) {
	if (key != 0) {
		if (key != 'y') {
			return 0;
		}
		xinpattr_Selected_Input(g_computerConfirmYesButton);
		return 1;
	}
	return xbtnpush_iupdate_Button(input, frame, clip, 0, leftEvent, rightEvent, x, y);
}

// FUNCTION: XW 0x4A96B0
int16_t computer_iupdate_ConfirmNo(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								   int rightEvent, int16_t x, int16_t y) {
	if (key != 0) {
		if (key != 'n') {
			return 0;
		}
		xinpattr_Selected_Input(g_computerConfirmNoButton);
		return 1;
	}
	return xbtnpush_iupdate_Button(input, frame, clip, 0, leftEvent, rightEvent, x, y);
}

// FUNCTION: XW 0x4A9710
void computer_idraw_Exit(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	(void)input;
	(void)clip;
	if (refresh != 0) {
		xstyle_Style_Paint_Border(frame, 0);
		xfont_Print_Clipped_Text(g_computerConfirmPrompt, frame->left + COMPUTER_CONFIRM_TEXT_X,
								 frame->top + COMPUTER_CONFIRM_TEXT_Y, COMPUTER_CONFIRM_FONT,
								 COMPUTER_CONFIRM_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x4A9750
void computer_iuser_Exit(Input* input, int time) {
	(void)time;
	switch (input->id) {
		case COMPUTER_EXIT_YES:
			if (xinpattr_Get_Input_Selected(input)) {
				xdialog_Set_Dialog_Exit(COMPUTER_EXIT_YES);
			}
			break;
		case COMPUTER_EXIT_NO:
			if (xinpattr_Get_Input_Selected(input)) {
				xdialog_Set_Dialog_Exit(COMPUTER_EXIT_NO);
			}
			break;
	}
}
