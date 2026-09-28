#include "xw/flight/xw.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/fscript.h"
#include "xw_dos94/flight/music.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_hud.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/flight_music.h"
#endif
#ifdef XW_MODERN
#include "xw_runtime/audio/music_policy.h"
#include "xw_runtime/timing/flight_timing.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/cdaudio.h"
#include "xw/audio/fsfx.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/audio/sound.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/flight_input.h"
#include "xw/flight/fview.h"
#include "xw/flight/gate.h"
#include "xw/flight/hud/festring.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/dynamix.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/move.h"
#include "xw/flight/object/static.h"
#include "xw/flight/player/targeting.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/shell_flight.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/backdrp2.h"
#include "xw/render/flight_hyperspace.h"
#include "xw/render/flight_starfield.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/std3d.h"
#include "xw/render/sw3d.h"
#include "xw/util/shared.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/flight_sim.h"
#include "xw_runtime/snapshot/render_capture.h"
#include "xw_runtime/snapshot/render_objects.h"
#endif
#include "xw_runtime/timing/host_clock.h"

#include <landru/timer.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C5238
const uint8_t g_hudShipIdByCraftType[XW_HUD_SHIP_ID_COUNT] = {
	0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  26,
	26,  28,  28,  30,  31,  32,  48,  33,  34,  35,  36,  37,  38,  38,  38,  49,  50,  51,  52,
	53,  54,  55,  56,  56,  56,  56,  56,  56,  56,  56,  63,  57,  58,  59,  60,  60,  60,  60,
	60,  109, 110, 111, 111, 112, 113, 113, 114, 115, 115, 116, 117, 117, 116, 117, 117, 116, 117,
	117, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110,
	109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 109, 110, 0,   0,   0
};

// GLOBAL: XW 0x4CEF48
int g_localLightsLevel = 1;

// GLOBAL: XW 0x4CF8F8
uint8_t g_dynamicMusicInitialStartMinuteChoices[XW_MUSIC_START_CHOICE_COUNT] = { 0, 4, 8, 12 };

// GLOBAL: XW 0x4CF8FC
uint8_t g_dynamicMusicInitialStartSecondChoices[XW_MUSIC_START_CHOICE_COUNT] = { 0, 1, 40, 52 };

// GLOBAL: XW 0x4F4A30
uint16_t g_legacyOscillatorFrameCounter = 0;

// GLOBAL: XW 0x4F4A34
int16_t g_legacyOscillatorDirection = 0;

// GLOBAL: XW 0x4F4A38
uint16_t g_previousFrameMeasuredTicks = 0;

// GLOBAL: XW 0x4F4A40
int g_objectPointLightCount = 0;

// GLOBAL: XW 0x62B018
int g_renderObjectListCount = 0;

// GLOBAL: XW 0x62B320
uint32_t g_dynamicMusicLastUpdateTick = 0;

// GLOBAL: XW 0x62B4E6
int16_t g_targetHighlightBlinkTicks = 0;

// GLOBAL: XW 0x62B500
struct RenderObjectListEntry* g_renderListHead = NULL;

// GLOBAL: XW 0x62B508
uint16_t g_flightAccumulatedTicks = 0;

// GLOBAL: XW 0x62B5FC
uint16_t g_renderSphereRadius = 0;

// GLOBAL: XW 0x62B6C8
int g_objectViewX = 0;

// GLOBAL: XW 0x62B6CC
int g_objectViewY = 0;

// GLOBAL: XW 0x62B6D0
int g_objectViewZ = 0;

// GLOBAL: XW 0x62B6D4
uint16_t g_targetHighlightObjectAndBlinkBits = 0;

// GLOBAL: XW 0x62B700
XwFlightRandomBytePair g_flightRandomBytePairs[XW_RANDOM_PAIR_COUNT] = { 0 };

// GLOBAL: XW 0x62B904
uint16_t g_textureCacheFlushPending = 0;

// GLOBAL: XW 0x62B908
int g_dynamicMusicResumeMinute = 0;

// GLOBAL: XW 0x62B930
uint8_t g_flightField62B930 = 0;

// GLOBAL: XW 0x62B938
uint8_t g_showSimStepScale = 0;

// GLOBAL: XW 0x62BACC
int g_dynamicMusicResumeOffsetSeconds = 0;

// GLOBAL: XW 0x62BAD0
uint8_t g_flightField62BAD0 = 0;

// GLOBAL: XW 0x62BAE4
uint16_t g_simStepScale;

// GLOBAL: XW 0x62BAF0
int g_flightResolutionScale = 0;

// GLOBAL: XW 0x62BB00
uint32_t g_dynamicMusicCurrentTick = 0;

// GLOBAL: XW 0x62BB08
XwMissionClock g_missionCountdownClock = { 0 };

// GLOBAL: XW 0x62BB18
int g_dynamicMusicResumeSecond = 0;

// GLOBAL: XW 0x62BC84
int g_dynamicMusicSavedRemainingMs = 0;

// GLOBAL: XW 0x62BCC0
struct XwFlightGlobalCountdownTimers g_flightGlobalCountdownTimers = { 0 };

// GLOBAL: XW 0x62BD98
int g_dynamicMusicTrackRemainingMs = 0;

// GLOBAL: XW 0x62C700
struct RenderObjectListEntry* g_renderObjectListEntries = NULL;

// GLOBAL: XW 0x62C704
uint16_t g_elapsedTicks = 0;

// GLOBAL: XW 0x62C70C
uint32_t g_dynamicMusicElapsedMs = 0;

// GLOBAL: XW 0x62C924
uint8_t g_flightField62C924 = 0;

// GLOBAL: XW 0x62C930
XwMissionClock g_missionElapsedClock = { 0 };

// GLOBAL: XW 0x62CC70
int g_camRelWorldZ = 0;

// GLOBAL: XW 0x62CC74
int g_camRelWorldX = 0;

// GLOBAL: XW 0x62CC6C
uint8_t g_legacyOscillatorValue = 0;

// GLOBAL: XW 0x62CC80
ObjectPointLight g_objectPointLights[XW_POINT_LIGHT_CAPACITY] = { 0 };

// GLOBAL: XW 0x62CD00
int g_camRelWorldY = 0;

// GLOBAL: XW 0x62D0E0
uint8_t g_dynamicMusicState = 0;

// GLOBAL: XW 0x62D118
uint8_t g_snapshotField62D118 = 0;

// GLOBAL: XW 0x62D138
uint8_t g_flightField62D138 = 0;

// GLOBAL: XW 0x62D14C
uint8_t g_dynamicMusicOutcomeLatched = 0;

// GLOBAL: XW 0x62D360
uint8_t calcframerate = 0;

// GLOBAL: XW 0x637324
uint8_t g_flightField637324 = 0;

// GLOBAL: XW 0x637350
struct CraftData* g_curCraft = NULL;

// GLOBAL: XW 0x637364
int g_dynamicMusicBaseTrackDurationMs = 0;

// FUNCTION: XW 0x42E590
void Xw_InitFlightResolution(void) {
	int resolutionMode = g_flightResolutionMode;

	if (resolutionMode != g_flightDefaultResolutionMode) {
		g_flightBytesPerPixel = FLIGHT_DISPLAY_INDEXED_BYTES_PER_PIXEL;
		FlightDisplay_UnlockSurface();
		FlightDisplay_RebuildForMode(g_flightResolutionMode);
		FlightDisplay_LockSurface();
		resolutionMode = g_flightResolutionMode;
	}
	if (resolutionMode == FLIGHT_DISPLAY_MODE_101H || resolutionMode == FLIGHT_DISPLAY_MODE_111H ||
		resolutionMode == FLIGHT_DISPLAY_MODE_1FFH) {
		g_flightScreenWidth = FLIGHT_DISPLAY_WIDTH;
		g_flightScreenHeight = FLIGHT_DISPLAY_HEIGHT;
		g_hudClearHeight = FLIGHT_DISPLAY_HIGH_HUD_CLEAR_HEIGHT;
		nullsub_SharedNoOp();
		g_surfacePitch = FLIGHT_DISPLAY_HIGH_RESOLUTION_PITCH;
		g_projScaleInt = 1 << FLIGHT_DISPLAY_HIGH_PROJECTION_SHIFT;
		g_projScaleHalfInt = (1 << FLIGHT_DISPLAY_HIGH_PROJECTION_SHIFT) / 2;
		g_flightResolutionScale = 2;
		g_projPerspectiveShift = FLIGHT_DISPLAY_HIGH_PROJECTION_SHIFT;
		g_projAspectY = 0;
		g_hudCockpitResolutionDirectory[3] = '6';
		g_hudCockpitResolutionDirectory[4] = '4';
		return;
	}
	g_swFramebufferClearChunkSize = FLIGHT_DISPLAY_LOW_CLEAR_CHUNK_SIZE;
	g_flightLowResolutionState = 1;
	g_flightScreenWidth = FLIGHT_DISPLAY_LOW_RESOLUTION_WIDTH;
	g_flightScreenHeight = FLIGHT_DISPLAY_LOW_RESOLUTION_HEIGHT;
	g_hudClearHeight = FLIGHT_DISPLAY_LOW_HUD_CLEAR_HEIGHT;
	g_projScaleInt = 1 << FLIGHT_DISPLAY_LOW_PROJECTION_SHIFT;
	g_projScaleHalfInt = (1 << FLIGHT_DISPLAY_LOW_PROJECTION_SHIFT) / 2;
	g_flightResolutionScale = 1;
	g_projPerspectiveShift = FLIGHT_DISPLAY_LOW_PROJECTION_SHIFT;
	g_projAspectY = FLIGHT_DISPLAY_LOW_ASPECT_Q16;
	g_hudCockpitResolutionDirectory[3] = '3';
	g_hudCockpitResolutionDirectory[4] = '2';
}

