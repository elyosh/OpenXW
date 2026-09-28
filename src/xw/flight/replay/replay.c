#include "xw/flight/replay/replay.h"

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_hud.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/storage/replay_format.h"
#endif
#ifdef XW_MODERN
#include "xw_dos94/flight/hud/replay.h"
#endif

#include "xw/assets/file.h"
#include "xw/assets/model_mesh.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/flight_input.h"
#include "xw/flight/fview.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/shipext.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/replay_edit_task.h"
#include "xw_runtime/runtime/replay_save_task.h"
#include "xw_runtime/runtime/replay_screen_task.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C9780
char g_ReplayStatusStrings[REPLAY_STATUS_STRING_COUNT][REPLAY_STATUS_STRING_SIZE] = {
	"      OK", " STOPPED", "DISABLED", "CAPTURED", "        ", "  HOMING",
	"SHLDS DN", "HULL DMG", " WAITING", " CLOSING", "IN RANGE"
};

// GLOBAL: XW 0x4C9850
uint16_t g_replayButtonY[REPLAY_BUTTON_COUNT] = {
	2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   382, 382, 428, 428, 428, 428, 394, 394,
	352, 352, 352, 352, 352, 352, 352, 352, 352, 352, 383, 383, 428, 428, 428, 428, 394, 394
};

// GLOBAL: XW 0x4C98A0
uint16_t g_replayButtonX[REPLAY_BUTTON_COUNT] = {
	94, 94, 164, 164, 234, 234, 420, 420, 490, 490, 62, 62, 64, 64, 150, 150, 150, 150,
	74, 74, 124, 124, 178, 178, 348, 348, 516, 516, 84, 84, 86, 86, 171, 171, 172, 172
};

// GLOBAL: XW 0x4C98EC
uint8_t g_ReplayMusicActive = 1;

// GLOBAL: XW 0x4CF0E0
char g_ReplayStartFilename[11] = "+start.rpy";

// GLOBAL: XW 0x4CF100
char g_ReplayClipName[REPLAY_CLIP_NAME_CAPACITY] = "test";

// GLOBAL: XW 0x62AD80
XwFlightCamera g_replayCamera = { 0 };

// GLOBAL: XW 0x62B6B4
uint8_t g_ReplayFastForward = 0;

// GLOBAL: XW 0x62B6B5
uint8_t g_ReplayPlaybackActive = 0;

// GLOBAL: XW 0x62B6C0
uint8_t* g_ReplayInputPointer = NULL;

// GLOBAL: XW 0x62B91C
int16_t g_ReplayRecordedFlightAvailable = 0;

// GLOBAL: XW 0x62BAC4
unsigned int g_ReplayFrameCount = 0;

// GLOBAL: XW 0x62BAE2
int16_t g_ReplayFastForwardTimer = 0;

// GLOBAL: XW 0x62BAF8
unsigned int g_ReplayCapacityFrames = 0;

// GLOBAL: XW 0x62BC88
uint8_t* g_ReplayBufferStart = NULL;

// GLOBAL: XW 0x62BC8E
uint16_t g_ReplayRandomSeed = 0;

// GLOBAL: XW 0x62D154
unsigned int g_ReplayPlaybackFrameIndex = 0;

// GLOBAL: XW 0x62D362
uint16_t g_replayviewmode = 0;

// GLOBAL: XW 0x6377A4
unsigned int g_ReplayBufferIndex = 0;

// GLOBAL: XW 0x6377A8
uint16_t g_ReplayRecording = 0;

// GLOBAL: XW 0x6377B2
uint8_t g_ReplayReturnToExistingCheckpoint = 0;

// GLOBAL: XW 0x63A26E
int16_t g_ReplayMessageTimer = 0;

// GLOBAL: XW 0x63A270
uint16_t g_trackobject = 0;

// GLOBAL: XW 0x63A272
uint16_t g_replayChaseObjectType = 0;

// GLOBAL: XW 0x63A274
uint8_t g_ReplayReenterSimulation = 0;

// GLOBAL: XW 0x63A275
uint8_t g_ReplayChaseStatusVisible = 0;

// GLOBAL: XW 0x63A276
uint8_t g_ReplayExitRequested = 0;

// GLOBAL: XW 0x63A278
uint16_t g_replayTrackedObjectType = 0;

// GLOBAL: XW 0x63A27A
int16_t g_ReplaySavedVolume = 0;

// FUNCTION: XW 0x41C820
int replay_loadreplayinput(void) {
#ifdef XW_MODERN
	replay_replaymessage(XW_MSG_CAMERA_FOOTAGE_LOADING);
	if (!XwReplayFormat_RefillInput()) {
		XwPort_Fail(XwReplayFormat_Error());
		return 0;
	}
	return 1;
#else
	uint8_t* buffer;
	XwFile* savedStream;
	unsigned int frameIndex;
	replay_replaymessage(XW_MSG_CAMERA_FOOTAGE_LOADING);
	buffer = g_ReplayBufferStart;
	savedStream = g_stream;
	if (fediskio_tryopenfile("+input.spl", "rb", 1) != 0) {
		if (g_ReplayPlaybackFrameIndex != 0 &&
			File_RawSeek(g_stream, (int32_t)(REPLAY_INPUT_RECORD_SIZE * g_ReplayPlaybackFrameIndex),
						 SEEK_SET) != 0) {
			File_RawClose(g_stream);
			g_stream = savedStream;
			return 0;
		}
		for (frameIndex = 0; frameIndex < REPLAY_REFILL_RECORD_COUNT; ++frameIndex) {
			if (File_RawRead(&buffer[frameIndex * REPLAY_INPUT_RECORD_SIZE], sizeof(uint8_t),
							 REPLAY_INPUT_RECORD_SIZE, g_stream) != REPLAY_INPUT_RECORD_SIZE) {
				File_RawClose(g_stream);
				g_stream = savedStream;
				return 0;
			}
		}
		File_RawClose(g_stream);
		g_stream = savedStream;
		return 1;
	}
	g_stream = savedStream;
	return 0;
#endif
}

