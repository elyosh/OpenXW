#include "xw/flight/replay/replayio.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_types.h"
#endif
#ifdef XW_MODERN
#include "xw_dos94/flight/hud/panel.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/file.h"
#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/transfm2.h"
#include "xw/render/flight_starfield.h"
#include "xw/render/flight_view.h"
#include "xw/render/rtsvga2.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/replay_viewer_task.h"
#include "xw_runtime/storage/replay_format.h"
#endif

#include <landru/timer.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C9910
void* g_ReplaySnapshotBlockPointers[REPLAYIO_SNAPSHOT_TABLE_COUNT] = { &g_objectTable,
																	   &g_missionObjects,
																	   &g_craftTable,
																	   &g_warheadGuidanceTable,
																	   &g_missionElapsedClock,
																	   &g_missionCountdownClock,
																	   &g_missionHeader,
																	   &g_missionFlightGroupStates,
																	   &g_missionRuntimeState,
																	   &g_playerFlightState,
																	   &g_flightCamera,
																	   &g_snapshotField62D12A,
																	   &g_snapshotField62D0E2,
																	   &g_snapshotField62D118,
																	   &g_targetHighlightObjectAndBlinkBits,
																	   &g_targetHighlightBlinkTicks,
																	   &g_flightGlobalCountdownTimers,
																	   &g_nextDebrisObjectSlot,
																	   &g_surfaceCellDamageStates,
																	   &g_surfaceGunCells,
																	   &g_surfaceGunCellCount,
																	   &g_surfaceGunCellRefreshTicks,
																	   &g_backdropPackedDirections,
																	   &g_backdropModelTypes,
																	   &g_backdropPositiveYCount,
																	   &g_backdropNegativeYCount,
																	   &g_backdropPositiveZCount,
																	   &g_backdropNegativeZCount,
																	   &g_backdropPositiveXCount,
																	   &g_backdropNegativeXCount,
																	   &g_starDensity,
																	   &g_deathStarSurfaceModeActive,
																	   &g_surfaceGoalCellHashSlots,
																	   &g_surfaceGoalCellKeys,
																	   &g_backdropsEnabled,
																	   &g_debrisEnabled,
																	   &g_craftExplosionSpawnThreshold,
																	   &g_starshipDetail,
																	   &g_surfaceObjectDetailLimit,
																	   &g_trenchObjectDetailLimit,
																	   &g_deathStarDetailLevel,
																	   &g_hyperspaceEffectObjectCount,
																	   &g_shipDetailValue,
																	   &g_shipDetailPolyCount,
																	   &g_drawMarkingsFlag,
																	   &g_flightGraphicsDetailPreset,
																	   &g_missionCheatOptionsUsed,
																	   &g_flightMusicVolume,
																	   &g_flightMusicEnabled,
																	   &g_flightSfxVolume,
																	   &g_flightSfxEnabled,
																	   &g_flightEngineSoundEnabled,
																	   &g_flightDigitalSoundEnabled,
																	   &g_flightVoiceEnabled,
																	   &g_flightReplayDiskCacheKB,
																	   &g_flightReplayDiskCacheEnabled,
																	   &g_unlimitedWeaponsEnabled,
																	   &g_flightInvulnerabilityEnabled,
																	   &g_flightCraftCollisionsEnabled,
																	   &g_flightHighDetailStarfield,
																	   &g_flightBackdropsPreference,
																	   &g_flightDebrisPreference,
																	   &g_flightMarkingsPreference,
																	   &g_flightStarfighterDetail,
																	   &g_flightStarshipDetail,
																	   &g_flightDeathStarDetail,
																	   &g_flightEngineGlowPreference,
																	   &g_modelTextureQuality,
																	   &g_flightBrightnessSetting,
																	   &g_gateGunTimer,
																	   &g_provingGroundsCheckpointBlinkTicks,
																	   &g_dynamicMusicState,
																	   &g_dynamicMusicOutcomeLatched,
																	   &g_dynamicMusicTrackRemainingMs,
																	   &g_surfaceVictoryExitSecond,
																	   &g_surfaceSpecialTargetHit,
																	   &g_surfaceSpecialTargetCollisionMode,
																	   &g_trenchSpecialCellVoicePlayed,
																	   &g_deathStarRenderObject,
																	   &g_deathStarRenderCraft,
																	   NULL };

