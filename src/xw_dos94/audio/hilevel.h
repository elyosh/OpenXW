#ifndef XW_DOS94_AUDIO_HILEVEL_H
#define XW_DOS94_AUDIO_HILEVEL_H
#include <stdint.h>

/* 0x119EE */
int16_t Dos94_hilevel_ImSetMusicVol(int16_t volume);

/* 0x11A16 */
int16_t Dos94_hilevel_ImSetSfxVol(int16_t volume);

/* 0x11A3E */
int16_t Dos94_hilevel_ImSetVoiceVol(int16_t volume);

/* 0x11A66 */
int16_t Dos94_hilevel_ImStartSfx(intptr_t soundId);

/* 0x11A98 */
int16_t Dos94_hilevel_ImStartVoice(intptr_t soundId);

/* 0x11ACA */
int16_t Dos94_hilevel_ImStartMusic(intptr_t soundId);
#endif