// FUNCTION: XW 0x41C900
XwFlightMessageId replay_savereplay(void) {
#ifdef XW_MODERN
	return XwReplaySave_Run();
#else
	char clipPath[REPLAY_SAVE_TEXT_CAPACITY];
	char clipName[REPLAY_SAVE_TEXT_CAPACITY];
	XwFile* clipFile;
	unsigned int blockIndex;
	unsigned int blockSize;
	replay_replaymessage(XW_MSG_ENTER_FILENAME_PROMPT);
	clipName[0] = 0;
	festring_setfontsize(FLIGHT_FONT_TINY);
	if (g_flightResolutionMode != REPLAY_SAVE_LOW_RESOLUTION_MODE)
		replay_editstring(REPLAY_SAVE_HIGH_LEFT, REPLAY_SAVE_HIGH_TOP, REPLAY_CLIP_NAME_CAPACITY - 1,
						  clipName, REPLAY_SAVE_BACKGROUND_COLOR);
	else
		replay_editstring(REPLAY_SAVE_LOW_LEFT, REPLAY_SAVE_LOW_TOP, REPLAY_CLIP_NAME_CAPACITY - 1, clipName,
						  REPLAY_SAVE_BACKGROUND_COLOR);
	festring_setfontsize(FLIGHT_FONT_MICRO);
	if (clipName[0] == 0)
		return XW_MSG_REPLAY_CLIP_NOT_SAVED;
	strcpy(clipPath, clipName);
	strcat(clipPath, ".clp");
	g_stream = File_RawOpen(clipPath, "rb");
	if (g_stream != NULL) {
		uint8_t overwriteKey;
		File_RawClose(g_stream);
		replay_replaymessage(XW_MSG_FILE_EXISTS_REPLACE_PROMPT);
		FlightDisplay_UnlockSurface();
		FlightDisplay_PresentBackBuffer();
		FlightDisplay_Flip();
		FlightDisplay_LockSurface();
		overwriteKey = FlightInput_GetNextKey();
		if (overwriteKey != 'y' && overwriteKey != 'Y')
			return XW_MSG_REPLAY_CLIP_NOT_SAVED;
	}
	if (fediskio_tryopenfile(clipPath, "wb", 0) == 0)
		return XW_MSG_FILE_ERROR;
	File_RawPutChar(g_ReplayFrameCount, g_stream);
	File_RawPutChar(g_ReplayFrameCount >> REPLAY_HEADER_BYTE_BITS, g_stream);
	File_RawPutChar(g_ReplayFrameCount >> (2 * REPLAY_HEADER_BYTE_BITS), g_stream);
	File_RawPutChar(g_ReplayFrameCount >> (3 * REPLAY_HEADER_BYTE_BITS), g_stream);
	File_RawPutChar(g_ReplayRandomSeed, g_stream);
	File_RawPutChar(g_ReplayRandomSeed >> REPLAY_HEADER_BYTE_BITS, g_stream);
	clipFile = g_stream;
	if (fediskio_tryopenfile(g_ReplayStartFilename, "rb", 0) == 0) {
		File_RawClose(g_stream);
		File_RawClose(clipFile);
		File_RawRemove(clipPath);
		return XW_MSG_FILE_ERROR;
	}
	for (blockIndex = 0, blockSize = g_ReplaySnapshotBlockSizes[0]; blockSize != 0;
		 blockSize = g_ReplaySnapshotBlockSizes[++blockIndex]) {
		if (replay_copybytesinfile(blockSize, g_stream, clipFile) == 0) {
			File_RawRemove(clipPath);
			return XW_MSG_FILE_ERROR;
		}
	}
	if (replay_copybytesinfile(REPLAYIO_DISK_FLIGHT_GROUP_SIZE * MISSION_FLIGHT_GROUP_COUNT, g_stream,
							   clipFile) == 0) {
		File_RawRemove(clipPath);
		return XW_MSG_FILE_ERROR;
	}
	if (replay_copybytesinfile(MODEL_TYPE_RECORD_COUNT, g_stream, clipFile) == 0) {
		File_RawRemove(clipPath);
		return XW_MSG_FILE_ERROR;
	}
	File_RawClose(g_stream);
	if (g_ReplaySpoolEnabled != 0) {
		unsigned int frameIndex;
		if (fediskio_tryopenfile("+input.spl", "rb", 0) == 0) {
			File_RawClose(clipFile);
			File_RawRemove(clipPath);
			return XW_MSG_FILE_ERROR;
		}
		for (frameIndex = 0; frameIndex < g_ReplayFrameCount; ++frameIndex) {
			if (replay_copybytesinfile(REPLAY_INPUT_RECORD_SIZE, g_stream, clipFile) == 0) {
				File_RawRemove(clipPath);
				return XW_MSG_FILE_ERROR;
			}
		}
		File_RawClose(g_stream);
	} else {
		uint8_t* inputBuffer = g_ReplayBufferStart;
		unsigned int inputIndex = 0;
		unsigned int frameIndex;
		for (frameIndex = 0; frameIndex < g_ReplayFrameCount; ++frameIndex) {
			unsigned int byteIndex;
			for (byteIndex = 0; byteIndex < REPLAY_INPUT_RECORD_SIZE; ++byteIndex) {
				uint16_t character = inputBuffer[inputIndex++];
				File_RawPutChar(character, clipFile);
				if (File_RawHasError(clipFile)) {
					File_RawClose(clipFile);
					File_RawRemove(clipPath);
					return XW_MSG_FILE_ERROR;
				}
			}
		}
	}
	if (File_RawClose(clipFile) == EOF) {
		File_RawRemove(clipPath);
		return XW_MSG_FILE_ERROR;
	}
	strcpy(g_ReplayClipName, clipName);
	return XW_MSG_REPLAY_CLIP_SAVED;
#endif
}

// FUNCTION: XW 0x41CD00
int16_t replay_copybytesinfile(unsigned int count, XwFile* source, XwFile* destination) {
	unsigned int bytesCopied;
	for (bytesCopied = 0; bytesCopied < count; ++bytesCopied) {
		uint16_t character = (uint16_t)File_RawGetChar(source);
		if (File_RawHasError(source)) {
			File_RawClose(source);
			File_RawClose(destination);
			return 0;
		}
		File_RawPutChar(character, destination);
		if (File_RawHasError(destination)) {
			File_RawClose(source);
			File_RawClose(destination);
			return 0;
		}
	}
	return 1;
}

// FUNCTION: XW 0x41CD80
int16_t replay_loadreplay(void) {
#ifdef XW_MODERN
	return XwReplayFormat_LoadFilm();
#else
	char clipPath[REPLAY_CLIP_PATH_CAPACITY];
	XwFile* clipFile;
	unsigned int blockIndex;
	unsigned int blockSize;
	strcpy(clipPath, g_ReplayClipName);
	if (clipPath[0] == 0)
		return 0;
	strcat(clipPath, ".clp");
	if (fediskio_tryopenfile(clipPath, "rb", 0) == 0)
		return 0;
	g_ReplayFrameCount = File_RawGetChar(g_stream);
	g_ReplayFrameCount += (uint32_t)File_RawGetChar(g_stream) << REPLAY_HEADER_BYTE_BITS;
	g_ReplayFrameCount += (uint32_t)File_RawGetChar(g_stream) << (2 * REPLAY_HEADER_BYTE_BITS);
	g_ReplayFrameCount += (uint32_t)File_RawGetChar(g_stream) << (3 * REPLAY_HEADER_BYTE_BITS);
	g_ReplayRandomSeed = File_RawGetChar(g_stream);
	g_ReplayRandomSeed += (uint32_t)File_RawGetChar(g_stream) << REPLAY_HEADER_BYTE_BITS;
	clipFile = g_stream;
	File_RemoveFromDataDirectory(g_ReplayStartFilename);
	if (fediskio_tryopenfile(g_ReplayStartFilename, "wb", 0) == 0) {
		File_RawClose(clipFile);
		return 0;
	}
	for (blockIndex = 0, blockSize = g_ReplaySnapshotBlockSizes[0]; blockSize != 0;
		 blockSize = g_ReplaySnapshotBlockSizes[++blockIndex]) {
		if (replay_copybytesinfile(blockSize, clipFile, g_stream) == 0)
			return 0;
	}
	if (replay_copybytesinfile(REPLAYIO_DISK_FLIGHT_GROUP_SIZE * MISSION_FLIGHT_GROUP_COUNT, clipFile,
							   g_stream) == 0)
		return 0;
	if (replay_copybytesinfile(MODEL_TYPE_RECORD_COUNT, clipFile, g_stream) == 0)
		return 0;
	if (File_RawClose(g_stream) == EOF) {
		File_RawClose(clipFile);
		return 0;
	}
	if (g_ReplaySpoolEnabled != 0) {
		unsigned int frameIndex;
		if (fediskio_tryopenfile("+input.spl", "wb", 0) == 0) {
			File_RawClose(clipFile);
			return 0;
		}
		for (frameIndex = 0; frameIndex < g_ReplayFrameCount; ++frameIndex) {
			if (replay_copybytesinfile(REPLAY_INPUT_RECORD_SIZE, clipFile, g_stream) == 0)
				return 0;
		}
		File_RawClose(clipFile);
		if (File_RawClose(g_stream) == EOF)
			return 0;
	} else {
		uint8_t* inputBuffer = g_ReplayBufferStart;
		unsigned int inputIndex = 0;
		unsigned int frameIndex;
		if (g_ReplayFrameCount > REPLAY_REFILL_RECORD_COUNT)
			g_ReplayFrameCount = REPLAY_REFILL_RECORD_COUNT;
		for (frameIndex = 0; frameIndex < g_ReplayFrameCount; ++frameIndex) {
			unsigned int byteIndex;
			for (byteIndex = 0; byteIndex < REPLAY_INPUT_RECORD_SIZE; ++byteIndex) {
				uint8_t character = File_RawGetChar(clipFile);
				if (File_RawHasError(clipFile)) {
					File_RawClose(clipFile);
					return 0;
				}
				inputBuffer[inputIndex++] = character;
			}
		}
		File_RawClose(clipFile);
	}
	return 1;
#endif
}

// FUNCTION: XW 0x41D0A0
void replay_rewindreplay(void) {
	char levelSection[REPLAY_LEVEL_SECTION_CAPACITY];
	if (g_flightAudioMode != 0 && g_ReplayMusicActive == 1) {
		g_ReplaySavedVolume = hilevel_ImGetMasterVol();
		hilevel_ImSetMasterVol(0);
		lolevel_ImPause();
		g_ReplayMusicActive = 0;
	}
#ifdef XW_MODERN
	if (!replayio_copyfromsave(g_ReplayStartFilename)) {
		XwPort_Fail("Cannot rewind the flight recording");
		return;
	}
#else
	replayio_copyfromsave(g_ReplayStartFilename);
#endif
	if (g_missionRuntimeState.provingGroundsActive != 0) {
		levelSection[0] = 0;
		sprintf(levelSection, "level%d", g_missionRuntimeState.provingGroundsLevel);
		shipext_Get_Mission_Path(g_currentMissionFile, levelSection, 1);
	}
	g_ReplayPlaybackFrameIndex = 0;
	g_gameRandomSeed = g_ReplayRandomSeed;
	g_ReplayProgressPercent = REPLAY_PROGRESS_INVALID;
	if (g_ReplaySpoolEnabled != 0)
		replay_loadreplayinput();
	g_ReplayBufferIndex = 0;
	g_ReplayInputPointer = g_ReplayBufferStart;
	g_ReplayPlaybackActive = 0;
	replay_drawreplaybutton(REPLAY_BUTTON_STOP);
}

