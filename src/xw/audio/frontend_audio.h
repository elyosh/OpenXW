#ifndef XW_AUDIO_FRONTEND_AUDIO_H
#define XW_AUDIO_FRONTEND_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/memhdl.h>
#include <stddef.h>
#include <stdint.h>

struct IDirectSoundBuffer;

extern unsigned int g_frontendWaveDataOffset;
extern uint8_t g_frontendSavedRefillEnabled;
extern uint8_t g_frontendRefillEnabled;
extern uint8_t g_frontendLoopAsSfx;
extern uint8_t g_frontendPlaybackStarted;
extern uint8_t g_frontendUsesStreamBuffer;
extern unsigned int g_frontendPlayCursor;
extern struct IDirectSoundBuffer* g_frontendAudioBuffer;
extern LandruHandle g_frontendStreamReadHandle;
extern uint8_t g_frontendPauseDepth;

extern int g_frontendPreviousCursorDistance;
extern int g_frontendBytesPlayed;
extern uint8_t g_frontendDrainPending;
extern unsigned int g_frontendStreamWriteOffset;
extern unsigned int g_frontendRefillRequestBytes;
extern unsigned int g_frontendPreviousPlayCursor;

enum {
	FRONTEND_AUDIO_STREAM_CHANNEL = 1,
	FRONTEND_AUDIO_PATH_CAPACITY = 256,
	FRONTEND_AUDIO_DRIVE_PREFIX_LENGTH = 2,
	FRONTEND_AUDIO_WHOLE_FILE_LIMIT = 251000,
	FRONTEND_AUDIO_VOLUME_SCALE = 8,
	FRONTEND_AUDIO_STREAM_BYTES = 500000,
	FRONTEND_AUDIO_PRIME_BYTES = FRONTEND_AUDIO_STREAM_BYTES / 2,
	FRONTEND_AUDIO_REFILL_MIN = 1000,
	FRONTEND_AUDIO_REFILL_MAX = 64000,
	FRONTEND_AUDIO_REFILL_ALIGNMENT = 4,
	FRONTEND_AUDIO_SILENCE_BYTE = 0x80
};

/* Declarations follow ascending original IDB address. */

/* 0x49EE40 */
int FrontendAudio_PlayFile(const char* filename, uint8_t loopAsSfx);

/* 0x49EFD0 */
int FrontendAudio_StartStreamFile(const char* filename);

/* 0x49F2F0 */
int FrontendAudio_UpdatePlayback(void);

/* 0x49F3D0 */
void FrontendAudio_RefillStreamBuffer(void);

/* 0x49F5E0 */
void FrontendAudio_Pause(void);

/* 0x49F640 */
void FrontendAudio_Resume(void);

/* 0x49F720 */
int FrontendAudio_CreateStreamBuffers(void);

/* 0x49F790 */
void FrontendAudio_StopAndRelease(void);

#ifdef __cplusplus
}
#endif

#endif
