#include "xw_runtime/runtime/flight_sim.h"
#include "xw_dos94/audio/gamesnd.h"
#include "xw_dos94/render/display.h"
#include "xw_runtime/audio/flight_music.h"
#include "xw_runtime/audio/music_policy.h"
#include "xw_runtime/runtime/flight_dispatch.h"
#include "xw_runtime/snapshot/render_capture.h"

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
#include "xw/flight/flight.h"
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
#include "xw/flight/xw.h"
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
#include "xw_runtime/config/preference_apply.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/flight_loading.h"
#include "xw_runtime/runtime/flight_mode.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/runtime/presentation.h"
#include "xw_runtime/timing/flight_timing.h"
#include "xw_runtime/timing/host_clock.h"
#include <landru/task.h>
#include <landru/timer.h>
#include <stdlib.h>
#include <string.h>

typedef enum FlightSimPhase {
	SIM_AFTER_ENTRY_REPLAY,
	SIM_AFTER_RESUME_REPLAY,
	SIM_HYPERSPACE,
	SIM_NEW_MISSION,
	SIM_PREPARE_RENDERER,
	SIM_WAIT_CLOCK,
	SIM_FLIGHT,
	SIM_POST_FLIGHT,
	SIM_PROMPT,
	SIM_AFTER_POST_REPLAY,
	SIM_PILOTS,
	SIM_FINISH
} FlightSimPhase;

typedef struct FlightSimState {
	FlightSimPhase phase, preparedPhase;
	XwFlightFrame frame;
	int16_t savedExitReason;
	uint8_t savedBackdrops, savedDebris;
	uint16_t promptMusicVolume;
	int musicSession;
	int cleanedUp;
	bool retainResources;
} FlightSimState;

static FlightSimState* active_sim;

int XwFlightSim_IsActive(void) { return active_sim != NULL; }

int XwFlightSim_IsPlayerControl(void) {
	return active_sim && landru_task_top() == active_sim &&
		   (active_sim->phase == SIM_FLIGHT || active_sim->phase == SIM_HYPERSPACE ||
			active_sim->phase == SIM_WAIT_CLOCK) &&
		   !g_replayviewmode && !g_missionRuntimeState.flightExitRequested &&
		   active_sim->frame.input.phase != XW_INPUT_AFTER_REPLAY;
}

int XwFlightSim_IsPaused(void) { return active_sim && active_sim->frame.input.phase == XW_INPUT_PAUSE; }

