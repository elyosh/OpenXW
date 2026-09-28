#include "xw_runtime/runtime/register_task.h"
#include "xw/flight/flight.h"

#include "xw/assets/file.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shipext.h"
#include "xw/util/shared.h"

#include <landru/error.h>
#include <landru/io.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>
#include <string.h>

static LandruTaskStepResult step_speech_wait(void* self) {
	(void)self;
	return soundext_Count_Resource_Instances(g_RegisterSpeech[REGISTER_SPEECH_WAIT]) == 1
			   ? LANDRU_TASK_STEP_YIELD
			   : LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable speech_wait_vtable = { step_speech_wait, NULL, NULL, NULL };

void XwRegister_WaitForSpeech(void) {
	if (soundext_Count_Resource_Instances(g_RegisterSpeech[REGISTER_SPEECH_WAIT]) != 1)
		return;
	if (landru_task_push(&speech_wait_vtable) == NULL)
		abort();
}

typedef struct XwRegisterDeleteState {
	Input* dialog;
	int16_t hadKeyButtons;
	int16_t exitCode;
	DialogSubResultHandler complete;
	void* context;
} XwRegisterDeleteState;

static XwRegisterDeleteState delete_state;

static void delete_dialog_cleanup(Input* dialog) {
	delete_state.exitCode = xdialog_Get_Dialog_Exit();
	xdialog_Clear_Dialog_Exit();
	xinput_Free_Inputs(dialog);
	if (delete_state.hadKeyButtons == 0)
		xio_Clear_Key_Buttons();
	delete_state.dialog = NULL;
	if (g_quitRequested) {
		delete_state.complete = NULL;
		delete_state.context = NULL;
	}
}

static void delete_dialog_complete(int16_t unusedResult, void* unusedContext) {
	DialogSubResultHandler complete = delete_state.complete;
	void* context = delete_state.context;
	int16_t exitCode = delete_state.exitCode;
	(void)unusedResult;
	(void)unusedContext;
	delete_state.complete = NULL;
	delete_state.context = NULL;
	if (complete != NULL)
		complete(exitCode, context);
}

void XwRegister_ScheduleDeleteDialog(Input* dialog, int16_t hadKeyButtons, DialogSubResultHandler complete,
									 void* context) {
	if (delete_state.dialog != NULL)
		abort();
	delete_state.dialog = dialog;
	delete_state.hadKeyButtons = hadKeyButtons;
	delete_state.complete = complete;
	delete_state.context = context;
	/* Input callbacks defer the push until the owning callback batch has returned. */
	if (landru_task_depth() >= LANDRU_TASK_STACK_DEPTH) {
		delete_dialog_cleanup(dialog);
		abort();
	}
	if (!xdialog_Schedule_Owned_Sub_Dialog(dialog, delete_dialog_cleanup, delete_dialog_complete, NULL))
		abort();
}

typedef struct XwRegisterProtectState {
	Input* dialog;
	int16_t hadKeyButtons;
	int16_t exitCode;
	DialogSubResultHandler complete;
	void* context;
} XwRegisterProtectState;

static XwRegisterProtectState protect_state;

static void protect_dialog_cleanup(Input* dialog) {
	protect_state.exitCode = xdialog_Get_Dialog_Exit();
	xdialog_Clear_Dialog_Exit();
	xinput_Free_Inputs(dialog);
	if (protect_state.hadKeyButtons == 0)
		xio_Clear_Key_Buttons();
	protect_state.dialog = NULL;
	if (g_quitRequested) {
		protect_state.complete = NULL;
		protect_state.context = NULL;
	}
}

static void protect_dialog_complete(int16_t unusedResult, void* unusedContext) {
	DialogSubResultHandler complete = protect_state.complete;
	void* context = protect_state.context;
	int16_t exitCode = protect_state.exitCode;
	(void)unusedResult;
	(void)unusedContext;
	protect_state.complete = NULL;
	protect_state.context = NULL;
	if (complete != NULL)
		complete(exitCode != REGISTER_PROTECT_CANCEL_INPUT, context);
}

void XwRegister_ScheduleProtectDialog(Input* dialog, int16_t hadKeyButtons, DialogSubResultHandler complete,
									  void* context) {
	if (protect_state.dialog != NULL)
		abort();
	protect_state.dialog = dialog;
	protect_state.hadKeyButtons = hadKeyButtons;
	protect_state.complete = complete;
	protect_state.context = context;
	/* Input callbacks defer the push until the owning callback batch has returned. */
	if (landru_task_depth() >= LANDRU_TASK_STACK_DEPTH) {
		protect_dialog_cleanup(dialog);
		abort();
	}
	if (!xdialog_Schedule_Owned_Sub_Dialog(dialog, protect_dialog_cleanup, protect_dialog_complete, NULL))
		abort();
}

void XwRegister_CompletePilotDeletion(int16_t result, void* context) {
	char pilotFilename[REGISTER_PILOT_BUTTON_PATH_CAPACITY];
	(void)context;
	if (result == REGISTER_DELETE_CANCEL)
		return;
	if (result == REGISTER_DELETE_CONFIRM) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotFilename);
		strcat(pilotFilename, ".PLT");
		XwStorage_Remove(pilotFilename);
		register_Set_Reg_String_Button_Name(g_RegisterPilotNameInput, g_sharedEmptyString);
		register_Delete_Pilot_Record();
		xview_Refresh_View();
	} else {
		register_Revive_Pilot_Record();
		xview_Refresh_View();
	}
}

