#include "xw/frontend/brief.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_adapter.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/storage.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/lolevel.h"
#include "xw/audio/sound.h"
#include "xw/audio/soundext.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/shell_flight.h"
#include "xw/frontend/player.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/brief_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/brief_music_task.h"
#include "xw_runtime/runtime/brief_view_task.h"
#endif

#include <ctype.h>
#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/file.h>
#include <landru/font.h>
#include <landru/fourcc.h>
#include <landru/inpattr.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/paint.h>
#include <landru/paragrp.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4D0E08
XwOfficerAnimationStep g_briefingDodonnaAnimationSteps[XW_BRIEFING_DODONNA_FRAME_COUNT] = {
	{ 0, 2 },  { 1, 3 },  { 0, 5 },  { 0, 7 },  { 0, 7 },  { 0, 7 },  { 1, 8 },  { 0, 11 },
	{ 0, 11 }, { 0, 11 }, { 1, 12 }, { 0, 15 }, { 0, 15 }, { 0, 15 }, { 1, 16 }, { 0, 17 },
	{ 1, 18 }, { 0, 19 }, { 1, 20 }, { 0, 23 }, { 0, 23 }, { 0, 23 }, { 0, 24 }, { 0, 25 },
	{ 1, 26 }, { 0, 27 }, { 1, 28 }, { 0, 29 }, { 1, 30 }, { 0, 31 }, { 1, 32 }, { 0, 33 },
	{ 1, 34 }, { 0, 35 }, { 1, 36 }, { 0, 37 }, { 1, 38 }, { 0, 39 }, { 1, 40 }, { 0, 41 },
	{ 1, 42 }, { 0, 43 }, { 1, 44 }, { 0, 45 }, { 1, 46 }, { 0, 47 }, { 1, 48 }, { 0, 51 },
	{ 0, 51 }, { 0, 51 }, { 1, 52 }, { 0, 54 }, { 0, 54 }, { 1, 55 }, { 0, 1 }
};

// GLOBAL: XW 0x4D0EE8
XwOfficerAnimationStep g_briefingAckbarAnimationSteps[XW_BRIEFING_ACKBAR_FRAME_COUNT] = {
	{ 0, 2 },  { 1, 3 },  { 0, 4 },  { 1, 5 },  { 0, 8 },  { 0, 8 },  { 0, 8 },  { 1, 9 },  { 0, 11 },
	{ 1, 11 }, { 1, 12 }, { 0, 13 }, { 0, 15 }, { 0, 15 }, { 0, 16 }, { 1, 17 }, { 0, 19 }, { 0, 19 },
	{ 1, 20 }, { 0, 21 }, { 1, 22 }, { 0, 24 }, { 0, 24 }, { 1, 25 }, { 0, 27 }, { 0, 27 }, { 0, 28 },
	{ 1, 29 }, { 0, 30 }, { 1, 32 }, { 1, 32 }, { 0, 33 }, { 1, 34 }, { 0, 35 }, { 1, 36 }, { 0, 37 },
	{ 1, 39 }, { 1, 39 }, { 0, 41 }, { 0, 41 }, { 1, 42 }, { 0, 43 }, { 1, 44 }, { 0, 45 }, { 0, 1 }
};

// GLOBAL: XW 0x4D0FA0
int16_t g_briefingScriptOpcodeArgCounts[XW_BRIEF_COMMAND_COUNT] = {
	0, 0, 1, 0, 4, 0, 4, 4, 4, 4, 0, 1, 1, 1, 1, 2, 2, 1, 1, 0, 0,
	0, 1, 1, 1, 1, 0, 3, 3, 3, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// GLOBAL: XW 0x4D1038
char g_briefingFilmNames[XW_BRIEFING_FILM_COUNT][XW_BRIEFING_RESOURCE_NAME_CAPACITY] = { "briefdf4",
																						 "briefaf4" };

// GLOBAL: XW 0x4D11A8
char g_briefingIconResourceNames[XW_BRIEFING_MAP_ICON_ACTOR_COUNT][XW_BRIEFING_RESOURCE_NAME_CAPACITY] = {
	"iconsg64", "iconsr64", "iconsb64"
};

// GLOBAL: XW 0x4D12C8
int16_t g_briefingMapDefaultIconColorGroups[XW_BRIEFING_MAP_ICON_BASE_COUNT] = { 0, 0, 0, 1, 1, 1, 1, 2, 2,
																				 2, 2, 2, 0, 0, 2, 1, 1, 1,
																				 1, 1, 1, 1, 1, 1, 0 };

// GLOBAL: XW 0x4D13A0
char g_briefingSelectionTypeLabels[XW_BRIEFING_PREVIEW_TYPE_COUNT][XW_BRIEFING_TYPE_LABEL_SIZE] = {
	"X-W", "Y-W", "A-W", "T/F", "T/I", "T/B", "GUN", "TRN", "SHU", "TUG", "CON",
	"FRT", "CRS", "FRG", "CRV", "STD", "T/A", "MIN", "COM", "NAV", "PRB", "B-W"
};

// GLOBAL: XW 0x4D13F8
int16_t g_briefingSelectionTextColors[XW_BRIEFING_SELECTION_COLOR_COUNT] = { 62, 54, 50 };

// GLOBAL: XW 0x4D1400
int16_t g_briefingFocusX[XW_BRIEFING_FOCUS_CAPACITY] = { 4, 118, 156, 200, 316, 4, 104, 156, 210, 316, 0, 0 };

// GLOBAL: XW 0x4D1418
int16_t g_briefingFocusY[XW_BRIEFING_FOCUS_CAPACITY] = { 104, 157, 157, 157, 104, 104,
														 185, 185, 185, 104, 0,   0 };

// GLOBAL: XW 0x4D1430
int16_t g_briefingMissionChoiceFocusX[XW_BRIEFING_CHOICE_FOCUS_CAPACITY] = { 4,   118, 156, 200, 316, 4,
																			 128, 128, 180, 316, 4,   104,
																			 156, 210, 316, 0 };

// GLOBAL: XW 0x4D1450
int16_t g_briefingMissionChoiceFocusY[XW_BRIEFING_CHOICE_FOCUS_CAPACITY] = { 104, 157, 157, 157, 104, 104,
																			 171, 171, 171, 104, 104, 185,
																			 185, 185, 104, 0 };

// GLOBAL: XW 0x4D1470
int16_t g_localPilotAssignmentToken = XW_LOCAL_PILOT_ASSIGNMENT_TOKEN;

// GLOBAL: XW 0x4F4B48
XwBriefingMusicState g_briefMusicState = { 0 };

// GLOBAL: XW 0x4F4B58
int16_t g_briefingTextSoundActive = 0;

// GLOBAL: XW 0x4F4B5C
int16_t g_briefingTextSoundRequested = 0;

// GLOBAL: XW 0x4F5828
uint16_t g_launchGroupInitialStatus[XW_LAUNCH_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F5848
int16_t g_launchSelectedGroupIndex = 0;

// GLOBAL: XW 0x4F584C
Actor* g_briefingRewindButtonActor = NULL;

// GLOBAL: XW 0x4F5850
int16_t g_launchCraftPilotTokens[XW_LAUNCH_GROUP_CAPACITY][XW_LAUNCH_CRAFTS_PER_GROUP] = { { 0 } };

// GLOBAL: XW 0x4F5910
Actor* g_briefingAckbarActor = NULL;

// GLOBAL: XW 0x4F5924
Actor* g_briefingPlayButtonActor = NULL;

// GLOBAL: XW 0x4F5928
uint16_t g_launchGroupCraftCounts[XW_LAUNCH_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F5948
Actor* g_briefingRightDoorActor = NULL;

// GLOBAL: XW 0x4F5950
Actor* g_briefingDodonnaActor = NULL;

// GLOBAL: XW 0x4F595C
int16_t g_briefingHasMissionChoice = 0;

// GLOBAL: XW 0x4F5960
int16_t g_briefingRequestedNarrationPage = 0;

// GLOBAL: XW 0x4F5968
uint16_t g_launchGroupMissionIndices[XW_LAUNCH_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F5988
Actor* g_briefingLogButtonActor = NULL;

// GLOBAL: XW 0x4F5990
char g_launchGroupNames[XW_LAUNCH_GROUP_CAPACITY][XW_LAUNCH_GROUP_NAME_CAPACITY] = { { 0 } };

// GLOBAL: XW 0x4F5A90
Input* g_briefingRightDoorInput = NULL;

// GLOBAL: XW 0x4F5A94
Actor* g_briefingStarsActor = NULL;

// GLOBAL: XW 0x4F5A98
Input* g_briefingMapInput = NULL;

// GLOBAL: XW 0x4F5A9C
Sound* g_briefingNarrationSound = NULL;

// GLOBAL: XW 0x4F5AA0
Film* g_briefingFilm = NULL;

// GLOBAL: XW 0x4F5AA4
int16_t g_briefingMissionChoice = 0;

// GLOBAL: XW 0x4F5AA8
int16_t g_launchGroupCount = 0;

// GLOBAL: XW 0x4F5AB0
Rect g_briefingOfficerRect = { 0 };

// GLOBAL: XW 0x4F5AB8
Actor* g_briefingInteriorActor = NULL;

// GLOBAL: XW 0x4F5ABC
Actor* g_briefingMedalsButtonActor = NULL;

// GLOBAL: XW 0x4F5AC0
LandruHandle g_briefingRuntimeHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F5AC4
Actor* g_briefingNextPageButtonActor = NULL;

// GLOBAL: XW 0x4F5AC8
Input* g_briefingWorldInput = NULL;

// GLOBAL: XW 0x4F5ACC
Input* g_briefingDoorHintInput = NULL;

// GLOBAL: XW 0x4F5AD0
int16_t g_briefingOfficerRegionRestored = 0;

// GLOBAL: XW 0x4F5AD8
uint16_t g_launchGroupFormations[XW_LAUNCH_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F5AF8
int16_t g_launchAssignedPilotCount = 0;

// GLOBAL: XW 0x4F5AFC
Actor* g_briefingLeftDoorActor = NULL;

// GLOBAL: XW 0x4F5B00
int16_t g_briefingLoadedNarrationPage = 0;

// GLOBAL: XW 0x4F5B04
Actor* g_briefingBwingRadarActor = NULL;

// GLOBAL: XW 0x4F5B08
Actor* g_briefingPreviousPageButtonActor = NULL;

// GLOBAL: XW 0x4F5B0C
Input* g_briefingHintCompanionInput = NULL;

// GLOBAL: XW 0x4F5B10
LandruHandle g_briefingOfficerRegionBuffer = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F5B14
Input* g_briefingLeftDoorInput = NULL;

// GLOBAL: XW 0x4F5B18
Actor* g_briefingMapIconActors[XW_BRIEFING_MAP_ICON_ACTOR_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F5B24
Actor* g_briefingTextActor = NULL;

// GLOBAL: XW 0x4F5B28
BriefingRuntimeState* g_briefingRuntime = NULL;

// GLOBAL: XW 0x4F5B30
REGISTER_PilotFileRecord g_briefingPilotRecord = { 0 };

// GLOBAL: XW 0x4F61E0
XwCraftSpecies g_launchGroupCraftTypes[XW_LAUNCH_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F6200
Actor* g_briefingSelectionPreviewActor = NULL;

// GLOBAL: XW 0x4F6204
Actor* g_briefingBackgroundActor = NULL;

// GLOBAL: XW 0x4F6208
Actor* g_briefingStopButtonActor = NULL;

// GLOBAL: XW 0x4F620C
Actor* g_briefingOfficerRestoreActor = NULL;

// GLOBAL: XW 0x4F6210
uint16_t g_launchGroupPlayerCraftOrdinals[XW_LAUNCH_GROUP_CAPACITY] = { 0 };

// GLOBAL: XW 0x4F6230
int16_t g_briefingSelectedIconPlusOne = 0;

// GLOBAL: XW 0x4F6240
int16_t g_briefingFocusIndex = 0;

// GLOBAL: XW 0x4F6244
int16_t g_launchSelectedCraftIndex = 0;

// FUNCTION: XW 0x433E20
void brief_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		const char* musicName;
		g_briefMusicState.previousMusic = NULL;
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_BRIEFING_COMBAT: {
				ResFile* transitionResource;
				musicName = "patrol";
				transitionResource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\bfmusic.lfd");
				if (transitionResource == NULL)
					transitionResource = xres_Open_Resource("bfmusic.lfd");
				g_briefMusicState.music = xsound_Res_Music(transitionResource, "patrol");
				xres_Close_Resource(transitionResource);
				g_briefMusicState.previousMusic = xsound_Find_Gmid("adrift");
				if (g_briefMusicState.previousMusic != NULL) {
#ifdef XW_MODERN
					if (XwMusicPolicy_UsesImuse()) {
#endif
						Sound* currentMusic;
						Sound* previousMusic;
						int previousGroup;
						unsigned int previousBeat;
						int previousTick;
#ifndef XW_MODERN
						int currentTick;
#endif
						soundext_Start_Resource_Sound(g_briefMusicState.music);
#ifdef XW_MODERN
						Dos94_soundext_SetVolume(g_briefMusicState.music, BRIEF_MUSIC_OPEN_VOLUME);
#else
					soundext_SetVolume(0, BRIEF_MUSIC_OPEN_VOLUME);
#endif
						previousMusic = g_briefMusicState.previousMusic;
						currentMusic = g_briefMusicState.music;
						soundext_ShareParts(previousMusic, currentMusic);
#ifdef XW_MODERN
						if (XwBriefMusic_WaitForTick(film))
							return;
#else
					do {
						currentTick = soundext_GetMusicParam(g_briefMusicState.music, XW_SOUND_QUERY_TICK, 0);
					} while (currentTick == 0);
#endif
						j_lolevel_ImPause();
						previousGroup =
							soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_CHUNK, 0);
						previousBeat =
							soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_BEAT, 0);
						previousTick =
							soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_TICK, 0);
						j_lolevel_ImResume();
						soundext_ScanMidi(g_briefMusicState.music, previousGroup, previousBeat, previousTick);
						soundext_JumpMidi(g_briefMusicState.previousMusic, BRIEF_MUSIC_PREVIOUS_GROUP, 0, 0);
#ifdef XW_MODERN
					}
#endif
				} else {
					g_briefMusicState.previousMusic = xsound_Find_Gmid("mission");
					if (g_briefMusicState.previousMusic != NULL) {
						soundext_ClearTriggers();
						soundext_SetPartEnabled(g_briefMusicState.previousMusic, BRIEF_MUSIC_MISSION_CHANNEL,
												0);
						soundext_SetTriggerContext(g_briefMusicState.previousMusic,
												   BRIEF_MUSIC_TRANSITION_MARKER);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, g_briefMusicState.music->id, 0,
													 0, 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_SET_VOLUME, g_briefMusicState.music->id,
													 BRIEF_MUSIC_OPEN_VOLUME, 0, 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
						soundext_SetTriggerContext(g_briefMusicState.previousMusic,
												   BRIEF_MUSIC_TRANSITION_MARKER);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_STOP,
													 g_briefMusicState.previousMusic->id, 0, 0, 0, 0, 0);
						soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
					}
				}
				break;
			}
			case XW_SCENE_BRIEFING_TOUR:
				musicName = "wrkmarch";
				break;
			default:
				musicName = "patrol";
				break;
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
	}
}

// FUNCTION: XW 0x434120
void brief_CloseMusic(void) {
	int exitScene;
	if (ShellPreferences_GetMusicEnabled() == 0) {
		return;
	}
	soundext_ClearTriggers();
	exitScene = xerror_Get_Landru_Exit();
	if (exitScene == XW_SCENE_COMBAT_SIMULATOR_ROOM && g_briefMusicState.previousMusic != NULL &&
		xsound_Find_Gmid("mission") == g_briefMusicState.previousMusic &&
		soundext_Count_Resource_Instances(g_briefMusicState.previousMusic) == 1 &&
		soundext_Count_Resource_Instances(g_briefMusicState.music) != 1) {
		return;
	}
	if (g_briefMusicState.music != NULL) {
		xsound_Clear_Sound_Keep(g_briefMusicState.music);
		xsound_Free_Sound(g_briefMusicState.music);
	}
	if (g_briefMusicState.previousMusic != NULL) {
		xsound_Clear_Sound_Keep(g_briefMusicState.previousMusic);
		xsound_Free_Sound(g_briefMusicState.previousMusic);
	}
}

// FUNCTION: XW 0x4341D0
void brief_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	if (g_briefMusicState.previousMusic == NULL) {
		return;
	}
	if (shellext_Get_Cur_Scene() != XW_SCENE_BRIEFING_COMBAT) {
		return;
	}
	if (soundext_Count_Resource_Instances(g_briefMusicState.previousMusic) == 1) {
		int musicPosition = soundext_GetMusicParam(g_briefMusicState.previousMusic, XW_SOUND_QUERY_BEAT, 0);
		if (musicPosition > XW_BRIEF_MUSIC_FADE_POSITION &&
			g_briefMusicState.legacyLevel > XW_BRIEF_MUSIC_MIN_LEVEL) {
#ifdef XW_MODERN
			Dos94_soundext_SetGroup(g_briefMusicState.previousMusic, --g_briefMusicState.legacyLevel);
#else
			soundext_SetGroup(0, --g_briefMusicState.legacyLevel);
#endif
			return;
		}
		if (musicPosition > XW_BRIEF_MUSIC_POSITION_4) {
			soundext_SetHook(g_briefMusicState.previousMusic, XW_SOUND_CONTROL_DIRECT,
							 XW_BRIEF_MUSIC_CONTROL_4, 0);
		} else if (musicPosition > XW_BRIEF_MUSIC_POSITION_3) {
			soundext_SetHook(g_briefMusicState.previousMusic, XW_SOUND_CONTROL_DIRECT,
							 XW_BRIEF_MUSIC_CONTROL_3, 0);
		} else if (musicPosition > XW_BRIEF_MUSIC_POSITION_2) {
			soundext_SetHook(g_briefMusicState.previousMusic, XW_SOUND_CONTROL_DIRECT,
							 XW_BRIEF_MUSIC_CONTROL_2, 0);
		}
	}
}

