#include "xw_runtime/runtime/brief_music_task.h"
#include "xw_runtime/audio/music_policy.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/brief.h"

#include <landru/task.h>
#include <stdlib.h>

typedef struct XwBriefMusicWait {
	Film* film;
} XwBriefMusicWait;

static LandruTaskStepResult step_music_wait(void* self) {
	XwBriefMusicWait* wait = self;
	int previousGroup;
	unsigned int previousBeat;
	int previousTick;
	const char* musicName = "patrol";
	Film* film = wait->film;
	if (XwMusicPolicy_UsesImuse()) {
		if (soundext_GetMusicParam(g_briefMusicState.music, XW_SOUND_QUERY_TICK, 0) == 0)
			return LANDRU_TASK_STEP_YIELD;
		j_lolevel_ImPause();
		previousGroup = soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_CHUNK, 0);
		previousBeat = soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_BEAT, 0);
		previousTick = soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_TICK, 0);
		j_lolevel_ImResume();
		soundext_ScanMidi(g_briefMusicState.music, previousGroup, previousBeat, previousTick);
		soundext_JumpMidi(g_briefMusicState.previousMusic, BRIEF_MUSIC_PREVIOUS_GROUP, 0, 0);
	}
	g_briefMusicState.film = film;
	g_briefMusicState.music = xsound_Find_Gmid(musicName);
	g_briefMusicState.legacyLevel = BRIEF_MUSIC_INITIAL_LEVEL;
	if (g_briefMusicState.music == NULL) {
		ResFile* musicResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\bfmusic.lfd");
		if (musicResource == NULL)
			musicResource = xres_Open_Resource("bfmusic.lfd");
		g_briefMusicState.music = xsound_Res_Music(musicResource, musicName);
		xres_Close_Resource(musicResource);
		soundext_Start_Resource_Sound(g_briefMusicState.music);
		soundext_FadeVolume(g_briefMusicState.music, BRIEF_MUSIC_OPEN_VOLUME, BRIEF_MUSIC_OPEN_DURATION);
	}
	xsound_Set_Sound_Keep(g_briefMusicState.music);
	if (g_briefMusicState.previousMusic != NULL)
		xsound_Set_Sound_Keep(g_briefMusicState.previousMusic);
	xsound_Set_Sound_User_Function(g_briefMusicState.music, brief_user_Music);
	return LANDRU_TASK_STEP_DONE;
}

static const LandruTaskVtable music_wait_vtable = { step_music_wait, NULL, NULL, NULL };

int XwBriefMusic_WaitForTick(Film* film) {
	XwBriefMusicWait* wait;
	if (!XwMusicPolicy_UsesImuse())
		return 0;
	if (soundext_GetMusicParam(g_briefMusicState.music, XW_SOUND_QUERY_TICK, 0) != 0)
		return 0;
	wait = landru_task_push(&music_wait_vtable);
	if (wait == NULL)
		abort();
	wait->film = film;
	return 1;
}
