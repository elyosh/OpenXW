#include "xw_runtime/runtime/shell_test_task.h"

#include "xw/frontend/shell_test.h"
#include "xw/frontend/shellext.h"
#include "xw/landru_config.h"

#include <landru/dlg.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

static bool choice_pending;
static bool choice_ready;
static int16_t selected_choice;

static void scene_choice_complete(int16_t result, void* context) {
	(void)context;
	choice_pending = false;
	choice_ready = true;
	selected_choice = result - 1;
	ShellTest_UpdateSceneSelection(0);
}

int XwShellTest_ReadSceneChoice(const char* labels, int16_t* choice) {
	if (choice_ready) {
		*choice = selected_choice;
		choice_ready = false;
		return 1;
	}
	if (!choice_pending) {
		choice_pending = xdlg_Schedule_Choices(labels, scene_choice_complete, NULL);
	}
	return 0;
}

static bool outcome_choice_pending;
static bool outcome_choice_ready;
static int16_t selected_outcome_choice;

static void outcome_choice_complete(int16_t result, void* context) {
	(void)context;
	outcome_choice_pending = false;
	outcome_choice_ready = true;
	selected_outcome_choice = result - 1;
	ShellTest_UpdateMissionOutcome(0);
}

int XwShellTest_ReadOutcomeChoice(const char* labels, int16_t* choice) {
	if (outcome_choice_ready) {
		*choice = selected_outcome_choice;
		outcome_choice_ready = false;
		return 1;
	}
	if (!outcome_choice_pending) {
		outcome_choice_pending = xdlg_Schedule_Choices(labels, outcome_choice_complete, NULL);
	}
	return 0;
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	xview_Clear_View_Update_Function();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, NULL, NULL, NULL };

void XwShellTest_RunView(void) {
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	if (landru_task_push(&finish_view_vtable) == NULL)
		abort();
	j_xviewadd_Handle_View();
}
