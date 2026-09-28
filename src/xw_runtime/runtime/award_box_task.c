#include "xw_runtime/runtime/award_box_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/award_box.h"
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

typedef struct XwAwardBoxView {
	ResFile* resourceFile;
} XwAwardBoxView;

static void finish_view_end(void* self) {
	XwAwardBoxView* state = self;
	xview_Clear_View_Update_Function();
	xres_Close_Resource(state->resourceFile);
}

static LandruTaskStepResult finish_view(void* self) {
	(void)self;
	AwardBox_CloseMusic();
	soundext_RecheckSfxPreference();
	xcursor_Hide_Cursor();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable finish_view_vtable = { finish_view, finish_view_end, NULL, NULL };

void XwAwardBox_RunView(ResFile* resourceFile) {
	XwAwardBoxView* state;
	if (landru_task_depth() > LANDRU_TASK_STACK_DEPTH - 2)
		abort();
	state = landru_task_push(&finish_view_vtable);
	if (state == NULL)
		abort();
	state->resourceFile = resourceFile;
	j_xviewadd_Handle_View();
}
