#ifndef XW_AUDIO_SOUNDEXT_H
#define XW_AUDIO_SOUNDEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>

typedef struct XwSoundTriggerContext XwSoundTriggerContext;
typedef struct XwShellSfxEntry XwShellSfxEntry;

enum {
	XW_SHELL_SFX_COUNT = 67,
	XW_SHELL_SPEECH_COUNT = 59,
	XW_SOUND_RESOURCE_NAME_CAPACITY = 10,
	XW_SOUND_SPEECH_NAME_BUFFER_CAPACITY = 16,
	XW_SOUND_RESOURCE_PATH_CAPACITY = 64,
	XW_SOUND_RESOURCE_PRIORITY = 128
};

enum XwShellSpeechIndex {
	XW_SHELL_SPEECH_ACKBAR_1 = 0,
	XW_SHELL_SPEECH_ACKBAR_2 = 1,
	XW_SHELL_SPEECH_ACKBAR_3 = 2,
	XW_SHELL_SPEECH_ALL_RIGHT = 3,
	XW_SHELL_SPEECH_ANTILLES = 4,
	XW_SHELL_SPEECH_BOARDING_1 = 5,
	XW_SHELL_SPEECH_BOARDING_2 = 6,
	XW_SHELL_SPEECH_BRING = 7,
	XW_SHELL_SPEECH_COMPLETE = 8,
	XW_SHELL_SPEECH_CONGRATS = 9,
	XW_SHELL_SPEECH_CRATE = 10,
	XW_SHELL_SPEECH_FOUND = 13,
	XW_SHELL_SPEECH_HAVE_YOU_NOW = 15,
	XW_SHELL_SPEECH_HES_OK = 16,
	XW_SHELL_SPEECH_LIFT_OFF = 17,
	XW_SHELL_SPEECH_LOCATION = 18,
	XW_SHELL_SPEECH_ON_HIM = 19,
	XW_SHELL_SPEECH_PLANET = 20,
	XW_SHELL_SPEECH_PLEASED = 21,
	XW_SHELL_SPEECH_PRINCESS = 23,
	XW_SHELL_SPEECH_REBEL = 24,
	XW_SHELL_SPEECH_RED_TWO = 26,
	XW_SHELL_SPEECH_REGISTER = 27,
	XW_SHELL_SPEECH_SHAKE_HIM = 28,
	XW_SHELL_SPEECH_SHUTTLE = 29,
	XW_SHELL_SPEECH_STATION = 30,
	XW_SHELL_SPEECH_TARKIN_1 = 31,
	XW_SHELL_SPEECH_TARKIN_2 = 32,
	XW_SHELL_SPEECH_THAT_IT = 33,
	XW_SHELL_SPEECH_VADRGRAT = 35,
	XW_SHELL_SPEECH_WELCOME = 36,
	XW_SHELL_SPEECH_YOU_MAY = 37,
	XW_SHELL_SPEECH_SIR = 38,
	XW_SHELL_SPEECH_OUR_TIES = 39,
	XW_SHELL_SPEECH_EXCELLENT = 40,
	XW_SHELL_SPEECH_THE_ATTACK = 41,
	XW_SHELL_SPEECH_MOVE_OUR = 42,
	XW_SHELL_SPEECH_ONCE_SIR = 43,
	XW_SHELL_SPEECH_RED_LEADER = 44,
	XW_SHELL_SPEECH_STAY_CLOSE = 45,
	XW_SHELL_SPEECH_DUTY_REGISTRATION = 46,
	XW_SHELL_SPEECH_GOOD_LUCK = 47,
	XW_SHELL_SPEECH_DONT_WORRY = 48,
	XW_SHELL_SPEECH_TRY_AGAIN = 49,
	XW_SHELL_SPEECH_NOT_BAD = 50,
	XW_SHELL_SPEECH_STILL_NEED = 51,
	XW_SHELL_SPEECH_TRAINING_EXCELLENT = 52,
	XW_SHELL_SPEECH_READY_COMBAT = 53
};

