#include "xw_runtime/runtime/flight_input.h"
#include "xw_runtime/config/preference_apply.h"
#include "xw_runtime/input/flight_controls.h"
#include "xw_runtime/runtime/flight_frame.h"
#include "xw_runtime/runtime/port.h"

#include "xw/assets/file.h"
#include "xw/audio/fsfx.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/util/shared.h"
#include <string.h>

static uint16_t read_input_word(const uint8_t* data) {
	return data[0] | ((uint16_t)data[1] << REPLAY_INPUT_BYTE_SHIFT);
}

static void write_input_word(uint8_t* data, uint16_t value) {
	data[0] = (uint8_t)value;
	data[1] = (uint8_t)(value >> REPLAY_INPUT_BYTE_SHIFT);
}

/* Modern records append flags and an absolute throttle word after independent roll. */
static bool read_replay_input(XwFlightInput* state) {
	const uint8_t* input = g_ReplayInputPointer;
	state->flags = input[10];
	state->throttle = read_input_word(input + 11);
	if ((state->flags & ~XW_INPUT_THROTTLE_PRESENT) ||
		(!(state->flags & XW_INPUT_THROTTLE_PRESENT) && state->throttle)) {
		XwPort_Fail("Flight recording contains an invalid throttle command");
		return false;
	}
	g_currentActionKey = read_input_word(input);
	g_scaledInputYaw = (int16_t)read_input_word(input + 2);
	g_scaledInputPitch = (int16_t)read_input_word(input + 4);
	g_flightKeyMods = input[6];
	g_xwInputRoll = (int16_t)read_input_word(input + 8);
	g_ReplayInputPointer += REPLAY_INPUT_RECORD_SIZE;
	user_nextreplaycount();
	return true;
}

static void record_input(const XwFlightInput* state) {
	uint8_t* input = g_ReplayInputPointer;
	memset(input, 0, REPLAY_INPUT_RECORD_SIZE);
	write_input_word(input, g_currentActionKey);
	input[REPLAY_INPUT_ELAPSED_OFFSET] = (uint8_t)g_elapsedTicks;
	if (!g_flightCamera.manualControlActive) {
		write_input_word(input + 2, g_scaledInputYaw);
		write_input_word(input + 4, g_scaledInputPitch);
		input[6] = (uint8_t)g_flightKeyMods;
		write_input_word(input + 8, g_xwInputRoll);
	}
	input[10] = state->flags;
	write_input_word(input + 11, state->throttle);
	g_ReplayInputPointer += REPLAY_INPUT_RECORD_SIZE;
	user_nextreplaystore();
}

static void apply_player_input(const XwFlightInput* input) {
	bool eligible = XwFlightControls_ThrottleEligible();
	uint16_t object = g_playerFlightState.objectIndex;
	const CraftData* craft = g_playerFlightState.craft;
	user_inputforplane();
	/* Recorded throttle wins over same-tick keys, never across a craft transition. */
	if (eligible && object == g_playerFlightState.objectIndex && craft == g_playerFlightState.craft)
		XwFlightControls_ApplyThrottle(input);
}

