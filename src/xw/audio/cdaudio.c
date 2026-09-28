#include "xw/audio/cdaudio.h"

#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#endif

#include "xw/flight/flight.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x561C88
int g_musicCdPlaybackComplete;

// GLOBAL: XW 0x561C8C
int g_musicCdTrackCount;

// GLOBAL: XW 0x561C90
int g_musicCdSavedAuxVolume = 0;

// GLOBAL: XW 0x561C98
uint32_t g_musicCdTrackCache[30];

// GLOBAL: XW 0x561D10
int g_musicCdCurrentTrack;

// GLOBAL: XW 0x561D14
MCIDEVICEID g_musicCdMciDeviceId;

// FUNCTION: XW 0x4A27A0
int CDAudio_Initialize(void) {
	uint32_t deviceCount;
	uintptr_t deviceIndex;
	uint32_t stereoVolume;
	uint32_t trackNumber;
	MMRESULT result;
	MCI_SET_PARMS setParams;
	MCI_STATUS_PARMS statusParams;
	MCI_OPEN_PARMSA openParams;
	AUXCAPSA caps;
	if (g_flightMainWindowHandle == NULL)
		return 0;
#ifdef XW_MODERN
	if (!XwMusicPolicy_CdAvailable())
		return 0;
	CDAudio_CloseDevice();
	g_musicCdTrackCount = 0;
#endif
	if (g_musicCdMciDeviceId != 0) {
		mciSendCommandA(g_musicCdMciDeviceId, MCI_CLOSE, 0, NULL);
		g_musicCdMciDeviceId = 0;
		memset(g_musicCdTrackCache, 0, sizeof(g_musicCdTrackCache));
		g_musicCdCurrentTrack = 0;
		g_musicCdPlaybackComplete = 0;
	}
	g_musicCdSavedAuxVolume = CDAUDIO_AUX_VOLUME_UNSAVED;
	deviceCount = auxGetNumDevs();
	for (deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
		memset(&caps, 0, sizeof(caps));
		auxGetDevCapsA(deviceIndex, &caps, sizeof(caps));
		if (caps.wTechnology == AUXCAPS_CDAUDIO && (caps.dwSupport & AUXCAPS_VOLUME) != 0 &&
			auxGetVolume(deviceIndex, &stereoVolume) == MMSYSERR_NOERROR) {
			g_musicCdSavedAuxVolume = (uint16_t)stereoVolume;
			break;
		}
	}
	/* Only fields selected by the MCI flags are initialized, as in the original. */
	openParams.lpstrDeviceType = "cdaudio";
	result = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE, &openParams);
	if (result != 0) {
#ifdef XW_MODERN
		return XwMusicPolicy_FailCd("Cannot open CD music");
#else
		g_musicCdMciDeviceId = 0;
		return 0;
#endif
	}
	g_musicCdMciDeviceId = openParams.wDeviceID;
	setParams.dwTimeFormat = MCI_FORMAT_TMSF;
	result = mciSendCommandA(g_musicCdMciDeviceId, MCI_SET, MCI_SET_TIME_FORMAT, &setParams);
	if (result == 0) {
		statusParams.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
		result = mciSendCommandA(g_musicCdMciDeviceId, MCI_STATUS, MCI_STATUS_ITEM, &statusParams);
	}
	if (result != 0) {
#ifdef XW_MODERN
		return XwMusicPolicy_FailCd("Cannot read CD music metadata");
#else
		mciSendCommandA(g_musicCdMciDeviceId, MCI_CLOSE, 0, NULL);
		g_musicCdMciDeviceId = 0;
		return 0;
#endif
	}
#ifdef XW_MODERN
	if (!statusParams.dwReturn ||
		statusParams.dwReturn > sizeof g_musicCdTrackCache / sizeof g_musicCdTrackCache[0])
		return XwMusicPolicy_FailCd("Invalid CD music track count");
#endif
	g_musicCdTrackCount = statusParams.dwReturn;
	for (trackNumber = 1; trackNumber <= (uint32_t)g_musicCdTrackCount; ++trackNumber) {
		statusParams.dwItem = MCI_STATUS_LENGTH;
		statusParams.dwTrack = trackNumber;
		result =
			mciSendCommandA(g_musicCdMciDeviceId, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, &statusParams);
		if (result != 0) {
#ifdef XW_MODERN
			return XwMusicPolicy_FailCd("Cannot read a CD music track length");
#else
			mciSendCommandA(g_musicCdMciDeviceId, MCI_CLOSE, 0, NULL);
			g_musicCdMciDeviceId = 0;
			return 0;
#endif
		}
		g_musicCdTrackCache[trackNumber - 1] = statusParams.dwReturn;
	}
#ifdef XW_MODERN
	if (CDAudio_GetTrackEndTimeMs(2) <= 0 || CDAudio_GetTrackEndTimeMs(3) <= 0 ||
		CDAudio_GetTrackEndTimeMs(7) <= 0)
		return XwMusicPolicy_FailCd("Required CD music tracks 2, 3 and 7 are unavailable");
#endif
	return 1;
}