// GLOBAL: XW 0x4C9A58
unsigned int g_ReplaySnapshotBlockSizes[REPLAYIO_SNAPSHOT_TABLE_COUNT] = {
	REPLAYIO_DISK_OBJECT_SIZE * XW_OBJECT_COUNT,
	REPLAYIO_DISK_MISSION_OBJECT_SIZE* MISSION_OBJECT_COUNT,
	REPLAYIO_DISK_CRAFT_SIZE* XW_CRAFT_OBJECT_COUNT,
	REPLAYIO_DISK_WARHEAD_GUIDANCE_SIZE* LASER_WARHEAD_GUIDANCE_COUNT,
	REPLAYIO_DISK_MISSION_CLOCK_SIZE,
	REPLAYIO_DISK_MISSION_CLOCK_SIZE,
	REPLAYIO_DISK_MISSION_HEADER_SIZE,
	REPLAYIO_DISK_FLIGHT_GROUP_STATE_SIZE* MISSION_FLIGHT_GROUP_COUNT,
	REPLAYIO_DISK_MISSION_RUNTIME_SIZE,
	REPLAYIO_DISK_PLAYER_SIZE,
	REPLAYIO_DISK_CAMERA_SIZE,
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint8_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	REPLAYIO_DISK_COUNTDOWN_TIMERS_SIZE,
	sizeof(uint16_t),
	REPLAYIO_DISK_SURFACE_DAMAGE_SIZE* DEATH_STAR_SURFACE_CELL_COUNT,
	REPLAYIO_DISK_SURFACE_GUN_SIZE* DEATH_STAR_SURFACE_GUN_CELL_CAPACITY,
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint8_t) * CREATE_BACKDROP_CAPACITY,
	sizeof(uint8_t) * CREATE_BACKDROP_CAPACITY,
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint8_t),
	sizeof(uint16_t) * DEATH_STAR_SURFACE_GOAL_COUNT,
	sizeof(uint16_t) * DEATH_STAR_SURFACE_GOAL_COUNT,
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint8_t),
	sizeof(uint16_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint16_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint32_t),
	sizeof(uint8_t),
	sizeof(uint16_t),
	sizeof(uint16_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint32_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	sizeof(uint8_t),
	REPLAYIO_DISK_OBJECT_SIZE,
	REPLAYIO_DISK_CRAFT_SIZE,
	0
};

// GLOBAL: XW 0x62AFF2
uint8_t g_replaySpoolMessagesSuppressed = 0;

// GLOBAL: XW 0x62D0E2
uint16_t g_snapshotField62D0E2 = 0;

// GLOBAL: XW 0x62D12A
uint16_t g_snapshotField62D12A = 0;

// GLOBAL: XW 0x637369
uint8_t g_ReplaySpoolEnabled = 0;

// GLOBAL: XW 0x63A262
uint8_t* g_replayCockpitResources[REPLAYIO_PANEL_RESOURCE_COUNT] = { NULL, NULL, NULL };

// FUNCTION: XW 0x41EE50
int16_t replayio_copytosave(const char* filename) {
#ifdef XW_MODERN
	return XwReplayFormat_SaveCheckpoint(filename);
#else
	unsigned int index;
	if (fediskio_tryopenfile(filename, "wb", 0) != 0) {
		const void* blockData = g_ReplaySnapshotBlockPointers[0];
		for (index = 0; index < REPLAYIO_SNAPSHOT_BLOCK_COUNT; ++index) {
			if (blockData == NULL)
				break;
			if (fediskio_writefileblock(blockData, g_ReplaySnapshotBlockSizes[index], 1, g_stream) != 1) {
				File_RawClose(g_stream);
				return 0;
			}
			blockData = g_ReplaySnapshotBlockPointers[index + 1];
		}
		for (index = 0; index < sizeof(g_missionFlightGroups); ++index)
			File_RawPutChar(((uint8_t*)g_missionFlightGroups)[index], g_stream);
		for (index = 0; index < MODEL_TYPE_RECORD_COUNT; ++index)
			File_RawPutChar(g_modelTypeTable[index].flags, g_stream);
		File_RawClose(g_stream);
		msg_clearmessagequeue();
		return (int8_t)File_RawHasError(g_stream) == 0;
	}
	return 0;
#endif
}