// FUNCTION: XW 0x4342A0
int16_t brief_LoadUiSounds(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_6, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_CLOSE_2, 0, NULL, 1, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TEXT_5, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_TARGET_5, 0, NULL, 0, 0);
		g_briefingTextSoundActive = 0;
	}
	return ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x434300
void brief_HandleSoundAction(XwBriefingSoundAction action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case XW_BRIEF_SOUND_DOOR_OPEN:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_6);
				break;
			case XW_BRIEF_SOUND_DOOR_CLOSE:
				soundext_Play_SFX(XW_SHELL_SFX_DOOR_CLOSE_2);
				break;
			case XW_BRIEF_SOUND_TARGET:
				soundext_Play_SFX(XW_SHELL_SFX_TARGET_5);
				break;
			case XW_BRIEF_SOUND_REQUEST_TEXT:
				if (g_briefingTextSoundActive == 0) {
					soundext_Play_SFX(XW_SHELL_SFX_TEXT_5);
					g_briefingTextSoundActive = 1;
				}
				g_briefingTextSoundRequested = 1;
				break;
			case XW_BRIEF_SOUND_STOP_TEXT:
				if (g_briefingTextSoundActive != 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_briefingTextSoundActive = 0;
				}
				g_briefingTextSoundRequested = 1;
				break;
			case XW_BRIEF_SOUND_TICK:
				if (g_briefingTextSoundRequested == 0) {
					soundext_Stop_SFX(XW_SHELL_SFX_TEXT_5);
					g_briefingTextSoundActive = 0;
				}
				g_briefingTextSoundRequested = 0;
				break;
		}
	}
}

// FUNCTION: XW 0x439390
XwShellSceneResult brief_Brief(struct XwShellContext* context) {
	ResFile* resource;
	Rect rect;
	Rect frame;
	int16_t iconIndex;
	int16_t filmIndex;
	PushButton* button;
	Input* pageLabel;
	g_briefingRequestedNarrationPage = -1;
	g_briefingLoadedNarrationPage = -1;
	g_briefingNarrationSound = NULL;
	g_briefingSelectedIconPlusOne = 0;
	g_briefingMissionChoice = 0;
	g_briefingFocusIndex = XW_BRIEFING_INITIAL_FOCUS;
	xio_Set_Mouse_Position(XW_BRIEFING_INITIAL_MOUSE_X, XW_BRIEFING_INITIAL_MOUSE_Y);
	g_briefingRuntimeHandle = xmemhdl_Alloc_Clear_Handle(sizeof(*g_briefingRuntime), LANDRU_MEMORY_RESOURCE);
	g_briefingRuntime = xmemhdl_Lock_Handle(g_briefingRuntimeHandle);
	nullsub_SharedNoOp();
	Shared_ReturnOne();
	brief_LoadPilotRecord();
	g_briefingTextActor = NULL;
	if (shellext_Get_Cur_Scene() == XW_SCENE_BRIEFING_TOUR) {
		brief_LoadMissionChoice(g_briefingMissionChoice, 0);
		g_briefingHasMissionChoice = 1;
		if (g_briefingPilotRecord.briefingMissionChoices[0] == XW_BRIEFING_NO_MISSION_CHOICE)
			g_briefingHasMissionChoice = 0;
		if (g_briefingPilotRecord.briefingMissionChoices[1] == XW_BRIEFING_NO_MISSION_CHOICE)
			g_briefingHasMissionChoice = 0;
		shipext_Set_Last_Briefed_Tour_Operation(g_briefingPilotRecord.currentTourOperation);
	} else {
		brief_InitRuntime(0);
		brief_LoadBriefingFile(g_shellMissionName);
		brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
		g_briefingHasMissionChoice = 0;
	}
	resource = xres_Open_Resource(":X-Wing Data\\RESOURCE\\brief640.lfd");
	if (resource == NULL)
		resource = xres_Open_Resource("brief640.lfd");
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xrect_Set_Rect(&rect, 0, 0, XW_BRIEFING_CANVAS_WIDTH, XW_BRIEFING_CANVAS_HEIGHT);
	filmIndex = shellext_Get_Cur_Scene() != XW_SCENE_BRIEFING_COMBAT;
	g_briefingFilm = xfilm_Res_Film(resource, g_briefingFilmNames[filmIndex], &rect, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_briefingFilm, context->standardPalette);
	g_briefingBackgroundActor = xactor_Find_Actor(FOURCC_DELT, "br_bg_f");
	xactor_Non_Refreshable_Actor(g_briefingBackgroundActor);
	g_briefingInteriorActor = xactor_Find_Actor(FOURCC_DELT, "br_srcnf");
	xactor_Non_Refreshable_Actor(g_briefingInteriorActor);
	g_briefingInteriorActor = xactor_Find_Actor(FOURCC_DELT, "br_intr");
	xactor_Non_Refreshable_Actor(g_briefingInteriorActor);
	g_briefingLeftDoorActor = xactor_Find_Actor(FOURCC_ANIM, "bdoor_l");
	xactor_Set_Actor_User_Function(g_briefingLeftDoorActor, brief_user_Door);
	g_briefingLeftDoorActor->id = 0;
	g_briefingRightDoorActor = xactor_Find_Actor(FOURCC_ANIM, "bdoor_r");
	xactor_Set_Actor_User_Function(g_briefingRightDoorActor, brief_user_Door);
	g_briefingRightDoorActor->id = 1;
	if (shipext_IsTourAvailable(XW_BRIEFING_BWING_TOUR)) {
		ResFile* bwingResource;
		bwingResource = xres_Open_Resource("bwing.lfd");
		for (iconIndex = 0; iconIndex < XW_BRIEFING_MAP_ICON_ACTOR_COUNT; ++iconIndex) {
			g_briefingMapIconActors[iconIndex] =
				xactanim_Res_Anim_Actor(resource, g_briefingIconResourceNames[iconIndex], &rect, 0, 0, 0);
			xactor_Set_Actor_Time(g_briefingMapIconActors[iconIndex], 0, 0);
		}
		g_briefingBwingRadarActor = xactdelt_Res_Delta_Actor(bwingResource, "bradar", &rect, 0, 0, 0);
		xactor_Set_Actor_Time(g_briefingBwingRadarActor, 0, 0);
		xres_Close_Resource(bwingResource);
	} else {
		for (iconIndex = 0; iconIndex < XW_BRIEFING_MAP_ICON_ACTOR_COUNT; ++iconIndex) {
			g_briefingMapIconActors[iconIndex] =
				xactanim_Res_Anim_Actor(resource, g_briefingIconResourceNames[iconIndex], &rect, 0, 0, 0);
			xactor_Set_Actor_Time(g_briefingMapIconActors[iconIndex], 0, 0);
		}
		g_briefingBwingRadarActor = NULL;
	}
	g_briefingSelectionPreviewActor = xactanim_Res_Anim_Actor(resource, "radar640", &rect, 0, 0, 0);
	xactor_Set_Actor_Time(g_briefingSelectionPreviewActor, 0, 0);
	if (Shared_ReturnZero() == 0) {
		g_briefingStarsActor = xactdelt_Res_Delta_Actor(resource, "stars640", &rect, 0, 0, 0);
		xactor_Set_Actor_Time(g_briefingStarsActor, 0, 0);
	} else
		g_briefingStarsActor = NULL;
	g_briefingOfficerRegionBuffer = xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_OFFICER_BUFFER_CAPACITY, 0);
	g_briefingOfficerRegionRestored = 1;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_BRIEFING_COMBAT:
			g_briefingDodonnaActor = xactor_Find_Actor(FOURCC_ANIM, "do_donna");
			xactor_Set_Actor_User_Function(g_briefingDodonnaActor, brief_UpdateDodonnaActor);
			xrect_Set_Rect(&g_briefingOfficerRect, XW_BRIEFING_DODONNA_LEFT, XW_BRIEFING_DODONNA_TOP,
						   XW_BRIEFING_DODONNA_RIGHT, XW_BRIEFING_DODONNA_BOTTOM);
			break;
		case XW_SCENE_BRIEFING_TOUR:
			g_briefingAckbarActor = xactor_Find_Actor(FOURCC_ANIM, "b_akbar");
			xactor_Set_Actor_User_Function(g_briefingAckbarActor, brief_UpdateAckbarActor);
			xrect_Set_Rect(&g_briefingOfficerRect, XW_BRIEFING_ACKBAR_LEFT, XW_BRIEFING_ACKBAR_TOP,
						   XW_BRIEFING_ACKBAR_RIGHT, XW_BRIEFING_ACKBAR_BOTTOM);
			break;
	}
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_BRIEFING_COMBAT:
			xrect_Set_Rect(&frame, XW_BRIEFING_MAP_LEFT, XW_BRIEFING_MAP_TOP, XW_BRIEFING_MAP_RIGHT,
						   XW_BRIEFING_MAP_BOTTOM);
			break;
		case XW_SCENE_BRIEFING_TOUR:
			xrect_Set_Rect(&frame, XW_BRIEFING_MAP_LEFT, XW_BRIEFING_MAP_TOP, XW_BRIEFING_MAP_RIGHT,
						   XW_BRIEFING_MAP_BOTTOM);
			break;
		default:
			frame = rect;
			break;
	}
	g_briefingTextActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, XW_BRIEFING_MAP_Z);
	xactor_Set_Actor_User_Function(g_briefingTextActor, brief_AdvanceOrResetAtEnd);
	xactor_Set_Actor_Draw_Function(g_briefingTextActor, brief_DrawMapViewport);
	xactor_Non_Refreshable_Actor(g_briefingTextActor);
	g_briefingTextActor->var1 = 0;
	g_briefingOfficerRestoreActor = xactcust_Alloc_Custom_Actor(0, &frame, 0, 0, XW_BRIEFING_RESTORE_Z);
	xactor_Set_Actor_Draw_Function(g_briefingOfficerRestoreActor, brief_RestoreOfficerRegion);
	xrect_Set_Rect(&rect, 0, 0, XW_BRIEFING_CANVAS_WIDTH, XW_BRIEFING_CANVAS_HEIGHT);
	g_briefingWorldInput = xinput_Alloc_Input(NULL, &rect, 0, 0);
	g_briefingMapInput = xinput_Alloc_Input(g_briefingWorldInput, &frame, 0, 0);
	xinpattr_Set_Input_Update_Function(g_briefingMapInput, brief_UpdateMapSelection);
	xinpattr_Set_Input_User_Function(g_briefingMapInput, brief_ApplyMapSelection);
	g_briefingMapInput->mouseUsage = allInput;
	xrect_Set_Rect(&rect, XW_BRIEFING_LEFT_DOOR_LEFT, XW_BRIEFING_LEFT_DOOR_TOP, XW_BRIEFING_LEFT_DOOR_RIGHT,
				   XW_BRIEFING_LEFT_DOOR_BOTTOM);
	g_briefingLeftDoorInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_briefingLeftDoorInput, brief_iupdate_Brief);
	xinpattr_Set_Input_User_Function(g_briefingLeftDoorInput, brief_iuser_Brief);
	g_briefingLeftDoorInput->mouseUsage = allInput;
	g_briefingLeftDoorInput->id = 0;
	xrect_Set_Rect(&rect, XW_BRIEFING_RIGHT_DOOR_LEFT, XW_BRIEFING_RIGHT_DOOR_TOP,
				   XW_BRIEFING_RIGHT_DOOR_RIGHT, XW_BRIEFING_RIGHT_DOOR_BOTTOM);
	g_briefingRightDoorInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(g_briefingRightDoorInput, brief_iupdate_Brief);
	xinpattr_Set_Input_User_Function(g_briefingRightDoorInput, brief_iuser_Brief);
	g_briefingRightDoorInput->mouseUsage = allInput;
	g_briefingRightDoorInput->id = 1;
	xrect_Set_Rect(&rect, XW_BRIEFING_PAGE_LABEL_LEFT, XW_BRIEFING_PAGE_LABEL_TOP,
				   XW_BRIEFING_PAGE_LABEL_RIGHT, XW_BRIEFING_PAGE_LABEL_BOTTOM);
	g_briefingDoorHintInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Draw_Function(g_briefingDoorHintInput, brief_DrawDoorHint);
	xinpattr_Hide_Input(g_briefingDoorHintInput);
	g_briefingHintCompanionInput = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xrect_Set_Rect(&rect, XW_BRIEFING_REWIND_LEFT, XW_BRIEFING_REWIND_TOP, XW_BRIEFING_REWIND_RIGHT,
				   XW_BRIEFING_REWIND_BOTTOM);
	button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, "Rewind",
								   XW_BRIEF_BUTTON_REWIND);
	xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	g_briefingRewindButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_rwnd");
	xrect_Set_Rect(&rect, XW_BRIEFING_STOP_LEFT, XW_BRIEFING_STOP_TOP, XW_BRIEFING_STOP_RIGHT,
				   XW_BRIEFING_STOP_BOTTOM);
	button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, "Stop",
								   XW_BRIEF_BUTTON_STOP);
	xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	g_briefingStopButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_stop");
	xrect_Set_Rect(&rect, XW_BRIEFING_PLAY_LEFT, XW_BRIEFING_PLAY_TOP, XW_BRIEFING_PLAY_RIGHT,
				   XW_BRIEFING_PLAY_BOTTOM);
	button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, "Play",
								   XW_BRIEF_BUTTON_PLAY);
	xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	g_briefingPlayButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_play");
	xrect_Set_Rect(&rect, XW_BRIEFING_PREVIOUS_LEFT, XW_BRIEFING_PREVIOUS_TOP, XW_BRIEFING_PREVIOUS_RIGHT,
				   XW_BRIEFING_PREVIOUS_BOTTOM);
	button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, NULL,
								   XW_BRIEF_BUTTON_PREVIOUS_PAGE);
	xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	g_briefingPreviousPageButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_left");
	xrect_Set_Rect(&rect, XW_BRIEFING_NEXT_LEFT, XW_BRIEFING_NEXT_TOP, XW_BRIEFING_NEXT_RIGHT,
				   XW_BRIEFING_NEXT_BOTTOM);
	button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, NULL,
								   XW_BRIEF_BUTTON_NEXT_PAGE);
	xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	g_briefingNextPageButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_right");
	xrect_Set_Rect(&rect, XW_BRIEFING_PAGE_LABEL_LEFT, XW_BRIEFING_PAGE_LABEL_TOP,
				   XW_BRIEFING_PAGE_LABEL_RIGHT, XW_BRIEFING_PAGE_LABEL_BOTTOM);
	pageLabel = xinput_Alloc_Input(g_briefingWorldInput, &rect, 0, 0);
	xinpattr_Set_Input_Update_Function(pageLabel, brief_UpdatePageLabel);
	xinpattr_Set_Input_Draw_Function(pageLabel, brief_DrawPageLabel);
	if (g_briefingHasMissionChoice) {
		xrect_Set_Rect(&rect, XW_BRIEFING_RIGHT_AUX_LEFT, XW_BRIEFING_RIGHT_AUX_TOP,
					   XW_BRIEFING_RIGHT_AUX_RIGHT, XW_BRIEFING_RIGHT_AUX_BOTTOM);
		button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton,
									   "Mission B", XW_BRIEF_BUTTON_MISSION_B);
		xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
		g_briefingLogButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_log");
		xrect_Set_Rect(&rect, XW_BRIEFING_LEFT_AUX_LEFT, XW_BRIEFING_LEFT_AUX_TOP, XW_BRIEFING_LEFT_AUX_RIGHT,
					   XW_BRIEFING_LEFT_AUX_BOTTOM);
		button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton,
									   "Mission A", XW_BRIEF_BUTTON_MISSION_A);
		xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	} else {
		xrect_Set_Rect(&rect, XW_BRIEFING_RIGHT_AUX_LEFT, XW_BRIEFING_RIGHT_AUX_TOP,
					   XW_BRIEFING_RIGHT_AUX_RIGHT, XW_BRIEFING_RIGHT_AUX_BOTTOM);
		button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, "Log",
									   XW_BRIEF_BUTTON_LOG_ALTERNATE);
		xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
		g_briefingLogButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_log");
		xrect_Set_Rect(&rect, XW_BRIEFING_LEFT_AUX_LEFT, XW_BRIEFING_LEFT_AUX_TOP, XW_BRIEFING_LEFT_AUX_RIGHT,
					   XW_BRIEFING_LEFT_AUX_BOTTOM);
		button = xbtnpush_Alloc_Button(g_briefingWorldInput, &rect, 0, brief_HandlePlaybackButton, "Medals",
									   XW_BRIEF_BUTTON_MEDALS_ALTERNATE);
		xinpattr_Set_Input_Draw_Function(&button->header, XwBrief_DrawPlaybackButton);
	}
	g_briefingMedalsButtonActor = xactor_Find_Actor(FOURCC_ANIM, "br_medal");
	xio_Set_Key_Buttons();
	xview_Set_View_Update_Function(brief_end_View);
