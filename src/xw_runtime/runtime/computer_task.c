#include "xw_runtime/runtime/computer_task.h"

#include "xw/frontend/computer.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/input/joystick.h"
#include <landru/view.h>
#include <string.h>

#include <landru/task.h>
#include <stdlib.h>

typedef struct XwComputerConfirmState {
	Input* dialog;
	int16_t exitCode;
	DialogSubResultHandler complete;
	void* context;
} XwComputerConfirmState;

static XwComputerConfirmState confirm_state;

static void confirm_dialog_cleanup(Input* dialog) {
	confirm_state.exitCode = xdialog_Get_Dialog_Exit();
	xinput_Free_Inputs(dialog);
	xdialog_Clear_Dialog_Exit();
	confirm_state.dialog = NULL;
}

static void confirm_dialog_complete(int16_t unusedResult, void* unusedContext) {
	DialogSubResultHandler complete = confirm_state.complete;
	void* context = confirm_state.context;
	int16_t accepted = confirm_state.exitCode != COMPUTER_EXIT_NO;
	(void)unusedResult;
	(void)unusedContext;
	confirm_state.complete = NULL;
	confirm_state.context = NULL;
	if (complete != NULL)
		complete(accepted, context);
}

void XwComputer_ScheduleBindingResetDialog(Input* dialog, DialogSubResultHandler complete, void* context) {
	if (confirm_state.dialog != NULL)
		abort();
	confirm_state.dialog = dialog;
	confirm_state.complete = complete;
	confirm_state.context = context;
	/* The callback batch cannot push tasks before this request is drained. */
	if (landru_task_depth() >= LANDRU_TASK_STACK_DEPTH) {
		confirm_dialog_cleanup(dialog);
		abort();
	}
	if (!xdialog_Schedule_Owned_Sub_Dialog(dialog, confirm_dialog_cleanup, confirm_dialog_complete, NULL))
		abort();
}

void XwComputer_ApplyBindingReset(int16_t accepted, void* context) {
	int buttonCount;
	int buttonIndex;
	(void)context;
	if (accepted == 0)
		return;
	memset(g_shellPreferences.joystickActions, 0, sizeof(g_shellPreferences.joystickActions));
	buttonCount = Joystick_GetButtonCount();
	for (buttonIndex = 0; buttonIndex < buttonCount; ++buttonIndex) {
		if (buttonIndex >= JOYSTICK_DEFAULT_ASSIGNMENT_LIMIT)
			break;
		switch (buttonIndex) {
			case 0:
				g_shellPreferences.joystickActions[0] = JOYSTICK_ACTION_FIRE;
				break;
			case 1:
				g_shellPreferences.joystickActions[1] = JOYSTICK_ACTION_ROLL_TARGET;
				break;
			case 2:
				g_shellPreferences.joystickActions[2] = JOYSTICK_ACTION_NEAREST_FIGHTER;
				break;
			case 3:
				g_shellPreferences.joystickActions[3] = JOYSTICK_ACTION_TOGGLE_COCKPIT;
				break;
			case 4:
				g_shellPreferences.joystickActions[4] = JOYSTICK_ACTION_NEAREST_ATTACKER;
				break;
			case 5:
				g_shellPreferences.joystickActions[5] = JOYSTICK_ACTION_IDENTIFY;
				break;
			case 6:
				g_shellPreferences.joystickActions[6] = JOYSTICK_ACTION_THIRD_THROTTLE;
				break;
			case 7:
				g_shellPreferences.joystickActions[7] = JOYSTICK_ACTION_FULL_THROTTLE;
				break;
			case 8:
				g_shellPreferences.joystickActions[8] = JOYSTICK_ACTION_MATCH_SPEED;
				break;
			case 9:
				g_shellPreferences.joystickActions[9] = JOYSTICK_ACTION_TWO_THIRDS_THROTTLE;
				break;
		}
	}
	xview_Refresh_View();
}