enum XwShellSfxIndex {
	XW_SHELL_SFX_TIE_APPROACH_1 = 0,
	XW_SHELL_SFX_TIE_APPROACH_2 = 1,
	XW_SHELL_SFX_TIE_APPROACH_2A = 2,
	XW_SHELL_SFX_TIE_APPROACH_4 = 3,
	XW_SHELL_SFX_FLY_SHOTS = 4,
	XW_SHELL_SFX_EXPLOSION_2 = 5,
	XW_SHELL_SFX_EXPLOSION_BIG = 6,
	XW_SHELL_SFX_EXPLOSION_FAR = 7,
	XW_SHELL_SFX_ROBOT_WALK = 8,
	XW_SHELL_SFX_GUN_BLAST = 9,
	XW_SHELL_SFX_TORPEDO_2 = 10,
	XW_SHELL_SFX_FLYBY_8 = 11,
	XW_SHELL_SFX_FLYBY_1 = 13,
	XW_SHELL_SFX_FLYBY_2 = 14,
	XW_SHELL_SFX_FLYBY_5 = 15,
	XW_SHELL_SFX_FLYBY_6 = 16,
	XW_SHELL_SFX_FLYBY_6A = 17,
	XW_SHELL_SFX_EXPLOSION_1 = 18,
	XW_SHELL_SFX_HOVER_HUM = 19,
	XW_SHELL_SFX_TRACTOR = 20,
	XW_SHELL_SFX_PING_1 = 21,
	XW_SHELL_SFX_TORCH = 23,
	XW_SHELL_SFX_R2_WHISTLE = 24,
	XW_SHELL_SFX_R2_A = 25,
	XW_SHELL_SFX_R2_B = 26,
	XW_SHELL_SFX_R2_C = 27,
	XW_SHELL_SFX_R2_D = 28,
	XW_SHELL_SFX_DOOR_1A = 29,
	XW_SHELL_SFX_TOUR_DOOR_CLOSE_1 = 30,
	XW_SHELL_SFX_DOOR_6 = 31,
	XW_SHELL_SFX_DOOR_CLOSE_2 = 32,
	XW_SHELL_SFX_GUARD = 33,
	XW_SHELL_SFX_DOOR_1 = 34,
	XW_SHELL_SFX_MENU_DOOR_CLOSE_1 = 35,
	XW_SHELL_SFX_TEXT_5 = 36,
	XW_SHELL_SFX_TARGET_5 = 38,
	XW_SHELL_SFX_HYDROL_3 = 39,
	XW_SHELL_SFX_HYDROL_1 = 40,
	XW_SHELL_SFX_SHUTTLE_1 = 41,
	XW_SHELL_SFX_SHUTTLE_2 = 42,
	XW_SHELL_SFX_SHUTTLE_3 = 43,
	XW_SHELL_SFX_SHUTTLE_4 = 44,
	XW_SHELL_SFX_SHUTTLE_5 = 45,
	XW_SHELL_SFX_HUM_1 = 46,
	XW_SHELL_SFX_BREATH = 47,
	XW_SHELL_SFX_PAIN_TOOL = 48,
	XW_SHELL_SFX_MESSAGE = 49,
	XW_SHELL_SFX_DROID = 51,
	XW_SHELL_SFX_DROID_4 = 52,
	XW_SHELL_SFX_BEEP_4 = 53,
	XW_SHELL_SFX_BEEP_5 = 54,
	XW_SHELL_SFX_BEEP_6 = 55,
	XW_SHELL_SFX_EXPLOSION_BIG_SECONDARY = 56,
	XW_SHELL_SFX_ALARM = 57,
	XW_SHELL_SFX_STAR_FIRE_1 = 58,
	XW_SHELL_SFX_STAR_FIRE_2 = 59,
	XW_SHELL_SFX_STAR_FIRE_3 = 60,
	XW_SHELL_SFX_RAMP = 61,
	XW_SHELL_SFX_BEEP_4_SECONDARY = 62,
	XW_SHELL_SFX_BEEP_5_SECONDARY = 63,
	XW_SHELL_SFX_BEEP_6_SECONDARY = 64,
	XW_SHELL_SFX_LOGON_C = 66
};

