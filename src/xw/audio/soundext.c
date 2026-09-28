#include "xw/audio/soundext.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#endif

#include "xw/audio/frontend_audio.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/audio/sound.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/util/folded.h"
#include "xw/util/shared.h"

#include <landru/fourcc.h>
#include <landru/res.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4DD908
XwShellSfxEntry g_shellSfxEntries[XW_SHELL_SFX_COUNT] = {
	{ "tieappr1", NULL, 0 }, { "tieappr2", NULL, 0 }, { "tieappr2a", NULL, 0 }, { "tieappr4", NULL, 0 },
	{ "flyshots", NULL, 0 }, { "explo2", NULL, 0 },   { "explobig", NULL, 0 },  { "explofar", NULL, 0 },
	{ "rbotwalk", NULL, 0 }, { "gunblast", NULL, 0 }, { "trpedo2", NULL, 0 },   { "flyby8", NULL, 0 },
	{ "landing", NULL, 0 },  { "flyby1", NULL, 0 },   { "flyby2", NULL, 0 },    { "flyby5", NULL, 0 },
	{ "flyby6", NULL, 0 },   { "flyby6a", NULL, 0 },  { "explo1", NULL, 0 },    { "hoverhum", NULL, 0 },
	{ "tractor", NULL, 0 },  { "ping1", NULL, 0 },    { "crane1", NULL, 0 },    { "torch", NULL, 0 },
	{ "r2whistl", NULL, 0 }, { "r2a", NULL, 0 },      { "r2b", NULL, 0 },       { "r2c", NULL, 0 },
	{ "r2d", NULL, 0 },      { "door-1a", NULL, 0 },  { "dr-cls-1", NULL, 0 },  { "door-6", NULL, 0 },
	{ "dr-cls-2", NULL, 0 }, { "guard", NULL, 0 },    { "door-1", NULL, 0 },    { "dr-cls-1", NULL, 0 },
	{ "text-5", NULL, 0 },   { "target-4", NULL, 0 }, { "target-5", NULL, 0 },  { "hydrol-3", NULL, 0 },
	{ "hydrol-1", NULL, 0 }, { "shuttle1", NULL, 0 }, { "shuttle2", NULL, 0 },  { "shuttle3", NULL, 0 },
	{ "shuttle4", NULL, 0 }, { "shuttle5", NULL, 0 }, { "hum1", NULL, 0 },      { "breath", NULL, 0 },
	{ "paintool", NULL, 0 }, { "message", NULL, 0 },  { "eject", NULL, 0 },     { "droid", NULL, 0 },
	{ "droid4", NULL, 0 },   { "beep4", NULL, 0 },    { "beep5", NULL, 0 },     { "beep6", NULL, 0 },
	{ "explobig", NULL, 0 }, { "alarm", NULL, 0 },    { "starfir1", NULL, 0 },  { "starfir2", NULL, 0 },
	{ "starfir3", NULL, 0 }, { "ramp", NULL, 0 },     { "beep4", NULL, 0 },     { "beep5", NULL, 0 },
	{ "beep6", NULL, 0 },    { "en-zp-4", NULL, 0 },  { "logonc", NULL, 0 },
};

// GLOBAL: XW 0x4DDD38
const char g_shellSpeechResourceNames[XW_SHELL_SPEECH_COUNT][XW_SOUND_RESOURCE_NAME_CAPACITY] = {
	"ackbar1",  "ackbar2",  "ackbar3",  "allright", "antilles", "bording1", "bording2", "bring",
	"complete", "congrats", "crate",    "engage",   "follow",   "found",    "gothim",   "havunow",
	"hesok",    "liftoff",  "location", "onhim",    "planet",   "pleased",  "prepare",  "princess",
	"rebel",    "redsquad", "redtwo",   "register", "shakehim", "shuttlex", "station",  "tarkin1",
	"tarkin2",  "thatit",   "tiesquad", "vadrgrat", "welcome",  "youmay",   "sir",      "ourties",
	"exellent", "theatak",  "moveour",  "oncesir",  "redlead",  "stayclos", "dutyreg",  "goodluck",
	"dontwory", "tryagain", "notbad",   "stilneed", "excellnt", "redycmbt", "wejust",   "sounlik1",
	"thasrite", "looklike", "relayit",
};

