#ifndef XW_DOS94_AUDIO_FMUSIC_H
#define XW_DOS94_AUDIO_FMUSIC_H
#include "xw/flight/mission/fscript.h"
extern uint16_t g_dos94MusicTrackCount;
/* 0x13240 */
XwMusicSoundId Dos94_fmusic_fmLoadSound(const char* name);
/* 0x132D0 */
void* Dos94_fmusic_GetPagedSound(uint32_t trackIndex);
/* 0x1356A */
uint16_t Dos94_fmusic_loadmusic(const char* path);
#endif