enum XwMusicTriggerCommand {
	XW_MUSIC_TRIGGER_START = 8,
	XW_MUSIC_TRIGGER_STOP = 9,
	XW_MUSIC_TRIGGER_SET_VOLUME = 0x102,
	XW_MUSIC_TRIGGER_SET_SPEED = 0x106,
	XW_MUSIC_TRIGGER_JUMP = 0x107,
	XW_MUSIC_TRIGGER_SET_HOOK = 0x10C,
	XW_MUSIC_TRIGGER_FADE_VOLUME = 0x10D,
	XW_MUSIC_TRIGGER_SHARE_PARTS = 0x119,
	XW_MUSIC_TRIGGER_END = 0xFFFF
};

enum XwDosSoundParameter {
	XW_DOS_SOUND_PARAM_GROUP = 4,
	XW_DOS_SOUND_PARAM_VOLUME = 6,
	XW_DOS_SOUND_PARAM_TRANSPOSE = 9
};

enum XwSoundQuery {
	XW_SOUND_QUERY_VOLUME = 1,
	XW_SOUND_QUERY_CHUNK = 6,
	XW_SOUND_QUERY_BEAT = 7,
	XW_SOUND_QUERY_TICK = 8,
	XW_SOUND_BEATS_PER_MEASURE = 4,
	XW_SOUND_BEAT_ORIGIN = 5,
	XW_SOUND_QUERY_UNSUPPORTED = -1
};

enum {
	XW_BRIEF_MUSIC_FADE_POSITION = 182,
	XW_BRIEF_MUSIC_MIN_LEVEL = 112,
	XW_BRIEF_MUSIC_POSITION_4 = 133,
	XW_BRIEF_MUSIC_POSITION_3 = 80,
	XW_BRIEF_MUSIC_POSITION_2 = 32,
	XW_BRIEF_MUSIC_CONTROL_4 = 4,
	XW_BRIEF_MUSIC_CONTROL_3 = 3,
	XW_BRIEF_MUSIC_CONTROL_2 = 2
};

enum XwSoundHookMode { XW_SOUND_CONTROL_DIRECT = 0, XW_SOUND_CONTROL_PACKED = 2 };

/* Original IDB size: 8 bytes. */
struct XwSoundTriggerContext {
	/* IDB +0x0 */
	int marker;
	/* IDB +0x4 */
	Sound* sound;
};

/* Original IDB size: 16 bytes. */
struct XwShellSfxEntry {
	/* IDB +0x0: NUL-terminated SFX resource name in a fixed 10-byte slot. */
	char name[XW_SOUND_RESOURCE_NAME_CAPACITY];
	/* IDB +0xA: Cached Sound pointer loaded from g_shellSfxResourceFile by ShellSound_LoadSfx. */
	Sound* sound;
	/* IDB +0xE: Nonzero preserves the cached pointer for one non-forced reset, then clears this flag. Loader
	 * clears it; all 67 initial values are zero and no nonzero writer is identified. */
	uint16_t retainCacheOnce;
};

extern XwShellSfxEntry g_shellSfxEntries[XW_SHELL_SFX_COUNT];
extern const char g_shellSpeechResourceNames[XW_SHELL_SPEECH_COUNT][XW_SOUND_RESOURCE_NAME_CAPACITY];
extern XwSoundTriggerContext g_soundTriggerContext;
extern ResFile* g_shellSpeechResourceFile;
extern int16_t g_commonUiSoundState;
extern ResFile* g_shellSfxResourceFile;
extern int16_t g_soundActionSavedGroupVolume;

typedef int16_t XwSoundAction;

enum XwSoundActionValues {
	XW_SOUND_ACTION_PAUSE = 0x1,
	XW_SOUND_ACTION_RESUME = 0x2,
	XW_SOUND_ACTION_START_MUSIC = 0x3,
	XW_SOUND_ACTION_START_SFX = 0x4,
	XW_SOUND_ACTION_START_SPEECH = 0x5,
	XW_SOUND_ACTION_STOP_SOUND = 0x6,
	XW_SOUND_ACTION_SET_VOLUME = 0x7,
	XW_SOUND_ACTION_FADE_VOLUME = 0x8,
	XW_SOUND_ACTION_SET_PAN = 0x9,
	XW_SOUND_ACTION_FADE_PAN = 0xA
};

/* Declarations follow ascending original IDB address. */

/* 0x43E4A0 */
int16_t soundext_RecheckSfxPreference(void);

/* 0x44FCA0 */
int16_t soundext_LoadCommonUiSounds(void);

