#ifndef XW_AUDIO_HILEVEL_H
#define XW_AUDIO_HILEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

extern int g_musicCdVolumeSetting;

enum { HILEVEL_CD_VOLUME_SCALE = 16, HILEVEL_VOICE_GAIN_NUMERATOR = 3, HILEVEL_VOICE_GAIN_DENOMINATOR = 2 };

/* Declarations follow ascending original IDB address. */

/* 0x484F20 */
int hilevel_ImSetVoiceVol(int volume);

/* 0x484F70 */
int hilevel_ImGetVoiceVol(void);

/* 0x484F80 */
int hilevel_ImGetSfxVol(void);

/* 0x484F90 */
int hilevel_ImGetMusicVol(void);

/* 0x484FA0 */
int hilevel_ImSetSfxVol(int volume);

/* 0x484FE0 */
int hilevel_SetCdAuxVolume(int volume);

/* 0x4852B0 */
int hilevel_ImSetMasterVol(int volume);

/* 0x4852F0 */
int hilevel_ImGetMasterVol(void);

/* 0x485340 */
int32_t hilevel_ImStartSfx(const char* soundName, int priority);

/* 0x485370 */
int32_t hilevel_ImStartVoice(const char* soundName, int priority);

#ifdef __cplusplus
}
#endif

#endif