// GLOBAL: XW 0x4F76DC
int16_t g_commonUiSoundState = 0;

// GLOBAL: XW 0x5619CC
XwSoundTriggerContext g_soundTriggerContext = { 0, NULL };

// GLOBAL: XW 0x566824
ResFile* g_shellSpeechResourceFile = NULL;

// GLOBAL: XW 0x566828
ResFile* g_shellSfxResourceFile = NULL;

// GLOBAL: XW 0x5B8A8A
int16_t g_soundActionSavedGroupVolume = 0;

// FUNCTION: XW 0x43E4A0
int16_t soundext_RecheckSfxPreference(void) {
	ShellPreferences_GetSfxEnabled();
	return ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x44FCA0
int16_t soundext_LoadCommonUiSounds(void) {
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_6, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_DOOR_CLOSE_2, 0, NULL, 1, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TEXT_5, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_TARGET_5, 0, NULL, 0, 0);
		g_commonUiSoundState = 0;
	}
	return ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x46B4A0
void soundext_ResetEnabledSfxCache(void) {
	if (ShellPreferences_GetSfxEnabled())
		soundext_ResetSfxCache(1);
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x49F810
int j_lolevel_ImPause(void) { return lolevel_ImPause(); }

// FUNCTION: XW 0x49F820
int j_lolevel_ImResume(void) { return lolevel_ImResume(); }

// FUNCTION: XW 0x49F830
int16_t soundext_Start_Resource_Sound(const Sound* sound) {
#ifdef XW_MODERN
	return Dos94_soundext_Start_Resource_Sound(sound);
#else
	int16_t result;
	if (!sound)
		return 0;
	result = sound->type == digitalSound
				 ? soundext_Start_Resource_SFX(sound)
				 : (int16_t)hilevel_ImStartResourceMusic(sound, XW_SOUND_RESOURCE_PRIORITY);
	return result;
#endif
}

// FUNCTION: XW 0x49F860
int16_t soundext_Start_Resource_SFX(const Sound* sound) {
#ifdef XW_MODERN
	return Dos94_soundext_Start_Resource_SFX(sound);
#else
	if (!sound)
		return 0;
	return hilevel_ImStartSfx(sound->res_name, XW_SOUND_RESOURCE_PRIORITY);
#endif
}

// FUNCTION: XW 0x49F880
int16_t soundext_Start_Resource_Voice(const Sound* sound) {
#ifdef XW_MODERN
	return Dos94_soundext_Start_Resource_Voice(sound);
#else
	if (!sound)
		return 0;
	return hilevel_ImStartVoice(sound->res_name, XW_SOUND_RESOURCE_PRIORITY);
#endif
}

// FUNCTION: XW 0x49F8A0
int16_t soundext_Stop_Resource_Sound(const Sound* sound) {
#ifdef XW_MODERN
	return Dos94_soundext_Stop_Resource_Sound(sound);
#else
	if (!sound)
		return 0;
	return lolevel_ImStopSound(sound->res_name);
#endif
}

// FUNCTION: XW 0x49F8C0
int32_t j_lolevel_ImStopAllSounds(void) { return lolevel_ImStopAllSounds(); }

// FUNCTION: XW 0x49F8D0
uint8_t soundext_Count_Resource_Instances(const Sound* sound) {
#ifdef XW_MODERN
	return Dos94_soundext_Count_Resource_Instances(sound);
#else
	Sound_FlushQueuedEffects();
	if (!sound)
		return 0;
	return lolevel_ImGetParam(sound->res_name, XW_SOUND_PARAM_INSTANCE_COUNT);
#endif
}

// FUNCTION: XW 0x49F8F0
int16_t soundext_GetMusicParam(Sound* sound, uint16_t selector, int unusedArgument) {
#ifdef XW_MODERN
	(void)unusedArgument;
	return Dos94_soundext_GetMusicParam(sound, selector);
#else
	(void)sound;
	(void)unusedArgument;
	switch (selector) {
		case XW_SOUND_QUERY_VOLUME:
			/* A Sound resource pointer is not a flight sound ID; this legacy query returns zero. */
			return Sound_GetParam(0, XW_DOS_SOUND_PARAM_VOLUME);
		case XW_SOUND_QUERY_CHUNK:
			return Shared_ReturnZero() - 1;
		case XW_SOUND_QUERY_BEAT: {
			int16_t measure = Shared_ReturnZero();
			return Shared_ReturnZero() + XW_SOUND_BEATS_PER_MEASURE * measure - XW_SOUND_BEAT_ORIGIN;
		}
		case XW_SOUND_QUERY_TICK:
			return Shared_ReturnZero();
		default:
			return XW_SOUND_QUERY_UNSUPPORTED;
	}
#endif
}

// FUNCTION: XW 0x49F9A0
int32_t soundext_SetPriority(intptr_t soundId, uint16_t priority) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_PRIORITY, priority) : -1;
#else
	return Sound_SetParam(soundId, XW_SOUND_PARAM_PRIORITY, priority);