#ifdef XW_MODERN
	XwBrief_RunView(resource);
#else
	brief_OpenMusic(resource, g_briefingFilm);
	brief_LoadUiSounds();
	FrontendAudio_PlayFile("XwingCD\\music\\regbrief.wav", 1);
	j_xviewadd_Handle_View();
	brief_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xio_Clear_Key_Buttons();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	LandruDisplay_ForwardLegacyNoOp(0);
	xres_Close_Resource(resource);
	brief_Free_Display_Map();
	nullsub_SharedNoOp();
	xmemhdl_Free_Handle(g_briefingRuntimeHandle);
	xmemhdl_Free_Handle(g_briefingOfficerRegionBuffer);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x439EB0
void brief_end_View(int time) {
	int16_t key;
	if (time == 0 && (uint16_t)xcursor_Is_Cursor_Visible() == 0)
		xcursor_Show_Cursor();
	key = xio_Get_Free_Key();
	if (key != 0) {
		if (g_briefingHasMissionChoice != 0) {
			if (shellext_MoveGridFocus(&g_briefingFocusIndex, g_briefingMissionChoiceFocusX,
									   g_briefingMissionChoiceFocusY, XW_BRIEFING_CHOICE_FOCUS_ROWS,
									   XW_BRIEFING_FOCUS_COLUMNS, key) != 0) {
				xio_Set_Mouse_Position(g_briefingMissionChoiceFocusX[g_briefingFocusIndex],
									   g_briefingMissionChoiceFocusY[g_briefingFocusIndex]);
				xio_Get_Key();
			}
		} else {
			if (shellext_MoveGridFocus(&g_briefingFocusIndex, g_briefingFocusX, g_briefingFocusY,
									   XW_BRIEFING_FOCUS_ROWS, XW_BRIEFING_FOCUS_COLUMNS, key) != 0) {
				xio_Set_Mouse_Position(g_briefingFocusX[g_briefingFocusIndex],
									   g_briefingFocusY[g_briefingFocusIndex]);
				xio_Get_Key();
			}
		}
	}
	brief_HandleSoundAction(XW_BRIEF_SOUND_TICK);
	if (g_briefingFilm->cur_cel != g_briefingFilm->cels)
		xactor_Refresh_Actor(g_briefingTextActor);
	if (g_briefingRequestedNarrationPage != g_briefingLoadedNarrationPage) {
		if (g_briefingNarrationSound != NULL) {
			xsound_Free_Sound(g_briefingNarrationSound);
			g_briefingNarrationSound = NULL;
		}
		g_briefingLoadedNarrationPage = g_briefingRequestedNarrationPage;
		brief_LoadNarration();
	}
}

// FUNCTION: XW 0x439FD0
void brief_RefreshRuntimePointer(int16_t refresh) {
	if (refresh != 0) {
		g_briefingRuntime = xmemhdl_Lock_Handle(g_briefingRuntimeHandle);
		xmemhdl_Unlock_Handle(g_briefingRuntimeHandle);
	}
}

// FUNCTION: XW 0x43A000
int16_t brief_UpdateMapSelection(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
								 int rightEvent, int16_t x, int16_t y) {
	Rect viewportRect;
	int16_t iconIndex;
	(void)frame;
	(void)clip;
	(void)leftEvent;
	(void)rightEvent;
	if (key != 0) {
		return 0;
	}
	if (g_briefingRuntime->playbackActive != 0) {
		return 0;
	}
	xrect_Copy_Rect(&viewportRect, &g_briefingRuntime->mapViewportRect);
	if ((int16_t)brief_Find_Ship_On_Screen(&viewportRect, x, y, &iconIndex) != 0) {
		input->var1 = iconIndex + 1;
	} else {
		g_briefingSelectedIconPlusOne = 0;
	}
	return 1;
}

// FUNCTION: XW 0x43A080
void brief_ApplyMapSelection(Input* input, int time) {
	(void)time;
	if (input->var1 != 0) {
		g_briefingSelectedIconPlusOne = input->var1;
		input->var1 = 0;
	}
}

// FUNCTION: XW 0x43A0A0
int16_t brief_iupdate_Brief(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent, int rightEvent,
							int16_t x, int16_t y) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0)
		return 0;
	if (leftEvent == XW_BRIEF_DOOR_ACTIVATE_EVENT || rightEvent == XW_BRIEF_DOOR_ACTIVATE_EVENT) {
		input->var1 = XW_BRIEF_DOOR_ACTION_EXIT;
		if (shellext_Get_Cur_Scene() == XW_SCENE_BRIEFING_COMBAT) {
			if (input->id == XW_BRIEF_DOOR_ABORT)
				input->var2 = XW_SCENE_COMBAT_SIMULATOR_ROOM;
			else
				input->var2 = XW_SCENE_REQUEST_COMBAT_LAUNCH;
		} else if (input->id == XW_BRIEF_DOOR_ABORT) {
			shipext_ResetMissionSelections();
			input->var2 = XW_SCENE_TOUR_RETURN_INDEPENDENCE;
		} else {
			input->var2 = XW_SCENE_REQUEST_TOUR_LAUNCH;
		}
	} else {
		input->var1 = XW_BRIEF_DOOR_ACTION_HOVER;
	}
	if (input->id == XW_BRIEF_DOOR_ABORT)
		g_briefingLeftDoorActor->var1 = 1;
	else
		g_briefingRightDoorActor->var1 = 1;
	return 1;
}

// FUNCTION: XW 0x43A140
void brief_iuser_Brief(Input* input, int time) {
	(void)time;
	switch (input->var1) {
		case 0:
			if (xinpattr_Is_Input_Visible(g_briefingDoorHintInput) &&
				input->id == g_briefingDoorHintInput->var1) {
				xinpattr_Show_Input(g_briefingHintCompanionInput);
				xinpattr_Hide_Input(g_briefingDoorHintInput);
				xactor_Refresh_Actor(g_briefingInteriorActor);
				xinpattr_Refresh_Input(g_briefingWorldInput);
			}
			break;
		case XW_BRIEF_DOOR_ACTION_EXIT: {
			int16_t exitCode = input->var2;
			if (exitCode == XW_SCENE_REQUEST_TOUR_LAUNCH) {
				brief_CommitMissionChoice();
				brief_WritePilotRecord();
				if (brief_PrepareLaunchAndSavePilot()) {
					switch (shipext_Get_Mission_Ship()) {
						case SHIPEXT_SHIP_AWING:
							xerror_Set_Landru_Exit(XW_SCENE_TOUR_LAUNCH_AWING);
							return;
						case SHIPEXT_SHIP_XWING:
							xerror_Set_Landru_Exit(XW_SCENE_TOUR_LAUNCH_XWING);
							return;
						case SHIPEXT_SHIP_YWING:
							xerror_Set_Landru_Exit(XW_SCENE_TOUR_LAUNCH_YWING);
							return;
						case SHIPEXT_SHIP_BWING:
							xerror_Set_Landru_Exit(XW_SCENE_TOUR_LAUNCH_BWING);
							return;
					}
				}
			} else if (exitCode == XW_SCENE_REQUEST_COMBAT_LAUNCH) {
				if (brief_PrepareLaunchAndSavePilot()) {
					switch (shipext_Get_Mission_Ship()) {
						case SHIPEXT_SHIP_AWING:
							xerror_Set_Landru_Exit(XW_SCENE_COMBAT_LAUNCH_AWING);
							return;
						case SHIPEXT_SHIP_XWING:
							xerror_Set_Landru_Exit(XW_SCENE_COMBAT_LAUNCH_XWING);
							return;
						case SHIPEXT_SHIP_YWING:
							xerror_Set_Landru_Exit(XW_SCENE_COMBAT_LAUNCH_YWING);
							return;
						case SHIPEXT_SHIP_BWING:
							xerror_Set_Landru_Exit(XW_SCENE_COMBAT_LAUNCH_BWING);
							return;
					}
				}
			}
			xerror_Set_Landru_Exit(exitCode);
			break;
		}
		case XW_BRIEF_DOOR_ACTION_HOVER:
			if (xinpattr_Is_Input_Visible(g_briefingWorldInput)) {
				xinpattr_Hide_Input(g_briefingHintCompanionInput);
				xinpattr_Show_Input(g_briefingDoorHintInput);
				xactor_Refresh_Actor(g_briefingInteriorActor);
				xinpattr_Refresh_Input(g_briefingDoorHintInput);
				g_briefingDoorHintInput->var1 = input->id;
			}
			input->var1 = 0;
			break;
	}
}

// FUNCTION: XW 0x43A340
void brief_DrawDoorHint(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char text[XW_BRIEF_DOOR_HINT_CAPACITY];
	(void)clip;
	if (refresh == 0) {
		return;
	}
	strcpy(text, input->var1 == 0 ? "Abort Mission" : "Enter Mission");
	xpaint_Paint_Clipped_Rect(frame, XW_BRIEF_DOOR_HINT_BACKGROUND);
	xrect_Offset_Rect(frame, XW_BRIEF_DOOR_HINT_OFFSET, XW_BRIEF_DOOR_HINT_OFFSET);
	xfont_Print_Centered_Text(text, frame, XW_BRIEF_DOOR_HINT_FONT, XW_BRIEF_DOOR_HINT_COLOR);
}

// FUNCTION: XW 0x43A3C0
void brief_HandlePlaybackButton(Input* input, int time) {
	(void)time;
	if (xinpattr_Get_Input_Selected(input) != 0) {
		int16_t id = input->id;
		switch (id) {
			case XW_BRIEF_BUTTON_PREVIOUS_PAGE:
			case XW_BRIEF_BUTTON_NEXT_PAGE:
				if (g_briefingRuntime->scriptCount > 1) {
					if (id == XW_BRIEF_BUTTON_NEXT_PAGE)
						brief_SelectPage(g_briefingRuntime->activeScriptIndex + 1);
					else
						brief_SelectPage(g_briefingRuntime->activeScriptIndex - 1);
					brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
					xinput_Refresh_System_Inputs();
					g_briefingSelectedIconPlusOne = 0;
				}
				break;
			case XW_BRIEF_BUTTON_REWIND:
				brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
				break;
			case XW_BRIEF_BUTTON_STOP:
				xinpattr_Refresh_Input(g_briefingWorldInput);
				g_briefingRuntime->playbackActive = 0;
				break;
			case XW_BRIEF_BUTTON_PLAY:
				xinpattr_Refresh_Input(g_briefingWorldInput);
				g_briefingRuntime->playbackActive = 1;
				g_briefingSelectedIconPlusOne = 0;
				break;
			case XW_BRIEF_BUTTON_MISSION_A:
			case XW_BRIEF_BUTTON_MISSION_B:
				if (id - XW_BRIEF_BUTTON_MISSION_A != g_briefingMissionChoice) {
					brief_LoadMissionChoice(id - XW_BRIEF_BUTTON_MISSION_A, 1);
					xview_Refresh_View();
				}
				break;
			case XW_BRIEF_BUTTON_LOG_ALTERNATE:
				if (shellext_Get_Cur_Scene() == XW_SCENE_BRIEFING_COMBAT)
					xerror_Set_Landru_Exit(XW_SCENE_PILOT_LOG_FROM_COMBAT_BRIEFING);
				else
					xerror_Set_Landru_Exit(XW_SCENE_PILOT_LOG_FROM_TOUR_BRIEFING);
				break;
			case XW_BRIEF_BUTTON_MEDALS_ALTERNATE:
				if (shellext_Get_Cur_Scene() == XW_SCENE_BRIEFING_COMBAT)
					xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_COMBAT_BRIEFING);
				else
					xerror_Set_Landru_Exit(XW_SCENE_UNIFORM_FROM_TOUR_BRIEFING);
				break;
			default:
				break;
		}
	}
}

// FUNCTION: XW 0x43A540
void brief_DrawPlaybackButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh) {
	if (refresh != 0) {
		Actor* buttonActor;
		switch (button->header.id) {
			case XW_BRIEF_BUTTON_PREVIOUS_PAGE:
				buttonActor = g_briefingPreviousPageButtonActor;
				break;
			case XW_BRIEF_BUTTON_NEXT_PAGE:
				buttonActor = g_briefingNextPageButtonActor;
				break;
			case XW_BRIEF_BUTTON_REWIND:
				buttonActor = g_briefingRewindButtonActor;
				break;
			case XW_BRIEF_BUTTON_STOP:
				buttonActor = g_briefingStopButtonActor;
				break;
			case XW_BRIEF_BUTTON_PLAY:
				buttonActor = g_briefingPlayButtonActor;
				break;
			case XW_BRIEF_BUTTON_LOG:
			case XW_BRIEF_BUTTON_LOG_ALTERNATE:
				buttonActor = g_briefingLogButtonActor;
				break;
			case XW_BRIEF_BUTTON_MEDALS:
			case XW_BRIEF_BUTTON_MEDALS_ALTERNATE:
				buttonActor = g_briefingMedalsButtonActor;
				break;
			default:
				buttonActor = NULL;
				break;
		}
		if (buttonActor != NULL) {
			xactor_Set_Actor_State(buttonActor, button->pressed, 0);
			xactanim_Draw_Anim_Actor(buttonActor, frame, clip, XW_BRIEF_BUTTON_ACTOR_X,
									 XW_BRIEF_BUTTON_ACTOR_Y, refresh);
		}
	}
	if (button->header.id >= XW_BRIEF_BUTTON_LOG) {
		xrect_Offset_Rect(frame, 0, XW_BRIEF_BUTTON_LABEL_Y);
		xfont_Print_Centered_Text(button->labels, frame, XW_BRIEFING_PAGE_FONT, XW_BRIEFING_PAGE_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x43A620
int16_t brief_UpdatePageLabel(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
							  int rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (key != 0)
		return 0;
	if (leftEvent == XW_BRIEF_PAGE_CLICK_EVENT || rightEvent == XW_BRIEF_PAGE_CLICK_EVENT)
		brief_Seek_Page_Section(g_briefingRuntime->activeScriptIndex);
	return 1;
}

// FUNCTION: XW 0x43A660
void brief_DrawPageLabel(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	char pageLabel[XW_BRIEFING_PAGE_LABEL_CAPACITY];

	(void)input;
	(void)clip;
	if (refresh != 0) {
		xpaint_Paint_Clipped_Rect(frame, XW_BRIEFING_PAGE_BACKGROUND_COLOR);
		sprintf(pageLabel, "Page %d of %d", g_briefingRuntime->activeScriptIndex + 1,
				g_briefingRuntime->scriptCount);
		xfont_Print_Centered_Text(pageLabel, frame, XW_BRIEFING_PAGE_FONT, XW_BRIEFING_PAGE_TEXT_COLOR);
	}
}

// FUNCTION: XW 0x43A6C0
void brief_user_Door(Actor* actor, int time) {
	int16_t needsRefresh = 1;
	(void)time;
	if (actor->var1 == 0) {
		if (actor->state == 1) {
			brief_HandleSoundAction(XW_BRIEF_SOUND_DOOR_CLOSE);
		}
		if (actor->state != 0) {
			xactor_Set_Actor_State(actor, actor->state - 1, 0);
		} else {
			needsRefresh = 0;
		}
	} else {
		if (actor->state == 0) {
			brief_HandleSoundAction(XW_BRIEF_SOUND_DOOR_OPEN);
		}
		if (actor->state != actor->arraySize - 1) {
			xactor_Set_Actor_State(actor, actor->state + 1, 0);
		} else {
			needsRefresh = 0;
		}
		actor->var1 = 0;
	}
	if (needsRefresh != 0) {
		xactor_Refresh_Actor(actor);
	}
}

// FUNCTION: XW 0x43A750
void brief_AdvanceOrResetAtEnd(Actor* actor, int time) {
	(void)actor;
	if (g_briefingRuntime->playbackActive != 0 && time != 0) {
		int16_t script = g_briefingRuntime->activeScriptIndex;
		if (g_briefingRuntime->scriptCurrentTime[script] < g_briefingRuntime->scriptEndTime[script])
			brief_Step_Page(script, 0);
		else
			brief_Rewind_Page(script);
		brief_Move_Display_Map();
	}
}

// FUNCTION: XW 0x43A7B0
int16_t brief_DrawMapViewport(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)actor;
	(void)x;
	(void)y;
	brief_DrawViewportAndSelection(frame, clip, refresh);
	xcanvas_Max_Drawing_Canvas_Clip();
	brief_SetOfficerRegionRestored(0);
	return 1;
}

// FUNCTION: XW 0x43A7E0
void brief_UpdateAckbarActor(Actor* actor, int time) {
	(void)time;
	if (g_briefingRuntime->playbackActive != 0 &&
		g_briefingRuntime->activeScriptIndex == XW_BRIEFING_OFFICER_SCRIPT) {
		int state = actor->state;
		int16_t nextFrame;
		if (state < actor->arraySize - 1) {
			if (g_shellPreferences.sfxEnabled != 0 && g_shellPreferences.sfxVolume != 0) {
				if (g_briefingNarrationSound != NULL &&
					soundext_Count_Resource_Instances(g_briefingNarrationSound) != 0) {
					xactor_Set_Actor_State(
						actor, g_briefingAckbarAnimationSteps[state].narrationNextFramePlusOne - 1, 0);
				} else {
					for (nextFrame = state + 1;
						 g_briefingAckbarAnimationSteps[nextFrame].skipWithoutNarration != 0; ++nextFrame) {
					}
					xactor_Set_Actor_State(actor, nextFrame, 0);
				}
			} else {
				for (nextFrame = state + 1;
					 g_briefingAckbarAnimationSteps[nextFrame].skipWithoutNarration != 0; ++nextFrame) {
				}
				xactor_Set_Actor_State(actor, nextFrame, 0);
			}
		} else {
			xactor_Set_Actor_State(actor, 0, 0);
		}
	} else if (g_briefingRuntime->activeScriptIndex != XW_BRIEFING_OFFICER_SCRIPT) {
		xactor_Set_Actor_State(actor, 0, 0);
	}
}

// FUNCTION: XW 0x43A8F0
void brief_UpdateDodonnaActor(Actor* actor, int time) {
	(void)time;
	if (g_briefingRuntime->playbackActive != 0 &&
		g_briefingRuntime->activeScriptIndex == XW_BRIEFING_OFFICER_SCRIPT) {
		int state = actor->state;
		int16_t nextFrame;
		if (state < actor->arraySize - 1) {
			if (g_shellPreferences.sfxEnabled != 0 && g_shellPreferences.sfxVolume != 0) {
				if (g_briefingNarrationSound != NULL &&
					soundext_Count_Resource_Instances(g_briefingNarrationSound) != 0) {
					xactor_Set_Actor_State(
						actor, g_briefingDodonnaAnimationSteps[state].narrationNextFramePlusOne - 1, 0);
				} else {
					for (nextFrame = state + 1;
						 g_briefingDodonnaAnimationSteps[nextFrame].skipWithoutNarration != 0; ++nextFrame) {
					}
					xactor_Set_Actor_State(actor, nextFrame, 0);
				}
			} else {
				for (nextFrame = state + 1;
					 g_briefingDodonnaAnimationSteps[nextFrame].skipWithoutNarration != 0; ++nextFrame) {
				}
				xactor_Set_Actor_State(actor, nextFrame, 0);
			}
		} else {
			xactor_Set_Actor_State(actor, 0, 0);
		}
	} else if (g_briefingRuntime->activeScriptIndex != XW_BRIEFING_OFFICER_SCRIPT) {
		xactor_Set_Actor_State(actor, 0, 0);
	}
}

// FUNCTION: XW 0x43AA00
int16_t brief_RestoreOfficerRegion(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh) {
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	(void)refresh;
	xcanvas_Max_Drawing_Canvas_Clip();
	brief_SetOfficerRegionRestored(1);
	return 1;
}

// FUNCTION: XW 0x43AA20
void brief_SelectPage(int16_t pageIndex) {
	int16_t scriptCount = g_briefingRuntime->scriptCount;
	if (pageIndex >= scriptCount)
		pageIndex -= scriptCount;
	if (pageIndex < 0)
		pageIndex += scriptCount;
	g_briefingRuntime->activeScriptIndex = pageIndex;
	brief_Reseek_Page(pageIndex);
	brief_ApplyLayout(g_briefingRuntime->scriptLayoutIndex[pageIndex]);
}

// FUNCTION: XW 0x43AA70
void brief_Rewind_Page(int16_t scriptIndex) {
	int16_t slot;
	g_briefingRuntime->mapCenter[0] = 0;
	g_briefingRuntime->mapCenter[1] = 0;
	g_briefingRuntime->mapTargetCenter[0] = 0;
	g_briefingRuntime->mapTargetCenter[1] = 0;
	g_briefingRuntime->mapScale[0] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->mapScale[1] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[0] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[1] = XW_BRIEF_INITIAL_MAP_SCALE;
	g_briefingRuntime->field_37DC = 0;
	for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->textSlotActive) /
								sizeof(g_briefingRuntime->textSlotActive[0]));
		 ++slot)
		g_briefingRuntime->textSlotActive[slot] = 0;
	for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->fgMarkerActive) /
								sizeof(g_briefingRuntime->fgMarkerActive[0]));
		 ++slot)
		g_briefingRuntime->fgMarkerActive[slot] = 0;
	for (slot = 0;
		 slot < (int)(sizeof(g_briefingRuntime->labelActive) / sizeof(g_briefingRuntime->labelActive[0]));
		 ++slot)
		g_briefingRuntime->labelActive[slot] = 0;
	g_briefingRuntime->scriptCurrentTime[scriptIndex] = 0;
	g_briefingRuntime->scriptCursorWordIndex[scriptIndex] = 0;
	brief_Step_Page(scriptIndex, 1);
}

