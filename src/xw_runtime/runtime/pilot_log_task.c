#include "xw_runtime/runtime/pilot_log_task.h"
#include "xw_dos94/frontend/pilot_log.h"
#include "xw_runtime/runtime/profile.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/pilot_log.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"

#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/task.h>
#include <landru/view.h>
#include <stdlib.h>

typedef struct XwPilotLogView {
	ResFile* resourceFile;
} XwPilotLogView;

static void finish_view_end(void* self) {
	XwPilotLogView* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	if (XwProfile_DosFrontend())
		Dos94_PilotLog_CloseMusic();
	else
		PilotLog_CloseMusic();
	soundext_RecheckSfxPreference();
	xcursor_Hide_Cursor();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwPilotLog_RunView(ResFile* resourceFile) {
	XwPilotLogView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