#endif
}

// FUNCTION: XW 0x49F9C0
int32_t soundext_SetVolume(intptr_t soundId, uint16_t volume) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_VOL, volume) : -1;
#else
	return Sound_SetParam(soundId, XW_SOUND_PARAM_VOLUME, volume);
#endif
}

// FUNCTION: XW 0x49F9E0
int32_t soundext_SetTranspose(intptr_t soundId, int16_t skipReset, int16_t value) {
#ifdef XW_MODERN
	if (!g_dos94Imuse)
		return -1;
	if (!skipReset)
		imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_TRANSPOSE, 0);
	return imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_TRANSPOSE, value);
#else
	if (skipReset != 0)
		return Sound_SetParam(soundId, XW_DOS_SOUND_PARAM_TRANSPOSE, value);
	Sound_SetParam(soundId, XW_DOS_SOUND_PARAM_TRANSPOSE, 0);
	return Sound_SetParam(soundId, XW_DOS_SOUND_PARAM_TRANSPOSE, value);
#endif
}

// FUNCTION: XW 0x49FA30
int32_t soundext_SetGroup(intptr_t soundId, uint16_t value) {
#ifdef XW_MODERN
	return g_dos94Imuse ? imuse_set_param(g_dos94Imuse, soundId, IMUSE_PARAM_SOUND_GROUP, value) : -1;
#else
	return Sound_SetParam(soundId, XW_DOS_SOUND_PARAM_GROUP, value);
#endif
}

// FUNCTION: XW 0x49FA50
int16_t soundext_JumpMidi(Sound* sound, int groupIndex, unsigned int beatIndex, int tick) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		return Dos94_soundext_JumpMidi(sound, groupIndex, beatIndex, tick);
#endif
	(void)sound;
	(void)groupIndex;
	(void)beatIndex;
	(void)tick;
	/* The stripped backend ignores the original position arguments and flag. */
	return Shared_ReturnZero();
}

// FUNCTION: XW 0x49FA90
int16_t soundext_ScanMidi(Sound* sound, int groupIndex, unsigned int beatIndex, int tick) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		return Dos94_soundext_ScanMidi(sound, groupIndex, beatIndex, tick);
#endif
	(void)sound;
	(void)groupIndex;
	(void)beatIndex;
	(void)tick;
	/* The stripped backend ignores the original music-position arguments. */
	return Shared_ReturnZero();
}