// FUNCTION: XW 0x41D1C0
void replay_drawreplaybutton(uint16_t buttonIndex) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_REPLAY_BUTTONS);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_replay_drawreplaybutton(buttonIndex);
		{
#ifdef XW_MODERN
			XwHud_Pop(hud_pane);
#endif
			return;
		}
	}
#endif
	if ((uint8_t)g_flightDisplaySurfaceMode == 0 && buttonIndex < REPLAY_BUTTON_BANK_SIZE)
		buttonIndex += REPLAY_BUTTON_BANK_SIZE;
	g_flightBlitSpriteFn(g_hudPanelSpriteDataByIndex[buttonIndex + REPLAY_BUTTON_SPRITE_BASE],
						 g_replayButtonX[buttonIndex], g_replayButtonY[buttonIndex], 0, 0);
	if (buttonIndex == REPLAY_BUTTON_TRACKED_CLEAR) {
		festring_setbound(REPLAY_PANEL_NAME_LEFT, REPLAY_TRACKED_TOP, REPLAY_PANEL_NAME_RIGHT,
						  REPLAY_TRACKED_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		g_flightFillClipRectFn();
		festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP, REPLAY_PANEL_STATUS_RIGHT,
						  REPLAY_TRACKED_BOTTOM);
		g_flightFillClipRectFn();
	}
	if (buttonIndex == REPLAY_BUTTON_ALT_TRACKED_CLEAR) {
		festring_setbound(REPLAY_ALT_PANEL_NAME_LEFT, REPLAY_TRACKED_TOP, REPLAY_ALT_PANEL_NAME_RIGHT,
						  REPLAY_ALT_TRACKED_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		g_flightFillClipRectFn();
		festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP, REPLAY_ALT_PANEL_STATUS_RIGHT,
						  REPLAY_ALT_TRACKED_BOTTOM);
		g_flightFillClipRectFn();
	}
	if (buttonIndex == REPLAY_BUTTON_TRACKED_SHOW) {
		uint16_t status;
		festring_setbound(REPLAY_PANEL_NAME_LEFT, REPLAY_TRACKED_TOP, REPLAY_PANEL_NAME_RIGHT,
						  REPLAY_TRACKED_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		festring_setfontsize(FLIGHT_FONT_MICRO);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_PANEL_NAME_LEFT, REPLAY_TRACKED_TOP);
		replay_outputobjectname(g_trackobject);
		if (g_trackobject < XW_MISSION_OBJECT_REF_BASE)
			g_replayTrackedObjectType = g_objectTable[g_trackobject].objectType;
		else
			g_replayTrackedObjectType =
				g_missionObjects[g_trackobject - XW_MISSION_OBJECT_REF_BASE].objectType;
		festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP, REPLAY_PANEL_STATUS_RIGHT,
						  REPLAY_TRACKED_BOTTOM);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP);
		festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
		status = replay_getstatusnum(g_trackobject);
		festring_outstringcenter(g_ReplayStatusStrings[status]);
	}
	if (buttonIndex == REPLAY_BUTTON_ALT_TRACKED_SHOW) {
		uint16_t status;
		festring_setbound(REPLAY_ALT_PANEL_NAME_LEFT, REPLAY_TRACKED_TOP, REPLAY_ALT_PANEL_NAME_RIGHT,
						  REPLAY_ALT_TRACKED_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		festring_setfontsize(FLIGHT_FONT_MICRO);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_ALT_PANEL_NAME_LEFT, REPLAY_TRACKED_TOP);
		replay_outputobjectname(g_trackobject);
		if (g_trackobject < XW_MISSION_OBJECT_REF_BASE)
			g_replayTrackedObjectType = g_objectTable[g_trackobject].objectType;
		else
			g_replayTrackedObjectType =
				g_missionObjects[g_trackobject - XW_MISSION_OBJECT_REF_BASE].objectType;
		festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP, REPLAY_ALT_PANEL_STATUS_RIGHT,
						  REPLAY_ALT_TRACKED_BOTTOM);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP);
		festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
		status = replay_getstatusnum(g_trackobject);
		festring_outstringcenter(g_ReplayStatusStrings[status]);
	}
	if (buttonIndex == REPLAY_BUTTON_CHASE_CLEAR) {
		festring_setbound(REPLAY_PANEL_NAME_LEFT, REPLAY_CHASE_TOP, REPLAY_PANEL_NAME_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		festring_setfontsize(FLIGHT_FONT_MICRO);
		g_flightFillClipRectFn();
		festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP, REPLAY_PANEL_STATUS_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		g_flightFillClipRectFn();
		g_ReplayChaseStatusVisible = 0;
	}
	if (buttonIndex == REPLAY_BUTTON_ALT_CHASE_CLEAR) {
		festring_setbound(REPLAY_ALT_PANEL_NAME_LEFT, REPLAY_ALT_CHASE_TOP, REPLAY_ALT_PANEL_NAME_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		g_flightFillClipRectFn();
		festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP, REPLAY_ALT_PANEL_STATUS_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		g_flightFillClipRectFn();
		g_ReplayChaseStatusVisible = 0;
	}
	if (buttonIndex == REPLAY_BUTTON_CHASE_SHOW) {
		uint16_t status;
		festring_setbound(REPLAY_PANEL_NAME_LEFT, REPLAY_CHASE_TOP, REPLAY_PANEL_NAME_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		festring_setfontsize(FLIGHT_FONT_MICRO);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_PANEL_NAME_LEFT, REPLAY_CHASE_TOP);
		replay_outputobjectname(g_replayCamera.focusObjectRef);
		if (g_replayCamera.focusObjectRef < XW_MISSION_OBJECT_REF_BASE)
			g_replayChaseObjectType = g_objectTable[g_replayCamera.focusObjectRef].objectType;
		else
			g_replayChaseObjectType =
				g_missionObjects[g_replayCamera.focusObjectRef - XW_MISSION_OBJECT_REF_BASE].objectType;
		festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP, REPLAY_PANEL_STATUS_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP);
		festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
		status = replay_getstatusnum(g_replayCamera.focusObjectRef);
		festring_outstringcenter(g_ReplayStatusStrings[status]);
		g_ReplayChaseStatusVisible = 1;
	}
	if (buttonIndex == REPLAY_BUTTON_ALT_CHASE_SHOW) {
		uint16_t status;
		festring_setbound(REPLAY_ALT_PANEL_NAME_LEFT, REPLAY_ALT_CHASE_TOP, REPLAY_ALT_PANEL_NAME_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		festring_setfontsize(FLIGHT_FONT_MICRO);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_ALT_PANEL_NAME_LEFT, REPLAY_ALT_CHASE_TOP);
		replay_outputobjectname(g_replayCamera.focusObjectRef);
		if (g_replayCamera.focusObjectRef < XW_MISSION_OBJECT_REF_BASE)
			g_replayChaseObjectType = g_objectTable[g_replayCamera.focusObjectRef].objectType;
		else
			g_replayChaseObjectType =
				g_missionObjects[g_replayCamera.focusObjectRef - XW_MISSION_OBJECT_REF_BASE].objectType;
		festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP, REPLAY_ALT_PANEL_STATUS_RIGHT,
						  REPLAY_CHASE_BOTTOM);
		g_flightFillClipRectFn();
		festring_setcursor(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP);
		festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
		status = replay_getstatusnum(g_replayCamera.focusObjectRef);
		festring_outstringcenter(g_ReplayStatusStrings[status]);
		g_ReplayChaseStatusVisible = 1;
	}

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41D7B0
void replay_outputobjectname(uint16_t objectRef) {
	if (objectRef < XW_MISSION_OBJECT_REF_BASE) {
		if (g_objectTable[objectRef].iff == 0)
			festring_settextcolor(REPLAY_TYPE_COLOR_IFF0);
		else if (g_objectTable[objectRef].iff == 1)
			festring_settextcolor(REPLAY_TYPE_COLOR_IFF1);
		else
			festring_settextcolor(REPLAY_TYPE_COLOR_OTHER);
		if (g_objectTable[objectRef].familyId == XW_OBJECT_FAMILY_CRAFT) {
			CraftData* craft = g_objectTable[objectRef].instanceData;
			festring_farstrcpy(g_craftTypeDefs[craft->craftTypeIndex].shortName);
			festring_farstradd(':');
			festring_farstradd(' ');
			festring_farstradd(FLIGHT_TEXT_COLOR_ESCAPE);
			if (g_objectTable[objectRef].iff == 0)
				festring_farstradd(REPLAY_NAME_COLOR_IFF0);
			else if (g_objectTable[objectRef].iff == 1)
				festring_farstradd(REPLAY_NAME_COLOR_IFF1);
			else
				festring_farstradd(REPLAY_NAME_COLOR_OTHER);
			if (g_objectTable[objectRef].genusId != XW_GENUS_STARFIGHTER && craft->isInspected == 0 &&
				g_objectTable[objectRef].iff != g_playerFlightState.object->iff) {
				festring_farstrcat("UNKNOWN");
			} else {
				festring_farstrcat(g_missionFlightGroups[craft->flightGroupIndex].name);
				if (g_missionFlightGroups[craft->flightGroupIndex].numberOfCraft > 1) {
					festring_farstradd(' ');
					festring_farstradd(craft->craftIndexInFlightGroup + '1');
				}
			}
		} else {
			uint16_t objectType = g_objectTable[objectRef].objectType;
#ifdef XW_MODERN
			objectType = XwFlightTypes_CanonicalType(objectType);
#endif
			if (objectType == XW_OBJ_WARHEAD_149)
				festring_farstrcpy("TORPEDO");
			else if (objectType == XW_OBJ_TRACKED_WARHEAD)
				festring_farstrcpy("MISSILE");
		}
	} else {
		uint16_t objectType;
		festring_settextcolor(REPLAY_STATIC_NAME_COLOR);
		objectType = g_missionObjects[objectRef - XW_MISSION_OBJECT_REF_BASE].objectType;
#ifdef XW_MODERN
		if (XwFlightTypes_Dos() && objectType >= 30 && objectType <= 32) {
			festring_farstrcpy(g_hudBuoyNameStrings[objectType - 30]);
			festring_outstringcenter(g_flightTextScratchBuffer);
			return;
		}
		if (XwFlightTypes_IsMine(objectType)) {
#else
		if (objectType >= PANEL_MINE_OBJECT_TYPE_FIRST && objectType <= PANEL_MINE_OBJECT_TYPE_LAST) {
#endif
			festring_farstrcpy("MINE");
		} else {
			if (g_objectTypeHudShipIds[objectType] >= PANEL_BUOY_HUD_ID_FIRST &&
				g_objectTypeHudShipIds[objectType] <= PANEL_BUOY_HUD_ID_LAST) {
#ifdef XW_MODERN
				festring_farstrcpy(g_hudBuoyNameStrings[g_objectTypeHudShipIds[objectType] -
														g_objectTypeHudShipIds[XwFlightTypes_ObjectType(
															PANEL_BUOY_BASE_OBJECT_TYPE)]]);
#else
				festring_farstrcpy(g_hudBuoyNameStrings[g_objectTypeHudShipIds[objectType] -
														g_objectTypeHudShipIds[PANEL_BUOY_BASE_OBJECT_TYPE]]);
#endif
			}
		}
	}
	festring_outstringcenter(g_flightTextScratchBuffer);
}

// FUNCTION: XW 0x41D9A0
int replay_getstatusnum(uint16_t objectRef) {
	uint16_t objectType;
	const CraftData* craft;
	const WarheadGuidanceState* warhead;
	if (objectRef < XW_MISSION_OBJECT_REF_BASE) {
		objectType = g_objectTable[objectRef].objectType;
		craft = g_objectTable[objectRef].instanceData;
		warhead = g_objectTable[objectRef].instanceData;
	} else {
		objectType = g_missionObjects[objectRef - XW_MISSION_OBJECT_REF_BASE].objectType;
		craft = NULL;
		warhead = NULL;
	}
#ifdef XW_MODERN
	if (XwFlightTypes_IsWarhead(objectType)) {
#else
	if (objectType == XW_OBJ_TRACKED_WARHEAD || objectType == XW_OBJ_WARHEAD_149) {
#endif
		return REPLAY_STATUS_WARHEAD_IDLE + (warhead->homingTier != 0);
	}
	if (g_modelTypeTable[objectType].familyId == XW_OBJECT_FAMILY_CRAFT) {
		if (craft->captorFlightGroupOverride != 0) {
			return REPLAY_STATUS_CAPTURED;
		}
		if (craft->workingSubsystems == 0) {
			return REPLAY_STATUS_DISABLED;
		}
		if (craft->shieldEnergy[XW_SHIELD_REAR] + craft->shieldEnergy[XW_SHIELD_FRONT] == 0 &&
			g_objectTable[objectRef].genusId != XW_GENUS_STARFIGHTER) {
			return REPLAY_STATUS_SHIELDS_DOWN;
		}
	}
	return REPLAY_STATUS_NORMAL;
}

// FUNCTION: XW 0x41DA80
void replay_outputclipname(void) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_REPLAY_TITLE);
#endif

#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_replay_outputclipname();
		{
#ifdef XW_MODERN
			XwHud_Pop(hud_pane);
#endif
			return;
		}
	}