// FUNCTION: XW 0x42E6C0
void Xw_simulator(FlightEntryMode entryMode) {
#ifdef XW_MODERN
	XwFlightSim_Begin(entryMode);
#else
	uint8_t savedMusicEnabled;
	uint8_t savedDebris;
	uint16_t replayWord;
	uint16_t promptMusicVolume = 0;
	int cdVolume;
	XwObjectTypeId objectType;
	int craftIndex;
	int brightness;
	int i;
	uint8_t savedBackdrops;
	uint8_t savedMusicVolume;
	int16_t savedExitReason;
	uint16_t savedReplayCapacity;
	int restoredBrightness;
	int musicStartChoice;
	unsigned int promptLeft;
	unsigned int promptRight;
	unsigned int promptTop;
	uint16_t promptBottom;
	uint8_t promptKey;
	int j;
	uint8_t pilotObjectRef;

	objectType = g_craftTypeToObjectType[XW_CRAFT_SPECIES_X_WING];
	g_playerEngineLoopSuppressed = 0;
	g_flightAudioMode = 1;
	g_provingGroundsCheckpointBlinkTicks = 0;
	g_flightDisplaySurfaceMode = (g_flightDisplaySurfaceMode & ~UINT8_MAX) | 1;
	g_frontendDisplayWndProcMode = 0;
	for (craftIndex = XW_CRAFT_SPECIES_X_WING; objectType != XW_OBJ_NONE; ++craftIndex) {
		g_objectTypeHudShipIds[objectType] = g_hudShipIdByCraftType[craftIndex];
		objectType = g_craftTypeToObjectType[craftIndex + 1];
	}
	g_ReplaySpoolEnabled = g_flightReplayDiskCacheEnabled;
	g_ReplayCapacityFrames =
		((unsigned int)g_flightReplayDiskCacheKB * XW_BYTES_PER_KB) / REPLAY_INPUT_RECORD_SIZE;
	g_flightField62D138 = 0;
	g_flightField637324 = 1;
	g_flightField62B930 = 0;
	g_flightField62C924 = 0;
	g_paletteCycleEnabled = 0;
	g_engineGlowEnabled = 1;
	g_snapshotField62D118 = 0;
	math2_SeedFromClock();
	g_dynamicMusicOutcomeLatched = 0;
	g_dynamicMusicState = XW_MUSIC_BASE_TRACK;
	g_sw3dSkipOddScanlines = g_flightInterlaceEnabled;
	brightness = g_flightBrightnessSetting;
	if ((unsigned int)brightness > USER_BRIGHTNESS_MAX)
		brightness = USER_BRIGHTNESS_MAX;
	g_flightBrightnessScaleQ8 = (brightness + USER_BRIGHTNESS_BIAS) * USER_BRIGHTNESS_STEP;
	FlightDisplay_LockSurface();
	Xw_InitFlightResolution();
	FlightDisplay_UnlockSurface();
	rtsvga2_blankVGA();
	FlightDisplay_LockSurface();
	rtsvga2_initgraphVGA();
	FlightDisplay_UnlockSurface();
	FlightDisplay_Flip();
	g_flightAccumulatedTicks = 0;
	feinput_setupgraphics(FEINPUT_INITIAL_GRAPHICS_DETAIL);
	g_flightField62B930 = 1;
	g_paletteBlankFlags = 0;
	FlightDisplay_ClearBackAndAuxiliarySurfaces();
	FlightDisplay_ClearOffscreenSurface();
	FlightDisplay_Flip();
	FlightDisplay_LockSurface();
	fediskio_Init_Buffers_and_Fonts();
	FlightDisplay_UnlockSurface();
	FlightDisplay_BlitRenderSurface();
	FlightDisplay_Flip();
	nullsub_SharedNoOp();
	for (i = 0; i < XW_RANDOM_PAIR_COUNT; ++i) {
		g_flightRandomBytePairs[i].value0To63 = rand() & XW_RANDOM_PAIR_LARGE_MASK;
		g_flightRandomBytePairs[i].value0To7 = rand() & XW_RANDOM_PAIR_SMALL_MASK;
	}
	g_legacyOscillatorValue = XW_OSCILLATOR_MAXIMUM;
	FlightDisplay_LockSurface();
	feinput_setupinputdevices();
	FlightDisplay_UnlockSurface();
	g_paletteCycleEnabled = 1;
	if (entryMode == FLIGHT_ENTRY_REPLAY_VIEWER) {
		g_flightDisplaySurfaceMode &= ~UINT8_MAX;
		g_hyperspaceflag = 0;
		g_flightField62BAD0 = 0;
		g_surfaceVictoryExitSecond = XW_REPLAY_SURFACE_EXIT_SECOND;
		replayio_replayscreen();
	} else {
		g_flightDisplaySurfaceMode = (g_flightDisplaySurfaceMode & ~UINT8_MAX) | 1;
		if (entryMode == FLIGHT_ENTRY_NEW_MISSION) {
			g_starfieldColorCacheReusable = 0;
			g_ReplayRecordedFlightAvailable = 0;
			g_hudCockpitResourcesLoaded = 0;
			FlightDisplay_LockSurface();
			create_loadmission(g_currentMissionFile);
			FlightDisplay_UnlockSurface();
			FlightDisplay_LockSurface();
			fediskio_InitResources();
			FlightDisplay_UnlockSurface();
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
			FlightDisplay_ClearBackAndAuxiliarySurfaces();
			FlightDisplay_Flip();
			if (g_missionFlightGroups[g_playerFlightState.flightGroupIndex].arrivalMethod != 0 &&
				g_flightTransitionsEnabled != 0) {
				create_createhyperin();
				savedBackdrops = g_backdropsEnabled;
				savedDebris = g_debrisEnabled;
				g_backdropsEnabled = 0;
				g_debrisEnabled = 0;
				g_hyperspaceflag = ANIM_HYPERSPACE_SETUP;
				anim_dohyperspace();
				g_flightAccumulatedTicks = 0;
				xtimer_Time_Elapsed();
				while (g_hyperspaceflag != 0)
					Xw_doframe();
				g_hyperspaceflag = 0;
				g_playerEngineLoopSuppressed = 1;
				fsfx_UpdatePlayerEngineLoop();
				g_playerEngineLoopSuppressed = 0;
				g_backdropsEnabled = savedBackdrops;
				g_debrisEnabled = savedDebris;
				FlightDisplay_LockSurface();
				create_loadmission(g_currentMissionFile);
				FlightDisplay_UnlockSurface();
			}
			FlightDisplay_LockSurface();
			create_createmission();
			FlightDisplay_UnlockSurface();
			FlightDisplay_BlitRenderSurface();
			FlightDisplay_Flip();
			FlightDisplay_BlitRenderSurface();
			g_flightResetPaletteFn();
			if (g_missionFlightGroups[g_playerFlightState.flightGroupIndex].arrivalMethod != 0) {
				FlightDisplay_LockSurface();
				msg_messageprintf(XW_MSG_HYPERSPACE_JUMP_COMPLETED);
				FlightDisplay_UnlockSurface();
			}
			if (g_missionRuntimeState.provingGroundsActive != 0 &&
				g_missionRuntimeState.provingGroundsLevel > 1u) {
				g_msgArgTable[0] = g_missionRuntimeState.provingGroundsLevel - 1;
				FlightDisplay_LockSurface();
				msg_messageprintf(XW_MSG_PREVIOUS_LEVEL_POINTS_AWARDED);
				FlightDisplay_UnlockSurface();
			}
		} else {
			uint8_t savedSfxVolume;
			uint8_t savedSfxEnabled;
			uint8_t savedDigitalSound;
			uint8_t savedVoice;
			uint8_t savedDiskCache;
			uint8_t savedUnlimitedWeapons;
			uint8_t savedInvulnerability;
			uint8_t savedCollisions;
			uint8_t savedStarfield;
			uint8_t savedBackdropsPreference;
			uint8_t savedDebrisPreference;
			int8_t savedDetailPreference;
			int8_t savedStarfighterDetail;
			int8_t savedStarshipDetail;
			int8_t savedDeathStarDetail;
			uint8_t savedEngineGlow;
			uint8_t savedBrightness;
			uint8_t savedTextureQuality;

			savedMusicEnabled = g_flightMusicEnabled;
			savedSfxVolume = g_flightSfxVolume;
			savedSfxEnabled = g_flightSfxEnabled;
			savedDigitalSound = g_flightDigitalSoundEnabled;
			savedVoice = g_flightVoiceEnabled;
			savedDiskCache = g_flightReplayDiskCacheEnabled;
			savedUnlimitedWeapons = g_unlimitedWeaponsEnabled;
			savedInvulnerability = g_flightInvulnerabilityEnabled;
			savedCollisions = g_flightCraftCollisionsEnabled;
			savedStarfield = g_flightHighDetailStarfield;
			savedMusicVolume = g_flightMusicVolume;
			savedExitReason = g_missionRuntimeState.flightExitReason;
			savedReplayCapacity = g_flightReplayDiskCacheKB;
			savedBackdropsPreference = g_flightBackdropsPreference;
			savedDebrisPreference = g_flightDebrisPreference;
			savedDetailPreference = g_flightMarkingsPreference;
			savedStarfighterDetail = g_flightStarfighterDetail;
			savedStarshipDetail = g_flightStarshipDetail;
			savedDeathStarDetail = g_flightDeathStarDetail;
			savedEngineGlow = g_flightEngineGlowPreference;
			savedTextureQuality = g_modelTextureQuality;
			savedBrightness = g_flightBrightnessSetting;
			replayio_copyfromsave("+savegame.rpy");
			g_flightMusicEnabled = savedMusicEnabled;
			g_flightSfxVolume = savedSfxVolume;
			g_flightSfxEnabled = savedSfxEnabled;
			g_flightDigitalSoundEnabled = savedDigitalSound;
			g_flightVoiceEnabled = savedVoice;
			g_flightReplayDiskCacheEnabled = savedDiskCache;
			g_unlimitedWeaponsEnabled = savedUnlimitedWeapons;
			g_flightInvulnerabilityEnabled = savedInvulnerability;
			g_flightCraftCollisionsEnabled = savedCollisions;
			g_flightHighDetailStarfield = savedStarfield;
			g_flightBackdropsPreference = savedBackdropsPreference;
			g_flightDebrisPreference = savedDebrisPreference;
			g_flightMarkingsPreference = savedDetailPreference;
			g_flightStarfighterDetail = savedStarfighterDetail;
			g_flightStarshipDetail = savedStarshipDetail;
			g_flightMusicVolume = savedMusicVolume;
			g_flightReplayDiskCacheKB = savedReplayCapacity;
			g_flightDeathStarDetail = savedDeathStarDetail;
			g_flightEngineGlowPreference = savedEngineGlow;
			g_flightBrightnessSetting = savedBrightness;
			restoredBrightness = savedBrightness;
			g_modelTextureQuality = savedTextureQuality;
			if (savedBrightness > USER_BRIGHTNESS_MAX)
				restoredBrightness = USER_BRIGHTNESS_MAX;
			g_flightBrightnessScaleQ8 = (restoredBrightness + USER_BRIGHTNESS_BIAS) * USER_BRIGHTNESS_STEP;
			replayio_restorereplaybuffer();
			FlightDisplay_LockSurface();
			fediskio_InitResources();
			FlightDisplay_UnlockSurface();
			FlightDisplay_LockSurface();
			panel_loadpaneldata();
			FlightDisplay_UnlockSurface();
			g_missionRuntimeState.flightExitRequested = 0;
			if (g_ReplayReturnToExistingCheckpoint != 0) {
				g_flightRenderTransitionHook();
				replayio_replayscreen();
			}
			if (g_ReplayReturnToExistingCheckpoint != 0) {
				if (replayio_savereplaybuffer() != 0) {
					g_missionRuntimeState.flightExitRequested = 1;
					g_missionRuntimeState.flightExitReason = USER_EXIT_REPLAY_CHECKPOINT;
				}
			} else {
				if (savedExitReason == USER_EXIT_OPTIONS) {
					if (g_ReplayRecording != 0) {
						replayWord =
							(int8_t)g_unlimitedWeaponsEnabled +
							((int8_t)g_flightInvulnerabilityEnabled * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = (int8_t)g_flightDebrisPreference +
									 ((int8_t)g_flightBackdropsPreference * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = g_flightStarshipDetail +
									 ((int8_t)g_flightHighDetailStarfield * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = g_flightStarfighterDetail +
									 (g_flightDeathStarDetail * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						user_nextreplaystore();
						replayWord = (int8_t)g_flightCraftCollisionsEnabled +
									 (g_flightMarkingsPreference * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = (int8_t)g_flightMusicEnabled +
									 ((int8_t)g_flightMusicVolume * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = (int8_t)g_flightSfxEnabled +
									 ((int8_t)g_flightSfxVolume * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord =
							(int8_t)g_flightDigitalSoundEnabled +
							2 * ((int8_t)g_flightVoiceEnabled +
								 ((int8_t)g_flightEngineGlowPreference * XW_REPLAY_GLOW_BYTE_FACTOR));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						user_nextreplaystore();
					}
					user_ApplyPreferences();
				} else if (savedExitReason == USER_EXIT_MAP || savedExitReason == USER_EXIT_DAMAGE ||
						   savedExitReason == USER_EXIT_BRIEFING) {
					if (g_ReplayRecording != 0) {
						replayWord = g_playerSubsystemRepairPriority[1] +
									 (g_playerSubsystemRepairPriority[0] * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = g_playerSubsystemRepairPriority[3] +
									 (g_playerSubsystemRepairPriority[2] * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = g_playerSubsystemRepairPriority[5] +
									 (g_playerSubsystemRepairPriority[4] * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						replayWord = g_playerSubsystemRepairPriority[7] +
									 (g_playerSubsystemRepairPriority[6] * (1 << REPLAY_INPUT_BYTE_SHIFT));
						memcpy(g_ReplayInputPointer, &replayWord, sizeof(replayWord));
						g_ReplayInputPointer += sizeof(replayWord);
						user_nextreplaystore();
					}
					memcpy(g_playerFlightState.savedSubsystemRepairPriority, g_playerSubsystemRepairPriority,
						   sizeof(g_playerFlightState.savedSubsystemRepairPriority));
				}
				FlightDisplay_Flip();
				FlightDisplay_ClearBackAndAuxiliarySurfaces();
				FlightDisplay_ClearOffscreenSurface();
				FlightDisplay_Flip();
				replayio_setreturnview();
				msg_messageinit();
				g_missionRuntimeState.flightExitRequested = 0;
			}
		}
		for (; g_flightAccumulatedTicks == 0; g_flightAccumulatedTicks += xtimer_Time_Elapsed())
			;
		g_dynamicMusicOutcomeLatched = 0;
		if (g_flightMusicVolume != 0 && g_flightMusicEnabled != 0 && CDAudio_Initialize() != 0) {
			cdVolume = UINT16_MAX * (int8_t)g_flightMusicVolume / XW_FLIGHT_INITIAL_VOLUME;
			CDAudio_SetAuxVolume((uint16_t)cdVolume);
			if (g_dynamicMusicSavedRemainingMs == 0) {
				musicStartChoice = math2_getrandom__auxiliary() & (XW_MUSIC_START_CHOICE_COUNT - 1);
				CDAudio_PlayTrackFromTime(XW_MUSIC_BASE_TRACK,
										  g_dynamicMusicInitialStartMinuteChoices[musicStartChoice],
										  g_dynamicMusicInitialStartSecondChoices[musicStartChoice]);
				g_dynamicMusicBaseTrackDurationMs = CDAudio_GetTrackEndTimeMs(XW_MUSIC_BASE_TRACK);
				g_dynamicMusicTrackRemainingMs =
					g_dynamicMusicBaseTrackDurationMs -
					XW_MILLISECONDS_PER_SECOND *
						(g_dynamicMusicInitialStartSecondChoices[musicStartChoice] +
						 XW_SECONDS_PER_MINUTE * g_dynamicMusicInitialStartMinuteChoices[musicStartChoice]);
				g_dynamicMusicLastUpdateTick = timeGetTime();
				g_dynamicMusicState = XW_MUSIC_BASE_TRACK;
			}
		} else {
			g_dynamicMusicTrackRemainingMs = INT32_MAX;
			g_dynamicMusicSavedRemainingMs = 0;
			g_dynamicMusicState = 0;
		}
		g_flightAccumulatedTicks = 0;
		xtimer_Time_Elapsed();
		while (g_missionRuntimeState.flightExitRequested == 0)
			Xw_doframe();
		if (g_flightMusicEnabled != 0 && (g_missionRuntimeState.flightExitReason == USER_EXIT_OPTIONS ||
										  g_missionRuntimeState.flightExitReason == USER_EXIT_DAMAGE ||
										  g_missionRuntimeState.flightExitReason == USER_EXIT_BRIEFING ||
										  g_missionRuntimeState.flightExitReason == USER_EXIT_MAP)) {
			g_dynamicMusicSavedRemainingMs = g_dynamicMusicTrackRemainingMs;
		}
		CDAudio_CloseDevice();
		if (g_missionRuntimeState.flightExitReason < USER_EXIT_MAP ||
			g_missionRuntimeState.flightExitRequested == XW_EXIT_REQUEST_ABORT) {
			g_playerFlightState.missionExitHullDamageQuarter =
				(uint16_t)math2_percentage(g_playerFlightState.craft->hullDamage,
										   g_playerFlightState.craft->hullMax) >>
				XW_HULL_QUARTER_SHIFT;
			if (g_missionRuntimeState.flightExitRequested == XW_EXIT_REQUEST_ABORT) {
				g_missionRuntimeState.flightExitRequested = 1;
				g_missionRuntimeState.flightExitReason = XW_MISSION_TIMEOUT_EXIT;
			}
			g_flightRenderTransitionHook();
			if (g_ReplayRecordedFlightAvailable != 0) {
				if (g_flightAudioMode != 0) {
					promptMusicVolume = hilevel_ImGetMasterVol();
					hilevel_ImSetMasterVol(0);
					lolevel_ImPause();
				}
				FlightDisplay_LockSurface();
				festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
				festring_setbackcolor(XW_REPLAY_PROMPT_BACKGROUND);
				g_flightFillClipRectFn();
				promptLeft = (unsigned int)g_flightScreenWidth / XW_REPLAY_PROMPT_WIDTH_DIVISOR;
				promptRight =
					g_flightScreenWidth - (unsigned int)g_flightScreenWidth / XW_REPLAY_PROMPT_WIDTH_DIVISOR;
				promptTop = (g_flightScreenHeight / 2) +
							-XW_REPLAY_PROMPT_HALF_LINES * g_flightFontLineHeight -
							(g_flightScreenHeight / XW_REPLAY_PROMPT_HEIGHT_DIVISOR);
				promptBottom = promptTop + XW_REPLAY_PROMPT_LINES * g_flightFontLineHeight;
				festring_setbound((unsigned int)g_flightScreenWidth / XW_REPLAY_PROMPT_WIDTH_DIVISOR - 1,
								  promptTop - 1, promptRight + 1, promptBottom + 1);
				festring_setbackcolor(XW_REPLAY_PROMPT_BORDER);
				g_flightFillClipRectFn();
				festring_setbound(promptLeft, promptTop, promptRight, promptBottom);
				festring_setbackcolor(XW_REPLAY_PROMPT_BACKGROUND);
				g_flightFillClipRectFn();
				festring_setcursor(promptLeft + 1,
								   promptTop + XW_REPLAY_PROMPT_HALF_LINES * g_flightFontLineHeight / 2);
				festring_setfontsize(FLIGHT_FONT_TINY);
				festring_settextcolor(XW_REPLAY_PROMPT_TEXT);
				festring_setdropcolor(XW_REPLAY_PROMPT_SHADOW);
				festring_outstringcenter("Do you want to view your flight recorder film (Y/N)?");
				FlightDisplay_UnlockSurface();
				g_flightResetPaletteFn();
				FlightDisplay_BlitRenderSurface();
				FlightDisplay_Flip();
				do {
					promptKey = FlightInput_GetNextKey();
				} while (promptKey != 'y' && promptKey != 'Y' && promptKey != 'n' && promptKey != 'N');
				if (promptKey == 'n' || promptKey == 'N') {
					FlightDisplay_LockSurface();
					festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
					festring_setbackcolor(XW_REPLAY_PROMPT_BACKGROUND);
					g_flightFillClipRectFn();
					FlightDisplay_UnlockSurface();
					FlightDisplay_BlitRenderSurface();
					FlightDisplay_Flip();
					if (g_flightAudioMode != 0) {
						hilevel_ImSetMasterVol(promptMusicVolume);
						lolevel_ImResume();
						lolevel_ImStopAllSounds();
					}
				} else {
					g_flightRenderTransitionHook();
					if (g_flightAudioMode != 0) {
						hilevel_ImSetMasterVol(promptMusicVolume);
						lolevel_ImResume();
					}
					FlightDisplay_LockSurface();
					festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
					festring_setbackcolor(XW_REPLAY_PROMPT_BACKGROUND);
					g_flightFillClipRectFn();
					FlightDisplay_UnlockSurface();
					FlightDisplay_BlitRenderSurface();
					FlightDisplay_Flip();
					replayio_replayscreen();
					g_flightRenderTransitionHook();
				}
			}
			if (g_missionRuntimeState.flightExitReason == XW_MISSION_TIMEOUT_EXIT)
				fediskio_updatepilotrecord(g_playerFlightState.objectIndex, 0, 0);
			for (j = 0; j < XW_FLIGHT_PILOT_SLOT_COUNT; ++j) {
				pilotObjectRef = g_PilotObjectRefs[j];
				if (pilotObjectRef != UINT8_MAX) {
					if (g_objectTable[pilotObjectRef].objectType != XW_OBJ_NONE)
						fediskio_updatepilotrecord(pilotObjectRef, 0, 0);
					else
						fediskio_updatepilotrecord(pilotObjectRef, ANIM_HYPERSPACE_LOST_PILOT, 1);
				}
			}
		} else {
			g_flightRenderTransitionHook();
		}
	}
	g_playerEngineLoopSuppressed = 1;
	fsfx_UpdatePlayerEngineLoop();
	g_playerEngineLoopSuppressed = 0;
	if (g_flightAudioMode != 0)
		lolevel_ImStopAllSounds();
	g_paletteCycleEnabled = 0;
	fediskio_FreeFlightHandles();
	nullsub_SharedNoOp();
	if (g_flightResolutionMode != (unsigned int)g_flightDefaultResolutionMode)
		FlightDisplay_RebuildForMode(FLIGHT_DISPLAY_MODE_101H);
	g_frontendDisplayWndProcMode = 1;
	g_flightDisplaySurfaceMode &= ~UINT8_MAX;
#endif
}

// FUNCTION: XW 0x42F3E0
void Xw_doframe(void) {
#ifdef XW_MODERN
	XwFlightFrame_Tick();
#else
	int16_t measuredTicks;
	int rendered;
	if (g_replayviewmode == 0) {
		while (g_flightAccumulatedTicks < XW_FRAME_MINIMUM_TICKS) {
			int elapsedTicks = xtimer_Time_Elapsed();
			g_flightAccumulatedTicks += elapsedTicks;
		}
		measuredTicks = g_flightAccumulatedTicks;
		g_previousFrameMeasuredTicks = g_flightAccumulatedTicks;
		g_flightAccumulatedTicks = 0;
		if (calcframerate != 0) {
			g_elapsedTicks = measuredTicks;
			g_simStepScale = XW_SIMULATION_TICKS_PER_SECOND / measuredTicks;
			if (g_simStepScale == 0) {
				g_simStepScale = 1;
				g_elapsedTicks = XW_SIMULATION_TICKS_PER_SECOND;
			}
		}
		calcframerate = 1;
	} else {
		g_elapsedTicks = g_ReplayInputPointer[REPLAY_INPUT_ELAPSED_OFFSET];
		g_simStepScale = XW_SIMULATION_TICKS_PER_SECOND / g_elapsedTicks;
		if (g_simStepScale == 0)
			g_simStepScale = 1;
	}
	if (g_showSimStepScale != 0) {
		festring_setfontsize(XW_FRAME_RATE_FONT_TIER);
		festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
		festring_setbackcolor(XW_FRAME_RATE_BACKGROUND);
		festring_settextcolor(XW_FRAME_RATE_FOREGROUND);
		g_flightTextShadowEnabled = 0;
		festring_setcursor(g_flightScreenWidth - XW_FRAME_RATE_RIGHT_MARGIN,
						   g_flightScreenHeight - XW_FRAME_RATE_BOTTOM_LINES * g_flightFontLineHeight);
#ifdef XW_MODERN
		int hud_pane = XwHud_Push(XW_SNAP_PANE_FRAME_RATE);
#endif
		panelrts_outnum(g_simStepScale, XW_FRAME_RATE_DIGITS, 1);
#ifdef XW_MODERN
		XwHud_Pop(hud_pane);
#endif
	}
	g_replayUiEventConsumed = 0;
	user_userinterface();
	rendered = 0;
	if (g_missionRuntimeState.flightExitRequested == 0 && g_replayUiEventConsumed == 0) {
		Xw_updatetime();
		create_updatefgstatus();
		pai_updateplaneai();
		if (g_deathStarSurfaceModeActive != 0)
			DeathStar_UpdateSurfaceGuns();
		if (g_missionRuntimeState.provingGroundsActive != 0)
			gate_updategateguns();
		laser_weaponsfire();
		dynamix_planedynamics();
		if (g_replayviewmode == 0) {
			Xw_updatescreen();
			rendered = 1;
			FlightDisplay_LockSurface();
			panel_updatepanel();
			FlightDisplay_UnlockSurface();
		} else {
			if (g_ReplayFastForward != 0) {
				if (g_elapsedTicks > (uint16_t)g_ReplayFastForwardTimer) {
					g_ReplayFastForwardTimer += XW_SIMULATION_TICKS_PER_SECOND;
					Xw_updatescreen();
					rendered = 1;
				}
				g_ReplayFastForwardTimer -= g_elapsedTicks;
			} else {
				Xw_updatescreen();
				rendered = 1;
			}
		}
		if (g_debrisEnabled != 0)
			create_checkdebris();
		collide_collisions();
		move_moveobjects();
		anim_updateanimation();
		if ((int16_t)g_legacyOscillatorFrameCounter++ > XW_OSCILLATOR_FRAME_LIMIT) {
			if (g_legacyOscillatorValue == XW_OSCILLATOR_MAXIMUM)
				g_legacyOscillatorDirection = -1;
			if (g_legacyOscillatorValue == 0)
				g_legacyOscillatorDirection = 1;
			g_legacyOscillatorFrameCounter = 0;
			g_legacyOscillatorValue += g_legacyOscillatorDirection;
		}
		Mission_UpdateLogic();
		msg_messageupdate();
		Xw_UpdateDynamicMusicState();
		if (g_fsfxLoaded != 0) {
			if (g_fsfxVoiceQueueCount != 0)
				fsfx_checkblastqueue();
			fsfx_checktieflyby();
		}
		fsfx_UpdatePlayerEngineLoop();
		if (rendered != 0) {
			FlightDisplay_Flip();
			if (g_useHardware3D != 0) {
				RenderScene_ClearFrameBuffers();
				Sound_FlushQueuedEffects();
				return;
			}
			FlightDisplay_BlitRenderSurface();
		}
	}
	Sound_FlushQueuedEffects();

#endif
}

// FUNCTION: XW 0x42F6A0
void Xw_QueueRenderObject(int objectIdx, int sortDepth) {
	if (g_renderObjectListCount < XW_RENDER_OBJECT_LIST_CAPACITY) {
		g_renderObjectListEntries[g_renderObjectListCount].sortDepth = sortDepth;
		g_renderObjectListEntries[g_renderObjectListCount].objectIdx = objectIdx;
		g_renderObjectListEntries[g_renderObjectListCount].next = g_renderListHead;
		g_renderListHead = &g_renderObjectListEntries[g_renderObjectListCount++];
	}
}

// FUNCTION: XW 0x42F710
void Xw_ResetRenderList(void) {
	g_renderObjectListCount = 0;
	g_renderListHead = NULL;
}

// FUNCTION: XW 0x42F720
void Xw_SortRenderListDepthAscending(void) {
	int runLength;
	RenderObjectListEntry* leftTail;
	RenderObjectListEntry* right;
	RenderObjectListEntry* previous;
	RenderObjectListEntry* left;
	int leftRunCount;
	int rightRunCount;
	int processedCount;

	for (runLength = 1; runLength < g_renderObjectListCount; runLength *= 2) {
		right = g_renderListHead;
		previous = NULL;
		left = g_renderListHead;
		for (processedCount = 0; processedCount < g_renderObjectListCount; processedCount += 2 * runLength) {
			for (leftRunCount = 0; leftRunCount < runLength && right != NULL; ++leftRunCount) {
				leftTail = right;
				right = right->next;
			}
			if (right == NULL) {
				break;
			}

			for (rightRunCount = 0; rightRunCount < runLength; ++rightRunCount) {
				while (left->sortDepth <= right->sortDepth) {
					previous = left;
					left = left->next;
					if (previous == leftTail) {
						break;
					}
				}
				if (previous == leftTail) {
					break;
				}

				leftTail->next = right->next;
				if (previous != NULL) {
					previous->next = right;
					previous = right;
					right->next = left;
				} else {
					g_renderListHead = right;
					right->next = left;
					previous = g_renderListHead;
				}
				right = leftTail->next;
				if (right == NULL) {
					break;
				}
			}

			if (previous == leftTail) {
				while (rightRunCount < runLength && right != NULL) {
					leftTail = right;
					++rightRunCount;
					right = right->next;
				}
			}
			left = right;
			previous = leftTail;
			if (right == NULL) {
				break;
			}
		}
	}
}

// FUNCTION: XW 0x42F810
void Xw_updatescreen(void) {
	int16_t objectIndex;
	int missionIndex;
	RenderObjectListEntry* entry;
#ifdef XW_MODERN
	XwFlightCamera_Update();
	XwRenderCapture_CaptureWorld();
#else
	if (g_replayviewmode != 0) {
		replay_calcreplayview();
	} else if (g_flightCamera.focusObjectRef == XW_PLAYER_NO_TARGET) {
		if (g_hyperspaceflag == ANIM_HYPERSPACE_IDLE) {
			trig2_ctop((int32_t)((uint32_t)g_playerFlightState.object->worldX -
								 (uint32_t)g_flightCamera.worldPosition.x),
					   (int32_t)((uint32_t)g_playerFlightState.object->worldY -
								 (uint32_t)g_flightCamera.worldPosition.y),
					   (int32_t)((uint32_t)g_playerFlightState.object->worldZ -
								 (uint32_t)g_flightCamera.worldPosition.z));
			g_flightCamera.viewRoll = 0;
			g_flightCamera.viewPitch = g_trig2Pitch;
			g_flightCamera.viewYaw = g_trig2Yaw;
		}
		fview_newcalcview(g_flightCamera.viewRoll, g_flightCamera.viewPitch, g_flightCamera.viewYaw, 0,
						  g_flightCamera.hudAimX, g_flightCamera.hudAimY, NULL);
	} else if (g_flightCamera.externalViewActive != 0) {
		int16_t historyIndex = g_flightCamera.angleHistory.writeIndex - XW_EXTERNAL_VIEW_LAG;
		if (historyIndex < 0)
			historyIndex += FLIGHT_VIEW_ANGLE_HISTORY_COUNT;
		g_flightCamera.viewRoll = g_flightCamera.angleHistory.roll[historyIndex];
		g_flightCamera.viewPitch = g_flightCamera.angleHistory.pitch[historyIndex];
		g_flightCamera.viewYaw = g_flightCamera.angleHistory.yaw[historyIndex];
		g_flightCamera.angleHistory.roll[g_flightCamera.angleHistory.writeIndex] =
			g_objectTable[g_flightCamera.focusObjectRef].roll;
		g_flightCamera.angleHistory.pitch[g_flightCamera.angleHistory.writeIndex] =
			g_objectTable[g_flightCamera.focusObjectRef].pitch;
		g_flightCamera.angleHistory.yaw[g_flightCamera.angleHistory.writeIndex] =
			g_objectTable[g_flightCamera.focusObjectRef].yaw;
		if (++g_flightCamera.angleHistory.writeIndex == FLIGHT_VIEW_ANGLE_HISTORY_COUNT)
			g_flightCamera.angleHistory.writeIndex = 0;
		fview_newcalcview(g_flightCamera.viewRoll, g_flightCamera.viewPitch, g_flightCamera.viewYaw, 0,
						  g_flightCamera.hudAimX, g_flightCamera.hudAimY, NULL);
		if (g_hyperspaceflag != ANIM_HYPERSPACE_EXTERNAL && g_hyperspaceflag != ANIM_HYPERSPACE_ARRIVE) {
			g_flightCamera.worldPosition.x = g_objectTable[g_flightCamera.focusObjectRef].worldX;
			g_flightCamera.worldPosition.y = g_objectTable[g_flightCamera.focusObjectRef].worldY;
			g_flightCamera.worldPosition.z = g_objectTable[g_flightCamera.focusObjectRef].worldZ;
		} else {
			g_flightCamera.worldPosition.z = 0;
			g_flightCamera.worldPosition.y = 0;
			g_flightCamera.worldPosition.x = 0;
		}
		g_flightCamera.worldPosition.x =
			(int32_t)((uint32_t)g_flightCamera.worldPosition.x -
					  (uint32_t)(((int64_t)g_camMatR2_X * g_flightCamera.externalDistance) >>
								 XW_CAMERA_MATRIX_SHIFT));
		g_flightCamera.worldPosition.y =
			(int32_t)((uint32_t)g_flightCamera.worldPosition.y -
					  (uint32_t)(((int64_t)g_camMatR2_Y * g_flightCamera.externalDistance) >>
								 XW_CAMERA_MATRIX_SHIFT));
		g_flightCamera.worldPosition.z =
			(int32_t)((uint32_t)g_flightCamera.worldPosition.z -
					  (uint32_t)(((int64_t)g_camMatR2_Z * g_flightCamera.externalDistance) >>
								 XW_CAMERA_MATRIX_SHIFT));
	} else {
		g_flightCamera.viewRoll = g_objectTable[g_flightCamera.focusObjectRef].roll;
		g_flightCamera.viewPitch = g_objectTable[g_flightCamera.focusObjectRef].pitch;
		g_flightCamera.viewYaw = g_objectTable[g_flightCamera.focusObjectRef].yaw;
		fview_newcalcview(g_flightCamera.viewRoll, g_flightCamera.viewPitch, g_flightCamera.viewYaw,
						  g_flightCamera.viewAngleD, g_flightCamera.hudAimX, g_flightCamera.hudAimY,
						  &g_objectTable[g_flightCamera.focusObjectRef]);
		g_flightCamera.worldPosition.x = g_objectTable[g_flightCamera.focusObjectRef].worldX;
		g_flightCamera.worldPosition.y = g_objectTable[g_flightCamera.focusObjectRef].worldY;
		g_flightCamera.worldPosition.z = g_objectTable[g_flightCamera.focusObjectRef].worldZ;
		if (g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex &&
			g_hyperspaceflag != ANIM_HYPERSPACE_DEPART && g_hyperspaceflag != ANIM_HYPERSPACE_RETURN) {
			g_flightCamera.worldPosition.x = (int32_t)((uint32_t)g_flightCamera.worldPosition.x +
													   (uint32_t)g_playerFlightState.rotatedCockpitOffset.x);
			g_flightCamera.worldPosition.y = (int32_t)((uint32_t)g_flightCamera.worldPosition.y +
													   (uint32_t)g_playerFlightState.rotatedCockpitOffset.y);
			g_flightCamera.worldPosition.z = (int32_t)((uint32_t)g_flightCamera.worldPosition.z +
													   (uint32_t)g_playerFlightState.rotatedCockpitOffset.z);
		}
	}
#endif
	g_flightLockOffscreenSurface = 0;
	g_flightSurfaceAlreadyLocked = 0;
	if (g_textureCacheFlushPending != 0) {
		if (g_useHardware3D != 0)
			j_std3D_FlushTextureCache();
		g_textureCacheFlushPending = 0;
	}
	RenderScene_Initialize(1);
	g_sceneBillboardQueueCount = 0;
	if (g_useHardware3D != 0) {
		g_billboardObjectOrTypeIndex = XW_BACKDROP_OBJECT_REF;
		backdrp2_backdrop();
		FlightDisplay_LockSurface();
		if (g_hyperspaceflag != ANIM_HYPERSPACE_DEPART && g_hyperspaceflag != ANIM_HYPERSPACE_RETURN)
			FlightStarfield_Render();
		FlightDisplay_UnlockSurface();
	}
	if (g_deathStarSurfaceModeActive != 0 &&
		(g_camMatR2_Z < XW_SURFACE_VIEW_FORWARD_LIMIT || g_flightCamera.worldPosition.z < 0))
		DeathStar_DrawSurfaceAndTrench();
	Xw_ResetRenderList();
	for (objectIndex = 0; objectIndex < g_mainObjectSlotEnd; ++objectIndex) {
		if (objectIndex == g_debrisObjectSlotStart &&
			(g_debrisEnabled == 0 || g_missionRuntimeState.provingGroundsActive != 0)) {
			objectIndex = g_debrisObjectSlotEnd;
			if (objectIndex == g_mainObjectSlotEnd)
				break;
		}
		if ((objectIndex != g_flightCamera.focusObjectRef || g_flightCamera.externalViewActive != 0 ||
			 g_replayviewmode != 0) &&
			g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
			uint16_t radius = g_modelTypeTable[g_objectTable[objectIndex].objectType].maxBoundsExtent;
			g_renderSphereRadius = radius;
			switch (g_objectTable[objectIndex].genusId) {
				case XW_GENUS_STARFIGHTER:
				case XW_GENUS_TRANSPORT:
				case XW_GENUS_UTILITY:
				case XW_GENUS_FREIGHTER:
				case XW_GENUS_STARSHIP:
					g_curCraft = (CraftData*)g_objectTable[objectIndex].instanceData;
					if (Xw_checkobjecteyexyz(objectIndex, radius))
						Xw_QueueRenderObject(objectIndex, g_objectViewZ);
					break;
				case XW_GENUS_PLAYER_PROJECTILE:
				case XW_GENUS_OTHER_PROJECTILE:
					if (Xw_checkobjecteyexyz(objectIndex, radius))
						Xw_QueueRenderObject(objectIndex, g_objectViewZ);
					break;
				case XW_GENUS_DEBRIS:
					if (Xw_checkobjecteyexyz(objectIndex, radius))
						Xw_QueueRenderObject(objectIndex, g_objectViewZ);
					break;
				case XW_GENUS_EXPLOSION_EFFECT:
					if (Xw_checkobjecteyexyz(objectIndex, radius))
						Xw_QueueRenderObject(objectIndex, g_objectViewZ);
					break;
				default:
					break;
			}
		}
	}
	for (missionIndex = 0; missionIndex < MISSION_OBJECT_COUNT; ++missionIndex) {
		if (g_hyperspaceflag != ANIM_HYPERSPACE_DEPART && g_hyperspaceflag != ANIM_HYPERSPACE_RETURN) {
			uint16_t modelType = g_missionObjects[missionIndex].objectType;
			if (modelType != XW_OBJ_NONE) {
				uint16_t radius = g_modelTypeTable[modelType].maxBoundsExtent;
				unsigned int genus = g_missionObjects[missionIndex].genusId;
				g_renderSphereRadius = radius;
				if (genus >= XW_GENUS_MINE && (genus <= XW_GENUS_DEBRIS || genus == XW_GENUS_SCENERY) &&
					Xw_checkstaticobjecteyexyz(g_missionObjects[missionIndex].worldX,
											   g_missionObjects[missionIndex].worldY,
											   g_missionObjects[missionIndex].worldZ, radius)) {
					if (modelType >= XW_SPINNING_MODEL_FIRST && modelType <= XW_SPINNING_MODEL_LAST
#ifdef XW_MODERN
						&& XwFlightTiming_MissionPoseElapsed(missionIndex)
#endif
					) {
#ifdef XW_MODERN
						XwFlightClock clock = XwFlightTiming_EnterReference();
#endif
						g_missionObjects[missionIndex].rollAngle8 +=
							g_elapsedTicks * (missionIndex >> XW_SPIN_ROLL_GROUP_SHIFT) /
							XW_SPIN_ROLL_DIVISOR;
						g_missionObjects[missionIndex].pitchAngle8 +=
							g_elapsedTicks * (missionIndex >> XW_SPIN_PITCH_GROUP_SHIFT) /
							XW_SPIN_PITCH_DIVISOR;
						g_missionObjects[missionIndex].yawAngle8 +=
							g_elapsedTicks * (XW_SPIN_YAW_BASE - (missionIndex >> XW_SPIN_ROLL_GROUP_SHIFT)) /
							XW_SPIN_ROLL_DIVISOR;
#ifdef XW_MODERN
						XwFlightTiming_RestoreClock(clock);
#endif
					}
#ifdef XW_MODERN
					XwRenderObjects_MissionPose(missionIndex);
#endif
					fview_newcalcrotate(g_missionObjects[missionIndex].rollAngle8 << XW_MISSION_ANGLE_SHIFT,
										g_missionObjects[missionIndex].pitchAngle8 << XW_MISSION_ANGLE_SHIFT,
										g_missionObjects[missionIndex].yawAngle8 << XW_MISSION_ANGLE_SHIFT, 0,
										NULL);
					Xw_QueueRenderObject(missionIndex + XW_MISSION_OBJECT_REF_BASE, g_objectViewZ);
					g_objectPointLightCount = 0;
				}
			}
		} else if (missionIndex < g_hyperspaceEffectObjectCount) {
			int16_t x = g_missionObjects[missionIndex].worldX;
			int16_t y = g_missionObjects[missionIndex].worldY;
			int16_t z = g_missionObjects[missionIndex].worldZ;
			g_renderSphereRadius = XW_HYPERSPACE_EFFECT_RADIUS;
			Xw_checkstaticobjecteyexyz(x, y, z, XW_HYPERSPACE_EFFECT_RADIUS);
			FlightHyperspace_DrawTransitionEffectObject(missionIndex);
			z = -z;
			g_missionObjects[missionIndex].worldZ = z;
			Xw_checkstaticobjecteyexyz(x, y, z, g_renderSphereRadius);
			FlightHyperspace_DrawTransitionEffectObject(missionIndex);
			g_missionObjects[missionIndex].worldZ = -z;
			if (missionIndex < (g_hyperspaceEffectObjectCount >> 1)) {
				int16_t savedX, savedZ;
				z >>= 1;
				x = (int16_t)-x >> 1;
				Xw_checkstaticobjecteyexyz(x, y, z, g_renderSphereRadius);
				savedX = g_missionObjects[missionIndex].worldX;
				savedZ = g_missionObjects[missionIndex].worldZ;
				g_missionObjects[missionIndex].worldX = x;
				g_missionObjects[missionIndex].worldZ = z;
				FlightHyperspace_DrawTransitionEffectObject(missionIndex);
				x >>= 1;
				g_missionObjects[missionIndex].worldX = savedX;
				g_missionObjects[missionIndex].worldZ = savedZ;
				z = (int16_t)-z >> 1;
				Xw_checkstaticobjecteyexyz(x, y, z, g_renderSphereRadius);
				savedX = g_missionObjects[missionIndex].worldX;
				savedZ = g_missionObjects[missionIndex].worldZ;
				g_missionObjects[missionIndex].worldX = x;
				g_missionObjects[missionIndex].worldZ = z;
				FlightHyperspace_DrawTransitionEffectObject(missionIndex);
				g_missionObjects[missionIndex].worldX = savedX;
				g_missionObjects[missionIndex].worldZ = savedZ;
			}
		}
	}
	Xw_SortRenderListDepthAscending();
	for (entry = g_renderListHead; entry != NULL; entry = entry->next) {
		int16_t renderIndex = entry->objectIdx;
		if (renderIndex < g_mainObjectSlotEnd) {
			uint16_t genus = g_objectTable[renderIndex].genusId;
			switch (genus) {
				case XW_GENUS_STARFIGHTER:
				case XW_GENUS_TRANSPORT:
				case XW_GENUS_UTILITY:
				case XW_GENUS_FREIGHTER:
				case XW_GENUS_STARSHIP:
					g_curCraft = (CraftData*)g_objectTable[renderIndex].instanceData;
					Xw_getobjecteyexyz(renderIndex);
					if (genus == XW_GENUS_SCENERY)
						g_transformLightDirectionToObjectSpace = 0;
					fview_newcalcrotate(g_objectTable[renderIndex].roll, g_objectTable[renderIndex].pitch,
										g_objectTable[renderIndex].yaw, 0, &g_objectTable[renderIndex]);
					Xw_MakeLocalLights(&g_objectTable[renderIndex]);
					RenderScene_QueueCraftDamageBillboards(renderIndex);
					RenderScene_DrawObjectModel(&g_objectTable[renderIndex]);
					g_objectPointLightCount = 0;
					g_transformLightDirectionToObjectSpace = 1;
					break;
				case XW_GENUS_PLAYER_PROJECTILE:
				case XW_GENUS_OTHER_PROJECTILE:
					Xw_getobjecteyexyz(renderIndex);
					fview_newcalcrotate(g_objectTable[renderIndex].roll, g_objectTable[renderIndex].pitch,
										g_objectTable[renderIndex].yaw, 0, &g_objectTable[renderIndex]);
					RenderScene_DrawRollAlignedObjectModel(renderIndex);
					break;
				case XW_GENUS_DEBRIS:
					Xw_getobjecteyexyz(renderIndex);
					fview_newcalcrotate(g_objectTable[renderIndex].roll, g_objectTable[renderIndex].pitch,
										g_objectTable[renderIndex].yaw, 0, &g_objectTable[renderIndex]);
					if (g_objectTable[renderIndex].objectType == XW_MULTI_MESH_DEBRIS_MODEL)
						RenderScene_DrawAllObjectRootMeshes(renderIndex);
					else
						anim_drawverysimpleobject(renderIndex);
					break;
				case XW_GENUS_EXPLOSION_EFFECT:
					Xw_getobjecteyexyz(renderIndex);
					fview_newcalcrotate(g_objectTable[renderIndex].roll, g_objectTable[renderIndex].pitch,
										g_objectTable[renderIndex].yaw, 0, &g_objectTable[renderIndex]);
					anim_drawverysimpleobject(renderIndex);
					break;
				default:
					break;
			}
		} else {
			renderIndex -= XW_MISSION_OBJECT_REF_BASE;
			if (g_missionObjects[renderIndex].genusId >= XW_GENUS_MINE &&
				(g_missionObjects[renderIndex].genusId <= XW_GENUS_DEBRIS ||
				 g_missionObjects[renderIndex].genusId == XW_GENUS_SCENERY)) {
				if (g_missionObjects[renderIndex].genusId == XW_GENUS_SCENERY) {
					Xw_GetMissionObjectEyeXYZ(renderIndex);
					fview_newcalcrotate(g_missionObjects[renderIndex].rollAngle8 << XW_MISSION_ANGLE_SHIFT,
										g_missionObjects[renderIndex].pitchAngle8 << XW_MISSION_ANGLE_SHIFT,
										g_missionObjects[renderIndex].yawAngle8 << XW_MISSION_ANGLE_SHIFT, 0,
										NULL);
					gate_DrawCourseObject(renderIndex);
				} else {
					Xw_GetMissionObjectEyeXYZ(renderIndex);
					fview_newcalcrotate(g_missionObjects[renderIndex].rollAngle8 << XW_MISSION_ANGLE_SHIFT,
										g_missionObjects[renderIndex].pitchAngle8 << XW_MISSION_ANGLE_SHIFT,
										g_missionObjects[renderIndex].yawAngle8 << XW_MISSION_ANGLE_SHIFT, 0,
										NULL);
					static_drawstaticobject(renderIndex);
				}
				g_objectPointLightCount = 0;
			}
		}
	}
	if (g_useHardware3D == 0) {
		g_billboardObjectOrTypeIndex = XW_BACKDROP_OBJECT_REF;
		backdrp2_backdrop();
		FlightDisplay_LockSurface();
		if (g_hyperspaceflag != ANIM_HYPERSPACE_DEPART && g_hyperspaceflag != ANIM_HYPERSPACE_RETURN)
			FlightStarfield_Render();
		FlightDisplay_UnlockSurface();
	}
	g_sceneFlushDrawTargetMarkers = 1;
	sw3d_DrawVisibleFacesToSurface();
	g_sceneFlushDrawTargetMarkers = 0;
	if (g_useHardware3D == 0) {
		anim_sort_and_draw_bitmaps(1);
		Targeting_w_DrawObjectBox();
	}
	g_flightAccumulatedTicks += xtimer_Time_Elapsed();
	RenderScene_UnlockBuffers();
	if (g_useHardware3D != 0)
		FlightView_CompositeMaskedSoftwareSurface();
	g_flightAccumulatedTicks += xtimer_Time_Elapsed();
	g_flightLockOffscreenSurface = 1;
}

// FUNCTION: XW 0x430380
void Xw_getobjecteyexyz(uint16_t objectIndex) {
	int cameraDeltaX =
		(int32_t)((uint32_t)g_objectTable[objectIndex].worldX - (uint32_t)g_flightCamera.worldPosition.x);
	int cameraDeltaY =
		(int32_t)((uint32_t)g_objectTable[objectIndex].worldY - (uint32_t)g_flightCamera.worldPosition.y);
	int cameraDeltaZ =
		(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ - (uint32_t)g_flightCamera.worldPosition.z);
	g_camRelWorldX = cameraDeltaX;
	g_camRelWorldY = cameraDeltaY;
	g_camRelWorldZ = cameraDeltaZ;
	if (objectIndex < XW_CRAFT_OBJECT_COUNT) {
		g_curCraft = (CraftData*)g_objectTable[objectIndex].instanceData;
		g_curCraft->viewX = transfm2_geteyex(cameraDeltaX, cameraDeltaY, cameraDeltaZ);
		g_objectViewX = g_curCraft->viewX;
		g_curCraft->viewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
		g_objectViewY = g_curCraft->viewY;
		g_curCraft->viewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
		g_objectViewZ = g_curCraft->viewZ;
	} else {
		g_objectViewX = transfm2_geteyex(cameraDeltaX, cameraDeltaY, cameraDeltaZ);
		g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
		g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	}
}

// FUNCTION: XW 0x4304D0
void Xw_GetMissionObjectEyeXYZ(uint16_t missionObjectIndex) {
	int cameraDeltaX = (int32_t)((uint32_t)(g_missionObjects[missionObjectIndex].worldX *
											MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
								 (uint32_t)g_flightCamera.worldPosition.x);
	int cameraDeltaY = (int32_t)((uint32_t)(g_missionObjects[missionObjectIndex].worldY *
											MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
								 (uint32_t)g_flightCamera.worldPosition.y);
	int cameraDeltaZ = (int32_t)((uint32_t)(g_missionObjects[missionObjectIndex].worldZ *
											MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
								 (uint32_t)g_flightCamera.worldPosition.z);
	g_camRelWorldX = cameraDeltaX;
	g_camRelWorldY = cameraDeltaY;
	g_camRelWorldZ = cameraDeltaZ;
	g_objectViewX = transfm2_geteyex(cameraDeltaX, cameraDeltaY, cameraDeltaZ);
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
}

// FUNCTION: XW 0x430580
int16_t Xw_checkobjecteyexyz(uint16_t objectIndex, uint16_t sphereRadius) {
	int radius;
	int depthLimit;
	int absoluteEyeX;
	int absoluteEyeY;
	int cameraDeltaX =
		(int32_t)((uint32_t)g_objectTable[objectIndex].worldX - (uint32_t)g_flightCamera.worldPosition.x);
	int cameraDeltaY =
		(int32_t)((uint32_t)g_objectTable[objectIndex].worldY - (uint32_t)g_flightCamera.worldPosition.y);
	int cameraDeltaZ =
		(int32_t)((uint32_t)g_objectTable[objectIndex].worldZ - (uint32_t)g_flightCamera.worldPosition.z);
	g_camRelWorldX = cameraDeltaX;
	g_camRelWorldY = cameraDeltaY;
	g_camRelWorldZ = cameraDeltaZ;
	g_objectViewZ = transfm2_geteyez(cameraDeltaX, cameraDeltaY, cameraDeltaZ);
	radius = sphereRadius;
	depthLimit = g_objectViewZ;
	depthLimit = (int32_t)((uint32_t)depthLimit + radius);
	if (depthLimit < 0) {
		return 0;
	}
	if ((depthLimit >> XW_OBJECT_DEPTH_SHIFT) > radius) {
		return 0;
	}
	g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	absoluteEyeX = g_objectViewX;
	if (absoluteEyeX < 0) {
		absoluteEyeX = (int32_t)(0U - (uint32_t)absoluteEyeX);
	}
	if ((int32_t)((uint32_t)absoluteEyeX - radius) > depthLimit) {
		return 0;
	}
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	absoluteEyeY = g_objectViewY;
	if (absoluteEyeY < 0) {
		absoluteEyeY = (int32_t)(0U - (uint32_t)absoluteEyeY);
	}
	return (int32_t)((uint32_t)absoluteEyeY - radius) <= depthLimit;
}

// FUNCTION: XW 0x430670
int16_t Xw_checkstaticobjecteyexyz(int16_t worldX, int16_t worldY, int16_t worldZ, uint16_t sphereRadius) {
	int radius;
	int depthLimit;
	int absoluteEyeX;
	int absoluteEyeY;
	g_camRelWorldX = (int32_t)((uint32_t)(worldX * MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
							   (uint32_t)g_flightCamera.worldPosition.x);
	g_camRelWorldY = (int32_t)((uint32_t)(worldY * MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
							   (uint32_t)g_flightCamera.worldPosition.y);
	g_camRelWorldZ = (int32_t)((uint32_t)(worldZ * MISSION_OBJECT_WORLD_COORDINATE_SCALE) -
							   (uint32_t)g_flightCamera.worldPosition.z);
	g_objectViewZ = transfm2_geteyez(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	radius = sphereRadius;
	depthLimit = g_objectViewZ;
	depthLimit = (int32_t)((uint32_t)depthLimit + radius);
	if (depthLimit < 0) {
		return 0;
	}
	if ((depthLimit >> XW_OBJECT_DEPTH_SHIFT) > radius) {
		return 0;
	}
	g_objectViewX = transfm2_geteyex(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	absoluteEyeX = g_objectViewX;
	if (absoluteEyeX < 0) {
		absoluteEyeX = (int32_t)(0U - (uint32_t)absoluteEyeX);
	}
	if ((int32_t)((uint32_t)absoluteEyeX - radius) > depthLimit) {
		return 0;
	}
	g_objectViewY = transfm2_geteyey(g_camRelWorldX, g_camRelWorldY, g_camRelWorldZ);
	absoluteEyeY = g_objectViewY;
	if (absoluteEyeY < 0) {
		absoluteEyeY = (int32_t)(0U - (uint32_t)absoluteEyeY);
	}
	return (int32_t)((uint32_t)absoluteEyeY - radius) <= depthLimit;
}

// FUNCTION: XW 0x430760
void Xw_updatetime(void) {
	uint16_t elapsedTicks = g_elapsedTicks;
	uint16_t objectIndex, timerIndex, currentTargetRef;
#ifdef XW_MODERN
	uint16_t aiElapsedTicks = XwFlightTiming_ReferenceElapsed();
#endif
	for (timerIndex = 0; timerIndex < XW_GLOBAL_COUNTDOWN_TIMER_COUNT; ++timerIndex) {
		if (g_flightGlobalCountdownTimers.ticks[timerIndex] != 0) {
#ifdef XW_MODERN
			uint16_t timerElapsedTicks = elapsedTicks;
			int16_t remainingTicks;
			switch (timerIndex) {
				case XW_TIMER_MISSION_GOAL:
				case XW_TIMER_ANIMATION:
				case XW_TIMER_WEAPON_POWER:
				case XW_TIMER_ARRIVAL_TRIGGER:
				case XW_TIMER_ARRIVAL_DELAY:
					timerElapsedTicks = aiElapsedTicks;
					break;
				default:
					break;
			}
			remainingTicks = g_flightGlobalCountdownTimers.ticks[timerIndex] - timerElapsedTicks;
#else
			int16_t remainingTicks = g_flightGlobalCountdownTimers.ticks[timerIndex] - elapsedTicks;
#endif
			g_flightGlobalCountdownTimers.ticks[timerIndex] = remainingTicks;
			if (remainingTicks < 0)
				g_flightGlobalCountdownTimers.ticks[timerIndex] = 0;
		}
	}
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
			g_curCraft = g_objectTable[objectIndex].instanceData;
			if (g_curCraft->aiThinkTimerTicks != 0) {
#ifdef XW_MODERN
				g_curCraft->aiThinkTimerTicks -= aiElapsedTicks;
#else
				g_curCraft->aiThinkTimerTicks -= elapsedTicks;
#endif
				if ((int16_t)g_curCraft->aiThinkTimerTicks < 0)
					g_curCraft->aiThinkTimerTicks = 0;
			}
			if (g_curCraft->aiManeuverTimerTicks != 0) {
#ifdef XW_MODERN
				g_curCraft->aiManeuverTimerTicks -= aiElapsedTicks;
#else
				g_curCraft->aiManeuverTimerTicks -= g_elapsedTicks;
#endif
				if ((int16_t)g_curCraft->aiManeuverTimerTicks < 0)
					g_curCraft->aiManeuverTimerTicks = 0;
			}
			if (g_curCraft->aiManeuverAuxTimerTicks != 0) {
#ifdef XW_MODERN
				g_curCraft->aiManeuverAuxTimerTicks -= aiElapsedTicks;
#else
				g_curCraft->aiManeuverAuxTimerTicks -= g_elapsedTicks;
#endif
				if ((int16_t)g_curCraft->aiManeuverAuxTimerTicks < 0)
					g_curCraft->aiManeuverAuxTimerTicks = 0;
			}
		}
		elapsedTicks = g_elapsedTicks;
	}
	g_targetHighlightBlinkTicks -= g_elapsedTicks;
	if (g_targetHighlightBlinkTicks < 0) {
		uint16_t maxBoundsExtent = 0;
		currentTargetRef = g_playerFlightState.currentTargetObjectIdx;
		g_targetHighlightObjectAndBlinkBits ^= XW_TARGET_BLINK_BIT;
		if (currentTargetRef != XW_OBJECT_SLOT_UNAVAILABLE) {
			uint16_t objectType;
			pai_distancebetween(currentTargetRef, g_playerFlightState.objectIndex);
			currentTargetRef = g_playerFlightState.currentTargetObjectIdx;
			if (currentTargetRef < XW_MISSION_OBJECT_REF_BASE)
				objectType = g_objectTable[currentTargetRef].objectType;
			else
				objectType = g_missionObjects[currentTargetRef - XW_MISSION_OBJECT_REF_BASE].objectType;
			maxBoundsExtent = g_modelTypeTable[objectType].maxBoundsExtent;
			g_trig2PolarDistance >>= XW_TARGET_BLINK_DISTANCE_SHIFT;
		}
		if (g_targetHighlightObjectAndBlinkBits & XW_TARGET_BLINK_BIT)
			g_targetHighlightBlinkTicks = g_trig2PolarDistance < maxBoundsExtent
											  ? XW_TARGET_BLINK_LONG_TICKS
											  : XW_TARGET_BLINK_SHORT_TICKS;
		else
			g_targetHighlightBlinkTicks = g_trig2PolarDistance < maxBoundsExtent ? XW_TARGET_BLINK_SHORT_TICKS
																				 : XW_TARGET_BLINK_LONG_TICKS;
	} else
		currentTargetRef = g_playerFlightState.currentTargetObjectIdx;
	g_targetHighlightObjectAndBlinkBits &= XW_TARGET_BLINK_BIT;
	g_targetHighlightObjectAndBlinkBits |= currentTargetRef;
	if (g_hyperspaceflag != 0)
		return;
	g_missionElapsedClock.subsecondTicks -= g_elapsedTicks;
	if (g_missionElapsedClock.subsecondTicks <= 0) {
		uint16_t bestRepairPriority;
		uint16_t repairSubsystem;
		uint16_t priorityIndex, subsystemIndex;
		g_missionElapsedClock.subsecondTicks += XW_SIMULATION_TICKS_PER_SECOND;
		++g_missionElapsedClock.seconds;
		if (g_missionElapsedClock.seconds >= XW_SECONDS_PER_MINUTE) {
			g_missionElapsedClock.seconds = 0;
			++g_missionElapsedClock.minutes;
			if (g_missionElapsedClock.minutes >= XW_SECONDS_PER_MINUTE) {
				g_missionElapsedClock.minutes = 0;
				++g_missionElapsedClock.hours;
				if (g_missionElapsedClock.hours >= XW_HOURS_PER_DAY)
					g_missionElapsedClock.hours = 0;
			}
		}
		if (g_missionElapsedClock.seconds == g_surfaceVictoryExitSecond) {
			user_checkreplaycamera();
			g_missionRuntimeState.flightExitRequested = 1;
			g_missionRuntimeState.flightExitReason = XW_MISSION_TIMEOUT_EXIT;
		}
		--g_missionCountdownClock.seconds;
		if (g_missionCountdownClock.seconds == XW_CLOCK_BYTE_UNDERFLOW) {
			uint8_t remainingMinutes = g_missionCountdownClock.minutes - 1;
			g_missionCountdownClock.seconds = XW_SECONDS_PER_MINUTE - 1;
			g_missionCountdownClock.minutes = remainingMinutes;
			if (g_missionRuntimeState.mode == XW_TIMED_MISSION_MODE) {
				if (g_missionCountdownClock.minutes == 1) {
					msg_messageprintf(XW_MSG_MISSION_TWO_MINUTES_REMAINING);
					fsfx_triggersfx(XW_MISSION_MINUTE_WARNING_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
				}
				if (g_missionCountdownClock.minutes == 0) {
					msg_messageprintf(XW_MSG_MISSION_ONE_MINUTE_REMAINING);
					fsfx_triggersfx(XW_MISSION_MINUTE_WARNING_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
				}
			}
			if (g_missionCountdownClock.minutes == XW_CLOCK_BYTE_UNDERFLOW) {
				g_missionCountdownClock.seconds = 0;
				g_missionCountdownClock.minutes = 0;
				if (g_missionRuntimeState.provingGroundsActive != 0) {
					user_checkreplaycamera();
					g_missionRuntimeState.flightExitRequested = 1;
					g_missionRuntimeState.flightExitReason = XW_COURSE_TIMEOUT_EXIT;
				} else if (g_missionRuntimeState.mode == XW_TIMED_MISSION_MODE) {
					if (g_missionFlightGroups[g_playerFlightState.flightGroupIndex].departureMethod ==
						MISSION_DEPARTURE_MOTHERSHIP) {
						user_checkreplaycamera();
						g_missionRuntimeState.flightExitReason = XW_MISSION_TIMEOUT_EXIT;
						g_missionRuntimeState.flightExitRequested = 1;
					} else if (g_playerFlightState.hudSuppressed == 0) {
						g_hyperspaceAbortAndCollisionsAllowed = 0;
						g_hyperspaceflag = 1;
						g_flightCamera.externalViewActive = 0;
						g_flightCamera.manualControlActive = 0;
						g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
						panelrts_setnewpilotview(0);
						g_flightCamera.hudAimX = 0;
						g_flightCamera.hudAimY = 0;
						g_playerHyperspaceElapsedTicks = 0;
						fsfx_triggersfx(XW_TIMEOUT_HYPERSPACE_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
						msg_messageprintf(XW_MSG_MISSION_TIME_EXPIRED_HYPERSPACE);
					} else
						g_missionRuntimeState.flightExitRequested = 1;
				}
			}
		}
		if (g_missionRuntimeState.mode != XW_COUNTDOWN_SILENT_MODE_3 &&
			g_missionRuntimeState.mode != XW_COUNTDOWN_SILENT_MODE_5 &&
			g_missionCountdownClock.minutes == 0 &&
			g_missionCountdownClock.seconds < XW_COUNTDOWN_WARNING_SECONDS)
			fsfx_triggersfx(XW_COUNTDOWN_WARNING_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
		bestRepairPriority = XW_OBJECT_SLOT_UNAVAILABLE;
		repairSubsystem = XW_OBJECT_SLOT_UNAVAILABLE;
		for (priorityIndex = 0; priorityIndex < XW_PLAYER_SUBSYSTEM_COUNT; ++priorityIndex) {
			if (g_playerFlightState.subsystemHealth[g_playerSubsystemRepairPriority[priorityIndex]] == 0 &&
				priorityIndex < bestRepairPriority) {
				bestRepairPriority = priorityIndex;
				repairSubsystem = g_playerSubsystemRepairPriority[priorityIndex];
			}
		}
		for (subsystemIndex = 0; subsystemIndex < XW_PLAYER_SUBSYSTEM_COUNT; ++subsystemIndex) {
			if (g_playerFlightState.subsystemHealth[subsystemIndex] == 0 &&
				subsystemIndex == repairSubsystem) {
				if (g_playerFlightState.subsystemRepairTimers[subsystemIndex] == 0) {
					g_playerFlightState.subsystemHealth[subsystemIndex] = XW_SUBSYSTEM_FULL_HEALTH;
					g_playerFlightState.craft->workingSubsystems |= g_subsystemIdToFlag[subsystemIndex];
					g_msgArgTable[1] = XW_SUBSYSTEM_REPAIRED_MESSAGE_ARG;
					g_msgArgTable[0] = g_subsystemMessageArgById[subsystemIndex];
					msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
					fsfx_triggersfx(XW_SUBSYSTEM_REPAIRED_SOUND, XW_OBJECT_SLOT_UNAVAILABLE);
				} else
					--g_playerFlightState.subsystemRepairTimers[subsystemIndex];
			}
		}
		for (objectIndex = 0; objectIndex < XW_OBJECT_COUNT; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE)
				++g_objectTable[objectIndex].ageSeconds;
		}
	}
}

// FUNCTION: XW 0x430C20
void Xw_UpdateDynamicMusicState(void) {
#ifdef XW_MODERN
	if ((g_musicEventDispatchGate != 0)) {
		Dos94_Xw_UpdateDynamicMusicState();
		return;
	}
#endif
	if (g_flightMusicVolume != 0 && g_flightMusicEnabled != 0 && g_flightMusicPlaybackEnabled != 0
#ifdef XW_MODERN
		&& XwMusicPolicy_CdAvailable()
#endif
	) {
		uint8_t nextTrack = 0;
		if (g_dynamicMusicState == XW_MUSIC_BASE_TRACK && g_dynamicMusicOutcomeLatched == 0) {
			if (g_missionRuntimeState.objectivesUnfinishable != 0) {
				nextTrack = XW_MUSIC_FAILURE_TRACK;
			} else if (g_missionRuntimeState.objectivesCompleted == 1) {
				nextTrack = XW_MUSIC_SUCCESS_TRACK;
			}
			if (nextTrack != 0) {
				CDAudio_PlayTrackFromTime(nextTrack, 0, 0);
				g_dynamicMusicTrackRemainingMs = CDAudio_GetTrackEndTimeMs(nextTrack);
				g_dynamicMusicState = nextTrack;
				g_dynamicMusicLastUpdateTick = timeGetTime();
				g_dynamicMusicOutcomeLatched = 1;
			}
		}
		if (nextTrack == 0) {
			if (g_dynamicMusicSavedRemainingMs == 0) {
				g_dynamicMusicCurrentTick = timeGetTime();
				g_dynamicMusicElapsedMs = g_dynamicMusicCurrentTick - g_dynamicMusicLastUpdateTick;
				g_dynamicMusicLastUpdateTick = g_dynamicMusicCurrentTick;
				g_dynamicMusicTrackRemainingMs -= g_dynamicMusicElapsedMs;
			}
			if (g_dynamicMusicTrackRemainingMs <= 0 && g_dynamicMusicSavedRemainingMs == 0) {
				CDAudio_PlayTrackFromTime(XW_MUSIC_BASE_TRACK, 0, 0);
				g_dynamicMusicTrackRemainingMs = CDAudio_GetTrackEndTimeMs(XW_MUSIC_BASE_TRACK);
				g_dynamicMusicState = XW_MUSIC_BASE_TRACK;
			}
			if (g_dynamicMusicSavedRemainingMs != 0) {
#ifdef XW_MODERN
				g_dynamicMusicResumeOffsetSeconds = (int32_t)((uint32_t)g_dynamicMusicBaseTrackDurationMs -
															  (uint32_t)g_dynamicMusicSavedRemainingMs) /
													XW_MILLISECONDS_PER_SECOND;
#else
				g_dynamicMusicResumeOffsetSeconds =
					(g_dynamicMusicBaseTrackDurationMs - g_dynamicMusicSavedRemainingMs) /
					XW_MILLISECONDS_PER_SECOND;
#endif
				if (g_dynamicMusicResumeOffsetSeconds >= XW_SECONDS_PER_MINUTE) {
					g_dynamicMusicResumeMinute = g_dynamicMusicResumeOffsetSeconds / XW_SECONDS_PER_MINUTE;
					g_dynamicMusicResumeSecond = g_dynamicMusicResumeOffsetSeconds % XW_SECONDS_PER_MINUTE;
				} else {
					g_dynamicMusicResumeMinute = 0;
					g_dynamicMusicResumeSecond = g_dynamicMusicResumeOffsetSeconds;
				}
				g_dynamicMusicTrackRemainingMs = g_dynamicMusicSavedRemainingMs;
				g_dynamicMusicSavedRemainingMs = 0;
				g_dynamicMusicLastUpdateTick = timeGetTime();
				CDAudio_PlayTrackFromTime(g_dynamicMusicState, g_dynamicMusicResumeMinute,
										  g_dynamicMusicResumeSecond);
			}
		}
	} else {
		CDAudio_StopTrack();
		g_dynamicMusicTrackRemainingMs = 0;
		g_dynamicMusicSavedRemainingMs = 0;
	}
}

// FUNCTION: XW 0x430DF0
int Xw_MakeLocalLights(const struct ObjectRecord* object) {
	int lightCount = 0;
	g_objectPointLightCount = 0;
	if (g_localLightsLevel != 0) {
		unsigned int searchRadius =
			g_modelTypeTable[object->objectType].maxBoundsExtent + XW_POINT_LIGHT_RADIUS_MARGIN;
		int worldX = object->worldX;
		int worldY = object->worldY;
		int worldZ = object->worldZ;
		int objectIndex;
		for (objectIndex = 0; objectIndex < g_mainObjectSlotEnd; ++objectIndex) {
			if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
				g_objectTable[objectIndex].genusId == XW_GENUS_EXPLOSION_EFFECT) {
				int dx;
				int dy;
				int dz;
#ifdef XW_MODERN
				dx = (int32_t)((uint32_t)g_objectTable[objectIndex].worldX - (uint32_t)worldX);
				dy = (int32_t)((uint32_t)g_objectTable[objectIndex].worldY - (uint32_t)worldY);
				dz = (int32_t)((uint32_t)g_objectTable[objectIndex].worldZ - (uint32_t)worldZ);
#else
				dx = g_objectTable[objectIndex].worldX - worldX;
				dy = g_objectTable[objectIndex].worldY - worldY;
				dz = g_objectTable[objectIndex].worldZ - worldZ;
#endif
				if ((unsigned int)collide_roughdistance3d(dx, dy, dz) < searchRadius) {
					int dot;
#ifdef XW_MODERN
					dot = (int32_t)((uint32_t)object->cachedSideX * (uint32_t)dx +
									(uint32_t)object->cachedSideY * (uint32_t)dy +
									(uint32_t)object->cachedSideZ * (uint32_t)dz);
#else
					dot = object->cachedSideX * dx + object->cachedSideY * dy + object->cachedSideZ * dz;
#endif
					if (dot >= XW_LIGHT_CLAMP_LIMIT)
						dot = XW_LIGHT_CLAMP_MAX;
					if (dot <= -XW_LIGHT_CLAMP_LIMIT)
						dot = XW_LIGHT_CLAMP_MIN;
					g_objectPointLights[lightCount].x = (dot >> TRANSFM2_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
					dot = (int32_t)((uint32_t)object->cachedForwardX * (uint32_t)dx +
									(uint32_t)object->cachedForwardY * (uint32_t)dy +
									(uint32_t)object->cachedForwardZ * (uint32_t)dz);
#else
					dot = object->cachedForwardX * dx + object->cachedForwardY * dy +
						  object->cachedForwardZ * dz;
#endif
					if (dot >= XW_LIGHT_CLAMP_LIMIT)
						dot = XW_LIGHT_CLAMP_MAX;
					if (dot <= -XW_LIGHT_CLAMP_LIMIT)
						dot = XW_LIGHT_CLAMP_MIN;
					g_objectPointLights[lightCount].y = -(dot >> TRANSFM2_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
					dot = (int32_t)((uint32_t)object->cachedUpX * (uint32_t)dx +
									(uint32_t)object->cachedUpY * (uint32_t)dy +
									(uint32_t)object->cachedUpZ * (uint32_t)dz);
#else
					dot = object->cachedUpX * dx + object->cachedUpY * dy + object->cachedUpZ * dz;
#endif
					if (dot >= XW_LIGHT_CLAMP_LIMIT)
						dot = XW_LIGHT_CLAMP_MAX;
					if (dot <= -XW_LIGHT_CLAMP_LIMIT)
						dot = XW_LIGHT_CLAMP_MIN;
					g_objectPointLights[lightCount].z = (dot >> TRANSFM2_MATRIX_FRACTION_BITS);
					g_objectPointLights[lightCount].intensity = XW_LIGHT_DEFAULT_INTENSITY;
					switch (g_objectTable[objectIndex].objectType) {
						case XW_OBJ_ASTEROID_IMPACT:
						case XW_OBJ_EXPLOSION_138:
							switch (g_objectTable[objectIndex].animationState) {
								case XW_LIGHT_ANIMATION_FRAME_2:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_48;
									break;
								case XW_LIGHT_ANIMATION_FRAME_3:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_96;
									break;
								case XW_LIGHT_ANIMATION_FRAME_4:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_64;
									break;
								case XW_LIGHT_ANIMATION_FRAME_5:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_32;
									break;
							}
							break;
						case XW_OBJ_EXPLOSION_133:
						case XW_OBJ_EXPLOSION_134:
						case XW_OBJ_EXPLOSION_135:
						case XW_OBJ_EXPLOSION_136:
							switch (g_objectTable[objectIndex].animationState) {
								case XW_LIGHT_ANIMATION_FRAME_2:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_192;
									break;
								case XW_LIGHT_ANIMATION_FRAME_4:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_480;
									break;
								case XW_LIGHT_ANIMATION_FRAME_3:
								case XW_LIGHT_ANIMATION_FRAME_5:
								case XW_LIGHT_ANIMATION_FRAME_6:
								case XW_LIGHT_ANIMATION_FRAME_7:
								case XW_LIGHT_ANIMATION_FRAME_8:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_320;
									break;
								case XW_LIGHT_ANIMATION_FRAME_9:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_192;
									break;
								case XW_LIGHT_ANIMATION_FRAME_10:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_96;
									break;
								case XW_LIGHT_ANIMATION_FRAME_11:
									g_objectPointLights[lightCount].intensity = XW_LIGHT_INTENSITY_48;
									break;
							}
							break;
						default:
#ifdef XW_MODERN
							g_objectPointLights[lightCount].intensity =
								(int32_t)((uint32_t)g_flightBrightnessScaleQ8 - XW_LIGHT_BRIGHTNESS_BASE);
#else
							g_objectPointLights[lightCount].intensity =
								g_flightBrightnessScaleQ8 - XW_LIGHT_BRIGHTNESS_BASE;
#endif
							break;
					}
#ifdef XW_MODERN
					g_objectPointLights[lightCount].intensity =
						(int32_t)((uint32_t)g_objectPointLights[lightCount].intensity *
								  XW_LIGHT_INTENSITY_SCALE);
#else
					g_objectPointLights[lightCount].intensity *= XW_LIGHT_INTENSITY_SCALE;
#endif
					++lightCount;
					if (lightCount == XW_POINT_LIGHT_CAPACITY)
						break;
				}
			}
		}
		g_objectPointLightCount = lightCount;
	}
	return lightCount;
}