// FUNCTION: XW 0x49FAD0
int16_t soundext_SetPartEnabled(Sound* sound, int selector, int16_t enabled) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		return Dos94_soundext_SetPartEnabled(sound, selector, enabled);
#endif
	/* The stripped backend ignores sound, selector and the 0/127 enabled value. */
	(void)sound;
	(void)selector;
	(void)enabled;
	return Shared_ReturnZero();
}

// FUNCTION: XW 0x49FB00
int16_t soundext_SetHook(Sound* sound, int16_t mode, int value, int channelIndex) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		return Dos94_soundext_SetHook(sound, mode, value, channelIndex);
#endif
	if (mode == XW_SOUND_CONTROL_DIRECT) {
		return lolevel_ImSetResourceHook(sound, (uint32_t)value);
	}
	if (mode == XW_SOUND_CONTROL_PACKED) {
		return lolevel_ImSetResourceHook(sound, (uint32_t)value | (((uint32_t)channelIndex + 1) << 8));
	}
	return -1;
}

/* DOS94 0x12558 fixes the fade parameter to volume (selector 1). */
// FUNCTION: XW 0x49FB50
int16_t soundext_FadeVolume(Sound* sound, int targetVolume, int durationTicks) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		return Dos94_soundext_FadeVolume(sound, targetVolume, durationTicks);
#endif
	return lolevel_ImFadeResourceParam(sound, XW_FOLDED_FADE_VOLUME, targetVolume, durationTicks);
}

// FUNCTION: XW 0x49FB70
int16_t soundext_SetTriggerContext(Sound* sound, uint16_t marker) {
#ifdef XW_MODERN
	return Dos94_soundext_SetTriggerContext(sound, marker);
#else
	g_soundTriggerContext.sound = sound;
	g_soundTriggerContext.marker = marker;
	return 0;
#endif
}

// FUNCTION: XW 0x49FB90
int16_t soundext_QueueTriggerCommand(uint16_t command, intptr_t arg0, intptr_t arg1, intptr_t arg2,
									 intptr_t arg3, int unusedArg4, int unusedArg5) {
#ifdef XW_MODERN
	if (g_dos94Imuse) {
		Dos94_soundext_QueueTriggerCommand(command, arg0, arg1, arg2, arg3);
		return 0;
	}
#endif
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)unusedArg4;
	(void)unusedArg5;
	/* The stripped marker-trigger backend never reads its command arguments. */
	switch (command) {
		case XW_MUSIC_TRIGGER_START:
			Shared_ReturnZero32();
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_STOP:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_SET_VOLUME:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_SET_SPEED:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_JUMP:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_SET_HOOK:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_FADE_VOLUME:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_SHARE_PARTS:
			Shared_ReturnZero32();
			break;
		case XW_MUSIC_TRIGGER_END:
			g_soundTriggerContext.marker = 0;
			g_soundTriggerContext.sound = NULL;
			break;
	}
	return 0;
}

// FUNCTION: XW 0x49FDA0
int16_t soundext_ClearTriggers(void) {
#ifdef XW_MODERN
	Dos94_soundext_ClearTriggers();
	return 0;
#else
	return lolevel_ImClearTrigger((uint32_t)-1, -1, -1);
#endif
}

// FUNCTION: XW 0x49FDB0
int16_t soundext_ShareParts(Sound* firstSound, Sound* secondSound) {
#ifdef XW_MODERN
	if (g_dos94Imuse)
		return Dos94_soundext_ShareParts(firstSound, secondSound);
#endif
	return lolevel_ImShareResourceParts(firstSound, secondSound);
}

// FUNCTION: XW 0x4A9790
void soundext_Open_Post_iMuse(void) {
	Shared_ReturnOne();
	xsound_Set_Sound_Action_Function(soundext_Action_iMuse);
	soundext_OpenResourceFiles();
}