#endif
	festring_setbackcolor(REPLAY_CLIP_NAME_BACKGROUND_COLOR);
	if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
		festring_setbound(REPLAY_FLIGHT_CLIP_NAME_LEFT, REPLAY_FLIGHT_CLIP_NAME_TOP,
						  REPLAY_FLIGHT_CLIP_NAME_RIGHT, REPLAY_FLIGHT_CLIP_NAME_BOTTOM);
		festring_setcursor(REPLAY_FLIGHT_CLIP_NAME_LEFT, REPLAY_FLIGHT_CLIP_NAME_TOP);
	} else {
		festring_setbound(REPLAY_STANDALONE_CLIP_NAME_LEFT, REPLAY_STANDALONE_CLIP_NAME_TOP,
						  REPLAY_STANDALONE_CLIP_NAME_RIGHT, REPLAY_STANDALONE_CLIP_NAME_BOTTOM);
		festring_setcursor(REPLAY_STANDALONE_CLIP_NAME_LEFT, REPLAY_STANDALONE_CLIP_NAME_TOP);
	}
	g_flightFillClipRectFn();
	festring_settextcolor(REPLAY_CLIP_NAME_TEXT_COLOR);
	festring_outstringcenter(g_ReplayClipName);

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41DB00
void replay_doreplayscreen(void) {
#ifdef XW_MODERN
	XwReplayScreen_Begin();
#else
	uint16_t lastChaseStatus;
	uint16_t lastTrackStatus;
	int16_t remainingPercent;
	g_replayCamera.focusObjectRef = g_playerFlightState.objectIndex;
	g_replayCamera.externalViewActive = 1;
	g_replayCamera.externalDistance = REPLAY_DEFAULT_ZOOM;
	g_replayCamera.manualControlActive = 0;
	g_replayCamera.hudAimY = 0;
	g_replayCamera.hudAimX = 0;
	g_trackobject = XW_PLAYER_NO_TARGET;
	g_textureCacheFlushPending = 1;
	Xw_updatescreen();
	FlightDisplay_LockSurface();
	g_ReplayPlaybackActive = 0;
	g_ReplayFastForward = 0;
	g_ReplayFastForwardTimer = 0;
	replay_drawreplaybutton(REPLAY_BUTTON_FOLLOW);
	replay_drawreplaybutton(REPLAY_BUTTON_CHASE_SHOW);
	replay_drawreplaybutton(REPLAY_BUTTON_TRACKING_OFF);
	replay_drawreplaybutton(REPLAY_BUTTON_TRACKED_CLEAR);
	replay_outputclipname();
	FlightDisplay_UnlockSurface();
	lastChaseStatus = REPLAY_STATUS_INVALID;
	lastTrackStatus = REPLAY_STATUS_INVALID;
	g_ReplayExitRequested = 0;
	do {
		FlightDisplay_LockSurface();
		replay_replayinput();
		if (g_flightAudioMode != 0) {
			if (g_ReplayPlaybackActive != 0 && g_ReplayFastForward == 0) {
				if (g_ReplayMusicActive == 0) {
					g_ReplayMusicActive = 1;
					hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
					lolevel_ImResume();
				}
			} else if (g_ReplayMusicActive == 1) {
				g_ReplayMusicActive = 0;
				g_ReplaySavedVolume = hilevel_ImGetMasterVol();
				hilevel_ImSetMasterVol(0);
				lolevel_ImPause();
			}
		}
		if (g_ReplayPlaybackActive != 0) {
			FlightDisplay_UnlockSurface();
			Xw_doframe();
			FlightDisplay_LockSurface();
			if (g_ReplayChaseStatusVisible != 0) {
				uint16_t chaseStatus;
				festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
				if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
					festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP, REPLAY_PANEL_STATUS_RIGHT,
									  REPLAY_CHASE_BOTTOM);
					festring_setcursor(REPLAY_PANEL_STATUS_LEFT, REPLAY_CHASE_TOP);
				} else {
					festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP,
									  REPLAY_ALT_PANEL_STATUS_RIGHT, REPLAY_CHASE_BOTTOM);
					festring_setcursor(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_ALT_CHASE_TOP);
				}
				chaseStatus = replay_getstatusnum(g_replayCamera.focusObjectRef);
				if (chaseStatus != lastChaseStatus) {
					g_flightFillClipRectFn();
					festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
					festring_outstringcenter(g_ReplayStatusStrings[chaseStatus]);
					lastChaseStatus = chaseStatus;
				}
			}
			if (g_trackobject != XW_PLAYER_NO_TARGET) {
				uint16_t trackStatus;
				festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
				if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
					festring_setbound(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP, REPLAY_PANEL_STATUS_RIGHT,
									  REPLAY_TRACKED_BOTTOM);
					festring_setcursor(REPLAY_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP);
				} else {
					festring_setbound(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP,
									  REPLAY_ALT_PANEL_STATUS_RIGHT, REPLAY_ALT_TRACKED_BOTTOM);
					festring_setcursor(REPLAY_ALT_PANEL_STATUS_LEFT, REPLAY_TRACKED_TOP);
				}
				trackStatus = replay_getstatusnum(g_trackobject);
				if (trackStatus != lastTrackStatus) {
					g_flightFillClipRectFn();
					festring_settextcolor(REPLAY_PANEL_STATUS_COLOR);
					festring_outstringcenter(g_ReplayStatusStrings[trackStatus]);
					lastTrackStatus = trackStatus;
				}
			}
		} else {
			uint16_t tickBeforeDraw = g_flightAccumulatedTicks;
			FlightDisplay_UnlockSurface();
			Xw_updatescreen();
			FlightDisplay_Flip();
			if (g_useHardware3D != 0)
				RenderScene_ClearFrameBuffers();
			else
				FlightDisplay_BlitRenderSurface();
			FlightDisplay_LockSurface();
			g_elapsedTicks = g_flightAccumulatedTicks - tickBeforeDraw;
			if (g_elapsedTicks == 0)
				g_elapsedTicks = 1;
			g_simStepScale = REPLAY_ADVANCE_TICKS / g_elapsedTicks;
			if (g_simStepScale == 0)
				g_simStepScale = 1;
		}
		festring_setfontsize(REPLAY_PROGRESS_FONT_TIER);
		if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
			festring_setbound(REPLAY_FLIGHT_PROGRESS_LEFT, REPLAY_FLIGHT_PROGRESS_TOP,
							  REPLAY_FLIGHT_PROGRESS_RIGHT, REPLAY_FLIGHT_PROGRESS_BOTTOM);
			festring_setcursor(REPLAY_FLIGHT_PROGRESS_CURSOR_X, REPLAY_FLIGHT_PROGRESS_TOP);
		} else {
			festring_setbound(REPLAY_STANDALONE_PROGRESS_LEFT, REPLAY_STANDALONE_PROGRESS_TOP,
							  REPLAY_STANDALONE_PROGRESS_RIGHT, REPLAY_STANDALONE_PROGRESS_BOTTOM);
			festring_setcursor(REPLAY_STANDALONE_PROGRESS_CURSOR_X, REPLAY_STANDALONE_PROGRESS_TOP);
		}
		festring_setbackcolor(REPLAY_PANEL_BACKGROUND_COLOR);
		remainingPercent =
			REPLAY_PROGRESS_FULL -
			math2_fraction(REPLAY_PROGRESS_FULL,
						   math2_longpercentage(g_ReplayPlaybackFrameIndex, g_ReplayCapacityFrames));
		if ((uint16_t)remainingPercent > REPLAY_PROGRESS_MAXIMUM)
			remainingPercent = REPLAY_PROGRESS_MAXIMUM;
		if (remainingPercent != g_ReplayProgressPercent) {
			g_ReplayProgressPercent = remainingPercent;
			g_flightFillClipRectFn();
			festring_settextcolor(REPLAY_CLIP_NAME_TEXT_COLOR);
			panelrts_outnum(remainingPercent, REPLAY_PROGRESS_DIGITS, REPLAY_PROGRESS_DIGITS);
		}
		if (g_elapsedTicks < (uint16_t)g_ReplayMessageTimer) {
			g_ReplayMessageTimer -= g_elapsedTicks;
		} else {
			if (g_ReplayMessageTimer != 0) {
				festring_setbackcolor(REPLAY_SAVE_BACKGROUND_COLOR);
				if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H)
					festring_setbound(0, REPLAY_LOW_MESSAGE_TOP, FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH,
									  FLIGHT_DISPLAY_LOW_RESOLUTION_HEIGHT);
				else
					festring_setbound(0, REPLAY_HIGH_MESSAGE_TOP, FLIGHT_DISPLAY_WIDTH,
									  FLIGHT_DISPLAY_HEIGHT);
				g_flightFillClipRectFn();
			}
			g_ReplayMessageTimer = 0;
		}
		FlightDisplay_UnlockSurface();
	} while (g_ReplayExitRequested == 0);