// FUNCTION: XW 0x4A2990
int CDAudio_PlayTrackFromTime(int trackNumber, int startMinute, int startSecond) {
	MCI_PLAY_PARMS playParams;
	uint32_t trackLengthMsf;
	if (trackNumber > g_musicCdTrackCount || trackNumber <= 0 || g_musicCdMciDeviceId == 0) {
		return 0;
	}
#ifdef XW_MODERN
	if (!XwMusicPolicy_CdAvailable())
		return 0;
	if (startMinute < 0 || startMinute > 255 || startSecond < 0 || startSecond >= 60 ||
		(startMinute * 60 + startSecond) * 1000 >= CDAudio_GetTrackEndTimeMs(trackNumber))
		return XwMusicPolicy_FailCd("Invalid CD music start position");
#endif
	memset(&playParams, 0, sizeof(playParams));
	/* Retain the original word-width minute conversion, including its overlap with seconds. */
	playParams.dwFrom =
		(uint8_t)trackNumber | (((uint16_t)startMinute | ((uint8_t)startSecond << CDAUDIO_TIME_BYTE_BITS))
								<< CDAUDIO_TIME_BYTE_BITS);
	trackLengthMsf = g_musicCdTrackCache[trackNumber - 1];
	playParams.dwTo = MCI_MAKE_TMSF(trackNumber, MCI_MSF_MINUTE(trackLengthMsf),
									MCI_MSF_SECOND(trackLengthMsf), MCI_MSF_FRAME(trackLengthMsf));
	playParams.dwCallback = g_flightMainWindowHandle;
	if (mciSendCommandA(g_musicCdMciDeviceId, MCI_PLAY, MCI_NOTIFY | MCI_FROM | MCI_TO, &playParams) != 0) {
#ifdef XW_MODERN
		return XwMusicPolicy_FailCd("Cannot play CD music");
#else
		return 0;
#endif
	}
	g_musicCdPlaybackComplete = 0;
	g_musicCdCurrentTrack = trackNumber;
	return 1;
}

// FUNCTION: XW 0x4A2A60
int CDAudio_StopTrack(void) {
	MCI_GENERIC_PARMS params;

	if (g_musicCdMciDeviceId == 0) {
		return 0;
	}
	if (g_musicCdCurrentTrack == 0) {
		return 0;
	}
	/* No MCI_NOTIFY flag: the original leaves the unused callback unspecified. */
	mciSendCommandA(g_musicCdMciDeviceId, MCI_STOP, 0, &params);
	g_musicCdCurrentTrack = 0;
	g_musicCdPlaybackComplete = 0;
	return 1;
}

// FUNCTION: XW 0x4A2AB0
void CDAudio_CloseDevice(void) {
	MCI_GENERIC_PARMS stopParams;
	AUXCAPSA caps;
	int deviceCount;
	int deviceIndex;
	uint32_t stereoVolume;
	if (g_musicCdMciDeviceId == 0) {
		return;
	}
	if (g_musicCdCurrentTrack != 0) {
		/* No MCI_NOTIFY flag: the unused callback remains unspecified. */
		mciSendCommandA(g_musicCdMciDeviceId, MCI_STOP, 0, &stopParams);
		g_musicCdCurrentTrack = 0;
		g_musicCdPlaybackComplete = 0;
	}
	mciSendCommandA(g_musicCdMciDeviceId, MCI_CLOSE, 0, NULL);
	g_musicCdMciDeviceId = 0;
	memset(g_musicCdTrackCache, 0, sizeof(g_musicCdTrackCache));
	g_musicCdCurrentTrack = 0;
	g_musicCdPlaybackComplete = 0;
	deviceCount = (int)auxGetNumDevs();
	if (g_musicCdSavedAuxVolume != CDAUDIO_AUX_VOLUME_UNSAVED) {
		stereoVolume = ((uint32_t)g_musicCdSavedAuxVolume << CDAUDIO_AUX_CHANNEL_BITS) +
					   (uint32_t)g_musicCdSavedAuxVolume;
		for (deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
			memset(&caps, 0, sizeof(caps));
			auxGetDevCapsA(deviceIndex, &caps, sizeof(caps));
			if (caps.wTechnology == AUXCAPS_CDAUDIO && (caps.dwSupport & AUXCAPS_VOLUME) != 0) {
				auxSetVolume(deviceIndex, stereoVolume);
			}
		}
	}
	g_musicCdSavedAuxVolume = CDAUDIO_AUX_VOLUME_UNSAVED;
}

// FUNCTION: XW 0x4A2BA0
int CDAudio_GetTrackEndTimeMs(int trackNumber) {
	if (g_musicCdMciDeviceId != 0 && trackNumber > 0 && trackNumber <= g_musicCdTrackCount) {
		/* The original WinMM seconds extraction narrows to a word before shifting. */
		return (MCI_MSF_MINUTE(g_musicCdTrackCache[trackNumber - 1]) * 60 +
				(uint8_t)((uint16_t)g_musicCdTrackCache[trackNumber - 1] >> 8)) *
				   1000 +
			   MCI_MSF_FRAME(g_musicCdTrackCache[trackNumber - 1]) * 1000 / 75;
	}
	return 0;
}

// FUNCTION: XW 0x4A2C20
int CDAudio_SetAuxVolume(unsigned int volume0To65535) {
	uint32_t deviceCount;
	uintptr_t deviceIndex;
	uint32_t stereoVolume;
	AUXCAPSA caps;

	if (g_musicCdMciDeviceId == 0) {
		return 0;
	}
	deviceCount = auxGetNumDevs();
	if (volume0To65535 > CDAUDIO_AUX_VOLUME_MAX) {
		volume0To65535 = CDAUDIO_AUX_VOLUME_MAX;
	}
	stereoVolume = (volume0To65535 << 16) + volume0To65535;
	for (deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
		memset(&caps, 0, sizeof(caps));
		auxGetDevCapsA(deviceIndex, &caps, sizeof(caps));
		if (caps.wTechnology == AUXCAPS_CDAUDIO && (caps.dwSupport & AUXCAPS_VOLUME) != 0) {
			auxSetVolume(deviceIndex, stereoVolume);
		}
	}
	return 1;
}