static void sim_setup(FlightEntryMode entryMode) {
	XwObjectTypeId objectType;
	int craftIndex, brightness, i;

	objectType = XwFlightTypes_MissionType(XW_CRAFT_SPECIES_X_WING);
	g_playerEngineLoopSuppressed = 0;
	g_flightAudioMode = 1;
	g_provingGroundsCheckpointBlinkTicks = 0;
	g_flightDisplaySurfaceMode = (g_flightDisplaySurfaceMode & ~UINT8_MAX) | 1;
	g_frontendDisplayWndProcMode = 0;
	for (craftIndex = XW_CRAFT_SPECIES_X_WING; objectType != XW_OBJ_NONE; ++craftIndex) {
		g_objectTypeHudShipIds[objectType] = g_hudShipIdByCraftType[craftIndex];
		objectType = XwFlightTypes_MissionType(craftIndex + 1);
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
	if (XwFlightTypes_Dos()) {
		g_flightScreenWidth = 320;
		g_flightScreenHeight = 200;
		g_flightResolutionMode = FLIGHT_DISPLAY_MODE_13H;
		g_flightBytesPerPixel = 1;
		XwFlightMode_InitBuffers();
		if (g_quitRequested)
			return;
		feinput_setupgraphics(FEINPUT_INITIAL_GRAPHICS_DETAIL);
		XwPresentation_SelectBaseSource(XW_PRESENT_DOS_FLIGHT);
	} else {
		XwPreferences_RestoreFlightResolution();
		FlightDisplay_LockSurface();
		Xw_InitFlightResolution();
		FlightDisplay_UnlockSurface();
		rtsvga2_blankVGA();
		FlightDisplay_LockSurface();
		rtsvga2_initgraphVGA();
		FlightDisplay_UnlockSurface();
		FlightDisplay_Flip();
		feinput_setupgraphics(FEINPUT_INITIAL_GRAPHICS_DETAIL);
		g_flightField62B930 = 1;
		g_paletteBlankFlags = 0;
		FlightDisplay_ClearBackAndAuxiliarySurfaces();
		FlightDisplay_ClearOffscreenSurface();
		FlightDisplay_Flip();
		FlightDisplay_LockSurface();
		XwFlightMode_InitBuffers();
		FlightDisplay_UnlockSurface();
		FlightDisplay_BlitRenderSurface();
		FlightDisplay_Flip();
	}
	/* A fresh allocation cannot retain the previous mission's input pointer.
	 * Resume restores the serialized recorder offset after setup. */
	g_ReplayInputPointer = g_ReplayBufferStart;
	if (entryMode == FLIGHT_ENTRY_NEW_MISSION) {
		g_ReplayBufferIndex = g_ReplayFrameCount = 0;
		g_ReplayRecording = g_ReplayRecordedFlightAvailable = 0;
	}
	g_flightAccumulatedTicks = 0;
	xtimer_Time_Elapsed();
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
}

static void sim_restore_checkpoint(FlightSimState* state, bool retained) {
	uint8_t savedMusicEnabled, savedMusicVolume, savedSfxVolume, savedSfxEnabled;
	uint8_t savedDigitalSound, savedVoice, savedDiskCache, savedUnlimitedWeapons;
	uint8_t savedInvulnerability, savedCollisions, savedStarfield;
	uint8_t savedBackdropsPreference, savedDebrisPreference, savedEngineGlow;
	uint8_t savedBrightness, savedTextureQuality;
	int8_t savedDetailPreference, savedStarfighterDetail, savedStarshipDetail, savedDeathStarDetail;
	uint16_t savedReplayCapacity;
	int restoredBrightness;

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
	state->savedExitReason = g_missionRuntimeState.flightExitReason;
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
	if (!replayio_copyfromsave("+savegame.rpy")) {
		XwPort_Fail("Cannot restore flight checkpoint");
		return;
	}
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
	if (XwFlightTypes_Dos())
		Dos94Display_Restore();
	if (!replayio_restorereplaybuffer()) {
		XwPort_Fail("Cannot restore flight replay input");
		return;
	}
	if (!retained) {
		FlightDisplay_LockSurface();
		XwFlightMode_LoadResources();
		FlightDisplay_UnlockSurface();
		if (g_quitRequested)
			return;
		FlightDisplay_LockSurface();
		XwFlightMode_LoadPanel();
		FlightDisplay_UnlockSurface();
	}
	g_missionRuntimeState.flightExitRequested = 0;
}

static void sim_restore_scene(FlightSimState* state) {
	uint16_t replayWord;
	if (state->savedExitReason == USER_EXIT_OPTIONS) {
		if (g_ReplayRecording != 0)
			XwPreferences_RecordReplayOptions();
		user_ApplyPreferences();
	} else if (state->savedExitReason == USER_EXIT_MAP || state->savedExitReason == USER_EXIT_DAMAGE ||
			   state->savedExitReason == USER_EXIT_BRIEFING) {
		if (g_ReplayRecording != 0) {
			replayWord = g_playerSubsystemRepairPriority[1] +
						 (g_playerSubsystemRepairPriority[0] * (1 << REPLAY_INPUT_BYTE_SHIFT));
			g_ReplayInputPointer[0] = (uint8_t)replayWord;
			g_ReplayInputPointer[1] = (uint8_t)(replayWord >> REPLAY_INPUT_BYTE_SHIFT);
			g_ReplayInputPointer += sizeof(replayWord);
			replayWord = g_playerSubsystemRepairPriority[3] +
						 (g_playerSubsystemRepairPriority[2] * (1 << REPLAY_INPUT_BYTE_SHIFT));
			g_ReplayInputPointer[0] = (uint8_t)replayWord;
			g_ReplayInputPointer[1] = (uint8_t)(replayWord >> REPLAY_INPUT_BYTE_SHIFT);
			g_ReplayInputPointer += sizeof(replayWord);
			replayWord = g_playerSubsystemRepairPriority[5] +
						 (g_playerSubsystemRepairPriority[4] * (1 << REPLAY_INPUT_BYTE_SHIFT));
			g_ReplayInputPointer[0] = (uint8_t)replayWord;
			g_ReplayInputPointer[1] = (uint8_t)(replayWord >> REPLAY_INPUT_BYTE_SHIFT);
			g_ReplayInputPointer += sizeof(replayWord);
			replayWord = g_playerSubsystemRepairPriority[7] +
						 (g_playerSubsystemRepairPriority[6] * (1 << REPLAY_INPUT_BYTE_SHIFT));
			g_ReplayInputPointer[0] = (uint8_t)replayWord;
			g_ReplayInputPointer[1] = (uint8_t)(replayWord >> REPLAY_INPUT_BYTE_SHIFT);
			g_ReplayInputPointer += sizeof(replayWord);
			memset(g_ReplayInputPointer, 0, REPLAY_INPUT_RECORD_SIZE - REPLAY_LEGACY_INPUT_RECORD_SIZE);
			g_ReplayInputPointer += REPLAY_INPUT_RECORD_SIZE - REPLAY_LEGACY_INPUT_RECORD_SIZE;
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
	if (g_quitRequested)
		return;
	msg_messageinit();
	g_missionRuntimeState.flightExitRequested = 0;
}

static void sim_new_mission_begin(FlightSimState* state) {
	XwRenderCapture_WorldChanged();
	g_starfieldColorCacheReusable = 0;
	g_ReplayRecordedFlightAvailable = 0;
	g_hudCockpitResourcesLoaded = 0;
	FlightDisplay_LockSurface();
	int loaded = create_loadmission(g_currentMissionFile);
	FlightDisplay_UnlockSurface();
	if (!loaded)
		XwFlightMode_ResourceError(g_currentMissionFile, "cannot read mission");
	if (g_quitRequested)
		return;
	FlightDisplay_LockSurface();
	XwFlightMode_LoadResources();
	FlightDisplay_UnlockSurface();
	if (g_quitRequested)
		return;
	if (!XwFlightTypes_Dos()) {
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
	}
	FlightDisplay_ClearBackAndAuxiliarySurfaces();
	FlightDisplay_Flip();

	if (g_missionFlightGroups[g_playerFlightState.flightGroupIndex].arrivalMethod != 0 &&
		g_flightTransitionsEnabled != 0) {
		create_createhyperin();
		if (g_quitRequested)
			return;
		XwFlightTiming_ResetWorld();
		state->savedBackdrops = g_backdropsEnabled;
		state->savedDebris = g_debrisEnabled;
		g_backdropsEnabled = 0;
		g_debrisEnabled = 0;
		g_hyperspaceflag = ANIM_HYPERSPACE_SETUP;
		anim_dohyperspace();
		g_flightAccumulatedTicks = 0;
		xtimer_Time_Elapsed();

		state->phase = SIM_HYPERSPACE;
	} else
		state->phase = SIM_NEW_MISSION;
}

static void sim_hyperspace_end(FlightSimState* state) {
	XwRenderCapture_ContinueFlight();
	g_hyperspaceflag = 0;
	g_playerEngineLoopSuppressed = 1;
	fsfx_UpdatePlayerEngineLoop();
	g_playerEngineLoopSuppressed = 0;
	g_backdropsEnabled = state->savedBackdrops;
	g_debrisEnabled = state->savedDebris;
	FlightDisplay_LockSurface();
	int loaded = create_loadmission(g_currentMissionFile);
	FlightDisplay_UnlockSurface();
	if (!loaded)
		XwFlightMode_ResourceError(g_currentMissionFile, "cannot read mission");
	if (g_quitRequested)
		return;
}

static void sim_new_mission_finish(void) {
	FlightDisplay_LockSurface();
	create_createmission();
	FlightDisplay_UnlockSurface();
	if (g_quitRequested)
		return;
	XwFlightTiming_ResetWorld();
	FlightDisplay_BlitRenderSurface();
	FlightDisplay_Flip();
	FlightDisplay_BlitRenderSurface();
	g_flightResetPaletteFn();
	if (g_missionFlightGroups[g_playerFlightState.flightGroupIndex].arrivalMethod != 0) {
		FlightDisplay_LockSurface();
		msg_messageprintf(XW_MSG_HYPERSPACE_JUMP_COMPLETED);
		FlightDisplay_UnlockSurface();
	}
	if (g_missionRuntimeState.provingGroundsActive != 0 && g_missionRuntimeState.provingGroundsLevel > 1u) {
		g_msgArgTable[0] = g_missionRuntimeState.provingGroundsLevel - 1;
		FlightDisplay_LockSurface();
		msg_messageprintf(XW_MSG_PREVIOUS_LEVEL_POINTS_AWARDED);
		FlightDisplay_UnlockSurface();
	}
}

static void sim_music_begin(void) {
	if (XwMusicPolicy_UsesImuse()) {
		XwFlightMusic_Start();
		g_flightAccumulatedTicks = 0;
		xtimer_Time_Elapsed();
		return;
	}
	int cdVolume, musicStartChoice;
	g_dynamicMusicOutcomeLatched = 0;
	int ready = g_flightMusicVolume != 0 && g_flightMusicEnabled != 0 && CDAudio_Initialize() != 0;
	if (ready) {
		cdVolume = UINT16_MAX * (int8_t)g_flightMusicVolume / XW_FLIGHT_INITIAL_VOLUME;
		CDAudio_SetAuxVolume((uint16_t)cdVolume);
		if (g_dynamicMusicSavedRemainingMs == 0) {
			musicStartChoice = math2_getrandom__auxiliary() & (XW_MUSIC_START_CHOICE_COUNT - 1);
			ready = CDAudio_PlayTrackFromTime(XW_MUSIC_BASE_TRACK,
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
	}
	if (!ready) {
		g_dynamicMusicTrackRemainingMs = INT32_MAX;
		g_dynamicMusicSavedRemainingMs = 0;
		g_dynamicMusicState = 0;
	}
	g_flightAccumulatedTicks = 0;
	xtimer_Time_Elapsed();
}

static void sim_prompt_begin(FlightSimState* state) {
	unsigned int promptLeft, promptRight, promptTop;
	uint16_t promptBottom;

	if (g_flightAudioMode != 0) {
		state->promptMusicVolume = hilevel_ImGetMasterVol();
		hilevel_ImSetMasterVol(0);
		lolevel_ImPause();
	}
	FlightDisplay_LockSurface();
	festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
	festring_setbackcolor(XW_REPLAY_PROMPT_BACKGROUND);
	g_flightFillClipRectFn();
	if (XwFlightTypes_Dos()) {
		festring_setbound(30, 61, 290, 80);
		festring_setbackcolor(0x43);
		g_flightFillClipRectFn();
		festring_setbound(31, 62, 289, 79);
		festring_setbackcolor(0x40);
		g_flightFillClipRectFn();
		festring_setcursor(40, 67);
	} else {
		promptLeft = (unsigned int)g_flightScreenWidth / XW_REPLAY_PROMPT_WIDTH_DIVISOR;
		promptRight =
			g_flightScreenWidth - (unsigned int)g_flightScreenWidth / XW_REPLAY_PROMPT_WIDTH_DIVISOR;
		promptTop = (g_flightScreenHeight / 2) + -XW_REPLAY_PROMPT_HALF_LINES * g_flightFontLineHeight -
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
	}
	festring_setfontsize(FLIGHT_FONT_TINY);
	festring_settextcolor(XW_REPLAY_PROMPT_TEXT);
	festring_setdropcolor(XW_REPLAY_PROMPT_SHADOW);
	festring_outstringcenter("Do you want to view your flight recorder film (Y/N)?");
	FlightDisplay_UnlockSurface();
	g_flightResetPaletteFn();
	FlightDisplay_BlitRenderSurface();
	FlightDisplay_Flip();
}

static void sim_prompt_decline(FlightSimState* state) {
	FlightDisplay_LockSurface();
	festring_setbound(0, 0, g_flightScreenWidth, g_flightScreenHeight);
	festring_setbackcolor(XW_REPLAY_PROMPT_BACKGROUND);
	g_flightFillClipRectFn();
	FlightDisplay_UnlockSurface();
	FlightDisplay_BlitRenderSurface();
	FlightDisplay_Flip();
	if (g_flightAudioMode != 0) {
		hilevel_ImSetMasterVol(state->promptMusicVolume);
		lolevel_ImResume();
		lolevel_ImStopAllSounds();
	}
}

static void sim_prompt_accept(FlightSimState* state) {
	g_flightRenderTransitionHook();
	if (g_flightAudioMode != 0) {
		hilevel_ImSetMasterVol(state->promptMusicVolume);
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
}

static void sim_pilot_records(void) {
	int j;
	uint8_t pilotObjectRef;

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
}

static void sim_cleanup(FlightSimState* state) {
	if (state->cleanedUp)
		return;
	state->cleanedUp = 1;

	g_playerEngineLoopSuppressed = 1;
	fsfx_UpdatePlayerEngineLoop();
	g_playerEngineLoopSuppressed = 0;
	if (g_flightAudioMode != 0)
		lolevel_ImStopAllSounds();
	g_paletteCycleEnabled = 0;
	if (state->retainResources)
		XwFlightMode_UnloadSounds();
	else
		XwFlightMode_FreeResources();
	nullsub_SharedNoOp();
	g_frontendDisplayWndProcMode = 1;
	g_flightDisplaySurfaceMode &= ~UINT8_MAX;
	XwPresentation_SelectBaseSource(XW_PRESENT_FRONTEND);
}

static void sim_prepare_renderer(FlightSimState* state, FlightSimPhase next) {
	state->preparedPhase = next;
	state->phase = SIM_PREPARE_RENDERER;
}

static LandruTaskStepResult sim_step(void* self) {
	FlightSimState* state = self;
	if (g_quitRequested)
		return LANDRU_TASK_STEP_DONE;
	if (state->phase == SIM_PREPARE_RENDERER) {
		if (XwFlightLoading_Waiting())
			return LANDRU_TASK_STEP_YIELD;
		state->phase = state->preparedPhase;
		XwFlightFrame_ResumeClock();
	}
	if (state->phase == SIM_AFTER_ENTRY_REPLAY)
		state->phase = SIM_FINISH;
	if (state->phase == SIM_AFTER_RESUME_REPLAY) {
		if (g_ReplayReturnToExistingCheckpoint != 0) {
			if (replayio_savereplaybuffer() != 0) {
				g_missionRuntimeState.flightExitRequested = 1;
				g_missionRuntimeState.flightExitReason = USER_EXIT_REPLAY_CHECKPOINT;
			}
		} else
			sim_restore_scene(state);
		state->phase = SIM_WAIT_CLOCK;
	}
	if (state->phase == SIM_HYPERSPACE) {
		if (g_hyperspaceflag != 0 || state->frame.phase != XW_FRAME_IDLE) {
			if (!XwFlightFrame_TickState(&state->frame))
				return LANDRU_TASK_STEP_YIELD;
			if (XwFlightLoading_Active())
				sim_prepare_renderer(state, state->phase);
			return LANDRU_TASK_STEP_FRAME_COMPLETE;
		}
		sim_hyperspace_end(state);
		if (g_quitRequested)
			return LANDRU_TASK_STEP_DONE;
		state->phase = SIM_NEW_MISSION;
	}
	if (state->phase == SIM_NEW_MISSION) {
		sim_new_mission_finish();
		if (g_quitRequested)
			return LANDRU_TASK_STEP_DONE;
		XwPort_RebaseClock();
		state->phase = SIM_WAIT_CLOCK;
		if (XwFlightLoading_Active()) {
			XwFlightLoading_SetResourcesReady(false);
			sim_prepare_renderer(state, SIM_WAIT_CLOCK);
			return LANDRU_TASK_STEP_YIELD;
		}
	}
	if (state->phase == SIM_WAIT_CLOCK) {
		if (g_flightAccumulatedTicks == 0) {
			g_flightAccumulatedTicks += xtimer_Time_Elapsed();
			if (g_flightAccumulatedTicks == 0)
				return LANDRU_TASK_STEP_YIELD;
		}
		sim_music_begin();
		if (g_quitRequested)
			return LANDRU_TASK_STEP_DONE;
		state->musicSession = 1;
		state->phase = SIM_FLIGHT;
	}
	if (state->phase == SIM_FLIGHT) {
		if (g_missionRuntimeState.flightExitRequested == 0 || state->frame.phase != XW_FRAME_IDLE) {
			if (!XwFlightFrame_TickState(&state->frame))
				return LANDRU_TASK_STEP_YIELD;
			if (XwFlightLoading_Active())
				sim_prepare_renderer(state, state->phase);
			return LANDRU_TASK_STEP_FRAME_COMPLETE;
		}
		state->phase = SIM_POST_FLIGHT;
	}
	if (state->phase == SIM_POST_FLIGHT) {
		if (g_flightMusicEnabled != 0 && (g_missionRuntimeState.flightExitReason == USER_EXIT_OPTIONS ||
										  g_missionRuntimeState.flightExitReason == USER_EXIT_DAMAGE ||
										  g_missionRuntimeState.flightExitReason == USER_EXIT_BRIEFING ||
										  g_missionRuntimeState.flightExitReason == USER_EXIT_MAP)) {
			g_dynamicMusicSavedRemainingMs = g_dynamicMusicTrackRemainingMs;
		}
		CDAudio_CloseDevice();
		XwFlightMusic_Stop();

		state->musicSession = 0;
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
				sim_prompt_begin(state);
				state->phase = SIM_PROMPT;
				return LANDRU_TASK_STEP_YIELD;
			}
			state->phase = SIM_PILOTS;
		} else {
			g_flightRenderTransitionHook();
			state->retainResources = XwFlightTypes_Dos() &&
									 g_missionRuntimeState.flightExitReason >= USER_EXIT_MAP &&
									 g_missionRuntimeState.flightExitReason <= USER_EXIT_OPTIONS;
			state->phase = SIM_FINISH;
		}
	}
	if (state->phase == SIM_PROMPT) {
		uint8_t key = FlightInput_GetNextKey();
		if (key == 'y' || key == 'Y') {
			state->phase = SIM_AFTER_POST_REPLAY;
			sim_prompt_accept(state);
			return LANDRU_TASK_STEP_YIELD;
		}
		if (key != 'n' && key != 'N')
			return LANDRU_TASK_STEP_YIELD;
		sim_prompt_decline(state);
		state->phase = SIM_PILOTS;
	}
	if (state->phase == SIM_AFTER_POST_REPLAY) {
		g_flightRenderTransitionHook();
		state->phase = SIM_PILOTS;
	}
	if (state->phase == SIM_PILOTS) {
		sim_pilot_records();
		state->phase = SIM_FINISH;
	}
	sim_cleanup(state);
	XwPort_RebaseClock();
	return LANDRU_TASK_STEP_DONE;
}

static void sim_end(void* self) {
	XwFlightLoading_End();
	FlightSimState* state = self;
	XwFlightFrame_CancelState(&state->frame);
	if (!state->cleanedUp && (state->phase == SIM_HYPERSPACE || (state->phase == SIM_PREPARE_RENDERER &&
																 state->preparedPhase == SIM_HYPERSPACE))) {
		g_backdropsEnabled = state->savedBackdrops;
		g_debrisEnabled = state->savedDebris;
		g_hyperspaceflag = 0;
	}
	if (!state->cleanedUp && state->phase == SIM_PROMPT && g_flightAudioMode != 0) {
		hilevel_ImSetMasterVol(state->promptMusicVolume);
		lolevel_ImResume();
	}
	if (state->musicSession)
		CDAudio_CloseDevice();
	XwFlightMusic_Stop();
	sim_cleanup(state);
	if (state->retainResources && !g_quitRequested)
		XwFlightMode_Suspend();
	else {
		if (state->retainResources)
			XwFlightMode_FreeResources();
		XwFlightMode_Deactivate();
	}
	if (!g_quitRequested)
		FlightDisplay_RebuildForMode(FLIGHT_DISPLAY_MODE_101H);
	active_sim = NULL;
}

static uint64_t sim_next_wake(const void* self) {
	const FlightSimState* state = self;
	if (state->phase == SIM_WAIT_CLOCK)
		return g_flightAccumulatedTicks ? 0 : xtimer_Elapsed_Delay_Us(1);
	if (state->phase == SIM_FLIGHT || state->phase == SIM_HYPERSPACE)
		return XwFlightFrame_NextWakeDelayUs(&state->frame);
	return UINT64_MAX;
}

static const LandruTaskVtable sim_vtable = { sim_step, sim_end, NULL, sim_next_wake };

void XwFlightSim_Begin(FlightEntryMode entryMode) {
	Dos94_gamesnd_game_Set_Flight_Sound();
	char error[256];
	bool retained = XwFlightMode_IsSuspended();
	if (retained && entryMode != FLIGHT_ENTRY_RESUME_SAVED) {
		XwFlightMode_DiscardSuspended();
		retained = false;
	}
	if (!XwFlightMode_Activate(error, sizeof error)) {
		XwFlightMode_ReportError(error);
		return;
	}
	FlightSimState* state = landru_task_push(&sim_vtable);
	if (state == NULL)
		abort();
	*state = (FlightSimState) { 0 };
	active_sim = state;
	if (entryMode != FLIGHT_ENTRY_REPLAY_VIEWER)
		XwFlightLoading_Begin();
	XwPresentation_Invalidate();
	if (entryMode == FLIGHT_ENTRY_NEW_MISSION)
		XwPreferences_BeginMission();
	if (retained) {
		g_frontendDisplayWndProcMode = 0;
		g_flightDisplaySurfaceMode = 1;
		g_playerEngineLoopSuppressed = 0;
		Dos94Display_Restore();
		XwFlightMode_LoadSounds();
	} else
		sim_setup(entryMode);
	if (g_quitRequested)
		return;
	if (entryMode == FLIGHT_ENTRY_REPLAY_VIEWER) {
		g_flightDisplaySurfaceMode &= ~UINT8_MAX;
		g_hyperspaceflag = 0;
		g_flightField62BAD0 = 0;
		g_surfaceVictoryExitSecond = XW_REPLAY_SURFACE_EXIT_SECOND;
		state->phase = SIM_AFTER_ENTRY_REPLAY;
		replayio_replayscreen();
	} else {
		g_flightDisplaySurfaceMode = (g_flightDisplaySurfaceMode & ~UINT8_MAX) | 1;
		if (entryMode != FLIGHT_ENTRY_NEW_MISSION) {
			sim_restore_checkpoint(state, retained);
			if (g_quitRequested)
				return;
			if (g_ReplayReturnToExistingCheckpoint != 0) {
				XwFlightLoading_End();
				g_flightRenderTransitionHook();
				state->phase = SIM_AFTER_RESUME_REPLAY;
				replayio_replayscreen();
			} else {
				sim_restore_scene(state);
				state->phase = SIM_WAIT_CLOCK;
			}
		} else
			sim_new_mission_begin(state);
	}
	if (XwFlightLoading_Active() && (state->phase == SIM_HYPERSPACE || state->phase == SIM_WAIT_CLOCK))
		sim_prepare_renderer(state, state->phase);
	XwPort_RebaseClock();
}