#endif
}

// FUNCTION: XW 0x41DF70
void replay_replayinput(void) {
	uint16_t savedPlayerRef;
	uint16_t chaseRef;
	uint16_t savedPlayerRefForTracking;
	uint16_t trackRef;
	uint16_t startRef;
	XwFlightMessageId saveMessageId;
	XwObjectTypeId objectType;
	int16_t mouseButtons;
	int16_t moveStep;
	int16_t chaseYawDelta;
	int16_t chasePitchDelta;
	int16_t elapsedMoveStep;
	int moveX;
	int moveY;
	int moveZ;
	int cameraX;
	int cameraY;
	int cameraZ;
	int16_t yawDelta;
	int16_t pitchDelta;

#ifdef XW_MODERN
	XwFlightClock cameraClock;
	XwReplayInputSaveState saveState = XwReplayInput_ResumeSave();
	if (saveState == XW_REPLAY_INPUT_SAVE_PENDING)
		return;
	if (saveState == XW_REPLAY_INPUT_SAVE_FINISHED)
		XwReplayScreen_ResetCameraClock();
	if (saveState == XW_REPLAY_INPUT_SAVE_IDLE) {
#endif
		feinput_getrawinput();
		feinput_checkinput();
		if (g_currentActionKey != 0) {
			switch (g_currentActionKey) {
				case 'P':
				case 'p':
					if (g_ReplayPlaybackActive == 0) {
						if (g_ReplayPlaybackFrameIndex < g_ReplayFrameCount) {
							g_ReplayPlaybackActive = 1;
							replay_replaymessage(XW_MSG_FILM_STARTED);
							replay_drawreplaybutton(REPLAY_BUTTON_PLAY);
						} else {
							replay_replaymessage(XW_MSG_FILM_END_REACHED);
						}
					} else {
						g_ReplayPlaybackActive = 0;
						replay_replaymessage(XW_MSG_FILM_STOPPED);
						replay_drawreplaybutton(REPLAY_BUTTON_STOP);
					}
					break;
				case 'A':
				case 'a':
					if (g_ReplayFastForward == 0) {
						if (g_ReplayPlaybackFrameIndex < g_ReplayFrameCount) {
							g_ReplayFastForward = 1;
							g_ReplayFastForwardTimer = REPLAY_ADVANCE_TICKS;
							g_ReplayPlaybackActive = 1;
							replay_replaymessage(XW_MSG_FILM_ADVANCE_ON);
							replay_drawreplaybutton(REPLAY_BUTTON_ADVANCE_ON);
							replay_drawreplaybutton(REPLAY_BUTTON_PLAY);
						} else {
							replay_replaymessage(XW_MSG_FILM_END_REACHED);
						}
					} else {
						g_ReplayFastForward = 0;
						g_ReplayFastForwardTimer = 0;
						replay_replaymessage(XW_MSG_FILM_ADVANCE_OFF);
						replay_drawreplaybutton(REPLAY_BUTTON_ADVANCE_OFF);
					}
					break;
				case 'R':
				case 'r':
					replay_drawreplaybutton(REPLAY_BUTTON_REWIND);
					replay_rewindreplay();
					replay_replaymessage(XW_MSG_FILM_REWOUND);
					replay_drawreplaybutton(REPLAY_BUTTON_REWIND_IDLE);
					break;
				case 'E':
				case 'e':
					replay_drawreplaybutton(REPLAY_BUTTON_EXIT);
					g_ReplayPlaybackActive = 0;
					g_ReplayExitRequested = 1;
					g_missionRuntimeState.flightExitReason = REPLAY_EXIT_FILM;
					break;
				case 'L':
				case 'l':
					if ((uint8_t)g_flightDisplaySurfaceMode == 0) {
						replay_drawreplaybutton(REPLAY_BUTTON_LOAD);
						g_ReplayPlaybackActive = 0;
						g_ReplayExitRequested = 1;
						g_missionRuntimeState.flightExitReason = REPLAY_EXIT_LOAD;
					}
					break;
				case REPLAY_KEY_ESCAPE:
					g_ReplayPlaybackActive = 0;
					g_ReplayExitRequested = 1;
					if ((uint8_t)g_flightDisplaySurfaceMode != 0)
						g_ReplayReturnToExistingCheckpoint = 1;
					else
						g_missionRuntimeState.flightExitReason = REPLAY_EXIT_TO_MENU;
					break;
				case 'C':
				case 'c':
					savedPlayerRef = g_playerFlightState.objectIndex;
					g_playerFlightState.objectIndex = REPLAY_TARGET_SELECTION_PLAYER_REF;
					if (g_currentActionKey == 'C')
						chaseRef = user_picknexttarget(g_replayCamera.focusObjectRef, -1);
					else
						chaseRef = user_picknexttarget(g_replayCamera.focusObjectRef, 1);
					g_replayCamera.focusObjectRef = chaseRef;
					g_playerFlightState.objectIndex = savedPlayerRef;
					replay_drawreplaybutton(REPLAY_BUTTON_CHASE_SHOW);
					if (g_replayCamera.manualControlActive != 0)
						replay_movecambehind(g_replayCamera.focusObjectRef);
					break;
				case 'F':
				case 'f':
					if (g_replayCamera.manualControlActive != 0) {
						g_replayCamera.manualControlActive = 0;
						replay_replaymessage(XW_MSG_CAMERA_FOLLOW_MODE);
						replay_drawreplaybutton(REPLAY_BUTTON_FOLLOW);
						replay_drawreplaybutton(REPLAY_BUTTON_CHASE_SHOW);
					} else {
						g_replayCamera.manualControlActive = 1;
						create_getworldposition(g_replayCamera.focusObjectRef, 0);
#ifdef XW_MODERN
						XwFlightCamera_CartesianToPolar(
							(int32_t)((uint32_t)g_resolvedWorldX - (uint32_t)g_flightCamera.worldPosition.x),
							(int32_t)((uint32_t)g_resolvedWorldY - (uint32_t)g_flightCamera.worldPosition.y),
							(int32_t)((uint32_t)g_resolvedWorldZ - (uint32_t)g_flightCamera.worldPosition.z));
#else
					trig2_ctop(g_resolvedWorldX - g_flightCamera.worldPosition.x,
							   g_resolvedWorldY - g_flightCamera.worldPosition.y,
							   g_resolvedWorldZ - g_flightCamera.worldPosition.z);
#endif
						g_replayCamera.viewRoll = 0;
						g_replayCamera.viewPitch = g_trig2Pitch;
						g_replayCamera.viewYaw = g_trig2Yaw;
						replay_replaymessage(XW_MSG_CAMERA_FREE_MODE);
						replay_drawreplaybutton(REPLAY_BUTTON_FREE);
						replay_drawreplaybutton(REPLAY_BUTTON_CHASE_CLEAR);
					}
					break;
				case 'O':
				case 'o':
					savedPlayerRefForTracking = g_playerFlightState.objectIndex;
					g_playerFlightState.objectIndex = REPLAY_TARGET_SELECTION_PLAYER_REF;
					if (g_currentActionKey == 'O')
						trackRef = user_picknexttarget(g_trackobject, -1);
					else
						trackRef = user_picknexttarget(g_trackobject, 1);
					g_trackobject = trackRef;
					g_playerFlightState.objectIndex = savedPlayerRefForTracking;
					replay_drawreplaybutton(REPLAY_BUTTON_TRACKING_ON);
					replay_drawreplaybutton(REPLAY_BUTTON_TRACKED_SHOW);
					break;
				case 'T':
				case 't':
					startRef = XW_PLAYER_NO_TARGET;
					if (g_trackobject == XW_PLAYER_NO_TARGET) {
						savedPlayerRefForTracking = g_playerFlightState.objectIndex;
						g_playerFlightState.objectIndex = REPLAY_TARGET_SELECTION_PLAYER_REF;
						trackRef = user_picknexttarget(startRef, 1);
						g_trackobject = trackRef;
						g_playerFlightState.objectIndex = savedPlayerRefForTracking;
						replay_drawreplaybutton(REPLAY_BUTTON_TRACKING_ON);
						replay_drawreplaybutton(REPLAY_BUTTON_TRACKED_SHOW);
					} else {
						g_trackobject = XW_PLAYER_NO_TARGET;
						replay_drawreplaybutton(REPLAY_BUTTON_TRACKING_OFF);
						replay_drawreplaybutton(REPLAY_BUTTON_TRACKED_CLEAR);
					}
					break;
				case 'S':
				case 's':
					if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
						if (g_flightAudioMode != 0 && g_ReplayMusicActive == 1) {
							g_ReplayMusicActive = 0;
							g_ReplaySavedVolume = hilevel_ImGetMasterVol();
							hilevel_ImSetMasterVol(0);
							lolevel_ImPause();
						}
#ifdef XW_MODERN
						XwRenderCapture_BeginReplayOverlay();
#endif
						FlightDisplay_UnlockSurface();
						FlightDisplay_Flip();
#ifdef XW_MODERN
						/* A real page flip changes which semantic owner backs the editor. */
						XwRenderCapture_BeginReplayOverlay();
#endif
						FlightDisplay_PresentBackBuffer();
						g_flightLockOffscreenSurface = 0;
						FlightDisplay_LockSurface();
						replay_drawreplaybutton(REPLAY_BUTTON_SAVE);
						saveMessageId = replay_savereplay();
#ifdef XW_MODERN
						if (saveMessageId == 0) {
							XwReplayInput_WaitForSave();
							return;
						}
#endif
						FlightDisplay_UnlockSurface();
						g_flightLockOffscreenSurface = 1;
						FlightDisplay_LockSurface();
						replay_replaymessage(saveMessageId);
						replay_outputclipname();
					} else {
						objectType = g_playerFlightState.object->objectType;
#ifdef XW_MODERN
						objectType = XwFlightTypes_CanonicalType(objectType);
#endif
						if (((objectType != XW_OBJ_NONE && objectType <= XW_OBJ_A_WING) ||
							 objectType == XW_OBJ_B_WING) &&
							g_playerFlightState.craft->objectKind == 0) {
							replay_drawreplaybutton(REPLAY_BUTTON_REENTER);
							g_ReplayPlaybackActive = 0;
							g_ReplayExitRequested = 1;
							g_ReplayReenterSimulation = 1;
						} else {
							replay_replaymessage(XW_MSG_SIMULATION_ENTRY_UNAVAILABLE);
						}
					}
					break;
				default:
					break;
			}
		}
#ifdef XW_MODERN
	}
	if (!XwReplayScreen_BeginCameraControls(&cameraClock))
		return;