typedef struct XwRegisterAcceptState {
	Input* input;
	int context;
	int started;
} XwRegisterAcceptState;

static LandruTaskStepResult step_accept_pilot(void* self) {
	XwRegisterAcceptState* state = self;
	char pilotFileName[REGISTER_PILOT_PATH_CAPACITY];
	if (!state->started) {
		state->started = 1;
		register_PlaySpeech(REGISTER_SPEECH_WAIT, state->context);
		return LANDRU_TASK_STEP_CONTINUE;
	}
	xerror_Set_Landru_Exit(state->input->var2);
	if (g_RegisterActivePilot == REGISTER_PILOT_SLOT_NONE) {
		register_Get_Reg_String_Button_Name(g_RegisterPilotNameInput, pilotFileName);
		strcpy(g_RegisterShellPilot.name, pilotFileName);
		g_RegisterShellPilot.field_18 = 0;
		g_RegisterShellPilot.deleted = 0;
		g_RegisterShellPilot.lost_status = 0;
		g_RegisterShellPilot.rank = 0;
		g_RegisterShellPilot.current_tour = 0;
		g_RegisterShellPilot.field_1D = 0;
		g_RegisterShellPilot.score = 0;
		strcat(pilotFileName, ".PLT");
		shipext_Revive_Pilot(pilotFileName, SHIPEXT_CREATE_PILOT);
	} else {
		register_Index_To_Pilot_Record(g_RegisterActivePilot, &g_RegisterShellPilot);
	}
	shipext_ResetMissionSelections();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable accept_pilot_vtable = { step_accept_pilot, NULL, NULL, NULL };

void XwRegister_ScheduleAcceptPilot(Input* input, int context) {
	XwRegisterAcceptState* state = landru_task_push(&accept_pilot_vtable);
	if (state == NULL)
		abort();
	state->input = input;
	state->context = context;
	state->started = 0;
}

void XwRegister_CompleteProtection(int16_t result, void* context) {
	(void)context;
	if (result == 0)
		xerror_Set_Landru_Exit(0);
	xio_Set_Mouse_Position(REGISTER_PROTECT_RETURN_MOUSE_X, REGISTER_PROTECT_RETURN_MOUSE_Y);
	register_end_View(XW_REGISTER_VIEW_RESUME_PROTECTION);
}