/* 0x46B4A0 */
void soundext_ResetEnabledSfxCache(void);

/* 0x49F810 */
int j_lolevel_ImPause(void);

/* 0x49F820 */
int j_lolevel_ImResume(void);

/* 0x49F830 */
int16_t soundext_Start_Resource_Sound(const Sound* sound);

/* 0x49F860 */
int16_t soundext_Start_Resource_SFX(const Sound* sound);

/* 0x49F880 */
int16_t soundext_Start_Resource_Voice(const Sound* sound);

/* 0x49F8A0 */
int16_t soundext_Stop_Resource_Sound(const Sound* sound);

/* 0x49F8C0 */
int32_t j_lolevel_ImStopAllSounds(void);

/* 0x49F8D0 */
uint8_t soundext_Count_Resource_Instances(const Sound* sound);

/* 0x49F8F0 */
int16_t soundext_GetMusicParam(Sound* sound, uint16_t selector, int unusedArgument);

/* 0x49F9A0 */
int32_t soundext_SetPriority(intptr_t soundId, uint16_t priority);

/* 0x49F9C0 */
int32_t soundext_SetVolume(intptr_t soundId, uint16_t volume);

/* 0x49F9E0 */
int32_t soundext_SetTranspose(intptr_t soundId, int16_t skipReset, int16_t value);

/* 0x49FA30 */
int32_t soundext_SetGroup(intptr_t soundId, uint16_t value);

/* 0x49FA50 */
int16_t soundext_JumpMidi(Sound* sound, int groupIndex, unsigned int beatIndex, int tick);

/* 0x49FA90 */
int16_t soundext_ScanMidi(Sound* sound, int groupIndex, unsigned int beatIndex, int tick);

/* 0x49FAD0 */
int16_t soundext_SetPartEnabled(Sound* sound, int selector, int16_t enabled);

/* 0x49FB00 */
int16_t soundext_SetHook(Sound* sound, int16_t mode, int value, int channelIndex);

/* Fade music or SFX volume over 60 Hz ticks; zero duration applies the target immediately. */
/* 0x49FB50 */
int16_t soundext_FadeVolume(Sound* sound, int targetVolume, int durationTicks);

/* 0x49FB70 */
int16_t soundext_SetTriggerContext(Sound* sound, uint16_t marker);

/* 0x49FB90 */
int16_t soundext_QueueTriggerCommand(uint16_t command, intptr_t arg0, intptr_t arg1, intptr_t arg2,
									 intptr_t arg3, int unusedArg4, int unusedArg5);

/* 0x49FDA0 */
int16_t soundext_ClearTriggers(void);

/* 0x49FDB0 */
int16_t soundext_ShareParts(Sound* firstSound, Sound* secondSound);

/* 0x4A9790 */
void soundext_Open_Post_iMuse(void);

/* 0x4A97C0 */
void soundext_Close_Post_iMuse(void);

/* 0x4A97F0 */
void soundext_compact_Sound(uint16_t postCompaction);

/* 0x4A9810 */
void soundext_Action_iMuse(XwSoundAction action, Sound* sound, int16_t value, int16_t duration);

/* 0x4A9970 */
void soundext_OpenResourceFiles(void);

/* 0x4A9A60 */
void soundext_CloseResourceFiles(void);

/* 0x4A9AA0 */
Sound* soundext_LoadSpeech(int speechIndex, int16_t userValue, void (*userFunction)(Sound* sound, int time),
						   int unused);

/* 0x4A9B10 */
void soundext_LoadSfx(int soundIndex, int16_t userValue, void (*userFunction)(Sound* sound, int time),
					  int16_t keepSound, int unused);

/* 0x4A9B80 */
void soundext_Play_SFX(int soundIndex);

/* 0x4A9B90 */
void soundext_PlaySfxMode1(int soundIndex);

/* 0x4A9BA0 */
void soundext_PlayCached(int soundIndex, int unusedMode);

/* 0x4A9BF0 */
void soundext_Fade_SFX(int soundIndex, int targetVolume, int duration);

/* 0x4A9C20 */
void soundext_Stop_SFX(int soundIndex);

/* 0x4A9C50 */
void soundext_ResetSfxCache(char forceReset);

#ifdef __cplusplus
}
#endif

#endif