#endif
	mouseButtons = g_flightKeyMods & REPLAY_MOUSE_BUTTON_MASK;
	if (mouseButtons == REPLAY_MOUSE_FORWARD || mouseButtons == REPLAY_MOUSE_BACKWARD) {
		moveStep = g_replayCamera.movementStep + REPLAY_MOVE_ACCELERATION;
		g_replayCamera.movementStep += REPLAY_MOVE_ACCELERATION;
		if (g_replayCamera.movementStep > REPLAY_MOVE_MAX) {
			moveStep = REPLAY_MOVE_MAX;
			g_replayCamera.movementStep = REPLAY_MOVE_MAX;
		}
		if (mouseButtons == REPLAY_MOUSE_FORWARD) {
			g_replayCamera.externalDistance -= user_framerateadjust(moveStep);
			if (g_replayCamera.externalDistance < 0)
				g_replayCamera.externalDistance = 0;
		} else {
			g_replayCamera.externalDistance += user_framerateadjust(moveStep);
			if (g_replayCamera.externalDistance > REPLAY_ZOOM_MAX)
				g_replayCamera.externalDistance = REPLAY_ZOOM_MAX;
		}
	} else {
		g_replayCamera.movementStep = REPLAY_MOVE_INITIAL;
	}
	if (g_replayCamera.manualControlActive == 0) {
		chaseYawDelta = user_framerateadjust(g_scaledInputYaw);
		chasePitchDelta = user_framerateadjust(g_scaledInputPitch);
		g_replayCamera.hudAimY += chaseYawDelta;
		g_replayCamera.hudAimX += chasePitchDelta;
	} else {
		fview_calcrotatemove(g_replayCamera.viewPitch, g_replayCamera.viewYaw, NULL);
		elapsedMoveStep = user_framerateadjust(g_replayCamera.movementStep);
		moveX = (g_craftMoveX * elapsedMoveStep) >> TRANSFM2_MATRIX_FRACTION_BITS;
		moveY = (g_craftMoveZ * elapsedMoveStep) >> TRANSFM2_MATRIX_FRACTION_BITS;
		moveZ = (g_craftMoveY * elapsedMoveStep) >> TRANSFM2_MATRIX_FRACTION_BITS;
		if (mouseButtons == REPLAY_MOUSE_BACKWARD) {
			moveX = -moveX;
			moveY = -moveY;
			moveZ = -moveZ;
		}
		if (mouseButtons == REPLAY_MOUSE_FORWARD || mouseButtons == REPLAY_MOUSE_BACKWARD) {
			g_replayCamera.worldPosition.x += (int16_t)moveX;
			cameraX = g_replayCamera.worldPosition.x;
			if (g_replayCamera.worldPosition.x < -REPLAY_WORLD_LIMIT) {
				cameraX = -REPLAY_WORLD_LIMIT;
				g_replayCamera.worldPosition.x = -REPLAY_WORLD_LIMIT;
			}
			if (cameraX > REPLAY_WORLD_LIMIT)
				g_replayCamera.worldPosition.x = REPLAY_WORLD_LIMIT;
			g_replayCamera.worldPosition.y += (int16_t)moveY;
			cameraY = g_replayCamera.worldPosition.y;
			if (g_replayCamera.worldPosition.y < -REPLAY_WORLD_LIMIT) {
				cameraY = -REPLAY_WORLD_LIMIT;
				g_replayCamera.worldPosition.y = -REPLAY_WORLD_LIMIT;
			}
			if (cameraY > REPLAY_WORLD_LIMIT)
				g_replayCamera.worldPosition.y = REPLAY_WORLD_LIMIT;
			cameraZ = (int16_t)moveZ + g_replayCamera.worldPosition.z;
			g_replayCamera.worldPosition.z = cameraZ;
			if (cameraZ < -REPLAY_WORLD_LIMIT) {
				cameraZ = -REPLAY_WORLD_LIMIT;
				g_replayCamera.worldPosition.z = -REPLAY_WORLD_LIMIT;
			}
			if (cameraZ > REPLAY_WORLD_LIMIT)
				g_replayCamera.worldPosition.z = REPLAY_WORLD_LIMIT;
		}
		yawDelta = user_framerateadjust(g_scaledInputYaw);
		pitchDelta = user_framerateadjust(g_scaledInputPitch);
		g_replayCamera.viewYaw += yawDelta;
		g_replayCamera.viewPitch -= pitchDelta;
	}
