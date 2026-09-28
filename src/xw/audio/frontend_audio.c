#include "xw/audio/frontend_audio.h"

#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#include "xw_runtime/runtime/profile.h"
#endif

#include "xw/audio/direct_sound.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/shell.h"
#include "xw/frontend/shell_preferences.h"

#ifdef XW_MODERN
#include "xw/audio/sound.h"
#include <aeron/log.h>
#endif
#include <landru/file.h>
#include <landru/stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x56198C
unsigned int g_frontendWaveDataOffset = 0;

// GLOBAL: XW 0x561990
uint8_t g_frontendSavedRefillEnabled = 0;

// GLOBAL: XW 0x561994
int g_frontendPreviousCursorDistance = 0;

// GLOBAL: XW 0x561998
int g_frontendBytesPlayed = 0;

// GLOBAL: XW 0x56199C
uint8_t g_frontendRefillEnabled = 0;

// GLOBAL: XW 0x5619A0
uint8_t g_frontendDrainPending = 0;

// GLOBAL: XW 0x5619A4
unsigned int g_frontendStreamWriteOffset = 0;

// GLOBAL: XW 0x5619A8
uint8_t g_frontendLoopAsSfx = 0;

// GLOBAL: XW 0x5619AC
unsigned int g_frontendRefillRequestBytes = 0;

// GLOBAL: XW 0x5619B0
uint8_t g_frontendPlaybackStarted = 0;

// GLOBAL: XW 0x5619B4
unsigned int g_frontendPreviousPlayCursor = 0;

// GLOBAL: XW 0x5619B8
uint8_t g_frontendUsesStreamBuffer = 0;

// GLOBAL: XW 0x5619BC
unsigned int g_frontendPlayCursor = 0;

// GLOBAL: XW 0x5619C0
struct IDirectSoundBuffer* g_frontendAudioBuffer = NULL;

// GLOBAL: XW 0x5619C4
LandruHandle g_frontendStreamReadHandle = 0;

// GLOBAL: XW 0x5619C8
uint8_t g_frontendPauseDepth = 0;

// FUNCTION: XW 0x49EE40
int FrontendAudio_PlayFile(const char* filename, uint8_t loopAsSfx) {
	char fullPath[FRONTEND_AUDIO_PATH_CAPACITY];
	int result;
#ifdef XW_MODERN
	if ((loopAsSfx && XwProfile_DosFrontend()) || (!loopAsSfx && XwMusicPolicy_UsesImuse()))
		return 1;
	if (!XwStorage_HasInstallation(XW_GAME_VERSION_98))
		return 0;
#endif
	result = 0;
	FrontendAudio_StopAndRelease();
	g_frontendPauseDepth = 0;
	g_frontendLoopAsSfx = loopAsSfx;
	if (*filename != 0 && g_installDriveLetter != 0) {
		LandruFile* file;
		fullPath[0] = g_installDriveLetter;
		fullPath[1] = ':';
		fullPath[2] = '\\';
		fullPath[3] = 0;
		strcat(fullPath, filename);
		file = xfile_Open_File(LANDRU_FILE_ROOT_ASSET, fullPath, "rb");
		if (file != NULL) {
			int fileBytes;
			xfile_Seek_File(file, 0, SEEK_END);
			fileBytes = xfile_Tell_File(file);
			xfile_Close_File(file);
			if (fileBytes <= FRONTEND_AUDIO_WHOLE_FILE_LIMIT) {
				int volume;
				DirectSound_LoadFileBuffer(&g_frontendAudioBuffer, fullPath, 0);
				g_frontendRefillEnabled = 0;
				g_frontendUsesStreamBuffer = 0;
				if (g_frontendLoopAsSfx != 0) {
					if (g_shellPreferences.sfxEnabled != 0 && g_shellPreferences.sfxVolume != 0)
						volume = FRONTEND_AUDIO_VOLUME_SCALE * g_shellPreferences.sfxVolume - 1;
					else
						volume = 0;
				} else if (g_shellPreferences.musicEnabled != 0 && g_shellPreferences.musicVolume != 0) {
					volume = FRONTEND_AUDIO_VOLUME_SCALE * g_shellPreferences.musicVolume - 1;
				} else
					volume = 0;
				DirectSound_PlayBufferAtOffset(g_frontendAudioBuffer, 0, g_frontendLoopAsSfx, volume);
				g_frontendPlaybackStarted = 1;
			} else {
				g_frontendUsesStreamBuffer = 1;
#ifdef XW_MODERN
				/* VFS streams use the same asset-relative path as the caller. */
				result = FrontendAudio_StartStreamFile(filename);
				if (!result)
					Aeron_LogWarn("xw.audio", "Cannot start frontend stream: %s", filename);
#else
				result = FrontendAudio_StartStreamFile(&fullPath[FRONTEND_AUDIO_DRIVE_PREFIX_LENGTH]);
#endif
			}
		}
	}
	return result;
}