// FUNCTION: XW 0x41EF40
int16_t replayio_copyfromsave(const char* filename) {
#ifdef XW_MODERN
	return XwReplayFormat_LoadCheckpoint(filename);
#else
	unsigned int index;
	if (fediskio_tryopenfile(filename, "rb", 1) != 0) {
		void* blockPointer = g_ReplaySnapshotBlockPointers[0];
		for (index = 0; index < REPLAYIO_SNAPSHOT_BLOCK_COUNT; ++index) {
			if (blockPointer == NULL)
				break;
			if (fediskio_readfileblock(blockPointer, g_ReplaySnapshotBlockSizes[index], 1, g_stream) != 1) {
				File_RawClose(g_stream);
				return 0;
			}
			blockPointer = g_ReplaySnapshotBlockPointers[index + 1];
		}
		for (index = 0; index < sizeof(g_missionFlightGroups); ++index)
			((uint8_t*)g_missionFlightGroups)[index] = File_RawGetChar(g_stream);
		for (index = 0; index < MODEL_TYPE_RECORD_COUNT; ++index)
			g_modelTypeTable[index].flags = File_RawGetChar(g_stream);
		msg_clearmessagequeue();
		File_RawClose(g_stream);
		return 1;
	}
	return 0;
#endif
}

// FUNCTION: XW 0x41F010
int16_t replayio_openreplayinputfile(void) {
	char spoolFilename[REPLAYIO_SPOOL_PATH_CAPACITY];
	strcpy(spoolFilename, "X-Wing Data\\");
	strcat(spoolFilename, "input.spl");
	File_RawRemove(spoolFilename);
	return 1;
}

// FUNCTION: XW 0x41F090
int16_t replayio_spoolreplayinput(void) {
	XwFile* savedStream;
	const uint8_t* buffer;
	unsigned int recordIndex, byteIndex;
	if (g_ReplaySpoolEnabled == 0)
		return 1;
	if (g_ReplayBufferIndex == 0)
		return 1;
	if (g_replaySpoolMessagesSuppressed == 0)
		msg_messageprintf(XW_MSG_CAMERA_FOOTAGE_SAVING);
	savedStream = g_stream;
	buffer = g_ReplayBufferStart;
	if (fediskio_tryopenfile("+input.spl", "ab", 0) != 0) {
		for (recordIndex = 0; recordIndex < g_ReplayBufferIndex; ++recordIndex) {
			for (byteIndex = 0; byteIndex < REPLAY_INPUT_RECORD_SIZE; ++byteIndex) {
				File_RawPutChar(buffer[recordIndex * REPLAY_INPUT_RECORD_SIZE + byteIndex], g_stream);
				if ((int8_t)File_RawHasError(g_stream) != 0) {
					File_RawClose(g_stream);
					g_stream = savedStream;
					return 0;
				}
			}
		}
		if (File_RawClose(g_stream) != EOF) {
			if (g_replaySpoolMessagesSuppressed == 0)
				msg_messageprintf(XW_MSG_CAMERA_FOOTAGE_SAVED);
			g_stream = savedStream;
			return 1;
		}
	}
	g_stream = savedStream;
	return 0;
}

// FUNCTION: XW 0x41F1B0
int16_t replayio_savereplaybuffer(void) {
#ifdef XW_MODERN
	return XwReplayFormat_SaveInputBuffer();
#else
	if (fediskio_tryopenfile("+rpybuff.tmp", "wb", 0) != 0) {
		uint8_t* buffer = g_ReplayBufferStart;
		unsigned int index;
#ifdef XW_MODERN
		int writeFailed;
		int closeFailed;
#endif
		for (index = REPLAY_BUFFER_CAPACITY; index != 0; --index)
			File_RawPutChar(buffer[REPLAY_BUFFER_CAPACITY - index], g_stream);
#ifdef XW_MODERN
		writeFailed = File_RawHasError(g_stream);
		closeFailed = File_RawClose(g_stream);
		return writeFailed == 0 && closeFailed == 0;
#else
		File_RawClose(g_stream);
		return (int8_t)File_RawHasError(g_stream) == 0;
#endif
	}
	return 0;
#endif
}

// FUNCTION: XW 0x41F220
int16_t replayio_restorereplaybuffer(void) {
#ifdef XW_MODERN
	return XwReplayFormat_LoadInputBuffer();
#else
	if (fediskio_tryopenfile("+rpybuff.tmp", "rb", 1) != 0) {
		uint8_t* buffer = g_ReplayBufferStart;
		unsigned int index;
		for (index = REPLAY_BUFFER_CAPACITY; index != 0; --index)
			buffer[REPLAY_BUFFER_CAPACITY - index] = File_RawGetChar(g_stream);
		File_RawClose(g_stream);
		return 1;
	}
	return 0;
#endif
}