#ifdef XW_MODERN
	XwFlightTiming_RestoreClock(cameraClock);
#endif
}

// FUNCTION: XW 0x41E6A0
void replay_calcreplayview(void) {
	uint16_t focusObjectRef = g_replayCamera.focusObjectRef;
	uint16_t objectType;
	int16_t roll, pitch, yaw;
	int16_t missionAngle;
	if (g_replayCamera.focusObjectRef < XW_MISSION_OBJECT_REF_BASE)
		objectType = g_objectTable[focusObjectRef].objectType;
	else
		objectType = g_missionObjects[focusObjectRef - XW_MISSION_OBJECT_REF_BASE].objectType;
	if (objectType != g_replayChaseObjectType) {
		g_replayCamera.focusObjectRef = g_playerFlightState.objectIndex;
		replay_drawreplaybutton(REPLAY_BUTTON_CHASE_SHOW);
		focusObjectRef = g_replayCamera.focusObjectRef;
	}
	if (g_replayCamera.manualControlActive == 0) {
		if (g_replayCamera.focusObjectRef < XW_MISSION_OBJECT_REF_BASE) {
			roll = g_objectTable[focusObjectRef].roll;
			pitch = g_objectTable[focusObjectRef].pitch;
			yaw = g_objectTable[focusObjectRef].yaw;
			g_replayCamera.viewRoll = roll;
			g_replayCamera.viewPitch = pitch;
		} else {
			missionAngle = g_missionObjects[focusObjectRef - XW_MISSION_OBJECT_REF_BASE].pitchAngle8
						   << REPLAY_MISSION_ANGLE_SHIFT;
			pitch = missionAngle;
			roll = g_missionObjects[focusObjectRef - XW_MISSION_OBJECT_REF_BASE].rollAngle8
				   << REPLAY_MISSION_ANGLE_SHIFT;
			missionAngle = g_missionObjects[focusObjectRef - XW_MISSION_OBJECT_REF_BASE].yawAngle8
						   << REPLAY_MISSION_ANGLE_SHIFT;
			g_replayCamera.viewRoll = roll;
			g_replayCamera.viewPitch = pitch;
			yaw = missionAngle;
		}
		g_replayCamera.viewYaw = yaw;
		g_flightCamera.viewPitch = pitch;
		g_flightCamera.viewYaw = yaw;
		g_flightCamera.viewRoll = roll;
		g_flightCamera.hudAimY = g_replayCamera.hudAimY;
		g_flightCamera.hudAimX = g_replayCamera.hudAimX;
		fview_newcalcview(roll, pitch, yaw, 0, g_replayCamera.hudAimX, g_replayCamera.hudAimY, NULL);
		replay_movecambehind(g_replayCamera.focusObjectRef);
		g_flightCamera.worldPosition = g_replayCamera.worldPosition;
	} else {
		g_flightCamera.worldPosition = g_replayCamera.worldPosition;
		g_replayCamera.manualControlActive = REPLAY_FREE_CAMERA_ACTIVE;
		g_flightCamera.viewPitch = g_replayCamera.viewPitch;
		g_flightCamera.viewYaw = g_replayCamera.viewYaw;
		g_flightCamera.viewRoll = 0;
		g_flightCamera.hudAimY = 0;
		g_flightCamera.hudAimX = 0;
		fview_newcalcview(0, g_replayCamera.viewPitch, g_replayCamera.viewYaw, 0, 0, 0, NULL);
	}
	if (g_trackobject != XW_OBJECT_SLOT_UNAVAILABLE) {
		if (g_trackobject < XW_MISSION_OBJECT_REF_BASE)
			objectType = g_objectTable[g_trackobject].objectType;
		else
			objectType = g_missionObjects[g_trackobject - XW_MISSION_OBJECT_REF_BASE].objectType;
		if (objectType != g_replayTrackedObjectType) {
			g_trackobject = XW_OBJECT_SLOT_UNAVAILABLE;
			replay_drawreplaybutton(REPLAY_BUTTON_TRACKING_OFF);
			replay_drawreplaybutton(REPLAY_BUTTON_TRACKED_CLEAR);
			return;
		}
		create_getworldposition(g_trackobject, 0);
#ifdef XW_MODERN
		XwFlightCamera_CartesianToPolar(
			(int32_t)((uint32_t)g_resolvedWorldX - (uint32_t)g_flightCamera.worldPosition.x),
			(int32_t)((uint32_t)g_resolvedWorldY - (uint32_t)g_flightCamera.worldPosition.y),
			(int32_t)((uint32_t)g_resolvedWorldZ - (uint32_t)g_flightCamera.worldPosition.z));
#else
		trig2_ctop(g_resolvedWorldX - g_flightCamera.worldPosition.x,
				   g_resolvedWorldY - g_flightCamera.worldPosition.y,
				   g_resolvedWorldZ - g_flightCamera.worldPosition.z);
#endif
		g_flightCamera.viewRoll = 0;
		g_replayCamera.viewRoll = 0;
		g_flightCamera.viewPitch = g_trig2Pitch;
		g_replayCamera.viewPitch = g_trig2Pitch;
		g_flightCamera.viewYaw = g_trig2Yaw;
		g_replayCamera.viewYaw = g_trig2Yaw;
		g_flightCamera.hudAimY = 0;
		g_flightCamera.hudAimX = 0;
		fview_newcalcview(0, g_trig2Pitch, g_trig2Yaw, 0, 0, 0, NULL);
	}
}

