#include "xw_runtime/runtime/filmview_task.h"
#include "xw/flight/flight.h"
#include "xw_dos94/frontend/filmview.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/assets/file.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/filmview.h"
#include "xw/frontend/shellext.h"

#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>
#include <string.h>

typedef struct XwFilmViewFileState {
	FILMVIEW_FileDialog dialog;
	int16_t hadKeyButtons;
	int16_t exitCode;
	bool active;
	DialogSubResultHandler complete;
	void* context;
} XwFilmViewFileState;

static XwFilmViewFileState file_state;
static bool file_result_ready;
static int16_t file_result;

static void file_scene_complete(int16_t accepted, void* unusedContext) {
	(void)unusedContext;
	file_result = accepted;
	file_result_ready = true;
	/* Resume before the parent view draws or processes another callback batch. */
	if (XwProfile_DosFrontend())
		Dos94_filmview_end_View(0);
	else
		filmview_end_View(0);
}

int16_t XwFilmView_HandleFileDialog(void) {
	if (file_result_ready) {
		file_result_ready = false;
		return file_result;
	}
	if (!file_state.active) {
		if (XwProfile_DosFrontend())
			Dos94_filmview_Do_FV_File_Dialog(file_scene_complete, NULL);
		else
			filmview_Do_FV_File_Dialog(file_scene_complete, NULL);
	}
	return XW_FILMVIEW_DIALOG_PENDING;
}

static void file_dialog_cleanup(Input* root) {
	file_result_ready = false;
	file_state.exitCode = xdialog_Get_Dialog_Exit();
	xdialog_Clear_Dialog_Exit();
	xfiledir_Free_Directory(&file_state.dialog.directory);
	xinput_Free_Inputs(root);
	if (file_state.hadKeyButtons == 0)
		xio_Clear_Key_Buttons();
	file_state.dialog.root = NULL;
	file_state.active = false;
	if (g_quitRequested) {
		file_state.complete = NULL;
		file_state.context = NULL;
	}
}

static void file_dialog_complete(int16_t unusedResult, void* unusedContext) {
	DialogSubResultHandler complete = file_state.complete;
	void* context = file_state.context;
	int16_t accepted = file_state.exitCode == FILMVIEW_ACCEPT_FILE;
	(void)unusedResult;
	(void)unusedContext;
	file_state.complete = NULL;
	file_state.context = NULL;
	if (complete != NULL)
		complete(accepted, context);
}

FILMVIEW_FileDialog* XwFilmView_BeginFileDialog(int16_t hadKeyButtons, DialogSubResultHandler complete,
												void* context) {
	if (file_state.active)
		abort();
	memset(&file_state, 0, sizeof(file_state));
	file_state.active = true;
	file_state.hadKeyButtons = hadKeyButtons;
	file_state.complete = complete;
	file_state.context = context;
	return &file_state.dialog;
}

void XwFilmView_ScheduleFileDialog(int16_t built) {
	if (!file_state.active)
		abort();
	if (built == 0) {
		if (file_state.hadKeyButtons == 0)
			xio_Clear_Key_Buttons();
		file_state.active = false;
		file_state.exitCode = 0;
		file_dialog_complete(0, NULL);
		return;
	}
	if (landru_task_depth() >= LANDRU_TASK_STACK_DEPTH) {
		file_dialog_cleanup(file_state.dialog.root);
		abort();
	}
	if (!xdialog_Schedule_Owned_Sub_Dialog(file_state.dialog.root, file_dialog_cleanup, file_dialog_complete,
										   NULL))
		abort();
}

typedef struct XwFilmViewDeleteState {
	Input* dialog;
	int16_t hadKeyButtons;
	int16_t exitCode;
	DialogSubResultHandler complete;
	void* context;
} XwFilmViewDeleteState;

static XwFilmViewDeleteState delete_state;

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
	int16_t accepted = delete_state.exitCode != FILMVIEW_DELETE_CANCEL;
	(void)unusedResult;
	(void)unusedContext;
	delete_state.complete = NULL;
	delete_state.context = NULL;
	if (complete != NULL)
		complete(accepted, context);
}

void XwFilmView_ScheduleDeleteDialog(Input* dialog, int16_t hadKeyButtons, DialogSubResultHandler complete,
									 void* context) {
	if (delete_state.dialog != NULL)
		abort();
	delete_state.dialog = dialog;
	delete_state.hadKeyButtons = hadKeyButtons;
	delete_state.complete = complete;
	delete_state.context = context;
	/* The owning callback batch cannot push tasks before draining this request. */
	if (landru_task_depth() >= LANDRU_TASK_STACK_DEPTH) {
		delete_dialog_cleanup(dialog);
		abort();
	}
	if (!xdialog_Schedule_Owned_Sub_Dialog(dialog, delete_dialog_cleanup, delete_dialog_complete, NULL))
		abort();
}

void XwFilmView_CompleteFileDeletion(int16_t accepted, void* context) {
	Input* input = context;
	FILMVIEW_FileDialog* dialog;
	DirEntry* entries;
	int16_t selectedIndex;
	char clipFilename[FILMVIEW_CLIP_PATH_CAPACITY];
	if (accepted == 0)
		return;
	dialog = input->varptr;
	entries = xmemhdl_Lock_Handle(dialog->directory.entries);
	selectedIndex = dialog->selected_file;
	if (selectedIndex < dialog->directory.count) {
		int16_t index;
		strcpy(clipFilename, g_filmviewSelectedName);
		strcat(clipFilename, ".clp");
		XwStorage_Remove(clipFilename);
		for (index = selectedIndex; index < dialog->directory.count - 1; ++index)
			entries[index] = entries[index + 1];
		--dialog->directory.count;
		if (dialog->selected_file >= dialog->directory.count && dialog->selected_file != 0)
			--dialog->selected_file;
		g_filmviewSelectedName[0] = '\0';
		dialog->hit_count = 0;
		if (dialog->directory.count != 0) {
			filmview_Set_Active_FV_File(dialog, 0, 1);
			dialog->hit_count = 0;
		} else {
			if (g_filmviewLoadInput != NULL)
				xinpattr_Hide_Input(g_filmviewLoadInput);
			if (g_filmviewDeleteInput != NULL)
				xinpattr_Hide_Input(g_filmviewDeleteInput);
		}
		xview_Refresh_View();
	}
}

typedef struct XwFilmViewScene {
	ResFile* resourceFile;
} XwFilmViewScene;

static void finish_scene_view_end(void* self) {
	XwFilmViewScene* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_scene_view(void* self) {
	(void)self;
	filmview_CloseMusic();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_scene_view_vtable = { finish_scene_view, finish_scene_view_end, NULL,
														   NULL };

void XwFilmView_RunView(ResFile* resourceFile) {
	XwFilmViewScene* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_scene_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
