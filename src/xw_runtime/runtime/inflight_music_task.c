#include "xw_runtime/runtime/inflight_music_task.h"
#include "xw_runtime/audio/music_policy.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/inflight_ui.h"

#include <landru/task.h>
#include <stdlib.h>

typedef struct XwInflightMusicWait {
	int unusedSceneContext;
} XwInflightMusicWait;

static LandruTaskStepResult step_music_wait(void* self) {
	XwInflightMusicWait* wait = self;
	int previousGroup;
	unsigned int previousBeat;
	int previousTick;
	const char* musicName = "patrol";
	int unusedSceneContext = wait->unusedSceneContext;
	if (XwMusicPolicy_UsesImuse()) {
		if (soundext_GetMusicParam(g_inflightMusicState.music, XW_SOUND_QUERY_TICK, 0) == 0)
			return LANDRU_TASK_STEP_YIELD;
		j_lolevel_ImPause();
		previousGroup = soundext_GetMusicParam(g_inflightMusicState.previousMusic, XW_SOUND_QUERY_CHUNK, 0);
		previousBeat = soundext_GetMusicParam(g_inflightMusicState.previousMusic, XW_SOUND_QUERY_BEAT, 0);
		previousTick = soundext_GetMusicParam(g_inflightMusicState.previousMusic, XW_SOUND_QUERY_TICK, 0);
		j_lolevel_ImResume();
		soundext_ScanMidi(g_inflightMusicState.music, previousGroup, previousBeat, previousTick);
		soundext_JumpMidi(g_inflightMusicState.previousMusic, INFLIGHT_MUSIC_PREVIOUS_GROUP, 0, 0);
	}
	g_inflightMusicState.unusedSceneContext = unusedSceneContext;
	g_inflightMusicState.music = xsound_Find_Gmid(musicName);
	g_inflightMusicState.legacyLevel = INFLIGHT_MUSIC_INITIAL_LEVEL;
	if (g_inflightMusicState.music == NULL) {
		ResFile* musicResource = xres_Open_Resource("ifmusic.lfd");
		g_inflightMusicState.music = xsound_Res_Music(musicResource, musicName);
		xres_Close_Resource(musicResource);
		soundext_Start_Resource_Sound(g_inflightMusicState.music);
		soundext_FadeVolume(g_inflightMusicState.music, INFLIGHT_MUSIC_OPEN_VOLUME,
							INFLIGHT_MUSIC_OPEN_DURATION);
	}
	xsound_Set_Sound_Keep(g_inflightMusicState.music);
	if (g_inflightMusicState.previousMusic != NULL)
		xsound_Set_Sound_Keep(g_inflightMusicState.previousMusic);
	xsound_Set_Sound_User_Function(g_inflightMusicState.music, InflightUI_user_Music);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable music_wait_vtable = { step_music_wait, NULL, NULL, NULL };

int XwInflightMusic_WaitForTick(int unusedSceneContext) {
	XwInflightMusicWait* wait;
	if (!XwMusicPolicy_UsesImuse())
		return 0;
	if (soundext_GetMusicParam(g_inflightMusicState.music, XW_SOUND_QUERY_TICK, 0) != 0)
		return 0;
	wait = landru_task_push(&music_wait_vtable);
	if (wait == NULL)
		abort();
	wait->unusedSceneContext = unusedSceneContext;
	return 1;
}