// FUNCTION: XW 0x49EFD0
int FrontendAudio_StartStreamFile(const char* filename) {
	unsigned int bytes1;
	unsigned int bytes2;
	void* region2;
	void* region1;
	unsigned int prefixBytes;
	int volume;
	if (xstream_Chain_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL, filename) != 0) {
		if (g_frontendLoopAsSfx == 0 ||
			xstream_Chain_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL, filename) != 0) {
			if (xstream_Use_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL, filename) != 0) {
				prefixBytes = FrontendAudio_CreateStreamBuffers();
				g_frontendStreamWriteOffset = prefixBytes;
				g_frontendBytesPlayed = 0;
				g_frontendPreviousPlayCursor = 0;
				if (prefixBytes == (unsigned int)-1) {
					FrontendAudio_StopAndRelease();
					return 0;
				}
				if (DirectSound_LockBufferRegion(g_frontendAudioBuffer, prefixBytes,
												 FRONTEND_AUDIO_STREAM_BYTES - prefixBytes, &region1, &bytes1,
												 &region2, &bytes2) != 0) {
					if (region1 != NULL)
						memset(region1, FRONTEND_AUDIO_SILENCE_BYTE, bytes1);
					if (region2 != NULL)
						memset(region2, FRONTEND_AUDIO_SILENCE_BYTE, bytes2);
					DirectSound_UnlockBufferRegion(g_frontendAudioBuffer, region1, bytes1, region2, bytes2);
					g_frontendRefillEnabled = 1;
					for (g_frontendDrainPending = 0;
						 (int)g_frontendStreamWriteOffset < FRONTEND_AUDIO_PRIME_BYTES;) {
						unsigned int requestedBytes =
							FRONTEND_AUDIO_PRIME_BYTES - g_frontendStreamWriteOffset;
						int bytesRead;
						if ((int)requestedBytes >= FRONTEND_AUDIO_REFILL_MAX)
							requestedBytes = FRONTEND_AUDIO_REFILL_MAX;
						bytesRead = xstream_Read_From_Stream_Buffer(
							FRONTEND_AUDIO_STREAM_CHANNEL, g_frontendStreamReadHandle, 0, requestedBytes, 1);
#ifdef XW_MODERN
						if (bytesRead < 0) {
							FrontendAudio_StopAndRelease();
							return 0;
						}
#endif
						if (bytesRead != -1) {
							if (bytesRead == 0) {
								g_frontendRefillEnabled = 0;
								break;
							}
							if (DirectSound_LockBufferRegion(g_frontendAudioBuffer,
															 g_frontendStreamWriteOffset, bytesRead, &region1,
															 &bytes1, &region2, &bytes2) != 0) {
								char* readBuffer = xmemhdl_Lock_Handle(g_frontendStreamReadHandle);
#ifdef XW_MODERN
								if (!readBuffer) {
									DirectSound_UnlockBufferRegion(g_frontendAudioBuffer, region1, bytes1,
																   region2, bytes2);
									FrontendAudio_StopAndRelease();
									return 0;
								}
#endif
								if (region1 != NULL && readBuffer != NULL) {
									memcpy(region1, readBuffer, bytes1);
									g_frontendStreamWriteOffset += bytes1;
									if ((int)g_frontendStreamWriteOffset >= FRONTEND_AUDIO_STREAM_BYTES)
										g_frontendStreamWriteOffset -= FRONTEND_AUDIO_STREAM_BYTES;
								}
								if (region2 != NULL && readBuffer != NULL) {
									memcpy(region2, readBuffer + bytes1, bytes2);
									g_frontendStreamWriteOffset = bytes2;
								}
								xmemhdl_Unlock_Handle(g_frontendStreamReadHandle);
								DirectSound_UnlockBufferRegion(g_frontendAudioBuffer, region1, bytes1,
															   region2, bytes2);
							}
#ifdef XW_MODERN
							else {
								FrontendAudio_StopAndRelease();
								return 0;
							}
#endif
						}
					}
					if (g_frontendLoopAsSfx != 0) {
						if (g_shellPreferences.sfxEnabled != 0 && g_shellPreferences.sfxVolume != 0)
							volume = FRONTEND_AUDIO_VOLUME_SCALE * g_shellPreferences.sfxVolume - 1;
						else
							volume = 0;
					} else if (g_shellPreferences.musicEnabled != 0 && g_shellPreferences.musicVolume != 0) {
						volume = FRONTEND_AUDIO_VOLUME_SCALE * g_shellPreferences.musicVolume - 1;
					} else
						volume = 0;
					DirectSound_PlayBufferAtOffset(g_frontendAudioBuffer, 0, 1, volume);
					g_frontendPlaybackStarted = 1;
					g_frontendPreviousCursorDistance = FRONTEND_AUDIO_PRIME_BYTES;
					return 1;
				}
				FrontendAudio_StopAndRelease();
				return 0;
			}
			if (g_frontendLoopAsSfx != 0)
				xstream_Unchain_Current_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL);
		}
		xstream_Unchain_Current_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL);
	}
	return 0;
}