// FUNCTION: XW 0x4A97C0
void soundext_Close_Post_iMuse(void) {
	soundext_CloseResourceFiles();
	xsound_Set_Sound_Action_Function(NULL);
	nullsub_SharedNoOp();
	j_lolevel_ImStopAllSounds();
}

// FUNCTION: XW 0x4A97F0
void soundext_compact_Sound(uint16_t postCompaction) {
	if (postCompaction != 0) {
		lolevel_ImResume();
	} else {
		lolevel_ImPause();
	}
}

// FUNCTION: XW 0x4A9810
void soundext_Action_iMuse(XwSoundAction action, Sound* sound, int16_t value, int16_t duration) {
#ifdef XW_MODERN
	if (action == XW_SOUND_ACTION_PAUSE)
		FrontendAudio_Pause();
	if (action == XW_SOUND_ACTION_RESUME)
		FrontendAudio_Resume();
	Dos94_soundext_Action_iMuse(action, sound, value, duration);
#else
	int lockCount;
	int i;
	lockCount = FlightDisplay_GetSurfaceLockCount();
	(void)duration;
	if (lockCount) {
		for (i = lockCount; i > 0; --i)
			FlightDisplay_UnlockSurface();
	}
	switch (action) {
		case XW_SOUND_ACTION_PAUSE:
			lolevel_ImPause();
			g_soundActionSavedGroupVolume = lolevel_ImSetGroupVol(0, 0);
			break;
		case XW_SOUND_ACTION_RESUME:
			lolevel_ImSetGroupVol(0, g_soundActionSavedGroupVolume);
			lolevel_ImResume();
			break;

		case XW_SOUND_ACTION_START_SFX:
			hilevel_ImStartSfx(sound->res_name, 0);
			break;
		case XW_SOUND_ACTION_START_SPEECH:
			hilevel_ImStartVoice(sound->res_name, 0);
			break;
		case XW_SOUND_ACTION_STOP_SOUND:
			lolevel_ImStopSound(sound->res_name);
			break;
		case XW_SOUND_ACTION_SET_VOLUME:
			lolevel_ImSetParam(sound->res_name, XW_SOUND_PARAM_VOLUME, value);
			break;

		case XW_SOUND_ACTION_SET_PAN:
			lolevel_ImSetParam(sound->res_name, XW_SOUND_PARAM_PAN, value);
			break;
		case XW_SOUND_ACTION_START_MUSIC:
		case XW_SOUND_ACTION_FADE_VOLUME:
		case XW_SOUND_ACTION_FADE_PAN:
			/* These operations call a stripped backend in this executable. */
			Shared_ReturnZero32();
			break;
	}
	if (lockCount) {
		for (; lockCount > 0; --lockCount)
			FlightDisplay_LockSurface();
	}
#endif
}

// FUNCTION: XW 0x4A9970
void soundext_OpenResourceFiles(void) {
	char resourcePath[XW_SOUND_RESOURCE_PATH_CAPACITY];
	char directoryPrefix[XW_SOUND_RESOURCE_PATH_CAPACITY];
	directoryPrefix[0] = 0;
	ShellPreferences_GetSfxEnabled();
	strcpy(resourcePath, directoryPrefix);
	strcat(resourcePath, "sfx.lfd");
	g_shellSfxResourceFile = xres_Open_Resource(resourcePath);
	ShellPreferences_GetSfxEnabled();
	strcpy(resourcePath, directoryPrefix);
	strcat(resourcePath, "speech.lfd");
	g_shellSpeechResourceFile = xres_Open_Resource(resourcePath);
}

// FUNCTION: XW 0x4A9A60
void soundext_CloseResourceFiles(void) {
	if (g_shellSfxResourceFile) {
		xres_Close_Resource(g_shellSfxResourceFile);
		g_shellSfxResourceFile = NULL;
	}
	if (g_shellSpeechResourceFile) {
		xres_Close_Resource(g_shellSpeechResourceFile);
		g_shellSpeechResourceFile = NULL;
	}
}

