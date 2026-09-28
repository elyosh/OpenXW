#include "xw_runtime/runtime/title_task.h"
#include "xw_runtime/audio/music_policy.h"

#include "xw/audio/soundext.h"

#include <landru/task.h>
#include <stdlib.h>

typedef struct XwTitleMusicWait {
	Sound* sound;
} XwTitleMusicWait;

static LandruTaskStepResult step_music_wait(void* self) {
	if (!XwMusicPolicy_UsesImuse())
		return LANDRU_TASK_STEP_DONE;
	XwTitleMusicWait* wait = self;
	return soundext_Count_Resource_Instances(wait->sound) == 1 ? LANDRU_TASK_STEP_YIELD
															   : LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable music_wait_vtable = { step_music_wait, NULL, NULL, NULL };

void XwTitle_WaitForMusic(Sound* sound) {
	if (!XwMusicPolicy_UsesImuse())
		return;
	if (soundext_Count_Resource_Instances(sound) != 1)
		return;
	XwTitleMusicWait* wait = landru_task_push(&music_wait_vtable);
	if (wait == NULL)
		abort();
	wait->sound = sound;
}
