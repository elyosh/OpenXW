#ifndef XW_AUDIO_LOLEVEL_H
#define XW_AUDIO_LOLEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { XW_SOUND_GROUP_ALL = 1, XW_SOUND_GROUP_MUSIC = 3 };

typedef int32_t XwNamedSoundParameter;

enum XwNamedSoundParameterValues {
	XW_SOUND_PARAM_INSTANCE_COUNT = 0x1,
	XW_SOUND_PARAM_PRIORITY = 0x5,
	XW_SOUND_PARAM_VOLUME = 0x6,
	XW_SOUND_PARAM_PAN = 0x7,
	XW_SOUND_PARAM_FREQUENCY_HZ = 0xB
};

/* Declarations follow ascending original IDB address. */

/* 0x485230 */
int32_t lolevel_ImStopAllSounds(void);

/* 0x485270 */
int lolevel_ImResume(void);

/* 0x485300 */
int lolevel_ImPause(void);

/* 0x4853A0 */
int32_t lolevel_ImSetParam(const char* soundName, XwNamedSoundParameter parameter, int value);

/* 0x485460 */
int32_t lolevel_ImStopSound(const char* soundName);

/* 0x4854B0 */
int16_t lolevel_ImSetGroupVol(int16_t group, int16_t volume);

/* 0x4854F0 */
int lolevel_ImGetParam(const char* soundName, int16_t parameter);

#ifdef __cplusplus
}
#endif

#endif