// FUNCTION: XW 0x43AB70
void brief_Step_Page(int16_t scriptIndex, int16_t initializeState) {
	int16_t cursor = g_briefingRuntime->scriptCursorWordIndex[scriptIndex];
	int16_t commandStart = cursor;
	int16_t commandTime = g_briefingRuntime->scriptWords[scriptIndex][cursor];
	int16_t refreshText = 0;
	int16_t args[XW_BRIEF_COMMAND_ARGUMENT_CAPACITY];
	g_briefingRuntime->field_37E6 = 0;
	g_briefingRuntime->textSlotsChanged = 0;
	g_briefingRuntime->fgMarkersChanged = 0;
	g_briefingRuntime->labelsChanged = 0;
	g_briefingRuntime->mapCenterDirty = 0;
	g_briefingRuntime->mapScaleDirty = 0;
	g_briefingRuntime->pauseMarkerReached = 0;
	if (commandTime <= g_briefingRuntime->scriptCurrentTime[scriptIndex]) {
		do {
			int16_t opcode, argCount;
			int argIndex;
			commandStart = cursor;
			commandTime = g_briefingRuntime->scriptWords[scriptIndex][cursor++];
			opcode = g_briefingRuntime->scriptWords[scriptIndex][cursor++];
			argCount = g_briefingScriptOpcodeArgCounts[opcode];
			for (argIndex = 0; argCount > 0; ++argIndex, --argCount)
				args[argIndex] = g_briefingRuntime->scriptWords[scriptIndex][cursor++];
			if (commandTime == g_briefingRuntime->scriptCurrentTime[scriptIndex]) {
				switch (opcode) {
					case XW_BRIEF_COMMAND_PAUSE:
						g_briefingRuntime->pauseMarkerReached = 1;
						break;
					case XW_BRIEF_COMMAND_CLEAR_TEXT: {
						int16_t slot;
						for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->textSlotActive) /
													sizeof(g_briefingRuntime->textSlotActive[0]));
							 ++slot) {
							if (g_briefingRuntime->textSlotActive[slot] != 0)
								g_briefingRuntime->textSlotActive[slot] = 0;
						}
						g_briefingRuntime->textSlotsChanged = 1;
						refreshText = 1;
						break;
					}
					case XW_BRIEF_COMMAND_TEXT_FIRST:
					case XW_BRIEF_COMMAND_TEXT_FIRST + 1:
					case XW_BRIEF_COMMAND_TEXT_FIRST + 2:
					case XW_BRIEF_COMMAND_TEXT_LAST: {
						int16_t slot = opcode - XW_BRIEF_COMMAND_TEXT_FIRST;
						if (g_briefingRuntime->textSlotActive[slot] == 0) {
							g_briefingRuntime->textSlotActive[slot] = 1;
							g_briefingRuntime->textSlotBlockIndex[slot] = args[0];
							if (slot == XW_BRIEF_NARRATION_TEXT_SLOT)
								g_briefingRequestedNarrationPage = args[0];
						}
						refreshText = 1;
						break;
					}
					case XW_BRIEF_COMMAND_CENTER:
						if (commandTime == 0 || initializeState != 0) {
							g_briefingRuntime->mapTargetCenter[0] = args[0];
							g_briefingRuntime->mapCenter[0] = args[0];
							g_briefingRuntime->mapTargetCenter[1] = args[1];
							g_briefingRuntime->mapCenter[1] = args[1];
						} else if (g_briefingRuntime->mapTargetCenter[0] != args[0] ||
								   g_briefingRuntime->mapTargetCenter[1] != args[1]) {
							g_briefingRuntime->mapTargetCenter[0] = args[0];
							g_briefingRuntime->mapTargetCenter[1] = args[1];
						}
						g_briefingRuntime->mapCenterDirty = 1;
						break;
					case XW_BRIEF_COMMAND_SCALE:
						if (commandTime == 0 || initializeState != 0) {
							g_briefingRuntime->mapTargetScale[0] = XW_BRIEFING_LAYOUT_SCALE * args[0];
							g_briefingRuntime->mapScale[0] = g_briefingRuntime->mapTargetScale[0];
							g_briefingRuntime->mapTargetScale[1] = XW_BRIEFING_LAYOUT_SCALE * args[1];
							g_briefingRuntime->mapScale[1] = g_briefingRuntime->mapTargetScale[1];
						} else if (g_briefingRuntime->mapTargetScale[0] != args[0] ||
								   g_briefingRuntime->mapTargetScale[1] != args[1]) {
							g_briefingRuntime->mapTargetScale[0] = XW_BRIEFING_LAYOUT_SCALE * args[0];
							g_briefingRuntime->mapTargetScale[1] = XW_BRIEFING_LAYOUT_SCALE * args[1];
						}
						g_briefingRuntime->mapScaleDirty = 1;
						break;
					case XW_BRIEF_COMMAND_CLEAR_MARKERS: {
						int16_t slot;
						for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->fgMarkerActive) /
													sizeof(g_briefingRuntime->fgMarkerActive[0]));
							 ++slot) {
							if (g_briefingRuntime->fgMarkerActive[slot] != 0)
								g_briefingRuntime->fgMarkerActive[slot] = 0;
						}
						g_briefingRuntime->fgMarkersChanged = 1;
						break;
					}
					case XW_BRIEF_COMMAND_MARKER_FIRST:
					case XW_BRIEF_COMMAND_MARKER_FIRST + 1:
					case XW_BRIEF_COMMAND_MARKER_FIRST + 2:
					case XW_BRIEF_COMMAND_MARKER_LAST: {
						int16_t slot = opcode - XW_BRIEF_COMMAND_MARKER_FIRST;
						if (g_briefingRuntime->fgMarkerActive[slot] == 0) {
							g_briefingRuntime->fgMarkerActive[slot] = 1;
							g_briefingRuntime->fgMarkerAge[slot] =
								initializeState != 0 ? XW_BRIEF_INITIALIZED_AGE : 0;
							g_briefingRuntime->fgMarkerIconIndex[slot] = args[0];
						}
						break;
					}
					case XW_BRIEF_COMMAND_CLEAR_LABELS: {
						int16_t slot;
						for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->labelActive) /
													sizeof(g_briefingRuntime->labelActive[0]));
							 ++slot) {
							if (g_briefingRuntime->labelActive[slot] != 0)
								g_briefingRuntime->labelActive[slot] = 0;
						}
						g_briefingRuntime->labelsChanged = 1;
						break;
					}
					case XW_BRIEF_COMMAND_LABEL_FIRST:
					case XW_BRIEF_COMMAND_LABEL_FIRST + 1:
					case XW_BRIEF_COMMAND_LABEL_FIRST + 2:
					case XW_BRIEF_COMMAND_LABEL_LAST: {
						int16_t slot = opcode - XW_BRIEF_COMMAND_LABEL_FIRST;
						if (g_briefingRuntime->labelActive[slot] == 0) {
							g_briefingRuntime->labelActive[slot] = 1;
							g_briefingRuntime->labelAge[slot] =
								initializeState != 0 ? XW_BRIEF_INITIALIZED_AGE : 0;
							g_briefingRuntime->labelTextIndex[slot] = args[0];
							g_briefingRuntime->labelX[slot] = args[1];
							g_briefingRuntime->labelY[slot] = args[2];
						}
						break;
					}
					case XW_BRIEF_COMMAND_CLEAR_FIELD_37DC:
						if (g_briefingRuntime->field_37DC != 0) {
							g_briefingRuntime->field_37DC = 0;
							g_briefingRuntime->field_37E6 = 1;
						}
						break;
					case XW_BRIEF_COMMAND_SET_FIELD_37DC:
						if (g_briefingRuntime->field_37DC == 0) {
							g_briefingRuntime->field_37DC = 1;
							g_briefingRuntime->field_37E4 =
								initializeState != 0 ? XW_BRIEF_INITIALIZED_AGE : 0;
							g_briefingRuntime->field_37DE = args[0];
							g_briefingRuntime->field_37E0 = args[1];
							g_briefingRuntime->field_37E2 = args[2];
						}
						break;
					default:
						break;
				}
			}
		} while (commandTime <= g_briefingRuntime->scriptCurrentTime[scriptIndex]);
	}
	++g_briefingRuntime->scriptCurrentTime[scriptIndex];
	g_briefingRuntime->scriptCursorWordIndex[scriptIndex] = commandStart;
	if (refreshText != 0 && g_briefingTextActor != NULL)
		xactor_Refresh_Actor(g_briefingTextActor);
}

