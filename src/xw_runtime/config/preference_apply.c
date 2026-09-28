/* Keep live audio/scene options separate from mission-owned renderer and replay storage. */
#include "xw_runtime/config/preference_apply.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/xw.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shellext.h"
#include "xw/input/joystick.h"
#include "xw/render/rtsvga2.h"
#include "xw_dos94/audio/fsfx.h"
#include "xw_runtime/audio/frontend_stream.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/player_engine.h"
#include "xw_runtime/config/config.h"
#include "xw_runtime/config/preferences.h"
#include "xw_runtime/runtime/profile.h"
#include <landru/fade.h>
#include <string.h>

static XwSavedShellPreferences observed;
static int applied_renderer;
static int applied_sb16_filter;

static void ApplyAudioFilter(void) {
	int enabled = XwConfig_Settings()->sb16_filter_enabled;
	if (enabled == applied_sb16_filter)
		return;
	if (g_dos94Imuse)
		imuse_set_wave_output_filter(g_dos94Imuse,
									 enabled ? IMUSE_WAVE_OUTPUT_FILTER_SB16 : IMUSE_WAVE_OUTPUT_FILTER_NONE);
	applied_sb16_filter = enabled;
}

bool XwPreferences_RendererPending(int resolution) { return resolution != applied_renderer; }

void XwPreferences_RestoreFlightResolution(void) {
	static const int modes[] = { RTSVGA2_MODE_13H, FLIGHT_DISPLAY_MODE_101H, FLIGHT_DISPLAY_MODE_111H,
								 FLIGHT_DISPLAY_MODE_1FFH };
	/* Menu surfaces overwrite the live mode; keep the mission's applied preference. */
	g_flightResolutionMode = XwProfile_DosFlight() ? RTSVGA2_MODE_13H : modes[applied_renderer];
}

static void ApplyLive(const XwShellPreferences* p) {
	g_flightTransitionsEnabled = p->transitionsEnabled;
	g_classicMissionsMirror = p->classicMissions;
	g_flightMusicEnabled = p->musicEnabled;
	g_flightMusicVolume = p->musicVolume;
	g_flightSfxEnabled = p->sfxEnabled;
	g_flightSfxVolume = p->sfxVolume;
	g_flightDigitalSoundEnabled = p->digitalSoundEnabled;
	g_flightVoiceEnabled = p->voiceEnabled;
	g_flightEngineSoundEnabled = p->engineSoundEnabled;
	shellext_Set_Prefs_Sound();
	XwFrontendAudio_ApplyVolume();
	if (!p->sfxEnabled || !p->engineSoundEnabled)
		for (int slot = FSFX_ENGINE_XWING_SLOT; slot < FSFX_SOUND_HANDLE_COUNT; ++slot)
			imuse_stop_sound(g_dos94Imuse, slot);
	xfade_SetSpeechTextEnabled(p->spokenTextEnabled != 0 || p->sfxEnabled == 0);
}

void XwPreferences_BeginMission(void) {
	const XwShellPreferences* p = &XwConfig_Settings()->game.preferences;
	const XwSettings* settings = XwConfig_Settings();
	/* Frontend surfaces reuse the live display mode; retain the last applied 1998 preference. */
	if (!XwProfile_DosFlight())
		applied_renderer = p->resolutionIndex;
	g_flightCraftCollisionsEnabled = settings->starfighter_collision_damage;
	g_flightInvulnerabilityEnabled = settings->player_invulnerable;
	g_unlimitedWeaponsEnabled = settings->unlimited_ammunition;
	g_flightReplayDiskCacheKB = p->replayDiskCacheKB;
	g_flightReplayDiskCacheEnabled = p->replayDiskCacheEnabled;
	g_flightHighDetailStarfield = p->highDetailStarfield;
	g_flightBackdropsPreference = p->backdropsEnabled;
	g_flightDebrisPreference = p->debrisEnabled;
	g_flightMarkingsPreference = p->markingsEnabled;
	g_flightEngineGlowPreference = p->engineGlowEnabled;
	g_flightStarfighterDetail = p->starfighterDetail;
	g_flightStarshipDetail = p->starshipDetail;
	g_flightDeathStarDetail = p->deathStarDetail;
	g_flightInterlaceEnabled = XwProfile_DosFlight() ? 0 : p->interlaceEnabled;
	XwPreferences_RestoreFlightResolution();
	g_modelTextureQuality = XwProfile_DosFlight() ? 0 : p->textureQuality;
	g_flightBrightnessSetting = p->brightness;
	memcpy(g_flightJoystickActions, p->joystickActions, sizeof g_flightJoystickActions);
	user_ApplyPreferences();
}