// FUNCTION: XW 0x41F280
void replayio_replayscreen(void) {
#ifdef XW_MODERN
	XwReplayViewer_Begin();
#else
	int brightness;
	uint32_t savedResolutionMode;
	unsigned int viewportOffset;

	if (g_flightAudioMode != 0) {
		g_ReplaySavedVolume = hilevel_ImGetMasterVol();
		hilevel_ImSetMasterVol(0);
		lolevel_ImPause();
		g_ReplayMusicActive = 0;
	}
	if ((uint8_t)g_flightDisplaySurfaceMode == 0 || g_ReplayReturnToExistingCheckpoint != 0 ||
		replayio_copytosave("+savegame.rpy") != 0) {
		g_ReplayReturnToExistingCheckpoint = 0;
		g_ReplayRecording = 0;
		g_replayviewmode = 1;
		FlightDisplay_LockSurface();
		if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
			festring_clearscreen();
			savedResolutionMode = g_flightResolutionMode;
			g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
			strcpy(g_ReplayClipName, "UNTITLED");
			strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
			strcat(g_hudCockpitBasePath, "camerap");
			strcat(g_hudCockpitBasePath, ".PNL");
			fediskio_loadbufferdata(g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
									REPLAYIO_CAMERA_SPRITE_COUNT, 0);
			g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
			panel_loadcontrolpanel("camera", g_replayCockpitResources, REPLAYIO_PANEL_RESOURCE_COUNT);
			g_flightSetPaletteRangeFn(
				(const struct RgbTriplet*)g_replayCockpitResources[REPLAYIO_PANEL_PALETTE], 0,
				REPLAYIO_PANEL_PALETTE_COUNT);
			festring_setbackcolor(g_flightColorEscapeBypassChar);
			g_flightFillClipRectFn();
			g_flightBlitSpriteFn(g_replayCockpitResources[REPLAYIO_PANEL_IMAGE], 0, 0, 0, 0);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H) {
				viewportOffset = g_flightComputePixelOffsetFn(0, REPLAYIO_LIVE_LOW_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH, REPLAYIO_LIVE_LOW_HEIGHT,
								  g_flightViewportMode, viewportOffset);
			} else {
				viewportOffset = g_flightComputePixelOffsetFn(0, REPLAYIO_LIVE_HIGH_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_WIDTH, REPLAYIO_LIVE_HIGH_HEIGHT, g_flightViewportMode,
								  viewportOffset);
			}
		} else {
			replay_loadreplay();
			replayio_copyfromsave(g_ReplayStartFilename);
			fediskio_InitResources();
			g_starfieldColorCacheReusable = 0;
			FlightDisplay_LockSurface();
			g_camMatR0_X = 0;
			g_camMatR0_Y = 0;
			g_camMatR0_Z = 0;
			g_camMatR1_X = 0;
			g_camMatR1_Y = 0;
			g_camMatR1_Z = 0;
			g_camMatR2_X = 0;
			g_camMatR2_Y = 0;
			g_camMatR2_Z = 0;
			g_flightVpWidth = 0;
			g_flightVpHeight = 0;
			FlightStarfield_Render();
			FlightDisplay_UnlockSurface();
			g_starfieldColorCacheReusable = 1;
			panel_loadpaneldata();
			msg_messageinit();
			g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
			strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
			strcat(g_hudCockpitBasePath, "camerap");
			strcat(g_hudCockpitBasePath, ".PNL");
			fediskio_loadbufferdata(g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
									REPLAYIO_CAMERA_SPRITE_COUNT, 0);
			g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
			panel_loadcontrolpanel("film", g_replayCockpitResources, REPLAYIO_PANEL_RESOURCE_COUNT);
			g_flightSetPaletteRangeFn(
				(const struct RgbTriplet*)g_replayCockpitResources[REPLAYIO_PANEL_PALETTE], 0,
				REPLAYIO_PANEL_PALETTE_COUNT);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H)
				festring_setbound(REPLAYIO_FILM_LOW_LEFT, REPLAYIO_FILM_LOW_TOP, REPLAYIO_FILM_LOW_RIGHT,
								  REPLAYIO_FILM_LOW_BOTTOM);
			else
				festring_setbound(0, REPLAYIO_FILM_HIGH_TOP, REPLAYIO_FILM_HIGH_RIGHT,
								  REPLAYIO_FILM_HIGH_BOTTOM);
			festring_setbackcolor(g_flightColorEscapeBypassChar);
			g_flightFillClipRectFn();
			festring_setbound(0, REPLAYIO_FOOTER_TOP, FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT);
			g_flightBlitSpriteFn(g_replayCockpitResources[REPLAYIO_PANEL_IMAGE], 0, 0, 0, 0);
			if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H) {
				viewportOffset = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_LOW_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH, REPLAYIO_FILM_LOW_HEIGHT,
								  g_flightViewportMode, viewportOffset);
			} else {
				viewportOffset = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_HIGH_TOP);
				SetFlightViewport(FLIGHT_DISPLAY_WIDTH, REPLAYIO_FILM_HIGH_HEIGHT, g_flightViewportMode,
								  viewportOffset);
			}
		}
		panel_copymaskdata(g_replayCockpitResources[REPLAYIO_PANEL_MASK], g_flightVpWidth, g_flightVpHeight,
						   0);
		g_projOffsetY = 0;
		replay_rewindreplay();
		FlightDisplay_UnlockSurface();
		do {
			g_ReplayReenterSimulation = 0;
			festring_showscreen();
			FlightDisplay_BlitRenderSurface();
			FlightDisplay_Flip();
			FlightDisplay_BlitRenderSurface();
			replay_doreplayscreen();
			if (g_ReplayMusicActive != 0) {
				g_ReplaySavedVolume = hilevel_ImGetMasterVol();
				hilevel_ImSetMasterVol(0);
				lolevel_ImPause();
				g_ReplayMusicActive = 0;
			}
			g_replayviewmode = 0;
			if ((uint8_t)g_flightDisplaySurfaceMode != 0) {
				if (savedResolutionMode != g_flightResolutionMode) {
					g_flightResolutionMode = savedResolutionMode;
					Xw_InitFlightResolution();
					rtsvga2_initgraphVGA();
					feinput_setupgraphics(g_flightGraphicsDetailPreset);
					fediskio_readfiletofarmemory("TINY64.FNT", gTinyFntBuf);
					fediskio_readfiletofarmemory("MICRO64.FNT", gMicroFntBuf);
					festring_setfontsize(FLIGHT_FONT_TINY);
				}
				replayio_copyfromsave("+savegame.rpy");
				if (g_ReplayReturnToExistingCheckpoint == 0)
					replayio_setreturnview();
				if (g_ReplayMusicActive == 0) {
					hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
					lolevel_ImResume();
					g_ReplayMusicActive = 1;
				}
				if (g_ReplayReturnToExistingCheckpoint != 0 && g_flightAudioMode != 0)
					lolevel_ImStopAllSounds();
			} else if (g_ReplayReenterSimulation != 0) {
				g_flightRenderTransitionHook();
				g_missionRuntimeState.flightExitRequested = 0;
				g_ReplayRecording = 0;
				g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
				g_flightCamera.externalViewActive = 0;
				g_flightCamera.hudStateLive = 0;
				g_flightCamera.hudAimY = 0;
				g_flightCamera.hudAimX = 0;
				g_flightCamera.rearViewHudOffset = 0;
				replayio_setreturnview();
				if (g_ReplayMusicActive == 0) {
					hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
					lolevel_ImResume();
					g_ReplayMusicActive = 1;
				}
				g_flightAccumulatedTicks = 0;
				xtimer_Time_Elapsed();
				while (g_missionRuntimeState.flightExitRequested == 0)
					Xw_doframe();
				g_ReplaySavedVolume = hilevel_ImGetMasterVol();
				hilevel_ImSetMasterVol(0);
				lolevel_ImPause();
				g_ReplayMusicActive = 0;
				g_flightRenderTransitionHook();
				FlightDisplay_ClearOffscreenSurface();
				g_ReplayRecording = 0;
				g_replayviewmode = 1;
				replay_loadreplay();
				replayio_copyfromsave(g_ReplayStartFilename);
				brightness = g_flightBrightnessSetting;
				if ((unsigned int)brightness > USER_BRIGHTNESS_MAX)
					brightness = USER_BRIGHTNESS_MAX;
				g_flightBrightnessScaleQ8 = (brightness + USER_BRIGHTNESS_BIAS) * USER_BRIGHTNESS_STEP;
				msg_messageinit();
				FlightDisplay_LockSurface();
				g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
				strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
				strcat(g_hudCockpitBasePath, "camerap");
				strcat(g_hudCockpitBasePath, ".PNL");
				fediskio_loadbufferdata(g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
										REPLAYIO_CAMERA_SPRITE_COUNT, 0);
				g_hudCockpitResourceWriteCursor = g_flightOffscreenBuffer;
				panel_loadcontrolpanel("film", g_replayCockpitResources, REPLAYIO_PANEL_RESOURCE_COUNT);
				g_flightSetPaletteRangeFn(
					(const struct RgbTriplet*)g_replayCockpitResources[REPLAYIO_PANEL_PALETTE], 0,
					REPLAYIO_PANEL_PALETTE_COUNT);
				if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H)
					festring_setbound(REPLAYIO_FILM_LOW_LEFT, REPLAYIO_FILM_LOW_TOP, REPLAYIO_FILM_LOW_RIGHT,
									  REPLAYIO_FILM_LOW_BOTTOM);
				else
					festring_setbound(0, REPLAYIO_FILM_HIGH_TOP, REPLAYIO_FILM_HIGH_RIGHT,
									  REPLAYIO_FILM_HIGH_BOTTOM);
				festring_setbackcolor(g_flightColorEscapeBypassChar);
				g_flightFillClipRectFn();
				festring_setbound(0, REPLAYIO_FOOTER_TOP, FLIGHT_DISPLAY_WIDTH, FLIGHT_DISPLAY_HEIGHT);
				g_flightBlitSpriteFn(g_replayCockpitResources[REPLAYIO_PANEL_IMAGE], 0, 0, 0, 0);
				if (g_flightResolutionMode == FLIGHT_DISPLAY_MODE_13H) {
					viewportOffset = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_LOW_TOP);
					SetFlightViewport(FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH, REPLAYIO_FILM_LOW_HEIGHT,
									  g_flightViewportMode, viewportOffset);
				} else {
					viewportOffset = g_flightComputePixelOffsetFn(0, REPLAYIO_FILM_HIGH_TOP);
					SetFlightViewport(FLIGHT_DISPLAY_WIDTH, REPLAYIO_FILM_HIGH_HEIGHT, g_flightViewportMode,
									  viewportOffset);
				}
				panel_copymaskdata(g_replayCockpitResources[REPLAYIO_PANEL_MASK], g_flightVpWidth,
								   g_flightVpHeight, 0);
				g_projOffsetY = 0;
				FlightDisplay_UnlockSurface();
				replay_rewindreplay();
			} else {
				g_flightRenderTransitionHook();
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_ClearOffscreenSurface();
				FlightDisplay_Flip();
				if (g_ReplayMusicActive == 0) {
					hilevel_ImSetMasterVol((uint16_t)g_ReplaySavedVolume);
					lolevel_ImResume();
					g_ReplayMusicActive = 1;
				}
			}
		} while (g_ReplayReenterSimulation != 0);
		fsfx_UpdatePlayerEngineLoop();
	}
