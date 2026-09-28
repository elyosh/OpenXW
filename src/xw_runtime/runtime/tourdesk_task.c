#include "xw_runtime/runtime/tourdesk_task.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/tourdesk.h"

#include <landru/task.h>
#include <stdlib.h>

static LandruTaskStepResult step_speech_wait(void* self) {
	(void)self;
	return soundext_Count_Resource_Instances(g_tourDeskSpeech[XW_TOURDESK_SPEECH_JOIN]) == 1
			   ? LANDRU_TASK_STEP_YIELD
			   : LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable speech_wait_vtable = { step_speech_wait, NULL, NULL, NULL };

void XwTourDesk_WaitForSpeech(void) {
	if (soundext_Count_Resource_Instances(g_tourDeskSpeech[XW_TOURDESK_SPEECH_JOIN]) != 1)
		return;
	if (landru_task_push(&speech_wait_vtable) == NULL)
		abort();
}