// FUNCTION: XW 0x49F2F0
int FrontendAudio_UpdatePlayback(void) {
	if (g_frontendAudioBuffer) {
		int playCursor = DirectSound_GetPlayCursor(g_frontendAudioBuffer);
		int cursorDistance = playCursor - g_frontendStreamWriteOffset;
		int bytesPlayed;
		g_frontendPlayCursor = playCursor;
		if (cursorDistance <= 0)
#ifdef XW_MODERN
			cursorDistance = (int32_t)((uint32_t)cursorDistance + FRONTEND_AUDIO_STREAM_BYTES);
#else
			cursorDistance += FRONTEND_AUDIO_STREAM_BYTES;
#endif
		bytesPlayed = playCursor - g_frontendPreviousPlayCursor + g_frontendBytesPlayed;
		g_frontendBytesPlayed = bytesPlayed;
		if (playCursor < (int)g_frontendPreviousPlayCursor)
#ifdef XW_MODERN
			g_frontendBytesPlayed = (int32_t)((uint32_t)bytesPlayed + FRONTEND_AUDIO_STREAM_BYTES);
#else
			g_frontendBytesPlayed = bytesPlayed + FRONTEND_AUDIO_STREAM_BYTES;
#endif
		g_frontendPreviousPlayCursor = playCursor;
		if (g_frontendDrainPending == 1 && cursorDistance < g_frontendPreviousCursorDistance) {
			DirectSound_StopBuffer(g_frontendAudioBuffer);
			g_frontendPlaybackStarted = 0;
			g_frontendDrainPending = 0;
		}
		if (g_frontendRefillEnabled == 1) {
			int refillRequestBytes;
#ifdef XW_MODERN
			refillRequestBytes = (int32_t)(((uint32_t)cursorDistance - FRONTEND_AUDIO_STREAM_BYTES / 2) &
										   ~(FRONTEND_AUDIO_REFILL_ALIGNMENT - 1u));
#else
			refillRequestBytes =
				(cursorDistance - FRONTEND_AUDIO_STREAM_BYTES / 2) & ~(FRONTEND_AUDIO_REFILL_ALIGNMENT - 1);
#endif
			g_frontendRefillRequestBytes = refillRequestBytes;
			if (refillRequestBytes > FRONTEND_AUDIO_REFILL_MAX) {
				refillRequestBytes = FRONTEND_AUDIO_REFILL_MAX;
				g_frontendRefillRequestBytes = refillRequestBytes;
			}
			if (refillRequestBytes < FRONTEND_AUDIO_REFILL_MIN) {
				refillRequestBytes = FRONTEND_AUDIO_REFILL_MIN;
				g_frontendRefillRequestBytes = refillRequestBytes;
			}
			if (cursorDistance > refillRequestBytes)
				FrontendAudio_RefillStreamBuffer();
		}
		g_frontendPreviousCursorDistance = cursorDistance;
	}
	return g_frontendBytesPlayed;
}