// FUNCTION: XW 0x43B0D0
int16_t brief_Seek_Page(int16_t scriptIndex, int16_t targetTime, int16_t initializeState) {
	if (targetTime != g_briefingRuntime->scriptCurrentTime[scriptIndex] - 1) {
		if (targetTime < g_briefingRuntime->scriptCurrentTime[scriptIndex])
			brief_Rewind_Page(scriptIndex);
		while (g_briefingRuntime->scriptCurrentTime[scriptIndex] <= targetTime)
			brief_Step_Page(scriptIndex, initializeState);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x43B140
void brief_Seek_Page_Section(int16_t scriptIndex) {
	int16_t foundStop = 0;
	int16_t nextOpcode = 0;
	int16_t textVisible = 0;
	int16_t visibleTextFrames = 0;
	int16_t originalTime = g_briefingRuntime->scriptCurrentTime[scriptIndex];
	int16_t seekTime, seekOpcode;
	brief_Rewind_Page(scriptIndex);
	while (foundStop == 0 && nextOpcode != XW_BRIEF_COMMAND_END) {
		int16_t slot;
		nextOpcode =
			g_briefingRuntime
				->scriptWords[scriptIndex][g_briefingRuntime->scriptCursorWordIndex[scriptIndex] + 1];
		if (g_briefingRuntime->textSlotsChanged != 0) {
			visibleTextFrames = 0;
			textVisible = 0;
		}
		for (slot = 0; slot < (int)(sizeof(g_briefingRuntime->textSlotActive) /
									sizeof(g_briefingRuntime->textSlotActive[0]));
			 ++slot) {
			if (g_briefingRuntime->textSlotActive[slot] != 0)
				textVisible = 1;
		}
		if (textVisible != 0)
			++visibleTextFrames;
		if ((g_briefingRuntime->mapScaleDirty != 0 || g_briefingRuntime->mapCenterDirty != 0 ||
			 g_briefingRuntime->pauseMarkerReached != 0 || visibleTextFrames == 1) &&
			g_briefingRuntime->scriptCurrentTime[scriptIndex] >= originalTime)
			foundStop = 1;
		else
			brief_Step_Page(scriptIndex, 1);
	}
	if ((g_briefingRuntime->pauseMarkerReached != 0 || visibleTextFrames == 1) && foundStop == 1) {
		seekTime = g_briefingRuntime->scriptCurrentTime[scriptIndex];
		seekOpcode = 0;
	} else {
		int cursor = g_briefingRuntime->scriptCursorWordIndex[scriptIndex];
		seekTime = g_briefingRuntime->scriptWords[scriptIndex][cursor];
		seekOpcode = g_briefingRuntime->scriptWords[scriptIndex][cursor + 1];
	}
	if (seekOpcode == XW_BRIEF_COMMAND_END)
		brief_Rewind_Page(scriptIndex);
	else
		brief_Seek_Page(scriptIndex, seekTime, 0);
}

// FUNCTION: XW 0x43B2B0
void brief_Reseek_Page(int16_t scriptIndex) {
	int16_t savedTime = g_briefingRuntime->scriptCurrentTime[scriptIndex];
	if (savedTime == 0)
		savedTime = 1;
	brief_Rewind_Page(scriptIndex);
	brief_Seek_Page(scriptIndex, savedTime - 1, 1);
}

// FUNCTION: XW 0x43B2F0
void brief_Move_Display_Map(void) {
	int16_t oldScaleX = g_briefingRuntime->mapScale[0];
	int16_t targetScaleX = g_briefingRuntime->mapTargetScale[0];
	int16_t maxScaleDistance = (int16_t)abs(oldScaleX - targetScaleX);
	int16_t scaleYDistance =
		(int16_t)abs(g_briefingRuntime->mapScale[1] - g_briefingRuntime->mapTargetScale[1]);
	int16_t scaleStep;
	int16_t oldCenterX, targetCenterX, maxCenterDistance, centerYDistance;
	int16_t scaledCenterDistance, centerStep;
	int centerStepBase;
	int16_t index;
	if (maxScaleDistance < scaleYDistance) {
		maxScaleDistance = scaleYDistance;
	}
	scaleStep = XW_BRIEFING_MAP_SCALE_STEP;
	if (maxScaleDistance >= XW_BRIEFING_MAP_FAST_SCALE_DISTANCE) {
		scaleStep = XW_BRIEFING_MAP_FAST_SCALE_STEP;
	}
	if (oldScaleX < XW_BRIEFING_MAP_FINE_SCALE_LIMIT) {
		scaleStep = XW_BRIEFING_MAP_FINE_SCALE_STEP;
	}
	g_briefingRuntime->mapScale[0] = brief_Move_To_Value(oldScaleX, targetScaleX, scaleStep);
	g_briefingRuntime->mapScale[1] =
		brief_Move_To_Value(g_briefingRuntime->mapScale[1], g_briefingRuntime->mapTargetScale[1], scaleStep);
	centerStepBase = XW_BRIEFING_MAP_COORDINATE_SCALE / g_briefingRuntime->mapScale[0] + 1;
	oldCenterX = g_briefingRuntime->mapCenter[0];
	targetCenterX = g_briefingRuntime->mapTargetCenter[0];
	maxCenterDistance = (int16_t)abs(oldCenterX - targetCenterX);
	centerYDistance = (int16_t)abs(g_briefingRuntime->mapCenter[1] - g_briefingRuntime->mapTargetCenter[1]);
	if (maxCenterDistance < centerYDistance) {
		maxCenterDistance = centerYDistance;
	}
	scaledCenterDistance = maxCenterDistance / (int16_t)centerStepBase;
	centerStep = XW_BRIEFING_MAP_CENTER_STEP_MULTIPLIER * centerStepBase;
	if (scaledCenterDistance >= XW_BRIEFING_MAP_FAST_CENTER_DISTANCE) {
		centerStep *= XW_BRIEFING_MAP_CENTER_STEP_MULTIPLIER;
	}
	g_briefingRuntime->mapCenter[0] = brief_Move_To_Value(oldCenterX, targetCenterX, centerStep);
	g_briefingRuntime->mapCenter[1] = brief_Move_To_Value(g_briefingRuntime->mapCenter[1],
														  g_briefingRuntime->mapTargetCenter[1], centerStep);
	for (index = 0;
		 index < (int)(sizeof(g_briefingRuntime->fgMarkerAge) / sizeof(g_briefingRuntime->fgMarkerAge[0]));
		 ++index) {
		if (g_briefingRuntime->fgMarkerActive[index] != 0) {
			++g_briefingRuntime->fgMarkerAge[index];
		}
	}
	for (index = 0;
		 index < (int)(sizeof(g_briefingRuntime->labelAge) / sizeof(g_briefingRuntime->labelAge[0]));
		 ++index) {
		if (g_briefingRuntime->labelActive[index] != 0) {
			++g_briefingRuntime->labelAge[index];
		}
	}
}

// FUNCTION: XW 0x43B490
int16_t brief_DrawViewportAndSelection(const Rect* viewportRect, const Rect* clipRect, int16_t redrawText) {
	Rect contentRect;
	Rect selectionRect;
	Rect clippedRect;
	char text[XW_BRIEFING_SELECTION_LABEL_SIZE];
	int16_t textSlot;
	if (redrawText != 0) {
		xpaint_Paint_Clipped_Rect((Rect*)viewportRect, 0);
		for (textSlot = 0; textSlot < (int)(sizeof(g_briefingRuntime->textViewportActive) /
											sizeof(g_briefingRuntime->textViewportActive[0]));
			 ++textSlot) {
			if (g_briefingRuntime->textViewportActive[textSlot] != 0) {
				xrect_Copy_Rect(&contentRect, &g_briefingRuntime->textViewportRects[textSlot]);
				xrect_Offset_Rect(&contentRect, viewportRect->left, viewportRect->top);
				xrect_Copy_Rect(&clippedRect, (Rect*)clipRect);
				xrect_Clip_Rect(&clippedRect, &contentRect);
				xcanvas_Set_Drawing_Canvas_Clip(&clippedRect);
				xpaint_Paint_Clipped_Rect(&contentRect, XW_BRIEFING_TEXT_BACKGROUND_COLOR);
				if (g_briefingRuntime->textSlotActive[textSlot] != 0) {
					int textBlockIndex = g_briefingRuntime->textSlotBlockIndex[textSlot];
					LandruHandle textHandle = g_briefingRuntime->textBlockHandles[textBlockIndex];
					LandruHandle attributeHandle = g_briefingRuntime->textAttributeHandles[textBlockIndex];
					xrect_Inset_Rect(&contentRect, XW_BRIEFING_TEXT_INSET_X, XW_BRIEFING_TEXT_INSET_Y);
					brief_Draw_Map_Paragraph(&contentRect, textHandle, attributeHandle, 0, textSlot);
				}
			}
		}
		xcanvas_Set_Drawing_Canvas_Clip((Rect*)clipRect);
	}
	if (g_briefingRuntime->mapViewportActive != 0) {
		xrect_Copy_Rect(&contentRect, &g_briefingRuntime->mapViewportRect);
		xrect_Offset_Rect(&contentRect, viewportRect->left, viewportRect->top);
		xrect_Copy_Rect(&clippedRect, (Rect*)clipRect);
		xrect_Clip_Rect(&clippedRect, &contentRect);
		xcanvas_Set_Drawing_Canvas_Clip(&clippedRect);
		brief_Draw_Display_Grid(&contentRect, &clippedRect);
		brief_DrawOverlays(&contentRect, &clippedRect);
		if (g_briefingSelectedIconPlusOne != 0) {
			int16_t previewType;
			int16_t colorOverride;
			int16_t colorGroup;
			int16_t characterIndex;
			int16_t showLabel;
			const char* labelText;
			xrect_Copy_Rect(&selectionRect, &contentRect);
			selectionRect.right = selectionRect.left + XW_BRIEFING_SELECTION_WIDTH;
			selectionRect.bottom = selectionRect.top + XW_BRIEFING_SELECTION_HEIGHT;
			if (xio_Mouse_X() < XW_BRIEFING_SELECTION_MOUSE_LIMIT) {
				xrect_Offset_Rect(&selectionRect, XW_BRIEFING_SELECTION_SHIFT, 0);
			}
			xpaint_Frame_Clipped_Rect(&selectionRect, XW_BRIEF_HIGHLIGHT_FRAME_COLOR);
			xrect_Inset_Rect(&selectionRect, 1, 1);
			xpaint_Paint_Clipped_Rect(&selectionRect, XW_BRIEF_HIGHLIGHT_FILL_COLOR);
			previewType = g_briefingRuntime->mapIconType[g_briefingSelectedIconPlusOne - 1];
			colorOverride = g_briefingRuntime->mapIconColorOverride[g_briefingSelectedIconPlusOne - 1];
			if (previewType <= XW_BRIEFING_LAST_SHIP_ICON_TYPE) {
				if (colorOverride == 0) {
					colorGroup = g_briefingMapDefaultIconColorGroups[previewType - 1];
				} else {
					colorGroup = colorOverride - 1;
				}
			} else {
				colorGroup = 0;
			}
			if (colorGroup < 0)
				colorGroup = 0;
			if (colorGroup > XW_BRIEFING_SELECTION_COLOR_COUNT - 1)
				colorGroup = XW_BRIEFING_SELECTION_COLOR_COUNT - 1;
			if (previewType >= XW_BRIEFING_ICON_MINE_FIRST && previewType <= XW_BRIEFING_ICON_MINE_LAST)
				previewType = XW_BRIEFING_ICON_MINE_FIRST;
			if (previewType >= XW_BRIEFING_ICON_COMMUNICATION && previewType < XW_BRIEFING_ICON_BWING)
				previewType -= XW_BRIEFING_MINE_VARIANT_COUNT;
			if (previewType == XW_BRIEFING_ICON_BWING) {
				previewType = XW_BRIEFING_PREVIEW_BWING;
				xactor_Set_Actor_State(g_briefingSelectionPreviewActor,
									   g_briefingSelectionPreviewActor->arraySize - 1, 0);
				xactanim_Draw_Anim_Actor(g_briefingSelectionPreviewActor, &contentRect, &clippedRect,
										 selectionRect.left + 1,
										 selectionRect.top + XW_BRIEFING_SELECTION_PREVIEW_Y, 1);
			} else {
				xactor_Set_Actor_State(g_briefingSelectionPreviewActor, previewType - 1, 0);
				xactanim_Draw_Anim_Actor(g_briefingSelectionPreviewActor, &contentRect, &clippedRect,
										 selectionRect.left + 1,
										 selectionRect.top + XW_BRIEFING_SELECTION_PREVIEW_Y, 1);
			}
			strcpy(text, g_briefingSelectionTypeLabels[previewType - 1]);
			strcat(text, ": ");
			showLabel = colorGroup == 0;
			showLabel |= (previewType > XW_BRIEFING_CONCEALED_TYPE_LAST) |
						 (previewType < XW_BRIEFING_CONCEALED_TYPE_FIRST);
			if (showLabel != 0) {
				labelText = g_briefingRuntime->mapIconName[g_briefingSelectedIconPlusOne - 1];
				strcat(text, labelText);
			} else {
				labelText = "UNKNOWN";
				strcat(text, labelText);
			}
			for (characterIndex = 0; characterIndex < (int)sizeof(text); ++characterIndex) {
				if (text[characterIndex] == 0)
					break;
				text[characterIndex] = toupper(text[characterIndex]);
			}
			xfont_Print_Clipped_Text(text, selectionRect.left + XW_BRIEFING_SELECTION_LABEL_X,
									 selectionRect.top + 1, 0, g_briefingSelectionTextColors[colorGroup]);
			return 1;
		}
	}
	return 1;
}

// FUNCTION: XW 0x43B8E0
int16_t brief_Draw_Display_Grid(const Rect* viewportRect, const Rect* clipRect) {
	Rect gridBounds;
	int16_t majorColor;
	int16_t minorColor;
	int16_t startColumn;
	int16_t startRow;
	int16_t startX;
	int16_t startY;
	int16_t gridX;
	int16_t gridY;
	int16_t column;
	int16_t row;
	int16_t showAllMinorLines;
	int16_t scaleX;
	int16_t scaleY;
	(void)clipRect;
	if (g_briefingRuntime->mapColorMode != 0) {
		majorColor = XW_BRIEFING_GRID_DARK_MAJOR_COLOR;
		minorColor = XW_BRIEFING_GRID_DARK_MINOR_COLOR;
	} else {
		majorColor = XW_BRIEFING_GRID_LIGHT_MAJOR_COLOR;
		minorColor = XW_BRIEFING_GRID_LIGHT_MINOR_COLOR;
	}
	brief_DrawBackground();
	xrect_Copy_Rect(&gridBounds, (Rect*)viewportRect);
	xpaint_Frame_Clipped_Rect(&gridBounds, minorColor);
	startColumn = g_briefingRuntime->mapCenter[0] / XW_BRIEFING_MAP_COORDINATE_SCALE;
	if (g_briefingRuntime->mapCenter[0] > 0 &&
		(g_briefingRuntime->mapCenter[0] & XW_BRIEFING_GRID_CELL_FRACTION_MASK) != 0)
		++startColumn;
	scaleX = g_briefingRuntime->mapScale[0];
	startX = (scaleX * ((-g_briefingRuntime->mapCenter[0]) & XW_BRIEFING_GRID_CELL_FRACTION_MASK)) >>
			 XW_BRIEFING_GRID_CELL_FRACTION_BITS;
	startX += gridBounds.left + ((gridBounds.right - gridBounds.left) >> 1);
	for (; startX > gridBounds.left; --startColumn)
		startX -= scaleX;
	startRow = g_briefingRuntime->mapCenter[1] / XW_BRIEFING_MAP_COORDINATE_SCALE;
	if (g_briefingRuntime->mapCenter[1] > 0 &&
		(g_briefingRuntime->mapCenter[1] & XW_BRIEFING_GRID_CELL_FRACTION_MASK) != 0)
		++startRow;
	scaleY = g_briefingRuntime->mapScale[1];
	startY = (scaleY * ((-g_briefingRuntime->mapCenter[1]) & XW_BRIEFING_GRID_CELL_FRACTION_MASK)) >>
			 XW_BRIEFING_GRID_CELL_FRACTION_BITS;
	startY += gridBounds.top + ((gridBounds.bottom - gridBounds.top) >> 1);
	for (; startY > gridBounds.top; --startRow)
		startY -= scaleY;
	gridX = startX;
	gridY = startY;
	column = startColumn;
	row = startRow;
	if (scaleX >= XW_BRIEFING_GRID_HALF_DETAIL_SCALE) {
		showAllMinorLines = scaleX >= XW_BRIEFING_GRID_FULL_DETAIL_SCALE;
		for (; gridX < gridBounds.right; gridX += g_briefingRuntime->mapScale[0], ++column) {
			if ((column & (XW_BRIEFING_GRID_MAJOR_INTERVAL - 1)) != 0 &&
				((column & (XW_BRIEFING_GRID_MAJOR_INTERVAL - 1)) == XW_BRIEFING_GRID_MAJOR_INTERVAL / 2 ||
				 showAllMinorLines))
				xpaint_Vert_Clipped_Line(gridX, gridBounds.top, gridBounds.bottom - gridBounds.top,
										 minorColor);
		}
		for (; gridY < gridBounds.bottom; gridY += g_briefingRuntime->mapScale[1], ++row) {
			if ((row & (XW_BRIEFING_GRID_MAJOR_INTERVAL - 1)) != 0 &&
				((row & (XW_BRIEFING_GRID_MAJOR_INTERVAL - 1)) == XW_BRIEFING_GRID_MAJOR_INTERVAL / 2 ||
				 showAllMinorLines))
				xpaint_Horiz_Clipped_Line(gridBounds.left, gridY, gridBounds.right - gridBounds.left,
										  minorColor);
		}
		gridX = startX;
		gridY = startY;
		column = startColumn;
		row = startRow;
	}
	for (; gridX < gridBounds.right; gridX += g_briefingRuntime->mapScale[0], ++column) {
		if ((column & (XW_BRIEFING_GRID_MAJOR_INTERVAL - 1)) == 0)
			xpaint_Vert_Clipped_Line(gridX, gridBounds.top, gridBounds.bottom - gridBounds.top, majorColor);
	}
	for (; gridY < gridBounds.bottom; gridY += g_briefingRuntime->mapScale[1], ++row) {
		if ((row & (XW_BRIEFING_GRID_MAJOR_INTERVAL - 1)) == 0)
			xpaint_Horiz_Clipped_Line(gridBounds.left, gridY, gridBounds.right - gridBounds.left, majorColor);
	}
	return 1;
}

// FUNCTION: XW 0x43BBD0
int16_t brief_DrawOverlays(const Rect* viewportRect, const Rect* clipRect) {
	int16_t positionSetIndex =
		g_briefingRuntime->scriptPositionSetIndex[g_briefingRuntime->activeScriptIndex];
	Rect viewport;
	Rect highlightRect;
	char text[XW_BRIEF_READOUT_CAPACITY];
	int16_t x, y, offsetX, offsetY;
	int markerIndex;
	int16_t iconIndex;
	int labelIndex;
	int markersRemaining;
	int labelsRemaining;
	xrect_Copy_Rect(&viewport, (Rect*)viewportRect);
	for (markerIndex = 0, markersRemaining = sizeof(g_briefingRuntime->fgMarkerActive) /
											 sizeof(g_briefingRuntime->fgMarkerActive[0]);
		 markersRemaining != 0; ++markerIndex, --markersRemaining) {
		if (g_briefingRuntime->fgMarkerActive[markerIndex] != 0) {
			int16_t markerIcon = g_briefingRuntime->fgMarkerIconIndex[markerIndex];
			brief_Map_To_Screen_Pos(&viewport, g_briefingRuntime->mapIconX[positionSetIndex][markerIcon],
									g_briefingRuntime->mapIconY[positionSetIndex][markerIcon], &x, &y);
			xrect_Set_Rect(&highlightRect, x - XW_BRIEFING_MARKER_RADIUS, y - XW_BRIEFING_MARKER_RADIUS,
						   x + XW_BRIEFING_MARKER_RADIUS + 1, y + XW_BRIEFING_MARKER_RADIUS + 1);
			brief_DrawCraftIconHighlight(&highlightRect, g_briefingRuntime->fgMarkerAge[markerIndex]);
		}
	}
	for (iconIndex = 0; iconIndex < g_briefingRuntime->mapIconCount; ++iconIndex) {
		int16_t state = g_briefingRuntime->mapIconType[iconIndex];
		int16_t colorOverride = g_briefingRuntime->mapIconColorOverride[iconIndex];
		int16_t mapX = g_briefingRuntime->mapIconX[positionSetIndex][iconIndex];
		int16_t mapY = g_briefingRuntime->mapIconY[positionSetIndex][iconIndex];
		int16_t group;
		--state;
		if (state <= XW_BRIEFING_MAP_ICON_BASE_COUNT - 1) {
			if (colorOverride == 0)
				group = g_briefingMapDefaultIconColorGroups[state];
			else
				group = colorOverride - 1;
		} else
			group = 0;
		if (group < 0)
			group = 0;
		if (group > XW_BRIEFING_MAP_ICON_ACTOR_COUNT - 1)
			group = XW_BRIEFING_MAP_ICON_ACTOR_COUNT - 1;
		if (g_briefingRuntime->mapScale[0] < XW_BRIEFING_MAP_ICON_LOW_ZOOM_THRESHOLD) {
			if (group != 0)
				state += XW_BRIEFING_MAP_ICON_BASE_COUNT;
			else
				state += XW_BRIEFING_MAP_ICON_GROUP0_LOW_ZOOM_OFFSET;
		}
		if (state >= 0) {
			Actor* actor;
			brief_Map_To_Screen_Pos(&viewport, mapX, mapY, &x, &y);
			xactor_Set_Actor_State(g_briefingMapIconActors[group], state, 0);
			xactanim_Get_Anim_Actor_Offset(g_briefingMapIconActors[group], &offsetX, &offsetY);
			actor = g_briefingMapIconActors[group];
			x -= offsetX + (actor->w >> 1);
			y -= offsetY + (actor->h >> 1);
			xactanim_Draw_Anim_Actor(actor, (Rect*)viewportRect, (Rect*)clipRect, x, y, 1);
		}
	}
	for (labelIndex = 0,
		labelsRemaining = sizeof(g_briefingRuntime->labelActive) / sizeof(g_briefingRuntime->labelActive[0]);
		 labelsRemaining != 0; ++labelIndex, --labelsRemaining) {
		if (g_briefingRuntime->labelActive[labelIndex] != 0) {
			int16_t textIndex = g_briefingRuntime->labelTextIndex[labelIndex];
			brief_Map_To_Screen_Pos(&viewport, g_briefingRuntime->labelX[labelIndex],
									g_briefingRuntime->labelY[labelIndex], &x, &y);
			strcpy(text, (const char*)xmemhdl_Lock_Handle(g_briefingRuntime->labelTextHandles[textIndex]));
			nullsub_SharedNoOp();
			brief_Draw_Double_Readout_Text(text, 0, x, y, g_briefingRuntime->labelAge[labelIndex]);
		}
	}
	return 1;
}

// FUNCTION: XW 0x43BEC0
void brief_DrawBackground(void) {
	Rect canvasBounds;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	if (g_briefingRuntime->mapColorMode != 0) {
		xpaint_Paint_Clipped_Rect(&canvasBounds, XW_BRIEFING_MAP_BACKGROUND_COLOR);
	} else {
		xpaint_Paint_Clipped_Rect(&canvasBounds, XW_BRIEFING_STARS_BACKGROUND_COLOR);
		if (g_briefingStarsActor != NULL) {
			xactdelt_Draw_Delta_Actor(g_briefingStarsActor, &canvasBounds, &canvasBounds, 0, 0, 1);
		}
	}
}

// FUNCTION: XW 0x43BF30
void brief_Draw_Map_Paragraph(const Rect* rect, LandruHandle textHandle, LandruHandle attributeHandle,
							  int16_t suppressCenteredHeadings, int16_t textSlotIndex) {
	Rect lineBounds;
	char lineBuffer[PLAYER_PARAGRAPH_BUFFER_SIZE];
	const unsigned char* textBytes;
	const unsigned char* attributeBytes;
	int16_t availableWidth;
	int16_t scanLineStart;
	int16_t lineStart;
	int16_t lineEnd;
	int16_t nextLineStart;
	int scanIndex;
	int lastSpaceIndex;
	int16_t finished;
	xrect_Copy_Rect(&lineBounds, (Rect*)rect);
	scanLineStart = 0;
	if (g_briefingRuntime->mapViewportActive != 0 && textSlotIndex == XW_BRIEFING_MAP_PARAGRAPH_SLOT)
		lineBounds.left += XW_BRIEFING_MAP_PARAGRAPH_INSET;
	lineBounds.bottom = lineBounds.top + PLAYER_PARAGRAPH_LINE_HEIGHT;
	lineBuffer[0] = 0;
	availableWidth = lineBounds.right - lineBounds.left;
	textBytes = xmemhdl_Lock_Handle(textHandle);
	attributeBytes = xmemhdl_Lock_Handle(attributeHandle);
	lineEnd = PLAYER_PARAGRAPH_NO_LINE;
	lineStart = PLAYER_PARAGRAPH_NO_LINE;
	scanIndex = 0;
	nextLineStart = 0;
	lastSpaceIndex = 0;
	finished = 0;
	xfont_Enable_FontID_Shadow(PLAYER_PARAGRAPH_FONT);
	xfont_Set_FontID_Bold_Color(PLAYER_PARAGRAPH_FONT, PLAYER_PARAGRAPH_BOLD_COLOR);
	do {
		int characterIndex = (int16_t)scanIndex;
		unsigned char character = textBytes[characterIndex];
		if (character == '$' || character == 0) {
			lineStart = scanLineStart;
			if (suppressCenteredHeadings == 0 && character == '$')
				lineEnd = (int16_t)(scanIndex - 1);
			else
				lineEnd = (int16_t)scanIndex;
			++scanIndex;
			nextLineStart = (int16_t)scanIndex;
			if (textBytes[(int16_t)scanIndex] == 0)
				finished = 1;
			scanLineStart = nextLineStart;
		} else {
			if (character == ' ')
				lastSpaceIndex = scanIndex;
			if (isspace(textBytes[characterIndex]) == 0) {
				for (;;) {
					characterIndex = (int16_t)scanIndex;
					if (isspace(textBytes[characterIndex]))
						break;
					character = textBytes[characterIndex];
					if (character == '$' || character == 0)
						break;
					++scanIndex;
					lineBuffer[characterIndex - scanLineStart] = (char)character;
				}
			} else {
				++scanIndex;
				lineBuffer[characterIndex - scanLineStart] = (char)textBytes[characterIndex];
			}
			lineBuffer[(int16_t)scanIndex - scanLineStart] = 0;
			if (xfont_Get_String_Width_0(PLAYER_PARAGRAPH_FONT, lineBuffer) >= (int16_t)availableWidth) {
				lineStart = scanLineStart;
				lineEnd = (int16_t)lastSpaceIndex;
				scanIndex = lastSpaceIndex + 1;
				nextLineStart = (int16_t)scanIndex;
				scanLineStart = nextLineStart;
			}
		}
		if (lineStart != PLAYER_PARAGRAPH_NO_LINE) {
			int outputIndex = 0;
			int inputIndex;
			uint16_t currentAttribute = 0;
			lineBuffer[0] = 0;
			for (inputIndex = lineStart; inputIndex <= lineEnd; ++inputIndex) {
				if (attributeBytes[inputIndex] != currentAttribute) {
					currentAttribute = attributeBytes[inputIndex];
					if (currentAttribute != 0)
						lineBuffer[(int16_t)outputIndex] = PLAYER_PARAGRAPH_BOLD_CONTROL;
					else
						lineBuffer[(int16_t)outputIndex] = PLAYER_PARAGRAPH_NORMAL_CONTROL;
					++outputIndex;
				}
				lineBuffer[(int16_t)outputIndex] = (char)textBytes[inputIndex];
				++outputIndex;
			}
			lineBuffer[(int16_t)outputIndex] = 0;
			if (suppressCenteredHeadings == 0 && lineBuffer[0] == '>')
				xfont_Print_Centered_Text(&lineBuffer[1], &lineBounds, PLAYER_PARAGRAPH_FONT,
										  PLAYER_PARAGRAPH_CENTER_COLOR);
			else
				xfont_Print_Clipped_Text(lineBuffer, lineBounds.left + PLAYER_PARAGRAPH_INSET,
										 lineBounds.top + PLAYER_PARAGRAPH_INSET, PLAYER_PARAGRAPH_FONT,
										 PLAYER_PARAGRAPH_TEXT_COLOR);
			lineEnd = PLAYER_PARAGRAPH_NO_LINE;
			lineStart = PLAYER_PARAGRAPH_NO_LINE;
			xrect_Offset_Rect(&lineBounds, 0, PLAYER_PARAGRAPH_LINE_HEIGHT);
		}
	} while (finished == 0);
	xfont_Disable_FontID_Shadow(PLAYER_PARAGRAPH_FONT);
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x43C230
void brief_Draw_Double_Readout_Text(const char* text, uint16_t fontId, int16_t x, int16_t y,
									int16_t revealCount) {
	if (revealCount >= 0)
		brief_Draw_Readout_Text(text, fontId, x, y, revealCount * XW_BRIEF_READOUT_DOUBLE_RATE);
}

// FUNCTION: XW 0x43C260
void brief_Draw_Readout_Text(const char* text, uint16_t fontId, int16_t x, int16_t y, int16_t revealCount) {
	Rect cursorRect;
	char revealedText[XW_BRIEF_READOUT_CAPACITY];
	int16_t textLength;
	int16_t revealEndIndex;
	int16_t remainingRampSteps;
	int16_t revealedWidth;
	int16_t rampIndex;
	if (revealCount >= 0) {
		textLength = (int16_t)strlen(text);
		strcpy(revealedText, text);
		if (revealCount < textLength + XW_BRIEF_READOUT_SETTLE_STEPS) {
			if (revealCount <= textLength) {
				revealedText[revealCount] = '\0';
				revealEndIndex = revealCount;
			} else {
				revealEndIndex = textLength;
			}
			remainingRampSteps = textLength - revealCount + 1;
			if (remainingRampSteps > XW_BRIEF_READOUT_RAMP_STEPS)
				remainingRampSteps = XW_BRIEF_READOUT_RAMP_STEPS;
			revealedWidth = xfont_Get_String_Width_0(fontId, revealedText);
			for (rampIndex = XW_BRIEF_READOUT_RAMP_STEPS - remainingRampSteps;
				 rampIndex <= XW_BRIEF_READOUT_RAMP_STEPS; ++rampIndex, --revealEndIndex) {
				if (revealEndIndex >= 0)
					revealedText[revealEndIndex] = '\0';
				xfont_Print_Clipped_Text(revealedText, x, y, fontId,
										 rampIndex + XW_BRIEF_READOUT_FIRST_COLOR);
			}
			xrect_Set_Rect(&cursorRect, revealedWidth + x + XW_BRIEF_READOUT_CURSOR_GAP, y,
						   revealedWidth + x + XW_BRIEF_READOUT_CURSOR_GAP + XW_BRIEF_READOUT_CURSOR_SIZE,
						   y + XW_BRIEF_READOUT_CURSOR_SIZE);
			if (revealCount < textLength)
				xpaint_Paint_Clipped_Rect(&cursorRect, XW_BRIEF_READOUT_FINAL_COLOR);
		} else {
			strcpy(revealedText, text);
			xfont_Print_Clipped_Text(revealedText, x, y, fontId, XW_BRIEF_READOUT_FINAL_COLOR);
		}
	}
}

// FUNCTION: XW 0x43C3D0
void brief_DrawCraftIconHighlight(const Rect* iconRect, int16_t highlightPhase) {
	Rect highlightRect;
	int16_t frameColor;
	int16_t frameCount;
	/* Shared Landru declares the read-only source parameter without const. */
	xrect_Copy_Rect(&highlightRect, (Rect*)iconRect);
	if (highlightPhase == XW_BRIEF_HIGHLIGHT_SOUND_PHASE && g_briefingRuntime->playbackActive != 0)
		brief_HandleSoundAction(XW_BRIEF_SOUND_TARGET);
	if (highlightPhase < XW_BRIEF_HIGHLIGHT_FILLED_PHASE) {
		if (highlightPhase < XW_BRIEF_HIGHLIGHT_MOVE_PHASE) {
			xrect_Inset_Rect(&highlightRect, -XW_BRIEF_HIGHLIGHT_INITIAL_OUTSET,
							 -XW_BRIEF_HIGHLIGHT_INITIAL_OUTSET);
			frameColor = XW_BRIEF_HIGHLIGHT_INITIAL_COLOR - XW_BRIEF_HIGHLIGHT_COLOR_STEP * highlightPhase;
			frameCount = highlightPhase + 1;
		} else if (highlightPhase < XW_BRIEF_HIGHLIGHT_SHRINK_PHASE) {
			xrect_Inset_Rect(&highlightRect,
							 XW_BRIEF_HIGHLIGHT_RECT_STEP * highlightPhase -
								 XW_BRIEF_HIGHLIGHT_RECT_STEP * XW_BRIEF_HIGHLIGHT_LAST_OUTLINE_PHASE,
							 XW_BRIEF_HIGHLIGHT_RECT_STEP * highlightPhase -
								 XW_BRIEF_HIGHLIGHT_RECT_STEP * XW_BRIEF_HIGHLIGHT_LAST_OUTLINE_PHASE);
			frameColor = XW_BRIEF_HIGHLIGHT_OUTLINE_COLOR;
			frameCount = XW_BRIEF_HIGHLIGHT_FRAME_COUNT;
		} else {
			int16_t outset;
			frameCount = XW_BRIEF_HIGHLIGHT_LAST_OUTLINE_PHASE - highlightPhase;
			outset = XW_BRIEF_HIGHLIGHT_RECT_STEP * frameCount;
			xrect_Inset_Rect(&highlightRect, -outset, -outset);
			frameColor = XW_BRIEF_HIGHLIGHT_OUTLINE_COLOR;
		}
		if (frameCount != 0) {
			unsigned int framesRemaining = frameCount;
			do {
				xpaint_Frame_Clipped_Rect(&highlightRect, frameColor);
				xrect_Inset_Rect(&highlightRect, XW_BRIEF_HIGHLIGHT_RECT_STEP, XW_BRIEF_HIGHLIGHT_RECT_STEP);
				frameColor += XW_BRIEF_HIGHLIGHT_COLOR_STEP;
			} while (--framesRemaining != 0);
		}
	} else {
		xrect_Inset_Rect(&highlightRect, -XW_BRIEF_HIGHLIGHT_RECT_STEP, -XW_BRIEF_HIGHLIGHT_RECT_STEP);
		xpaint_Paint_Clipped_Rect(&highlightRect, XW_BRIEF_HIGHLIGHT_FILL_COLOR);
		if ((highlightPhase & 1) != 0 || highlightPhase > XW_BRIEF_HIGHLIGHT_LAST_BLINK_PHASE)
			xpaint_Frame_Clipped_Rect(&highlightRect, XW_BRIEF_HIGHLIGHT_FRAME_COLOR);
	}
}

// FUNCTION: XW 0x43C500
void brief_InitRuntime(int16_t reuseTextBuffers) {
	int16_t index, layout, viewport;
	g_briefingLoadedNarrationPage = -1;
	if (g_briefingNarrationSound != NULL) {
		xsound_Free_Sound(g_briefingNarrationSound);
		g_briefingNarrationSound = NULL;
	}
	for (index = 0; index < XW_BRIEFING_PAGE_LABEL_CAPACITY; ++index) {
		if (reuseTextBuffers != 0) {
			LandruHandle handle = g_briefingRuntime->labelTextHandles[index];
			if (handle != LANDRU_NULL_HANDLE)
				xmemhdl_Handle_nSet(handle, 0, XW_BRIEF_LABEL_BUFFER_SIZE);
		} else {
			g_briefingRuntime->labelTextHandles[index] =
				xmemhdl_Alloc_Clear_Handle(XW_BRIEF_LABEL_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
		}
	}
	for (index = 0; index < XW_BRIEFING_TEXT_BUFFER_CAPACITY; ++index) {
		if (reuseTextBuffers != 0) {
			LandruHandle handle = g_briefingRuntime->textBlockHandles[index];
			if (handle != LANDRU_NULL_HANDLE)
				xmemhdl_Handle_nSet(handle, 0, XW_BRIEFING_TEXT_BUFFER_SIZE);
			handle = g_briefingRuntime->textAttributeHandles[index];
			if (handle != LANDRU_NULL_HANDLE)
				xmemhdl_Handle_nSet(handle, 0, XW_BRIEFING_TEXT_BUFFER_SIZE);
		} else {
			g_briefingRuntime->textBlockHandles[index] =
				xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_TEXT_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
			g_briefingRuntime->textAttributeHandles[index] =
				xmemhdl_Alloc_Clear_Handle(XW_BRIEFING_TEXT_BUFFER_SIZE, LANDRU_MEMORY_RESOURCE);
		}
	}
	g_briefingRuntime->playbackActive = 1;
	g_briefingRuntime->field_00C2 = 0;
	g_briefingRuntime->mapIconCount = 0;
	g_briefingRuntime->field_00C6 = 0;
	g_briefingRuntime->mapPositionSetCount = 1;
	g_briefingRuntime->field_1D66 = 0;
	g_briefingRuntime->layoutCount = 1;
	for (layout = 0; layout < XW_BRIEFING_LAYOUT_COUNT; ++layout) {
		for (viewport = 0; viewport < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewport) {
			if (viewport != XW_BRIEFING_LAYOUT_MAP_VIEWPORT || layout != 0) {
				xrect_Clear_Rect(&g_briefingRuntime->layoutRects[layout][viewport]);
				g_briefingRuntime->layoutActive[layout][viewport] = 0;
			} else {
				xrect_Set_Rect(&g_briefingRuntime->layoutRects[0][XW_BRIEFING_LAYOUT_MAP_VIEWPORT], 0, 0,
							   XW_BRIEF_DEFAULT_MAP_RIGHT, XW_BRIEF_DEFAULT_MAP_BOTTOM);
				g_briefingRuntime->layoutActive[0][XW_BRIEFING_LAYOUT_MAP_VIEWPORT] = 1;
			}
		}
	}
	g_briefingRuntime->activeScriptIndex = 0;
	g_briefingRuntime->scriptCount = 1;
	brief_InitDefaultScript(0);
	g_briefingRuntime->scriptPositionSetIndex[0] = 0;
	g_briefingRuntime->scriptLayoutIndex[0] = 0;
	g_briefingRuntime->mapCenter[0] = 0;
	g_briefingRuntime->mapCenter[1] = 0;
	g_briefingRuntime->mapTargetCenter[0] = 0;
	g_briefingRuntime->mapTargetCenter[1] = 0;
	g_briefingRuntime->mapScale[0] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapScale[1] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[0] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapTargetScale[1] = XW_BRIEF_RUNTIME_MAP_SCALE;
	g_briefingRuntime->mapViewportActive = 0;
	g_briefingRuntime->field_37DC = 0;
	for (index = 0; index < (int)(sizeof(g_briefingRuntime->textViewportActive) /
								  sizeof(g_briefingRuntime->textViewportActive[0]));
		 ++index)
		g_briefingRuntime->textViewportActive[index] = 0;
	for (index = 0; index < (int)(sizeof(g_briefingRuntime->textSlotActive) /
								  sizeof(g_briefingRuntime->textSlotActive[0]));
		 ++index)
		g_briefingRuntime->textSlotActive[index] = 0;
	for (index = 0; index < (int)(sizeof(g_briefingRuntime->fgMarkerActive) /
								  sizeof(g_briefingRuntime->fgMarkerActive[0]));
		 ++index)
		g_briefingRuntime->fgMarkerActive[index] = 0;
	for (index = 0;
		 index < (int)(sizeof(g_briefingRuntime->labelActive) / sizeof(g_briefingRuntime->labelActive[0]));
		 ++index)
		g_briefingRuntime->labelActive[index] = 0;
	g_briefingRuntime->textSlotActive[0] = 1;
	g_briefingRuntime->textSlotBlockIndex[0] = 1;
	brief_ApplyLayout(g_briefingRuntime->scriptLayoutIndex[g_briefingRuntime->activeScriptIndex]);
}

// FUNCTION: XW 0x43C860
void brief_Free_Display_Map(void) {
	uint16_t index;
	for (index = 0; index != sizeof(g_briefingRuntime->labelTextHandles) / sizeof(LandruHandle); ++index) {
		LandruHandle handle = g_briefingRuntime->labelTextHandles[index];
		if (handle != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(handle);
		}
	}
	for (index = 0; index != sizeof(g_briefingRuntime->textBlockHandles) / sizeof(LandruHandle); ++index) {
		LandruHandle textHandle = g_briefingRuntime->textBlockHandles[index];
		LandruHandle attributeHandle;
		if (textHandle != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(textHandle);
		}
		attributeHandle = g_briefingRuntime->textAttributeHandles[index];
		if (attributeHandle != LANDRU_NULL_HANDLE) {
			xmemhdl_Free_Handle(attributeHandle);
		}
	}
}

// FUNCTION: XW 0x43C8D0
void brief_InitDefaultScript(int16_t scriptIndex) {
	g_briefingRuntime->scriptWords[scriptIndex][0] = XW_BRIEF_SENTINEL_TIME;
	g_briefingRuntime->scriptWords[scriptIndex][1] = XW_BRIEF_COMMAND_END;
	g_briefingRuntime->scriptEndTime[scriptIndex] = XW_BRIEF_DEFAULT_END_TIME;
	g_briefingRuntime->scriptCurrentTime[scriptIndex] = 0;
	g_briefingRuntime->scriptCursorWordIndex[scriptIndex] = 0;
	g_briefingRuntime->scriptWordCount[scriptIndex] = XW_BRIEF_COMMAND_HEADER_WORDS;
	g_briefingRuntime->scriptPositionSetIndex[scriptIndex] = 0;
	g_briefingRuntime->scriptLayoutIndex[scriptIndex] = 0;
	brief_Rewind_Page(scriptIndex);
}

// FUNCTION: XW 0x43C970
void brief_ApplyLayout(int16_t layoutIndex) {
	int16_t viewportIndex;
	for (viewportIndex = 0; viewportIndex < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewportIndex) {
		if (viewportIndex != XW_BRIEFING_LAYOUT_MAP_VIEWPORT) {
			xrect_Copy_Rect(&g_briefingRuntime->textViewportRects[viewportIndex],
							&g_briefingRuntime->layoutRects[layoutIndex][viewportIndex]);
			xrect_Scale_Rect(&g_briefingRuntime->textViewportRects[viewportIndex], XW_BRIEFING_LAYOUT_SCALE,
							 XW_BRIEFING_LAYOUT_SCALE);
			g_briefingRuntime->textViewportActive[viewportIndex] =
				g_briefingRuntime->layoutActive[layoutIndex][viewportIndex];
		} else {
			xrect_Copy_Rect(&g_briefingRuntime->mapViewportRect,
							&g_briefingRuntime->layoutRects[layoutIndex][XW_BRIEFING_LAYOUT_MAP_VIEWPORT]);
			xrect_Scale_Rect(&g_briefingRuntime->mapViewportRect, XW_BRIEFING_LAYOUT_SCALE,
							 XW_BRIEFING_LAYOUT_SCALE);
			g_briefingRuntime->mapViewportActive =
				g_briefingRuntime->layoutActive[layoutIndex][XW_BRIEFING_LAYOUT_MAP_VIEWPORT];
		}
	}
	if (g_briefingRuntime->mapViewportActive != 0) {
		int16_t top = g_briefingRuntime->textViewportRects[0].top;
		int16_t mapHeight;
		if (g_briefingRuntime->textViewportRects[0].bottom - top < XW_BRIEFING_TEXT_MINIMUM_HEIGHT) {
			g_briefingRuntime->textViewportRects[0].bottom = top + XW_BRIEFING_TEXT_MINIMUM_HEIGHT;
		}
		mapHeight = g_briefingRuntime->mapViewportRect.bottom - g_briefingRuntime->mapViewportRect.top;
		g_briefingRuntime->mapViewportRect.top = g_briefingRuntime->textViewportRects[0].bottom + 1;
		g_briefingRuntime->mapViewportRect.bottom = mapHeight + g_briefingRuntime->mapViewportRect.top;
		g_briefingRuntime->textViewportRects[1].top = g_briefingRuntime->mapViewportRect.bottom + 1;
		g_briefingRuntime->textViewportRects[1].bottom = XW_BRIEFING_VIEWPORT_BOTTOM;
	} else {
		/* The original first write changes the third active flag, which is tested later. */
		if (g_briefingRuntime->textViewportActive[0] == 0) {
			g_briefingRuntime->textViewportActive[2] = XW_BRIEFING_VIEWPORT_BOTTOM;
		}
		for (viewportIndex = 1; viewportIndex < XW_BRIEFING_LAYOUT_MAP_VIEWPORT; ++viewportIndex) {
			if (g_briefingRuntime->textViewportActive[viewportIndex] == 0) {
				g_briefingRuntime->textViewportRects[viewportIndex - 1].bottom = XW_BRIEFING_VIEWPORT_BOTTOM;
			}
		}
	}
}

// FUNCTION: XW 0x43CB10
int brief_Find_Ship_On_Screen(const Rect* viewportRect, int16_t mouseX, int16_t mouseY,
							  int16_t* outIconIndex) {
	int16_t iconIndex;
	int16_t bestDistance = XW_BRIEFING_INITIAL_PICK_DISTANCE;
	int16_t projectedX;
	int16_t projectedY;
	*outIconIndex = 0;
	for (iconIndex = 0; iconIndex < g_briefingRuntime->mapIconCount; ++iconIndex) {
		if (g_briefingRuntime->mapIconType[iconIndex] <= XW_BRIEFING_LAST_SHIP_ICON_TYPE) {
			int positionSet = g_briefingRuntime->scriptPositionSetIndex[g_briefingRuntime->activeScriptIndex];
			int distanceX;
			brief_Map_To_Screen_Pos(viewportRect, g_briefingRuntime->mapIconX[positionSet][iconIndex],
									g_briefingRuntime->mapIconY[positionSet][iconIndex], &projectedX,
									&projectedY);
			distanceX = abs(mouseX - projectedX);
			if (distanceX < bestDistance) {
				int distanceY = abs(mouseY - projectedY);
				if (distanceY < bestDistance) {
					bestDistance = distanceY;
					if (distanceX >= distanceY) {
						bestDistance = distanceX;
					}
					*outIconIndex = iconIndex;
				}
			}
		}
	}
	return bestDistance < XW_BRIEFING_PICK_DISTANCE_LIMIT;
}

// FUNCTION: XW 0x43CBF0
void brief_Map_To_Screen_Pos(const Rect* viewportRect, int16_t mapX, int16_t mapY, int16_t* outX,
							 int16_t* outY) {
	*outX = (mapX - g_briefingRuntime->mapCenter[0]) * g_briefingRuntime->mapScale[0] /
			XW_BRIEFING_MAP_COORDINATE_SCALE;
	*outX += viewportRect->left + ((viewportRect->right - viewportRect->left) >> 1);
	*outY = (mapY - g_briefingRuntime->mapCenter[1]) * g_briefingRuntime->mapScale[1] /
			XW_BRIEFING_MAP_COORDINATE_SCALE;
	*outY += viewportRect->top + ((viewportRect->bottom - viewportRect->top) >> 1);
}

// FUNCTION: XW 0x43CC90
int16_t brief_Move_To_Value(int16_t current, int16_t target, int16_t step) {
	if (current > target) {
		current -= step;
		if (current < target) {
			current = target;
		}
	}
	if (current < target) {
		current += step;
		if (current > target) {
			current = target;
		}
	}
	return current;
}

// FUNCTION: XW 0x43CCC0
void brief_SetOfficerRegionRestored(int16_t restore) {
	if (restore != g_briefingOfficerRegionRestored) {
		Rect bufferRect;
		int16_t left, top, bufferWidth, bufferHeight;
		uint8_t* buffer;
		xrect_Copy_Rect(&bufferRect, &g_briefingOfficerRect);
		xrect_Origin_Rect(&bufferRect);
		left = g_briefingOfficerRect.left;
		top = g_briefingOfficerRect.top;
		bufferWidth = bufferRect.right;
		bufferHeight = bufferRect.bottom;
		buffer = xmemhdl_Lock_Handle(g_briefingOfficerRegionBuffer);
		if (restore != 0) {
			stub_Copy_From_Clipped_Buffer(buffer, &bufferRect, left, top, bufferWidth, bufferHeight);
		} else {
			stub_Copy_To_Clipped_Buffer(buffer, &bufferRect, left, top, bufferWidth, bufferHeight);
		}
		xmemhdl_Unlock_Handle(g_briefingOfficerRegionBuffer);
		g_briefingOfficerRegionRestored = restore;
	}
}

// FUNCTION: XW 0x43CD70
int16_t brief_LoadPilotRecord(void) {
	char pilotPath[XW_BRIEFING_PILOT_FILENAME_CAPACITY];
	LandruFile* stream;
	strcpy(pilotPath, g_RegisterShellPilot.name);
	strcat(pilotPath, ".PLT");
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "rb");
	if (stream != NULL) {
		register_ReadPilotRecord(stream, &g_briefingPilotRecord);
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x43CE20
void brief_WritePilotRecord(void) {
	char pilotPath[XW_BRIEFING_PILOT_PATH_CAPACITY];
	LandruFile* stream;
	strcpy(pilotPath, g_RegisterShellPilot.name);
	strcat(pilotPath, ".PLT");
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "wb");
	if (stream != NULL) {
		xfile_Write_Data_To_File(stream, &g_briefingPilotRecord, sizeof(g_briefingPilotRecord));
		xfile_Close_File(stream);
	}
}

// FUNCTION: XW 0x43CEC0
void brief_CommitMissionChoice(void) {
	g_briefingPilotRecord.selectedTourMission =
		g_briefingPilotRecord.briefingMissionChoices[g_briefingMissionChoice];
}

// FUNCTION: XW 0x43CEE0
void brief_LoadMissionChoice(int16_t choiceIndex, int16_t reuseTextBuffers) {
	ResFile* resource;
	LandruHandle paragraph;
	int16_t selectedMission;
	char tourName[XW_BRIEF_TOUR_RESOURCE_NAME_CAPACITY];
	g_briefingMissionChoice = choiceIndex;
#ifdef XW_MODERN
	resource = XwLandru_OpenMissionResource(":X-Wing Data\\Resource\\missions.lfd");
#else
	resource = xres_Open_Resource(":X-Wing Data\\Resource\\missions.lfd");
#endif
	if (resource == NULL)
#ifdef XW_MODERN
		resource = XwLandru_OpenMissionResource("missions.lfd");
#else
		resource = xres_Open_Resource("missions.lfd");
#endif
	sprintf(tourName, "tour%d", g_briefingPilotRecord.current_tour + 1);
	selectedMission = g_briefingPilotRecord.briefingMissionChoices[choiceIndex];
	g_briefingPilotRecord.selectedTourMission = selectedMission;
	paragraph = xparagrp_Res_Paragraph(resource, tourName);
	xparagrp_Get_Paragraph_String(paragraph, g_shellMissionName, 0, selectedMission);
	xparagrp_Free_Paragraph(paragraph);
	xres_Close_Resource(resource);
	brief_InitRuntime(reuseTextBuffers);
	brief_LoadBriefingFile(g_shellMissionName);
	brief_Rewind_Page(g_briefingRuntime->activeScriptIndex);
}

// FUNCTION: XW 0x43CFB0
int brief_LoadBriefingFile(const char* missionName) {
	uint16_t allowFallback;
	LandruFile* stream;
	char resolvedPath[XW_BRIEFING_MISSION_PATH_CAPACITY];
	char logicalPath[XW_BRIEFING_MISSION_PATH_CAPACITY];
	shipext_Get_Mission_Path(logicalPath, missionName, 0);
	if (logicalPath[0] == ';' || logicalPath[0] == '+') {
		strcpy(resolvedPath, "X-Wing Data\\");
		strcat(resolvedPath, &logicalPath[1]);
		allowFallback = 1;
	} else {
		strcpy(resolvedPath, logicalPath);
		/* The original fallback flag is uninitialized for an unprefixed path. */
		allowFallback = 0;
	}
#ifdef XW_MODERN
	stream = XwStorage_OpenMission(resolvedPath);
#else
	stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
#endif
	if (stream == NULL) {
		if (allowFallback != 0) {
			if (logicalPath[0] == ';') {
				strcpy(resolvedPath, "c:\\XwingCD\\");
				strcat(resolvedPath, &logicalPath[1]);
				resolvedPath[0] = (char)g_installDriveLetter;
			} else {
				strcpy(resolvedPath, &logicalPath[1]);
			}
#ifdef XW_MODERN
			stream = XwStorage_OpenMission(resolvedPath);
#else
			stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
#endif
		}
	}
	if (stream != NULL) {
		int16_t layoutIndex;
		uint16_t layoutCount;
		uint16_t discardedHeaderWord;
		xfile_Read_Word_From_File(stream, &discardedHeaderWord);
		brief_ReadIconData(stream);
		xfile_Read_Word_From_File(stream, &layoutCount);
		g_briefingRuntime->layoutCount = layoutCount;
		for (layoutIndex = 0; layoutIndex < g_briefingRuntime->layoutCount; ++layoutIndex) {
			brief_ReadLayout(stream, layoutIndex);
		}
		brief_ReadScripts(stream);
		brief_ReadExtendedIconData(stream);
		brief_ReadTextBuffers(stream);
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x43D1E0
void brief_ReadScripts(XwFile* stream) {
	uint16_t scriptCount;
	uint16_t wordCount;
	uint16_t endTime;
	uint16_t positionSetIndex;
	uint16_t layoutIndex;
	int16_t scriptIndex;

	xfile_Read_Word_From_File(stream, &scriptCount);
	g_briefingRuntime->activeScriptIndex = 0;
	g_briefingRuntime->scriptCount = scriptCount;
	for (scriptIndex = 0; scriptIndex < (int16_t)scriptCount; ++scriptIndex) {
		xfile_Read_Word_From_File(stream, &endTime);
		xfile_Read_Word_From_File(stream, &wordCount);
		xfile_Read_Word_From_File(stream, &positionSetIndex);
		xfile_Read_Word_From_File(stream, &layoutIndex);
		g_briefingRuntime->scriptEndTime[scriptIndex] = endTime;
		g_briefingRuntime->scriptWordCount[scriptIndex] = wordCount;
		g_briefingRuntime->scriptPositionSetIndex[scriptIndex] = positionSetIndex;
		g_briefingRuntime->scriptLayoutIndex[scriptIndex] = layoutIndex;
		xfile_Read_Data_From_File(stream, g_briefingRuntime->scriptWords[scriptIndex],
								  (int16_t)wordCount *
									  (int)sizeof(g_briefingRuntime->scriptWords[scriptIndex][0]));
	}
	brief_ApplyLayout(g_briefingRuntime->scriptLayoutIndex[g_briefingRuntime->activeScriptIndex]);
}

// FUNCTION: XW 0x43D300
void brief_ReadIconData(XwFile* stream) {
	uint16_t iconCount, positionSetCount, value;
	int16_t positionSetIndex, iconIndex;
	xfile_Read_Word_From_File(stream, &iconCount);
	xfile_Read_Word_From_File(stream, &positionSetCount);
	g_briefingRuntime->field_00C2 = 0;
	g_briefingRuntime->mapIconCount = iconCount;
	g_briefingRuntime->field_00C6 = 0;
	g_briefingRuntime->mapPositionSetCount = positionSetCount;
	for (positionSetIndex = 0; positionSetIndex < (int16_t)positionSetCount; ++positionSetIndex) {
		for (iconIndex = 0; iconIndex < (int16_t)iconCount; ++iconIndex) {
			xfile_Read_Word_From_File(stream, &value);
			g_briefingRuntime->mapIconX[positionSetIndex][iconIndex] = value;
			xfile_Read_Word_From_File(stream, &value);
			g_briefingRuntime->mapIconY[positionSetIndex][iconIndex] = value;
			xfile_Read_Word_From_File(stream, &value);
			g_briefingRuntime->mapIconZ[positionSetIndex][iconIndex] = value;
		}
	}
	for (iconIndex = 0; iconIndex < (int16_t)iconCount; ++iconIndex) {
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->mapIconType[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->mapIconColorOverride[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0232[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0282[iconIndex] = value;
		xfile_Read_Data_From_File(stream, g_briefingRuntime->mapIconName[iconIndex],
								  sizeof(g_briefingRuntime->mapIconName[iconIndex]));
		xfile_Read_Data_From_File(stream, g_briefingRuntime->field_0552[iconIndex],
								  sizeof(g_briefingRuntime->field_0552[iconIndex]));
		xfile_Read_Data_From_File(stream, g_briefingRuntime->field_07D2[iconIndex],
								  sizeof(g_briefingRuntime->field_07D2[iconIndex]));
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0A52[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0E62[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0EB2[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0F02[iconIndex] = value;
	}
}

// FUNCTION: XW 0x43D590
void brief_ReadLayout(XwFile* stream, int16_t layoutIndex) {
	Rect viewportRect;
	uint16_t active;
	int viewportIndex;
	for (viewportIndex = 0; viewportIndex < XW_BRIEFING_LAYOUT_VIEWPORT_COUNT; ++viewportIndex) {
		xfile_Read_Data_From_File(stream, &viewportRect, sizeof(viewportRect));
		xrect_Copy_Rect(&g_briefingRuntime->layoutRects[layoutIndex][viewportIndex], &viewportRect);
		xfile_Read_Word_From_File(stream, &active);
		g_briefingRuntime->layoutActive[layoutIndex][viewportIndex] = active;
	}
}

// FUNCTION: XW 0x43D610
void brief_ReadExtendedIconData(XwFile* stream) {
	uint16_t value;
	int16_t iconIndex;
	int16_t blockIndex;
	g_briefingRuntime->field_10E2 = 0;
	g_briefingRuntime->field_10E4 = 0;
	xfile_Read_Word_From_File(stream, &value);
	g_briefingRuntime->field_00CA = value;
	xfile_Read_Word_From_File(stream, &value);
	g_briefingRuntime->field_00CC = value;
	xfile_Read_Word_From_File(stream, &value);
	g_briefingRuntime->field_00D0 = value;
	xfile_Read_Word_From_File(stream, &value);
	g_briefingRuntime->mapColorMode = value;
	for (blockIndex = 0; blockIndex != XW_BRIEFING_EXTENDED_HEADER_BLOCK_COUNT; ++blockIndex)
		xfile_Read_Data_From_File(stream, g_briefingRuntime->extendedHeaderBlocks[blockIndex],
								  sizeof(g_briefingRuntime->extendedHeaderBlocks[blockIndex]));
	for (iconIndex = 0; iconIndex < g_briefingRuntime->mapIconCount; ++iconIndex) {
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0F52[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0FA2[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_0FF2[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1042[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1092[iconIndex] = value;
		xfile_Read_Data_From_File(stream, g_briefingRuntime->field_10E6[iconIndex],
								  sizeof(g_briefingRuntime->field_10E6[iconIndex]));
		xfile_Read_Data_From_File(stream, g_briefingRuntime->field_1316[iconIndex],
								  sizeof(g_briefingRuntime->field_1316[iconIndex]));
		xfile_Read_Data_From_File(stream, g_briefingRuntime->field_1546[iconIndex],
								  sizeof(g_briefingRuntime->field_1546[iconIndex]));
		xfile_Read_Data_From_File(stream, g_briefingRuntime->field_1776[iconIndex],
								  sizeof(g_briefingRuntime->field_1776[iconIndex]));
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_19A6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->mapIconPlayerShipFlag[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1A46[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1A96[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1AE6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1B36[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1B86[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1BD6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1C26[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1C76[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1CC6[iconIndex] = value;
		xfile_Read_Word_From_File(stream, &value);
		g_briefingRuntime->field_1D16[iconIndex] = value;
		if (g_briefingRuntime->mapIconPlayerShipFlag[iconIndex] != 0) {
			int16_t iconType = g_briefingRuntime->mapIconType[iconIndex];
			if (iconType == XW_BRIEFING_LAST_SHIP_ICON_TYPE) {
				shipext_Set_Mission_Ship(SHIPEXT_SHIP_BWING);
			} else {
				shipext_Set_Mission_Ship(iconType - 1);
			}
		}
	}
}

// FUNCTION: XW 0x43D9F0
void brief_ReadTextBuffers(XwFile* stream) {
	int16_t labelCount;
	int16_t textCount;
	int16_t length;
	int16_t labelIndex;
	int16_t textIndex;
	xfile_Read_Word_From_File(stream, (uint16_t*)&labelCount);
	for (labelIndex = 0; labelIndex < XW_BRIEFING_PAGE_LABEL_CAPACITY; ++labelIndex) {
		if (labelIndex < labelCount) {
			char* text;
			xfile_Read_Word_From_File(stream, (uint16_t*)&length);
			text = (char*)xmemhdl_Lock_Handle(g_briefingRuntime->labelTextHandles[labelIndex]);
			xfile_Read_Data_From_File(stream, text, length);
			text[length] = '\0';
			xmemhdl_Unlock_Handle(g_briefingRuntime->labelTextHandles[labelIndex]);
		} else {
			xmemhdl_Handle_nSet(g_briefingRuntime->labelTextHandles[labelIndex], 0, sizeof(char));
		}
	}
	xfile_Read_Word_From_File(stream, (uint16_t*)&textCount);
	for (textIndex = 0; textIndex < XW_BRIEFING_TEXT_BUFFER_CAPACITY; ++textIndex) {
		xmemhdl_Handle_nSet(g_briefingRuntime->textBlockHandles[textIndex], 0, XW_BRIEFING_TEXT_BUFFER_SIZE);
		xmemhdl_Handle_nSet(g_briefingRuntime->textAttributeHandles[textIndex], 0,
							XW_BRIEFING_TEXT_BUFFER_SIZE);
		if (textIndex < textCount) {
			char* text;
			uint8_t* attributes;
			xfile_Read_Word_From_File(stream, (uint16_t*)&length);
			text = (char*)xmemhdl_Lock_Handle(g_briefingRuntime->textBlockHandles[textIndex]);
			xfile_Read_Data_From_File(stream, text, length);
			text[length] = '\0';
			xmemhdl_Unlock_Handle(g_briefingRuntime->textBlockHandles[textIndex]);
			attributes = (uint8_t*)xmemhdl_Lock_Handle(g_briefingRuntime->textAttributeHandles[textIndex]);
			xfile_Read_Data_From_File(stream, attributes, length);
			xmemhdl_Unlock_Handle(g_briefingRuntime->textAttributeHandles[textIndex]);
		}
	}
}

// FUNCTION: XW 0x43DB80
void brief_LoadNarration(void) {
	char missionName[XW_BRIEFING_NARRATION_PATH_CAPACITY];
	char path[XW_BRIEFING_NARRATION_PATH_CAPACITY];
	int16_t index;
	LandruHandle data;
	strcpy(missionName, g_shellMissionName);
	for (index = 0; missionName[index]; index++) {
		if (missionName[index] == '.') {
			missionName[index] = 0;
			break;
		}
	}
#ifdef XW_MODERN
	if (shellext_Get_Cur_Scene() == XW_SCENE_BRIEFING_TOUR)
		sprintf(path, "talk/ackbar/%s.p%02d", missionName, g_briefingLoadedNarrationPage);
	else
		sprintf(path, "talk/dodonna/%s.p%02d", missionName, g_briefingLoadedNarrationPage);
#else
	if (shellext_Get_Cur_Scene() == XW_SCENE_BRIEFING_TOUR) {
		if (g_briefingLoadedNarrationPage < XW_BRIEFING_TWO_DIGIT_PAGE)
			sprintf(path, "c:\\XwingCD\\talk\\ackbar\\%s.p0%d", missionName, g_briefingLoadedNarrationPage);
		else
			sprintf(path, "c:\\XwingCD\\talk\\ackbar\\%s.p%d", missionName, g_briefingLoadedNarrationPage);
	} else if (g_briefingLoadedNarrationPage < XW_BRIEFING_TWO_DIGIT_PAGE) {
		sprintf(path, "c:\\XwingCD\\talk\\dodonna\\%s.p0%d", missionName, g_briefingLoadedNarrationPage);
	} else {
		sprintf(path, "c:\\XwingCD\\talk\\dodonna\\%s.p%d", missionName, g_briefingLoadedNarrationPage);
	}
	path[0] = g_installDriveLetter;
#endif
	data = xmemhdl_Alloc_Handle(XW_BRIEFING_NARRATION_BUFFER_SIZE, LANDRU_MEMORY_DEFAULT);
	if (data) {
#ifdef XW_MODERN
		LandruFile* stream = XwStorage_OpenInstallation(XwProfile_ActiveFrontend()->version, path);
#else
		LandruFile* stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, path, "rb");
#endif
		if (stream) {
			void* buffer = xmemhdl_Lock_Handle(data);
			xfile_Read_Data_From_File(stream, buffer, XW_BRIEFING_NARRATION_BUFFER_SIZE);
			xfile_Close_File(stream);
			xmemhdl_Unlock_Handle(data);
		} else {
			xmemhdl_Free_Handle(data);
			data = LANDRU_NULL_HANDLE;
		}
		if (data) {
			Sound* sound = xsound_Alloc_Sound(data, 0, XW_BRIEFING_NARRATION_BUFFER_SIZE);
#ifndef XW_MODERN
			const void* vocData;
			int16_t locks;
			int16_t remaining;
#endif
			if (!sound) {
				xmemhdl_Free_Handle(data);
				return;
			}
			xsound_Set_Sound_Name(sound, FOURCC_VOIC, missionName);
#ifndef XW_MODERN
			vocData = xmemhdl_Lock_Handle(data);
			locks = FlightDisplay_GetSurfaceLockCount();
			for (remaining = locks; remaining > 0; remaining--)
				FlightDisplay_UnlockSurface();
			Sound_LoadEffect(sound->res_name, sound->res_name, 1, vocData, XW_BRIEFING_NARRATION_BUFFER_SIZE);
			for (remaining = locks; remaining > 0; remaining--)
				FlightDisplay_LockSurface();
			xmemhdl_Unlock_Handle(data);
#endif
			xsound_Discard_Sound_Data(sound);
			sound->type = digitalSound;
			soundext_Start_Resource_Voice(sound);
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(sound, XW_BRIEFING_NARRATION_PRIORITY);
#else
			lolevel_ImSetParam(sound->res_name, XW_SOUND_PARAM_PRIORITY, XW_BRIEFING_NARRATION_PRIORITY);
#endif
			g_briefingNarrationSound = sound;
		}
	}
}

// FUNCTION: XW 0x43DD80
int16_t brief_PrepareLaunchAndSavePilot(void) {
	char pilotPath[XW_BRIEFING_PILOT_PATH_CAPACITY];
	LandruFile* stream;
	g_launchSelectedGroupIndex = 0;
	memset(g_launchCraftPilotTokens, UINT8_MAX, sizeof(g_launchCraftPilotTokens));
	g_launchGroupCount = 0;
	g_launchAssignedPilotCount = 0;
	brief_LoadLaunchFlightGroups();
	brief_AssignLocalPilotToPlayerCraft();
	brief_BuildLaunchPilotRoster();
	strcpy(pilotPath, g_RegisterShellPilot.name);
	strcat(pilotPath, ".PLT");
	g_briefingPilotRecord.combatShip = shipext_Get_Combat_Ship();
	g_briefingPilotRecord.combatMission = shipext_Get_Combat_Mission();
	stream = xfile_Open_File(LANDRU_FILE_ROOT_USER, pilotPath, "wb");
	if (stream != NULL) {
		xfile_Write_Data_To_File(stream, &g_briefingPilotRecord, sizeof(g_briefingPilotRecord));
		xfile_Write_Data_To_File(stream, &g_briefingPilotRecord, sizeof(g_briefingPilotRecord));
		xfile_Close_File(stream);
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x43DE90
void brief_BuildLaunchPilotRoster(void) {
	int16_t pilotCount = 0;
	int16_t groupIndex;
	memset(g_pilotSlotSkillValues, 0, sizeof(g_pilotSlotSkillValues));
	memset(g_pilotSlotCraftIndices, 0, sizeof(g_pilotSlotCraftIndices));
	memset(g_pilotSlotFlightGroupIndices, 0, sizeof(g_pilotSlotFlightGroupIndices));
	memset(g_PilotSlotNames, 0, sizeof(g_PilotSlotNames));
	for (groupIndex = 0; groupIndex < g_launchGroupCount; ++groupIndex) {
		int16_t craftIndex;
		int16_t craftCount = (int16_t)g_launchGroupCraftCounts[groupIndex];
		for (craftIndex = 0; craftIndex < craftCount; ++craftIndex) {
			if (g_launchCraftPilotTokens[groupIndex][craftIndex] == g_localPilotAssignmentToken) {
				g_pilotSlotCraftIndices[pilotCount] = (uint8_t)craftIndex;
				strcpy(g_PilotSlotNames[pilotCount], g_RegisterShellPilot.name);
				g_pilotSlotSkillValues[pilotCount] = g_RegisterShellPilot.skillValue;
				g_pilotSlotFlightGroupIndices[pilotCount] = (uint8_t)g_launchGroupMissionIndices[groupIndex];
				++pilotCount;
			}
		}
	}
}

// FUNCTION: XW 0x43DFE0
void brief_LoadLaunchFlightGroups(void) {
	char resolvedPath[XW_BRIEFING_MISSION_PATH_CAPACITY];
	XwMissionFlightGroup flightGroup;
	char logicalPath[XW_BRIEFING_MISSION_PATH_CAPACITY];
	LandruFile* stream;
	int16_t allowFallback;
	uint16_t missionGroupCount;
	uint16_t secondaryRecordCount;
	int missionGroupIndex;
	shipext_Get_Mission_Path(logicalPath, g_shellMissionName, 1);
	if (logicalPath[0] == ':') {
		strcpy(resolvedPath, "c:\\XwingCD\\");
		resolvedPath[0] = (char)g_installDriveLetter;
		strcat(resolvedPath, logicalPath);
		allowFallback = 0;
	} else if (logicalPath[0] == ';' || logicalPath[0] == '+') {
		strcpy(resolvedPath, "X-Wing Data\\");
		strcat(resolvedPath, &logicalPath[1]);
		allowFallback = 1;
	} else {
		strcpy(resolvedPath, logicalPath);
		/* The original leaves this flag uninitialized for ordinary paths. */
		allowFallback = 0;
	}
#ifdef XW_MODERN
	stream = XwStorage_OpenMission(resolvedPath);
#else
	stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
#endif
	if (stream == NULL) {
		if (allowFallback != 0) {
			if (logicalPath[0] == ';') {
				strcpy(resolvedPath, "c:\\XwingCD\\");
				strcat(resolvedPath, &logicalPath[1]);
				resolvedPath[0] = (char)g_installDriveLetter;
			} else {
				strcpy(resolvedPath, &logicalPath[1]);
			}
#ifdef XW_MODERN
			stream = XwStorage_OpenMission(resolvedPath);
#else
			stream = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, resolvedPath, "rb");
#endif
		}
	}
	if (stream != NULL) {
		xfile_Seek_File(stream,
						sizeof(XwMissionHeader) - sizeof(missionGroupCount) - sizeof(secondaryRecordCount),
						SEEK_CUR);
		xfile_Read_Word_From_File(stream, &missionGroupCount);
		xfile_Read_Word_From_File(stream, &secondaryRecordCount);
		g_launchGroupCount = 0;
		for (missionGroupIndex = 0; (int16_t)missionGroupIndex < (int16_t)missionGroupCount;
			 ++missionGroupIndex) {
			xfile_Read_Data_From_File(stream, &flightGroup, sizeof(flightGroup));
			if ((int16_t)flightGroup.craftType >= XW_CRAFT_SPECIES_X_WING &&
				(int16_t)flightGroup.craftType <= XW_CRAFT_SPECIES_A_WING &&
				(int16_t)flightGroup.iffOverride >= 0 &&
				(int16_t)flightGroup.iffOverride < XW_LAUNCH_IFF_LIMIT) {
				int16_t groupIndex = g_launchGroupCount;
				strcpy(g_launchGroupNames[groupIndex], flightGroup.name);
				g_launchGroupCraftTypes[groupIndex] = flightGroup.craftType;
				g_launchGroupInitialStatus[groupIndex] = flightGroup.initialStatus;
				g_launchGroupCraftCounts[groupIndex] = flightGroup.numberOfCraft;
				g_launchGroupFormations[groupIndex] = flightGroup.formation;
				g_launchGroupPlayerCraftOrdinals[groupIndex] = flightGroup.playerCraftOrdinalPlusOne;
				g_launchGroupMissionIndices[groupIndex] = missionGroupIndex;
				g_launchGroupCount = groupIndex + 1;
			}
		}
		xfile_Close_File(stream);
	}
}

// FUNCTION: XW 0x43E330
void brief_AssignLocalPilotToPlayerCraft(void) {
	int16_t groupIndex;
	for (groupIndex = 0; groupIndex < g_launchGroupCount; ++groupIndex) {
		int craftIndex;
		if (g_launchGroupPlayerCraftOrdinals[groupIndex] == 0) {
			continue;
		}
		g_launchSelectedGroupIndex = groupIndex;
		craftIndex = (int16_t)g_launchGroupPlayerCraftOrdinals[groupIndex] - 1;
		++g_launchAssignedPilotCount;
		g_launchSelectedCraftIndex = (int16_t)craftIndex;
		g_launchCraftPilotTokens[groupIndex][craftIndex] = g_localPilotAssignmentToken;
		return;
	}
}