// FUNCTION: XW 0x41E980
void replay_movecambehind(uint16_t objectRef) {
	uint16_t objectType;
	int16_t halfExtent;
	uint32_t clearanceX, clearanceY, clearanceZ;
	uint32_t distanceX, distanceY, distanceZ;
	create_getworldposition(objectRef, 0);
	g_replayCamera.worldPosition.x = g_resolvedWorldX;
	g_replayCamera.worldPosition.y = g_resolvedWorldY;
	g_replayCamera.worldPosition.z = g_resolvedWorldZ;
	if (objectRef < XW_MISSION_OBJECT_REF_BASE) {
		objectType = g_objectTable[objectRef].objectType;
	} else {
		objectType = g_missionObjects[objectRef - XW_MISSION_OBJECT_REF_BASE].objectType;
	}
	halfExtent = (int16_t)(g_modelTypeTable[objectType].maxBoundsExtent >> 1);
	clearanceX = (uint32_t)((uint64_t)((int64_t)halfExtent * g_camMatR2_X) >> TRANSFM2_MATRIX_FRACTION_BITS);
	clearanceY = (uint32_t)((uint64_t)((int64_t)halfExtent * g_camMatR2_Y) >> TRANSFM2_MATRIX_FRACTION_BITS);
	clearanceZ = (uint32_t)((uint64_t)((int64_t)halfExtent * g_camMatR2_Z) >> TRANSFM2_MATRIX_FRACTION_BITS);
	if (objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER || objectType == XW_OBJ_CALAMARI_CRUISER) {
		clearanceX *= REPLAY_CAPITAL_CLEARANCE_HALF_EXTENTS;
		clearanceY *= REPLAY_CAPITAL_CLEARANCE_HALF_EXTENTS;
		clearanceZ *= REPLAY_CAPITAL_CLEARANCE_HALF_EXTENTS;
	} else {
		clearanceX *= REPLAY_NORMAL_CLEARANCE_HALF_EXTENTS;
		clearanceY *= REPLAY_NORMAL_CLEARANCE_HALF_EXTENTS;
		clearanceZ *= REPLAY_NORMAL_CLEARANCE_HALF_EXTENTS;
	}
	distanceX = (uint32_t)((uint64_t)((int64_t)g_replayCamera.externalDistance * g_camMatR2_X) >>
						   TRANSFM2_MATRIX_FRACTION_BITS);
	distanceY = (uint32_t)((uint64_t)((int64_t)g_replayCamera.externalDistance * g_camMatR2_Y) >>
						   TRANSFM2_MATRIX_FRACTION_BITS);
	distanceZ = (uint32_t)((uint64_t)((int64_t)g_replayCamera.externalDistance * g_camMatR2_Z) >>
						   TRANSFM2_MATRIX_FRACTION_BITS);
	g_replayCamera.worldPosition.x =
		(int32_t)((uint32_t)g_replayCamera.worldPosition.x - (clearanceX + distanceX));
	g_replayCamera.worldPosition.y =
		(int32_t)((uint32_t)g_replayCamera.worldPosition.y - (clearanceY + distanceY));
	g_replayCamera.worldPosition.z =
		(int32_t)((uint32_t)g_replayCamera.worldPosition.z - (clearanceZ + distanceZ));
}

// FUNCTION: XW 0x41EB20
void replay_replaymessage(XwFlightMessageId messageId) {
#ifdef XW_MODERN
	int hud_pane = XwHud_Push(XW_SNAP_PANE_REPLAY_STATUS);
#endif

	const unsigned char* message;
	unsigned int index;
	unsigned int character;
	char lastCharacter;
	msg_readymessage();
	lastCharacter = (char)messageId;
	message = (const unsigned char*)g_flightMessageTemplates[messageId];
	for (index = 0; (character = message[index]) != '\0'; ++index) {
		if ((unsigned char)character < FLIGHT_TEXT_FIRST_DRAWABLE_BYTE) {
			festring_settextcolor(character + FLIGHT_TEXT_ENCODED_COLOR_BASE);
		} else {
			g_flightDrawCharFn(character);
			lastCharacter = message[index];
		}
	}
	if (lastCharacter != '?' && lastCharacter != '!' && lastCharacter != ':' && lastCharacter != ' ') {
		g_flightDrawCharFn('.');
	}
	festring_setautofill(1);
	g_flightDrawCharFn('\n');
	festring_setautofill(0);
	festring_setfontsize(FLIGHT_FONT_MICRO);
	g_ReplayMessageTimer = REPLAY_MESSAGE_DURATION_TICKS;

#ifdef XW_MODERN
	XwHud_Pop(hud_pane);
#endif
}

// FUNCTION: XW 0x41EBC0
void replay_editstring(int16_t x, int16_t y, uint8_t maxLength, char* text, uint8_t backgroundColor) {
#ifdef XW_MODERN
	XwReplayEdit_Push(x, y, maxLength, text, backgroundColor);
#else
	uint8_t length;
	uint8_t writeIndex;
	festring_setautofill(1);
	for (length = 0; text[length] != '\n' && text[length] != '\0' && length < maxLength; ++length) {
	}
	writeIndex = length;
	festring_setcursor(x, y);
	festring_outstring(text);
	festring_setbackcolor(REPLAY_EDIT_CARET_COLOR);
	g_flightDrawCharFn(' ');
	festring_setbackcolor(backgroundColor);
	g_flightDrawCharFn('\n');
	do {
		feinput_getinput();
		if ((int16_t)g_actionKey == REPLAY_EDIT_DELETE_ACTION) {
			g_actionKey = '\b';
		}
		if ((int16_t)g_actionKey == '\b' && length > 0)
			writeIndex = --length;
		if ((int16_t)g_actionKey < '0') {
			if ((int16_t)g_actionKey != '\r' && (int16_t)g_actionKey != '-' && (int16_t)g_actionKey != 0) {
				g_actionKey = REPLAY_EDIT_REJECTED_KEY;
			}
		} else if (((int16_t)g_actionKey < 'A' && (int16_t)g_actionKey >= '9' + 1) ||
				   ((int16_t)g_actionKey < 'a' && (int16_t)g_actionKey >= 'Z' + 1) ||
				   (int16_t)g_actionKey >= 'z' + 1) {
			g_actionKey = REPLAY_EDIT_REJECTED_KEY;
		}
		if ((int16_t)g_actionKey != REPLAY_EDIT_REJECTED_KEY && (int16_t)g_actionKey != '\r' &&
			(int16_t)g_actionKey != 0 && length < maxLength) {
			uint8_t character = (uint8_t)g_actionKey;
			text[writeIndex] = (char)character;
			if (character >= 'a' && character <= 'z')
				text[writeIndex] = (char)(character - ('a' - 'A'));
			writeIndex = ++length;
		}
		text[writeIndex] = '\0';
		if (g_actionKey != 0) {
			festring_setcursor(x, y);
			festring_outstring(text);
			festring_setbackcolor(REPLAY_EDIT_CARET_COLOR);
			g_flightDrawCharFn(' ');
			festring_setbackcolor(backgroundColor);
			g_flightDrawCharFn('\n');
		}
		FlightDisplay_UnlockSurface();
		FlightDisplay_PresentBackBuffer();
		FlightDisplay_Flip();
		FlightDisplay_LockSurface();
	} while (g_actionKey != '\r');
	festring_setcursor(x, y);
	festring_outstring(text);
	g_flightDrawCharFn('\n');
	festring_setautofill(0);
	FlightDisplay_UnlockSurface();
	FlightDisplay_PresentBackBuffer();
	FlightDisplay_Flip();
	FlightDisplay_LockSurface();
#endif
}