// FUNCTION: XW 0x49F3D0
void FrontendAudio_RefillStreamBuffer(void) {
	unsigned int bytes1;
	unsigned int bytes2;
	void* region2;
	void* region1;
	int bytesRead = xstream_Read_From_Stream_Buffer(FRONTEND_AUDIO_STREAM_CHANNEL, g_frontendStreamReadHandle,
													0, g_frontendRefillRequestBytes, 0);
#ifdef XW_MODERN
	if (bytesRead < 0) {
		Aeron_LogWarn("xw.audio", "Cannot refill frontend audio stream");
		FrontendAudio_StopAndRelease();
		return;
	}
#endif
	if (bytesRead != -1) {
		if (bytesRead != 0) {
			if (DirectSound_LockBufferRegion(g_frontendAudioBuffer, g_frontendStreamWriteOffset, bytesRead,
											 &region1, &bytes1, &region2, &bytes2) != 0) {
				char* readBuffer = xmemhdl_Lock_Handle(g_frontendStreamReadHandle);
#ifdef XW_MODERN
				if (!readBuffer) {
					DirectSound_UnlockBufferRegion(g_frontendAudioBuffer, region1, bytes1, region2, bytes2);
					FrontendAudio_StopAndRelease();
					return;
				}
#endif
				if (region1 != NULL && readBuffer != NULL) {
					memcpy(region1, readBuffer, bytes1);
					g_frontendStreamWriteOffset += bytes1;
					if ((int)g_frontendStreamWriteOffset >= FRONTEND_AUDIO_STREAM_BYTES)
						g_frontendStreamWriteOffset -= FRONTEND_AUDIO_STREAM_BYTES;
				}
				if (region2 != NULL && readBuffer != NULL) {
					memcpy(region2, readBuffer + bytes1, bytes2);
					g_frontendStreamWriteOffset = bytes2;
				}
				xmemhdl_Unlock_Handle(g_frontendStreamReadHandle);
				DirectSound_UnlockBufferRegion(g_frontendAudioBuffer, region1, bytes1, region2, bytes2);
			}
#ifdef XW_MODERN
			else {
				Aeron_LogWarn("xw.audio", "Cannot write frontend audio buffer");
				FrontendAudio_StopAndRelease();
				return;
			}
#endif
		} else if (g_frontendLoopAsSfx != 0) {
			xstream_Rotate_Current_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL);
#ifdef XW_MODERN
			bytesRead = xstream_Read_From_Stream_Buffer(
				FRONTEND_AUDIO_STREAM_CHANNEL, g_frontendStreamReadHandle, 0, g_frontendWaveDataOffset, 0);
			if (bytesRead < 0 || (unsigned int)bytesRead != g_frontendWaveDataOffset) {
				Aeron_LogWarn("xw.audio", "Cannot restart frontend audio stream");
				FrontendAudio_StopAndRelease();
				return;
			}
#else
			do {
				bytesRead =
					xstream_Read_From_Stream_Buffer(FRONTEND_AUDIO_STREAM_CHANNEL, g_frontendStreamReadHandle,
													0, g_frontendWaveDataOffset, 0);
			} while (bytesRead == -1);
#endif
		} else {
			if (DirectSound_LockBufferRegion(g_frontendAudioBuffer, g_frontendStreamWriteOffset, 0, &region1,
											 &bytes1, &region2, &bytes2) != 0) {
				if (region1 != NULL)
					memset(region1, FRONTEND_AUDIO_SILENCE_BYTE, bytes1);
				if (region2 != NULL)
					memset(region2, FRONTEND_AUDIO_SILENCE_BYTE, bytes2);
				DirectSound_UnlockBufferRegion(g_frontendAudioBuffer, region1, bytes1, region2, bytes2);
			}
			g_frontendDrainPending = 1;
			g_frontendRefillEnabled = 0;
		}
	}
}

