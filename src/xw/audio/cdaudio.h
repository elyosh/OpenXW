#ifndef XW_AUDIO_CDAUDIO_H
#define XW_AUDIO_CDAUDIO_H

#include <aeron/compat/mmsystem.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

extern int g_musicCdPlaybackComplete;
extern int g_musicCdTrackCount;
extern int g_musicCdSavedAuxVolume;
extern uint32_t g_musicCdTrackCache[30];
extern int g_musicCdCurrentTrack;
extern MCIDEVICEID g_musicCdMciDeviceId;

enum { CDAUDIO_AUX_VOLUME_MAX = 65535, CDAUDIO_AUX_VOLUME_UNSAVED = -1, CDAUDIO_AUX_CHANNEL_BITS = 16 };

enum { CDAUDIO_TIME_BYTE_BITS = 8 };

/* Declarations follow ascending original IDB address. */

/* 0x4A27A0 */
int CDAudio_Initialize(void);

/* 0x4A2990 */
int CDAudio_PlayTrackFromTime(int trackNumber, int startMinute, int startSecond);

/* 0x4A2A60 */
int CDAudio_StopTrack(void);

/* 0x4A2AB0 */
void CDAudio_CloseDevice(void);

/* 0x4A2BA0 */
int CDAudio_GetTrackEndTimeMs(int trackNumber);

/* 0x4A2C20 */
int CDAudio_SetAuxVolume(unsigned int volume0To65535);

#ifdef __cplusplus
}
#endif

#endif