#endif
}

// FUNCTION: XW 0x41FB00
void replayio_setreturnview(void) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		Dos94Panel_RestoreCraftSprites();
	else
#endif
	{
		g_hudPanelSpriteDataWriteCursor = g_hudPanelSpriteCraftDataStart;
		strcpy(g_hudCockpitBasePath, g_hudCockpitResolutionDirectory);
		strcat(g_hudCockpitBasePath, g_hudPanelSpriteFileInfo.baseName);
		strcat(g_hudCockpitBasePath, ".PNL");
		fediskio_loadbufferdata(
			g_hudCockpitBasePath, PANEL_SHARED_SPRITE_COUNT,
			g_hudPanelSpriteFileInfo.spriteCount + g_hudPanelSpriteFileInfo.spriteCountAddend, 0);
	}
	if (g_flightCamera.externalViewActive == 0 &&
		g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex) {
		uint16_t savedHudView = g_flightCamera.hudStateLive;
		g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
		g_flightCamera.hudStateLive = PANEL_DISPLAY_MODE_INVALID;
		panelrts_setnewpilotview(savedHudView);
	} else {
		g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
		g_flightCamera.hudStateLive = PANEL_DISPLAY_MODE_INVALID;
		panelrts_setnewpilotview(PANEL_VIEW_NO_COCKPIT);
	}
	msg_messageinit();
}