// FUNCTION: XW 0x49F5E0
void FrontendAudio_Pause(void) {
	if (!g_frontendPauseDepth) {
		uint8_t refillEnabled;
		if (g_frontendAudioBuffer && g_frontendPlaybackStarted == 1) {
			DirectSound_StopBuffer(g_frontendAudioBuffer);
			g_frontendPlayCursor = DirectSound_GetPlayCursor(g_frontendAudioBuffer);
		}
		refillEnabled = g_frontendRefillEnabled;
		g_frontendRefillEnabled = 0;
		g_frontendSavedRefillEnabled = refillEnabled;
	}
	++g_frontendPauseDepth;
}

// FUNCTION: XW 0x49F640
void FrontendAudio_Resume(void) {
	int locks = FlightDisplay_GetSurfaceLockCount();
	int i;
	for (i = 0; i < locks; ++i)
		FlightDisplay_UnlockSurface();
	if (--g_frontendPauseDepth == 0) {
		if (g_frontendAudioBuffer && g_frontendPlaybackStarted == 1) {
			int volume;
			int loopBuffer;
			if (g_frontendLoopAsSfx) {
				if (g_shellPreferences.sfxEnabled && g_shellPreferences.sfxVolume)
					volume = FRONTEND_AUDIO_VOLUME_SCALE * g_shellPreferences.sfxVolume - 1;
				else
					volume = 0;
			} else if (g_shellPreferences.musicEnabled && g_shellPreferences.musicVolume)
				volume = FRONTEND_AUDIO_VOLUME_SCALE * g_shellPreferences.musicVolume - 1;
			else
				volume = 0;
			loopBuffer = g_frontendLoopAsSfx != 0 || g_frontendUsesStreamBuffer != 0;
			DirectSound_PlayBufferAtOffset(g_frontendAudioBuffer, g_frontendPlayCursor, loopBuffer, volume);
		}
		g_frontendRefillEnabled = g_frontendSavedRefillEnabled;
	}
	for (i = 0; i < locks; ++i)
		FlightDisplay_LockSurface();
}

// FUNCTION: XW 0x49F720
int FrontendAudio_CreateStreamBuffers(void) {
	int prefixSampleBytes = -1;
	if (g_frontendAudioBuffer == NULL) {
		prefixSampleBytes = DirectSound_CreateBufferFromStreamHeader(
			&g_frontendAudioBuffer, FRONTEND_AUDIO_STREAM_BYTES, &g_frontendWaveDataOffset,
			FRONTEND_AUDIO_STREAM_CHANNEL);
		g_frontendPlaybackStarted = 0;
		g_frontendRefillEnabled = 0;
		g_frontendSavedRefillEnabled = 0;
		g_frontendStreamWriteOffset = 0;
	}
	if (g_frontendStreamReadHandle == LANDRU_NULL_HANDLE)
		g_frontendStreamReadHandle = xmemhdl_Alloc_Handle(FRONTEND_AUDIO_REFILL_MAX, 0);
	return prefixSampleBytes;
}

// FUNCTION: XW 0x49F790
void FrontendAudio_StopAndRelease(void) {
	g_frontendStreamWriteOffset = 0;
	g_frontendRefillEnabled = 0;
	g_frontendLoopAsSfx = 0;
	g_frontendDrainPending = 0;
	xstream_Unchain_Current_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL);
	xstream_Unchain_Current_Stream_File(FRONTEND_AUDIO_STREAM_CHANNEL);
	if (g_frontendAudioBuffer) {
		DirectSound_StopBuffer(g_frontendAudioBuffer);
		g_frontendPlaybackStarted = 0;
		DirectSound_ReleaseBuffer(&g_frontendAudioBuffer);
	}
	if (g_frontendStreamReadHandle) {
		xmemhdl_Free_Handle(g_frontendStreamReadHandle);
		g_frontendStreamReadHandle = 0;
	}
}