// FUNCTION: XW 0x4A9AA0
Sound* soundext_LoadSpeech(int speechIndex, int16_t userValue, void (*userFunction)(Sound* sound, int time),
						   int unused) {
	char name[XW_SOUND_SPEECH_NAME_BUFFER_CAPACITY];
	Sound* sound;
	(void)unused;
	strcpy(name, g_shellSpeechResourceNames[speechIndex]);
	sound = xsound_Res_Digital_Sound(g_shellSpeechResourceFile, name);
	xsound_Set_Sound_User_Function(sound, userFunction);
	sound->var1 = userValue;
	sound->var2 = 1;
	return sound;
}

// FUNCTION: XW 0x4A9B10
void soundext_LoadSfx(int soundIndex, int16_t userValue, void (*userFunction)(Sound* sound, int time),
					  int16_t keepSound, int unused) {
	(void)unused;
#ifdef XW_MODERN
	if (soundIndex < 0)
		return;
#endif
	if (soundIndex < XW_SHELL_SFX_COUNT) {
		XwShellSfxEntry* entry = &g_shellSfxEntries[soundIndex];
		Sound* sound;
		entry->retainCacheOnce = 0;
		if (!entry->sound)
			entry->sound = xsound_Res_Digital_Sound(g_shellSfxResourceFile, entry->name);
		sound = entry->sound;
		if (sound) {
			xsound_Set_Sound_User_Function(sound, userFunction);
			sound->var1 = userValue;
			sound->var2 = 0;
			if (keepSound)
				xsound_Set_Sound_Keep(sound);
		}
	}
}

// FUNCTION: XW 0x4A9B80
void soundext_Play_SFX(int soundIndex) { soundext_PlayCached(soundIndex, 0); }

// FUNCTION: XW 0x4A9B90
void soundext_PlaySfxMode1(int soundIndex) { soundext_PlayCached(soundIndex, 1); }

// FUNCTION: XW 0x4A9BA0
void soundext_PlayCached(int soundIndex, int unusedMode) {
	(void)unusedMode;
#ifdef XW_MODERN
	if (soundIndex < 0)
		return;
#endif
	if (soundIndex < XW_SHELL_SFX_COUNT) {
		Sound* sound = g_shellSfxEntries[soundIndex].sound;
		if (sound && sound->res_type == FOURCC_VOIC) {
			if (sound->var2)
				soundext_Start_Resource_Voice(sound);
			else
				soundext_Start_Resource_SFX(sound);
		}
	}
}

// FUNCTION: XW 0x4A9BF0
void soundext_Fade_SFX(int soundIndex, int targetVolume, int duration) {
#ifdef XW_MODERN
	if (soundIndex < 0)
		return;
#endif
	if (soundIndex < XW_SHELL_SFX_COUNT) {
		Sound* sound = g_shellSfxEntries[soundIndex].sound;
		if (sound)
			soundext_FadeVolume(sound, targetVolume, duration);
	}
}

// FUNCTION: XW 0x4A9C20
void soundext_Stop_SFX(int soundIndex) {
#ifdef XW_MODERN
	if (soundIndex < 0)
		return;
#endif
	if (soundIndex < XW_SHELL_SFX_COUNT) {
		Sound* sound = g_shellSfxEntries[soundIndex].sound;
		if (sound)
			soundext_Stop_Resource_Sound(sound);
	}
}

// FUNCTION: XW 0x4A9C50
void soundext_ResetSfxCache(char forceReset) {
	int index;
	int entriesRemaining;
	for (index = 0, entriesRemaining = XW_SHELL_SFX_COUNT; entriesRemaining != 0;
		 ++index, --entriesRemaining) {
		if (g_shellSfxEntries[index].retainCacheOnce == 0 || forceReset != 0) {
			g_shellSfxEntries[index].sound = NULL;
		} else {
			g_shellSfxEntries[index].retainCacheOnce = 0;
		}
	}
}