void XwPreferences_InitRuntime(void) {
	applied_sb16_filter = -1;
	ApplyAudioFilter();
	observed = XwConfig_Settings()->game;
	g_savedShellPreferences = observed;
	g_shellPreferences = observed.preferences;
	XwPreferences_BeginMission();
	ApplyLive(&observed.preferences);
}

void XwPreferences_ApplyPending(void) {
	ApplyAudioFilter();
	const XwSavedShellPreferences* next = &XwConfig_Settings()->game;
	if (XwPreferences_HasPendingSave() || !memcmp(&observed, next, sizeof observed))
		return;
	observed = *next;
	g_savedShellPreferences = *next;
	g_shellPreferences = next->preferences;
	ApplyLive(&next->preferences);
}

void XwPreferences_LoadRuntime(void) {
	if (!XwPreferences_HasPendingSave()) {
		observed = XwConfig_Settings()->game;
		g_savedShellPreferences = observed;
	}
	g_shellPreferences = g_savedShellPreferences.preferences;
	ApplyLive(&g_shellPreferences);
}

/* Two payload records follow the recovered Escape event in film input. */
void XwPreferences_RecordReplayOptions(void) {
	const uint8_t records[2][REPLAY_INPUT_RECORD_SIZE] = {
		{ g_unlimitedWeaponsEnabled, g_flightInvulnerabilityEnabled, g_flightDebrisPreference,
		  g_flightBackdropsPreference, g_flightStarshipDetail, g_flightHighDetailStarfield,
		  g_flightStarfighterDetail, g_flightDeathStarDetail },
		{ g_flightCraftCollisionsEnabled, g_flightMarkingsPreference, g_flightMusicEnabled,
		  g_flightMusicVolume, g_flightSfxEnabled, g_flightSfxVolume,
		  g_flightDigitalSoundEnabled | (g_flightVoiceEnabled << 1), g_flightEngineGlowPreference }
	};
	for (unsigned i = 0; i < 2; ++i) {
		memcpy(g_ReplayInputPointer, records[i], REPLAY_INPUT_RECORD_SIZE);
		g_ReplayInputPointer += REPLAY_INPUT_RECORD_SIZE;
		user_nextreplaystore();
	}
}

static void RecordGameplay(void) {
	if (!g_ReplayRecording)
		return;
	/* Keep the event and both payload records complete, including at cache boundaries. */
	if (g_ReplayFrameCount >= g_ReplayCapacityFrames || g_ReplayCapacityFrames - g_ReplayFrameCount < 3 ||
		(!g_ReplaySpoolEnabled && REPLAY_REFILL_RECORD_COUNT - g_ReplayBufferIndex < 3)) {
		g_ReplayRecording = 0;
		msg_messageprintf(XW_MSG_CAMERA_FILM_EXHAUSTED);
		return;
	}
	if (REPLAY_REFILL_RECORD_COUNT - g_ReplayBufferIndex < 3) {
		if (!replayio_spoolreplayinput()) {
			g_ReplayFrameCount -= g_ReplayBufferIndex;
			g_ReplayRecording = 0;
		}
		g_ReplayBufferIndex = 0;
		g_ReplayInputPointer = g_ReplayBufferStart;
		if (!g_ReplayRecording)
			return;
	}
	const uint8_t event[REPLAY_INPUT_RECORD_SIZE] = { USER_KEY_ESCAPE, 0, 0, 0, 0, 0, 0, g_elapsedTicks };
	memcpy(g_ReplayInputPointer, event, sizeof event);
	g_ReplayInputPointer += sizeof event;
	user_nextreplaystore();
	XwPreferences_RecordReplayOptions();
}

bool XwPreferences_ApplyGameplay(void) {
	const XwSettings* settings = XwConfig_Settings();
	if (g_replayviewmode || g_hyperspaceflag || g_playerFlightState.hudSuppressed ||
		(g_flightCraftCollisionsEnabled == settings->starfighter_collision_damage &&
		 g_flightInvulnerabilityEnabled == settings->player_invulnerable &&
		 g_unlimitedWeaponsEnabled == settings->unlimited_ammunition))
		return false;
	g_flightCraftCollisionsEnabled = settings->starfighter_collision_damage;
	g_flightInvulnerabilityEnabled = settings->player_invulnerable;
	g_unlimitedWeaponsEnabled = settings->unlimited_ammunition;
	user_ApplyPreferences();
	RecordGameplay();
	/* The replay decoder also consumes this event without advancing the simulation. */
	g_replayUiEventConsumed = 1;
	return true;
}