int XwFlightInput_Tick(XwFlightInput* state) {
	int16_t optionsVolume;
	uint16_t hudStateLive;
	int pauseKey;
	int16_t buttons;
	int16_t previousButtons;
	int16_t previousTargetMode;
	uint16_t heldTicks;
	uint16_t target;

	if (state->phase == XW_INPUT_PAUSE) {
		pauseKey = feinput_getrawinput();
		switch (pauseKey) {
			case '\b':
			case '[':
			case '\\':
			case ']':
			case USER_KEY_THROTTLE_0:
			case USER_KEY_THROTTLE_1:
			case USER_KEY_THROTTLE_2:
			case USER_KEY_THROTTLE_3:
			case USER_KEY_THROTTLE_4:
			case USER_KEY_THROTTLE_5:
			case USER_KEY_THROTTLE_6:
			case USER_KEY_THROTTLE_7:
			case USER_KEY_THROTTLE_8:
			case USER_KEY_THROTTLE_9:
			case USER_KEY_THROTTLE_10:
			case USER_KEY_THROTTLE_11:
			case USER_KEY_THROTTLE_12:
				pauseKey = 0;
				break;
			default:
				break;
		}
		if (pauseKey == 0)
			return 0;

		msg_messageprintf(XW_MSG_MISSION_RESUMED);
		XwFlightFrame_ResumeClock();
		calcframerate = 0;
		g_actionKey = 0;
		if (g_flightAudioMode != 0) {
			hilevel_ImSetMasterVol(state->savedMusicVolume);
			lolevel_ImResume();
			g_playerEngineLoopSuppressed = 0;
		}
		state->phase = XW_INPUT_RECORD;
	}
	if (state->phase == XW_INPUT_AFTER_REPLAY) {
		XwFlightFrame_ResumeClock();
		if (g_ReplayReturnToExistingCheckpoint != 0) {
			if (replayio_savereplaybuffer() != 0) {
				g_missionRuntimeState.flightExitRequested = 1;
				g_missionRuntimeState.flightExitReason = USER_EXIT_REPLAY_CHECKPOINT;
			} else {
				g_ReplayReturnToExistingCheckpoint = 0;
				msg_messageprintf(XW_MSG_FILE_ERROR);
			}
		} else {
			msg_messageprintf(XW_MSG_MISSION_RESUMED);
		}
		state->phase = XW_INPUT_RECORD;
	}
	if (state->phase == XW_INPUT_BEGIN) {
		state->flags = 0;
		state->throttle = 0;
		if (XwPreferences_ApplyGameplay()) {
			g_currentActionKey = 0;
			g_scaledInputYaw = g_scaledInputPitch = g_flightKeyMods = 0;
			g_xwInputRoll = 0;
			user_inputforplane();
			return 1;
		}
		if (g_replayviewmode == 0 &&
			(g_flightCamera.focusObjectRef != XW_PLAYER_NO_TARGET &&
			 g_objectTable[g_flightCamera.focusObjectRef].objectType == XW_OBJ_NONE)) {
			g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
			user_resetview();
		}
		if (g_replayviewmode != 0) {
			if (!read_replay_input(state))
				return 1;
			/* Options events carry two more records; button releases must not replace their key. */
			if (g_currentActionKey == USER_KEY_ESCAPE) {
				user_inputforplane();
				return 1;
			}
			state->phase = XW_INPUT_ACTION;
		} else {
			feinput_getrawinput();
			feinput_checkinput();
			switch (g_currentActionKey) {
				case USER_KEY_ESCAPE:
					XwPort_RequestSettings();
					g_currentActionKey = 0;
					break;
				case 'c':
					if (g_hyperspaceflag == 0 && (uint8_t)g_flightDisplaySurfaceMode != 0 &&
						g_playerFlightState.hudSuppressed == 0) {
						fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
						if (g_ReplayRecording != 0) {
							if (replayio_spoolreplayinput() == 0)
								g_ReplayFrameCount -= g_ReplayBufferIndex;
							g_ReplayBufferIndex = 0;
							g_ReplayRecording = 0;
							msg_messageprintf(XW_MSG_REPLAY_CAMERA_OFF);
						} else {
							if (replayio_copytosave(g_ReplayStartFilename) != 0) {
								g_ReplayFrameCount = 0;
								g_ReplayBufferIndex = 0;
								if (g_ReplaySpoolEnabled != 0)
									replayio_openreplayinputfile();
								g_ReplayInputPointer = g_ReplayBufferStart;
								g_ReplayRecording = 1;
								msg_messageprintf(XW_MSG_REPLAY_CAMERA_ON);
								g_ReplayRecordedFlightAvailable = 1;
								g_ReplayRandomSeed = g_gameRandomSeed;
							} else {
								msg_messageprintf(XW_MSG_REPLAY_CAMERA_START_FAILED);
								g_ReplayRecordedFlightAvailable = 0;
							}
						}
						calcframerate = 0;
						fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
					}
					break;
				case 'p':
				case USER_KEY_PAUSE:
					if (g_flightAudioMode != 0) {
						g_playerEngineLoopSuppressed = 1;
						fsfx_UpdatePlayerEngineLoop();
						state->savedMusicVolume = hilevel_ImGetMasterVol();
						hilevel_ImSetMasterVol(0);
						lolevel_ImPause();
					} else {
						state->savedMusicVolume = 0;
					}
					msg_messageprintf(XW_MSG_MISSION_PAUSED);
					XwFlightControls_ResetThrottle();
					state->phase = XW_INPUT_PAUSE;
					return 0;
				case 'v':
					if (g_hyperspaceflag == 0 && (uint8_t)g_flightDisplaySurfaceMode != 0 &&
						g_playerFlightState.hudSuppressed == 0) {
						if (g_ReplayRecordedFlightAvailable == 1) {
							if (g_ReplayRecording != 0) {
								if (replayio_spoolreplayinput() == 0)
									g_ReplayFrameCount -= g_ReplayBufferIndex;
								g_ReplayBufferIndex = 0;
								g_ReplayRecording = 0;
								msg_messageprintf(XW_MSG_REPLAY_CAMERA_OFF);
							}
							calcframerate = 0;
							g_flightRenderTransitionHook();
							replayio_replayscreen();
							state->phase = XW_INPUT_AFTER_REPLAY;
							return 0;
						} else {
							msg_messageprintf(XW_MSG_NO_FILM_RECORDED);
						}
					}
					break;
				case USER_KEY_REBUILD_COCKPIT:
					if (g_flightAudioMode != 0) {
						optionsVolume = hilevel_ImGetMasterVol();
						hilevel_ImSetMasterVol(0);
						lolevel_ImPause();
					} else {
						optionsVolume = 0;
					}
					nullsub_SharedNoOp();
					g_flightRenderTransitionHook();
					if (g_flightCamera.externalViewActive != 0 ||
						g_flightCamera.focusObjectRef != g_playerFlightState.objectIndex) {
						g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
						g_flightCamera.hudStateLive = PANEL_DISPLAY_MODE_INVALID;
						panelrts_setnewpilotview(PANEL_VIEW_NO_COCKPIT);
					} else {
						hudStateLive = g_flightCamera.hudStateLive;
						g_hudLoadedCockpitView = PANEL_VIEW_CACHE_INVALID;
						g_flightCamera.hudStateLive = PANEL_DISPLAY_MODE_INVALID;
						panelrts_setnewpilotview(hudStateLive);
					}
					msg_messageinit();
					if (g_flightAudioMode != 0) {
						hilevel_ImSetMasterVol(optionsVolume);
						lolevel_ImResume();
					}
					break;
				case USER_KEY_VERSION:
					msg_messageprintf(XW_MSG_VERSION_BANNER);
					break;
				default:
					break;
			}
			XwFlightControls_SampleThrottle(state);
			state->phase = XW_INPUT_RECORD;
		}
	}
	if (state->phase == XW_INPUT_RECORD && g_ReplayRecording == 1)
		record_input(state);
	state->phase = XW_INPUT_BEGIN;
	if (g_playerFlightState.hudSuppressed != 0) {
		if (g_playerFlightState.object->objectType != XW_OBJ_NONE) {
			if (g_currentActionKey == 'h')
				g_missionRuntimeState.flightExitRequested = 1;
		} else {
			g_missionRuntimeState.flightExitRequested = 1;
		}
	} else {
		if (g_hyperspaceflag == 0) {
			buttons = g_flightKeyMods & USER_INPUT_BUTTON_MASK;
			previousButtons = g_playerFlightState.savedKeyModifiers & USER_INPUT_BUTTON_MASK;
			if ((g_flightKeyMods & USER_FIRE_BUTTON_MASK) == USER_BUTTON_FIRE &&
				g_flightCamera.manualControlActive == 0)
				laser_fireplayerweapon();
			if (buttons != USER_BUTTON_CYCLE_TARGET && previousButtons == USER_BUTTON_CYCLE_TARGET)
				g_currentActionKey = 'r';
			if (buttons != USER_BUTTON_COCKPIT && previousButtons == USER_BUTTON_COCKPIT)
				g_currentActionKey = '.';
			if (buttons != USER_BUTTON_WEAPON && previousButtons == USER_BUTTON_WEAPON)
				g_currentActionKey = 'w';
			if (buttons != USER_BUTTON_TRANSFER && previousButtons == USER_BUTTON_TRANSFER)
				g_currentActionKey = USER_KEY_LASERS_TO_SHIELDS;
			if (buttons != USER_BUTTON_SHIELD && previousButtons == USER_BUTTON_SHIELD)
				g_currentActionKey = 's';
			previousTargetMode = g_playerFlightState.savedKeyModifiers & USER_TURN_MODIFIER_MASK;
			if ((g_flightKeyMods & USER_TURN_MODIFIER_MASK) == USER_TURN_ROLL_MODE) {
				if (previousTargetMode == USER_TURN_ROLL_MODE)
					heldTicks = g_elapsedTicks + g_playerFlightState.targetButtonHoldTicks;
				else
					heldTicks = g_elapsedTicks;
				g_playerFlightState.targetButtonHoldTicks = heldTicks;
				g_playerFlightState.savedKeyModifiers = g_flightKeyMods;
				if (heldTicks < USER_TARGET_HOLD_TICKS) {
					g_flightKeyMods &= ~USER_TURN_ROLL_MODE;
					apply_player_input(state);
					return 1;
				}
			} else {
				if (previousTargetMode == USER_TURN_ROLL_MODE &&
					g_playerFlightState.targetButtonHoldTicks < USER_TARGET_HOLD_TICKS) {
					target = user_picktarget();
					if (target != XW_PLAYER_NO_TARGET)
						user_setnewtarget(target);
				}
				g_playerFlightState.targetButtonHoldTicks = 0;
				g_playerFlightState.savedKeyModifiers = g_flightKeyMods;
			}
		}
		apply_player_input(state);
	}
	return 1;
}

void XwFlightInput_Cancel(XwFlightInput* state) {
	if (state->phase == XW_INPUT_PAUSE && g_flightAudioMode != 0) {
		hilevel_ImSetMasterVol(state->savedMusicVolume);
		lolevel_ImResume();
		g_playerEngineLoopSuppressed = 0;
	}
	*state = (XwFlightInput) { 0 };
}
