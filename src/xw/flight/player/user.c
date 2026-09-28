#include "xw/flight/player/user.h"

#ifdef XW_MODERN
#include "xw_dos94/render/detail.h"
#include "xw_dos94/render/transfm2.h"
#include "xw_runtime/hooks/orientation_hook.h"
#include "xw_runtime/input/flight_controls.h"
#include "xw_runtime/runtime/flight_camera.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/port.h"
#include "xw_runtime/storage/replay_format.h"
#include "xw_runtime/timing/flight_timing.h"
#include "xw_runtime/timing/player_timing.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/audio/hilevel.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/fview.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panel.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/frontend/inflight_options.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/math/math2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/sw3d.h"
#include "xw/util/shared.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/flight_frame.h"
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: XW 0x4C51A0
uint8_t g_subsystemIdToFlag[XW_PLAYER_SUBSYSTEM_COUNT] = { 0x02, 0x80, 0x10, 0x20, 0x40, 0x04, 0x01, 0x08 };

// GLOBAL: XW 0x4C51A8
uint8_t g_subsystemMessageArgById[XW_PLAYER_SUBSYSTEM_COUNT] = { 0x22, 0x24, 0xA6, 0x1C,
																 0x1B, 0x21, 0x23, 0xA5 };

// GLOBAL: XW 0x4C51B0
uint8_t g_subsystemRepairDuration[XW_PLAYER_SUBSYSTEM_COUNT] = { 180, 44, 45, 25, 100, 30, 50, 60 };

// GLOBAL: XW 0x4C51B8
uint8_t g_subsystemFailureHudMaskByRandomSlot[XW_HUD_FAILURE_RANDOM_SLOT_COUNT] = {
	2, 4, 8, 16, 2, 4, 8, 16, 2, 4, 8, 16, 2, 1, 8, 16
};

// GLOBAL: XW 0x4C9BF8
uint16_t g_starDensity = 1;

// GLOBAL: XW 0x4CEE20
const uint8_t g_viewKeyHudStateOffsets[USER_VIEW_DIGIT_COUNT] = { 0, 3, 4, 5, 2, 16, 6, 1, 0, 7 };

// GLOBAL: XW 0x4CEE30
const int16_t g_viewKeyPitchAngles[USER_VIEW_DIGIT_COUNT] = { 0, -24576, -32768, 24576, -16384,
															  0, 16384,  -6144,  0,     6144 };

// GLOBAL: XW 0x4CEE48
const uint8_t g_aiPlanStatusMessageIds[USER_AI_STATUS_COUNT] = {
	122, 123, 124, 123, 124, 123, 124, 123, 124, 125, 125, 126, 127, 128, 129, 130, 131, 132,
	132, 133, 134, 131, 132, 132, 133, 135, 135, 136, 137, 138, 139, 140, 131, 132, 133, 141,
	142, 131, 132, 133, 143, 143, 143, 144, 143, 145, 143, 146, 147, 146, 148, 149, 149, 149,
	149, 150, 151, 152, 153, 152, 154, 155, 155, 156, 157, 158, 159, 159, 124, 154, 0,   0
};

// GLOBAL: XW 0x4CEE90
int g_flightBrightnessScaleQ8 = 256;

// GLOBAL: XW 0x4CEE98
const uint16_t g_craftExplosionSpawnThresholdByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = {
	4096, 8192, 16384, 32767
};

// GLOBAL: XW 0x4CEEA0
const uint16_t g_starshipDetailByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 1, 2, 3, 4 };

// GLOBAL: XW 0x4CEEA8
const uint16_t g_starDensityByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 2, 1, 1, 1 };

// GLOBAL: XW 0x4CEEB0
const uint16_t g_backdropsEnabledByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 0, 0, 0, 1 };

// GLOBAL: XW 0x4CEEB8
const uint16_t g_debrisEnabledByGraphicsDetailPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 0, 0, 1, 1 };

// GLOBAL: XW 0x4CEEC0
const int16_t g_shipDetailValueByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 3, 2, 1, 0 };

// GLOBAL: XW 0x4CEEC8
const uint16_t g_shipDetailPolyCountByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 8, 12, 16, 16 };

// GLOBAL: XW 0x4CEED0
const int16_t g_drawMarkingsByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 0, 1, 1, 1 };

// GLOBAL: XW 0x4CEED8
const uint16_t g_deathStarDetailLevelByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 1, 2, 4, 5 };

// GLOBAL: XW 0x4CEEE0
const uint16_t g_surfaceObjectDetailLimitByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 8, 10, 12, 14 };

// GLOBAL: XW 0x4CEEE8
uint16_t g_trenchObjectDetailLimitByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 8, 12, 16, 19 };

// GLOBAL: XW 0x4CEEF0
const uint16_t g_hyperspaceEffectObjectCountByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 16, 32, 44, 60 };

// GLOBAL: XW 0x4CEEF8
const uint16_t g_gouraudEnableMaskByPreset[USER_GRAPHICS_DETAIL_PRESET_COUNT] = { 0, 0, 64, 64 };

// GLOBAL: XW 0x4CEF00
const int16_t g_shipDetailValueByDetail[USER_DETAIL_TABLE_COUNT] = { 3, 3, 2, 2,  2,  1,  1, 1,
																	 0, 0, 0, -1, -1, -1, 0, 0 };

// GLOBAL: XW 0x4CEF20
const uint8_t g_starshipDetailByDetail[USER_DETAIL_TABLE_COUNT] = { 1, 1, 1, 1, 1, 2, 2, 2,
																	3, 3, 3, 4, 4, 4, 0, 0 };

// GLOBAL: XW 0x4CEF30
const uint8_t g_deathStarDetailLevelByDetail[USER_DETAIL_TABLE_COUNT] = { 1, 1, 1, 1, 1, 1, 2, 2,
																		  3, 3, 4, 4, 5, 5, 0, 0 };

// GLOBAL: XW 0x4F4A2C
uint16_t g_targetAngleScore = 0;

// GLOBAL: XW 0x4F4A3C
uint16_t g_missionCheatOptionsUsed = 0;

// GLOBAL: XW 0x4F76CC
uint8_t g_unlimitedWeaponsEnabled = 0;

// GLOBAL: XW 0x4F76D0
uint8_t g_flightInvulnerabilityEnabled = 0;

// GLOBAL: XW 0x6285E0
uint8_t g_playerSubsystemRepairPriority[XW_PLAYER_SUBSYSTEM_COUNT] = { 0 };

// GLOBAL: XW 0x62AFEF
uint8_t g_engineGlowEnabled = 0;

// GLOBAL: XW 0x62AFF4
uint16_t g_hyperspaceEffectObjectCount = 0;

// GLOBAL: XW 0x62B008
uint8_t g_debrisEnabled = 0;

// GLOBAL: XW 0x62B914
uint16_t g_shipDetailPolyCount = 0;

// GLOBAL: XW 0x62BC80
uint16_t g_craftExplosionSpawnThreshold = 0;

// GLOBAL: XW 0x62BC92
uint16_t g_deathStarDetailLevel = 0;

// GLOBAL: XW 0x62BCA0
uint16_t g_surfaceObjectDetailLimit = 0;

// GLOBAL: XW 0x62BCE0
XwPlayerFlightState g_playerFlightState = { 0 };

// GLOBAL: XW 0x62BD9E
uint8_t g_transformLightDirectionToObjectSpace = 0;

// GLOBAL: XW 0x62BDA2
int16_t g_shipDetailValue = 0;

// GLOBAL: XW 0x62C922
uint16_t g_starshipDetail = 0;

// GLOBAL: XW 0x62D128
uint16_t g_trenchObjectDetailLimit = 0;

// GLOBAL: XW 0x63734A
uint8_t g_backdropsEnabled = 0;

// GLOBAL: XW 0x637368
uint8_t g_replayUiEventConsumed = 0;

// GLOBAL: XW 0x6377B4
int16_t g_drawMarkingsFlag = 0;

// GLOBAL: XW 0x6377B6
uint16_t g_gouraudEnableMask = 0;

// GLOBAL: XW 0x6377C0
uint8_t g_flightSfxGroupUnmuted = 0;

// GLOBAL: XW 0x6377C1
uint8_t g_flightMusicPlaybackEnabled = 0;

// FUNCTION: XW 0x42A640
void user_userinterface(void) {
#ifdef XW_MODERN
	XwFlightFrame_UpdateInput();
#else
	uint16_t replayKeyModifiers;
	int16_t optionsVolume;
	uint16_t hudStateLive;
	int16_t savedMusicVolume;
	int pauseKey;
	int16_t recordedModifiers;
	uint16_t buttons;
	uint16_t previousButtons;
	int16_t previousTargetMode;
	uint16_t heldTicks;
	uint16_t target;

	if (g_replayviewmode == 0 && (g_flightCamera.focusObjectRef != XW_PLAYER_NO_TARGET &&
								  g_objectTable[g_flightCamera.focusObjectRef].objectType == XW_OBJ_NONE)) {
		g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
		user_resetview();
	}
	if (g_replayviewmode != 0) {
		memcpy(&g_currentActionKey, g_ReplayInputPointer, sizeof(g_currentActionKey));
		g_ReplayInputPointer += sizeof(uint16_t);
		memcpy(&g_scaledInputYaw, g_ReplayInputPointer, sizeof(g_scaledInputYaw));
		g_ReplayInputPointer += sizeof(uint16_t);
		memcpy(&g_scaledInputPitch, g_ReplayInputPointer, sizeof(g_scaledInputPitch));
		g_ReplayInputPointer += sizeof(uint16_t);
		replayKeyModifiers = *g_ReplayInputPointer;
		g_ReplayInputPointer += sizeof(uint16_t);
		g_flightKeyMods = replayKeyModifiers;
		user_nextreplaycount();
	} else {
		feinput_getrawinput();
		feinput_checkinput();
		switch (g_currentActionKey) {
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
			case 'p':
			case USER_KEY_PAUSE:
				if (g_flightAudioMode != 0) {
					g_playerEngineLoopSuppressed = 1;
					fsfx_UpdatePlayerEngineLoop();
					savedMusicVolume = hilevel_ImGetMasterVol();
					hilevel_ImSetMasterVol(0);
					lolevel_ImPause();
				} else {
					savedMusicVolume = 0;
				}
				msg_messageprintf(XW_MSG_MISSION_PAUSED);
				do {
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
				} while (pauseKey == 0);
				msg_messageprintf(XW_MSG_MISSION_RESUMED);
				calcframerate = 0;
				g_actionKey = 0;
				if (g_flightAudioMode != 0) {
					hilevel_ImSetMasterVol(savedMusicVolume);
					lolevel_ImResume();
					g_playerEngineLoopSuppressed = 0;
				}
				break;
			case USER_KEY_VERSION:
				msg_messageprintf(XW_MSG_VERSION_BANNER);
				break;
			case USER_KEY_INTERLACE:
				g_sw3dSkipOddScanlines ^= 1u;
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
						File_RemoveFromDataDirectory(g_ReplayStartFilename);
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
					} else {
						msg_messageprintf(XW_MSG_NO_FILM_RECORDED);
					}
				}
				break;
			default:
				break;
		}
		if (g_ReplayRecording == 1) {
			memcpy(g_ReplayInputPointer, &g_currentActionKey, sizeof(g_currentActionKey));
			recordedModifiers = (uint16_t)(g_elapsedTicks << REPLAY_INPUT_BYTE_SHIFT);
			g_ReplayInputPointer += sizeof(uint16_t);
			if (g_flightCamera.manualControlActive != 0) {
				memset(g_ReplayInputPointer, 0, sizeof(uint16_t));
				g_ReplayInputPointer += sizeof(uint16_t);
				memset(g_ReplayInputPointer, 0, sizeof(uint16_t));
				g_ReplayInputPointer += sizeof(uint16_t);
			} else {
				memcpy(g_ReplayInputPointer, &g_scaledInputYaw, sizeof(g_scaledInputYaw));
				g_ReplayInputPointer += sizeof(uint16_t);
				memcpy(g_ReplayInputPointer, &g_scaledInputPitch, sizeof(g_scaledInputPitch));
				g_ReplayInputPointer += sizeof(uint16_t);
				recordedModifiers += g_flightKeyMods;
			}
			memcpy(g_ReplayInputPointer, &recordedModifiers, sizeof(recordedModifiers));
			g_ReplayInputPointer += sizeof(uint16_t);
			user_nextreplaystore();
		}
	}
	if (g_playerFlightState.hudSuppressed != 0) {
		if (g_playerFlightState.object->objectType == XW_OBJ_NONE) {
			g_missionRuntimeState.flightExitRequested = 1;
		} else if (g_currentActionKey == 'h') {
			g_missionRuntimeState.flightExitRequested = 1;
		}
	} else {
		if (g_hyperspaceflag == 0) {
			buttons = g_flightKeyMods & USER_INPUT_BUTTON_MASK;
			previousButtons = g_playerFlightState.savedKeyModifiers & USER_INPUT_BUTTON_MASK;
			if ((buttons & ~USER_TURN_ROLL_MODE) == USER_BUTTON_FIRE &&
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
					heldTicks = g_playerFlightState.targetButtonHoldTicks + g_elapsedTicks;
				else
					heldTicks = g_elapsedTicks;
				g_playerFlightState.targetButtonHoldTicks = heldTicks;
				g_playerFlightState.savedKeyModifiers = g_flightKeyMods;
				if (heldTicks < USER_TARGET_HOLD_TICKS) {
					g_flightKeyMods &= ~USER_TURN_ROLL_MODE;
					user_inputforplane();
					return;
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
		user_inputforplane();
	}
#endif
}

// FUNCTION: XW 0x42ADA0
void user_nextreplaycount(void) {
	++g_ReplayPlaybackFrameIndex;
	++g_ReplayBufferIndex;
	if (g_ReplayPlaybackFrameIndex >= g_ReplayFrameCount) {
		g_ReplayPlaybackActive = 0;
		replay_replaymessage(XW_MSG_FILM_END_REACHED);
		replay_drawreplaybutton(REPLAY_BUTTON_STOP);
		if (g_flightAudioMode != 0 && g_ReplayMusicActive == 1) {
			g_ReplaySavedVolume = hilevel_ImGetMasterVol();
			hilevel_ImSetMasterVol(0);
			lolevel_ImPause();
			g_ReplayMusicActive = 0;
		}
		return;
	}
	if (g_ReplayBufferIndex >= REPLAY_REFILL_RECORD_COUNT) {
		replay_loadreplayinput();
		g_ReplayBufferIndex = 0;
		g_ReplayInputPointer = g_ReplayBufferStart;
	}
}

// FUNCTION: XW 0x42ADF0
void user_nextreplaystore(void) {
	if (++g_ReplayBufferIndex >= REPLAY_REFILL_RECORD_COUNT) {
		if (g_ReplaySpoolEnabled != 0) {
			if (replayio_spoolreplayinput() == 0) {
				g_ReplayRecording = 0;
				g_ReplayFrameCount -= g_ReplayBufferIndex;
			}
			g_ReplayBufferIndex = 0;
			g_ReplayInputPointer = g_ReplayBufferStart;
			calcframerate = 0;
		} else {
			g_ReplayRecording = 0;
			msg_messageprintf(XW_MSG_CAMERA_FILM_EXHAUSTED);
		}
	}
	if (++g_ReplayFrameCount >= g_ReplayCapacityFrames) {
		g_ReplayRecording = 0;
		msg_messageprintf(XW_MSG_CAMERA_FILM_EXHAUSTED);
	}
}

// FUNCTION: XW 0x42AE90
void user_inputforplane(void) {
#ifdef XW_MODERN
	XwPlayerTiming_BeginControls();
#endif
	if (g_hyperspaceflag < ANIM_HYPERSPACE_SETUP && g_hyperspaceAbortAndCollisionsAllowed != 0 &&
		g_currentActionKey == 'h') {
		if (g_hyperspaceflag != 0) {
			msg_messageprintf(XW_MSG_HYPERSPACE_JUMP_ABORTED);
			g_hyperspaceflag = 0;
			return;
		}
	} else if (g_hyperspaceflag != 0) {
		anim_dohyperspace();
		return;
	}
	feinput_degitterinput();
	g_scaledInputPitch *= 2;
	switch (g_currentActionKey) {
		case 'o':
			g_playerFlightState.currentTargetObjectIdx = XW_PLAYER_NO_TARGET;
			break;
		case ' ':
			if (g_playerFlightState.incomingWarheadAlertState != 0) {
				switch (g_playerFlightState.incomingWarheadAlertState) {
					case USER_WARHEAD_ALERT_TRACKING:
						g_playerFlightState.currentTargetObjectIdx =
							g_playerFlightState.incomingWarheadObjectIndex;
						g_playerFlightState.missileLockState = 0;
						g_playerFlightState.craft->warheadLockTicks = 0;
						g_playerFlightState.incomingWarheadAlertState = 0;
						break;
					case USER_WARHEAD_ALERT_END_MISSION:
						user_checkreplaycamera();
						g_missionRuntimeState.flightExitRequested = 1;
						g_missionRuntimeState.flightExitReason = MISSION_GOAL_EVALUATION_EXIT_REASON;
						break;
					default:
						break;
				}
			}
			break;
		case '0':
		case USER_KEY_VIEW_ZERO:
			if (g_replayviewmode == 0 && (g_flightCamera.hudStateLive < USER_UP_VIEW_STATE ||
										  g_flightCamera.externalViewActive != 0)) {
				g_flightCamera.rearViewHudOffset ^= USER_REAR_VIEW_OFFSET;
				g_flightCamera.hudAimX = g_flightCamera.rearViewHudOffset << USER_VIEW_ANGLE_SHIFT;
				if (g_flightCamera.externalViewActive == 0 &&
					g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex)
					panelrts_setnewpilotview(g_flightCamera.hudStateLive ^ USER_REAR_VIEW_OFFSET);
			}
			break;
		case '5':
		case USER_KEY_VIEW_FIVE:
			if (g_replayviewmode == 0) {
				g_flightCamera.hudAimX = USER_UP_VIEW_ANGLE;
				g_flightCamera.hudAimY = 0;
				if (g_flightCamera.externalViewActive == 0 &&
					g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex)
					panelrts_setnewpilotview(USER_UP_VIEW_STATE);
			}
			break;
		case '1':
		case USER_KEY_VIEW_ONE:
		case '2':
		case USER_KEY_VIEW_TWO:
		case '3':
		case USER_KEY_VIEW_THREE:
		case '4':
		case USER_KEY_VIEW_FOUR:
		case '6':
		case USER_KEY_VIEW_SIX:
		case '7':
		case USER_KEY_VIEW_SEVEN:
		case '8':
		case USER_KEY_VIEW_EIGHT:
		case '9':
		case USER_KEY_VIEW_NINE: {
			uint16_t digit = g_currentActionKey;
			if (digit >= USER_KEY_VIEW_ONE)
				digit -= USER_KEY_VIEW_ZERO - '0';
			g_currentActionKey = digit;
			if (g_replayviewmode == 0) {
				if (g_flightCamera.externalViewActive == 0 &&
					g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex) {
					panelrts_setnewpilotview(g_flightCamera.rearViewHudOffset +
											 g_viewKeyHudStateOffsets[digit - '0']);
					digit = g_currentActionKey;
				}
				g_flightCamera.hudAimX = g_flightCamera.rearViewHudOffset << USER_VIEW_ANGLE_SHIFT;
				g_flightCamera.hudAimY = g_viewKeyPitchAngles[digit - '0'];
			}
			break;
		}
		case '.':
		case USER_KEY_COCKPIT:
			if (g_replayviewmode == 0) {
				if (g_flightCamera.externalViewActive == 0 &&
					g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex) {
					if (g_flightCamera.hudStateLive == USER_TARGET_ANNOUNCEMENT_HUD_STATE)
						panelrts_setnewpilotview(0);
					else
						panelrts_setnewpilotview(USER_TARGET_ANNOUNCEMENT_HUD_STATE);
				}
				g_flightCamera.hudAimX = 0;
				g_flightCamera.hudAimY = 0;
			}
			break;
		case USER_KEY_PLAYER_VIEW:
			if (g_replayviewmode == 0 && g_flightCamera.focusObjectRef != g_playerFlightState.objectIndex) {
				g_flightCamera.externalViewActive = 0;
				g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
				user_resetview();
				if (g_flightCamera.hudStateLive == USER_TARGET_ANNOUNCEMENT_HUD_STATE)
					panelrts_setnewpilotview(USER_TARGET_ANNOUNCEMENT_HUD_STATE);
				else
					panelrts_setnewpilotview(0);
			}
			break;
		case USER_KEY_WARHEAD_VIEW:
			if (g_replayviewmode == 0) {
				uint16_t focus = XW_CRAFT_OBJECT_COUNT - 1;
				int16_t found = -1;
				int16_t scan;
				if (g_flightCamera.focusObjectRef != g_playerFlightState.objectIndex)
					focus = g_flightCamera.focusObjectRef;
				for (scan = XW_CRAFT_OBJECT_COUNT; scan < USER_WARHEAD_SLOT_END; ++scan) {
					if (++focus >= USER_WARHEAD_SLOT_END)
						focus = XW_CRAFT_OBJECT_COUNT;
#ifdef XW_MODERN
					if (g_objectTable[focus].objectType == XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149) ||
						g_objectTable[focus].objectType == XwFlightTypes_ObjectType(XW_OBJ_TRACKED_WARHEAD)) {
#else
					if (g_objectTable[focus].objectType == XW_OBJ_WARHEAD_149 ||
						g_objectTable[focus].objectType == XW_OBJ_TRACKED_WARHEAD) {
#endif
						if (g_objectTable[focus].sourceObjectRef == g_playerFlightState.objectIndex)
							found = focus;
						break;
					}
				}
				if (found != -1) {
					if (g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex) {
						g_flightCamera.savedHudState = g_flightCamera.hudStateLive;
						g_flightCamera.externalViewActive = 0;
						g_flightCamera.savedHudAimX = g_flightCamera.hudAimX;
						g_flightCamera.savedHudAimY = g_flightCamera.hudAimY;
					}
					g_flightCamera.focusObjectRef = focus;
					user_resetview();
				}
			}
			break;
		case '/':
		case USER_KEY_EXTERNAL_VIEW:
		case USER_KEY_EXTERNAL_VIEW_ALT:
			if (g_replayviewmode == 0) {
				int wasExternal = g_flightCamera.externalViewActive != 0;
				g_flightCamera.externalViewActive = !wasExternal;
				if (!wasExternal && g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex) {
					g_flightCamera.savedHudState = g_flightCamera.hudStateLive;
					g_flightCamera.savedHudAimX = g_flightCamera.hudAimX;
					g_flightCamera.savedHudAimY = g_flightCamera.hudAimY;
				}
				user_resetview();
			}
			break;
		case '?':
		case USER_KEY_MANUAL_CAMERA:
		case USER_KEY_MANUAL_CAMERA_ALT:
			if (g_replayviewmode == 0) {
				if (g_flightCamera.externalViewActive != 0)
					g_flightCamera.manualControlActive = g_flightCamera.manualControlActive == 0;
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			}
			break;
		case '+':
		case '=': {
			uint16_t engine;
			for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
				user_increasepower(engine, USER_THROTTLE_STEP);
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			break;
		}
		case ']': {
			uint16_t engine;
			for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
				g_playerFlightState.craft->engineThrottle[engine] = XW_CRAFT_THROTTLE_FULL * 2 / 3;
			fsfx_triggersfx(FSFX_THROTTLE_SETTING_SLOT, FSFX_UNPOSITIONED_OBJECT);
			g_msgArgTable[0] = XW_MSG_THROTTLE_TWO_THIRDS_POWER;
			msg_messageprintf(XW_MSG_ENGINE_THROTTLE_SETTING);
			break;
		}
		case '-': {
			uint16_t engine;
			for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
				user_decreasepower(engine, USER_THROTTLE_STEP);
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			break;
		}
		case '[': {
			uint16_t engine;
			for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
				g_playerFlightState.craft->engineThrottle[engine] = XW_CRAFT_THROTTLE_FULL / 3;
			fsfx_triggersfx(FSFX_THROTTLE_SETTING_SLOT, FSFX_UNPOSITIONED_OBJECT);
			g_msgArgTable[0] = XW_MSG_THROTTLE_ONE_THIRD_POWER;
			msg_messageprintf(XW_MSG_ENGINE_THROTTLE_SETTING);
			break;
		}
		case '\\': {
			uint16_t engine;
			for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
				g_playerFlightState.craft->engineThrottle[engine] = 0;
			fsfx_triggersfx(FSFX_THROTTLE_SETTING_SLOT, FSFX_UNPOSITIONED_OBJECT);
			g_msgArgTable[0] = XW_MSG_THROTTLE_NO_POWER;
			msg_messageprintf(XW_MSG_ENGINE_THROTTLE_SETTING);
			break;
		}
		case '\b': {
			uint16_t engine;
			for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
				g_playerFlightState.craft->engineThrottle[engine] = XW_CRAFT_THROTTLE_FULL;
			fsfx_triggersfx(FSFX_THROTTLE_SETTING_SLOT, FSFX_UNPOSITIONED_OBJECT);
			g_msgArgTable[0] = XW_MSG_THROTTLE_FULL_POWER;
			msg_messageprintf(XW_MSG_ENGINE_THROTTLE_SETTING);
			break;
		}
		case 'w': {
			uint8_t mode = g_playerFlightState.selectedWeaponMode;
			uint8_t bank = ++g_playerFlightState.selectedWeaponBank;
			if (mode != 0) {
				if (bank >= g_playerFlightState.craft->warheadLauncherCount) {
					if (g_playerFlightState.craft->cannonClassCount != 0) {
						mode = 0;
						g_playerFlightState.selectedWeaponMode = mode;
					}
					bank = 0;
					g_playerFlightState.selectedWeaponBank = 0;
				}
			} else if (bank >= g_playerFlightState.craft->cannonClassCount) {
				if (g_playerFlightState.craft->warheadLauncherCount != 0) {
					mode = 1;
					g_playerFlightState.selectedWeaponMode = mode;
				}
				bank = 0;
				g_playerFlightState.selectedWeaponBank = 0;
			}
			g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
			if (mode == 0) {
				if ((g_playerFlightState.craft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) != 0)
					msg_messageprintf((XwFlightMessageId)(bank + XW_MSG_LASER_CANNONS_ARMED));
				else {
					g_msgArgTable[0] = bank + XW_MSG_SUBSYSTEM_LASER;
					msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
				}
			} else if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_LAUNCHER) != 0) {
#ifdef XW_MODERN
				/* Message IDs use the Windows projectile numbering. */
				g_msgArgTable[0] =
					XwFlightTypes_CanonicalType(
						g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadProjectileType[bank]) -
					LASER_WARHEAD_MESSAGE_TYPE_BASE;
#else
				g_msgArgTable[0] =
					g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadProjectileType[bank] -
					LASER_WARHEAD_MESSAGE_TYPE_BASE;
#endif
				msg_messageprintf(XW_MSG_WARHEAD_LAUNCHERS_ARMED);
			} else {
#ifdef XW_MODERN
				g_msgArgTable[0] =
					(g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadProjectileType[bank] !=
					 XwFlightTypes_ObjectType(XW_OBJ_WARHEAD_149)) +
					XW_MSG_SUBSYSTEM_TORPEDO_LAUNCHER;
#else
				g_msgArgTable[0] =
					(g_craftTypeDefs[g_playerFlightState.craftTypeIndex].warheadProjectileType[bank] !=
					 XW_OBJ_WARHEAD_149) +
					XW_MSG_SUBSYSTEM_TORPEDO_LAUNCHER;
#endif
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			break;
		}
		case 'f':
#ifdef XW_MODERN
			if (g_playerFlightState.object->objectType == XW_OBJ_X_WING ||
				g_playerFlightState.object->objectType == XwFlightTypes_ObjectType(XW_OBJ_B_WING)) {
#else
			if (g_playerFlightState.object->objectType == XW_OBJ_X_WING ||
				g_playerFlightState.object->objectType == XW_OBJ_B_WING) {
#endif
				g_playerFlightState.craft->sFoilState ^= XW_SFOIL_CLOSED;
				g_playerFlightState.craft->sFoilState |= XW_SFOIL_MOVING;
				fsfx_triggersfx(FSFX_SFOIL_MOVEMENT_SLOT, g_playerFlightState.objectIndex);
				if ((g_playerFlightState.craft->sFoilState & XW_SFOIL_CLOSED) != 0)
					msg_messageprintf(XW_MSG_SFOILS_CLOSING);
				else
					msg_messageprintf(XW_MSG_SFOILS_OPENING);
			}
			break;
		case 'x':
			if (g_playerFlightState.selectedWeaponMode != 0) {
				g_playerFlightState.craft->warheadLauncherFlags[g_playerFlightState.selectedWeaponBank] ^=
					USER_LAUNCHER_LINK_TOGGLE;
#ifdef XW_MODERN
				g_msgArgTable[0] = XwFlightTypes_CanonicalType(
									   g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
										   .warheadProjectileType[g_playerFlightState.selectedWeaponBank]) -
								   LASER_WARHEAD_MESSAGE_TYPE_BASE;
#else
				g_msgArgTable[0] = g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
									   .warheadProjectileType[g_playerFlightState.selectedWeaponBank] -
								   LASER_WARHEAD_MESSAGE_TYPE_BASE;
#endif
				msg_messageprintf((
					XwFlightMessageId)(((g_playerFlightState.craft
											 ->warheadLauncherFlags[g_playerFlightState.selectedWeaponBank] >>
										 1) &
										(XW_LAUNCHER_FIRE_MODE_MASK >> 1)) +
									   XW_MSG_WARHEAD_LAUNCHERS_SINGLE_FIRE));
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			} else {
				uint16_t link =
					g_playerFlightState.craft->laserState.linkMode[g_playerFlightState.selectedWeaponBank] +
					1;
				if (link > LASER_LINK_ALL)
					link = LASER_LINK_SINGLE;
				if (g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
							.laserGroupSlotCount[g_playerFlightState.selectedWeaponBank] !=
						USER_CANNON_PAIRED_GROUP_SIZE &&
					link == LASER_LINK_PAIR)
					link = LASER_LINK_ALL;
				g_playerFlightState.craft->laserState.linkMode[g_playerFlightState.selectedWeaponBank] = link;
				g_playerFlightState.craft->laserState.nextSlot[g_playerFlightState.selectedWeaponBank] =
					g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
						.laserGroupFirstSlot[g_playerFlightState.selectedWeaponBank];
				msg_messageprintf((XwFlightMessageId)(link + XW_MSG_CANNONS_SINGLE_FIRE - LASER_LINK_SINGLE));
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			}
			break;
		case USER_KEY_LASER_RECHARGE: {
			CraftData* craft;
			++g_playerFlightState.craft->laserRedirect;
			craft = g_playerFlightState.craft;
			if (craft->laserRedirect > LASER_RECHARGE_MAX) {
				craft->laserRedirect = 0;
				craft = g_playerFlightState.craft;
			}
			if ((craft->workingSubsystems & LASER_CANNON_SUBSYSTEM_MASK) != 0) {
				g_msgArgTable[0] = craft->laserRedirect + XW_MSG_POWER_TO_ENGINES_MAXIMUM;
				msg_messageprintf(XW_MSG_LASER_RECHARGE_RATE);
			} else {
				g_msgArgTable[0] = XW_MSG_SUBSYSTEM_LASER;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			break;
		}
		case ';':
		case USER_KEY_SHIELDS_TO_LASERS: {
			CraftData* craft = g_playerFlightState.craft;
			int16_t missing = 0;
			int16_t requested;
			int16_t transferred;
			uint16_t slot;
			for (slot = 0; slot < craft->laserSlotCount; ++slot)
				missing += LASER_CHARGE_MAX - craft->weaponSlots[slot].laserCharge;
			if (missing > USER_ENERGY_TRANSFER_LIMIT)
				missing = USER_ENERGY_TRANSFER_LIMIT;
			requested = USER_SHIELD_PER_LASER_CHARGE * missing;
			if (g_playerFlightState.craft->shieldDistribMode == LASER_SHIELD_DISTRIBUTE_FRONT) {
				int16_t energy = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT];
				if (energy < requested)
					requested = energy;
				transferred = requested;
				g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] = energy - requested;
			} else if (g_playerFlightState.craft->shieldDistribMode == LASER_SHIELD_DISTRIBUTE_REAR) {
				int16_t energy = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR];
				if (energy < requested)
					requested = energy;
				transferred = requested;
				g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] = energy - requested;
			} else {
				int16_t front = requested >> 1;
				int16_t rear = requested >> 1;
				if (g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] < front)
					front = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT];
				g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] -= front;
				craft = g_playerFlightState.craft;
				/* The original compares rear energy with the full request, not its half. */
				if (g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] < requested)
					rear = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR];
				transferred = rear + front;
				g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] -= rear;
			}
			transferred /= USER_SHIELD_PER_LASER_CHARGE;
			if (transferred != 0) {
				int16_t attempts;
				slot = 0;
				craft = g_playerFlightState.craft;
				for (attempts = 0; transferred > 0 && attempts < USER_ENERGY_TRANSFER_LIMIT; ++attempts) {
					int8_t charge = craft->weaponSlots[slot].laserCharge;
					if (charge != LASER_CHARGE_MAX) {
						craft->weaponSlots[slot].laserCharge = charge + 1;
						craft = g_playerFlightState.craft;
					}
					--transferred;
					if (++slot >= craft->laserSlotCount)
						slot = 0;
				}
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
				msg_messageprintf(XW_MSG_TRANSFER_SHIELDS_TO_LASERS);
			}
			break;
		}
		case 's':
			if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_SHIELDS) != 0) {
				uint8_t mode = ++g_playerFlightState.craft->shieldDistribMode;
				if (mode <= LASER_SHIELD_DISTRIBUTE_REAR) {
					if (mode == LASER_SHIELD_DISTRIBUTE_REAR)
						user_adjustshields(XW_SHIELD_REAR, XW_SHIELD_FRONT);
					else {
						uint16_t fraction =
							math2_percentage(g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
												 .nominalShieldEnergy[XW_SHIELD_FRONT],
											 g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
													 .nominalShieldEnergy[XW_SHIELD_FRONT] +
												 g_craftTypeDefs[g_playerFlightState.craftTypeIndex]
													 .nominalShieldEnergy[XW_SHIELD_REAR]);
						int16_t energy = g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] +
										 g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT];
						if (energy > 0) {
							g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] =
								math2_fraction(energy, fraction);
							g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] =
								energy - g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT];
						}
					}
				} else {
					g_playerFlightState.craft->shieldDistribMode = LASER_SHIELD_DISTRIBUTE_FRONT;
					user_adjustshields(XW_SHIELD_FRONT, XW_SHIELD_REAR);
				}
				g_msgArgTable[0] = g_playerFlightState.craft->shieldDistribMode + XW_MSG_SHIELDS_FULL_FORWARD;
				msg_messageprintf(XW_MSG_DEFLECTOR_SHIELD_DISTRIBUTION);
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			} else {
				g_msgArgTable[0] = XW_MSG_SUBSYSTEM_DEFLECTOR_SHIELD;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
			break;
		case USER_KEY_SHIELD_RECHARGE:
			if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_SHIELDS) != 0) {
				CraftData* craft;
				++g_playerFlightState.craft->shieldRedirect;
				craft = g_playerFlightState.craft;
				if (craft->shieldRedirect > LASER_RECHARGE_MAX) {
					craft->shieldRedirect = 0;
					craft = g_playerFlightState.craft;
				}
				g_msgArgTable[0] = craft->shieldRedirect + XW_MSG_POWER_TO_ENGINES_MAXIMUM;
				msg_messageprintf(XW_MSG_SHIELD_RECHARGE_RATE);
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			} else {
				g_msgArgTable[0] = XW_MSG_SUBSYSTEM_DEFLECTOR_SHIELD;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
			break;
		case '\'':
		case USER_KEY_LASERS_TO_SHIELDS: {
			CraftData* craft = g_playerFlightState.craft;
			if ((craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_SHIELDS) != 0) {
				int16_t capacity =
					XW_SHIELD_MAX_CHARGE_MULTIPLIER *
					g_craftTypeDefs[g_playerFlightState.craftTypeIndex].nominalShieldEnergy[XW_SHIELD_FRONT];
				int16_t remaining;
				if (craft->shieldDistribMode == LASER_SHIELD_DISTRIBUTE_FRONT)
					remaining = capacity - craft->shieldEnergy[XW_SHIELD_FRONT];
				else if (craft->shieldDistribMode == LASER_SHIELD_DISTRIBUTE_REAR)
					remaining = capacity - craft->shieldEnergy[XW_SHIELD_REAR];
				else
					remaining = XW_SHIELD_BANK_COUNT * capacity - craft->shieldEnergy[XW_SHIELD_REAR] -
								craft->shieldEnergy[XW_SHIELD_FRONT];
				if (remaining > USER_ENERGY_TRANSFER_LIMIT * USER_SHIELD_PER_LASER_CHARGE)
					remaining = USER_ENERGY_TRANSFER_LIMIT * USER_SHIELD_PER_LASER_CHARGE;
				remaining /= USER_SHIELD_PER_LASER_CHARGE;
				if (remaining != 0) {
					uint16_t slot = 0;
					int16_t attempts;
					for (attempts = 0; remaining > 0 && attempts < USER_ENERGY_TRANSFER_LIMIT; ++attempts) {
						int8_t charge = craft->weaponSlots[slot].laserCharge;
						if (charge > 0) {
							--remaining;
							craft->weaponSlots[slot].laserCharge = charge - 1;
							if (g_playerFlightState.craft->shieldDistribMode == LASER_SHIELD_DISTRIBUTE_FRONT)
								g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] +=
									USER_SHIELD_PER_LASER_CHARGE;
							else if (g_playerFlightState.craft->shieldDistribMode ==
									 LASER_SHIELD_DISTRIBUTE_REAR)
								g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] +=
									USER_SHIELD_PER_LASER_CHARGE;
							else {
								g_playerFlightState.craft->shieldEnergy[XW_SHIELD_FRONT] +=
									USER_SHIELD_PER_LASER_CHARGE / XW_SHIELD_BANK_COUNT;
								g_playerFlightState.craft->shieldEnergy[XW_SHIELD_REAR] +=
									USER_SHIELD_PER_LASER_CHARGE / XW_SHIELD_BANK_COUNT;
							}
						}
						craft = g_playerFlightState.craft;
						if (++slot >= craft->laserSlotCount)
							slot = 0;
					}
					fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
					msg_messageprintf(XW_MSG_TRANSFER_LASERS_TO_SHIELDS);
				}
			} else {
				g_msgArgTable[0] = XW_MSG_SUBSYSTEM_DEFLECTOR_SHIELD;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
			break;
		}
		case 't':
			if (g_playerFlightState.currentTargetObjectIdx == XW_PLAYER_NO_TARGET)
				user_setnewtarget(user_picknexttarget(g_playerFlightState.previousTargetObjectIdx, 1));
			else
				user_setnewtarget(user_picknexttarget(g_playerFlightState.currentTargetObjectIdx, 1));
			break;
		case 'y':
			if (g_playerFlightState.currentTargetObjectIdx == XW_PLAYER_NO_TARGET)
				user_setnewtarget(user_picknexttarget(g_playerFlightState.previousTargetObjectIdx, -1));
			else
				user_setnewtarget(user_picknexttarget(g_playerFlightState.currentTargetObjectIdx, -1));
			break;
		case 'u':
		case USER_KEY_TARGET_CROSSHAIR:
			user_setnewtarget(user_picktarget());
			break;
		case USER_KEY_FIRE:
			if (g_flightCamera.manualControlActive == 0)
				laser_fireplayerweapon();
			break;
		case 'r': {
			int16_t index;
			uint32_t distance = UINT32_MAX;
			uint16_t target = XW_PLAYER_NO_TARGET;
			for (index = 0; index < XW_CRAFT_OBJECT_COUNT; ++index) {
				if (g_objectTable[index].objectType != XW_OBJ_NONE &&
					index != g_playerFlightState.objectIndex &&
					g_objectTable[index].genusId == XW_GENUS_STARFIGHTER &&
					g_objectTable[index].iff != g_objectTable[g_playerFlightState.objectIndex].iff) {
					uint8_t kind = ((CraftData*)g_objectTable[index].instanceData)->objectKind;
					if (kind == XW_CRAFT_OBJECT_KIND_0 || kind == XW_CRAFT_OBJECT_KIND_6) {
						pai_distancebetween(g_playerFlightState.objectIndex, index);
						if ((uint32_t)g_trig2PolarDistance < distance) {
							target = index;
							distance = g_trig2PolarDistance;
						}
					}
				}
			}
			user_setnewtarget(target);
			break;
		}
		case 'i':
			g_playerFlightState.hudTargetDetailsEnabled ^= 1;
			g_hudCachedTargetObjectIdx = USER_HUD_TARGET_CACHE_RESET;
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			g_msgArgTable[0] = g_playerFlightState.hudTargetDetailsEnabled + XW_MSG_DISPLAY_TARGETING_MODE;
			msg_messageprintf(XW_MSG_COMBAT_DISPLAY_MODE);
			break;
		case USER_KEY_RECALL_TARGET_1:
		case USER_KEY_RECALL_TARGET_2:
		case USER_KEY_RECALL_TARGET_3:
		case USER_KEY_RECALL_TARGET_4: {
			uint16_t digit = g_currentActionKey - USER_KEY_RECALL_TARGET_1 + '5';
			uint16_t target;
			g_currentActionKey = digit;
			target = g_playerFlightState.savedTargetRefs[digit - '5'];
			if (target != XW_PLAYER_NO_TARGET) {
				int index =
					target >= XW_MISSION_OBJECT_REF_BASE ? target - XW_MISSION_OBJECT_REF_BASE : target;
				/* The original also checks the dynamic table for mission-reference slots. */
				if (g_objectTable[index].objectType != XW_OBJ_NONE) {
					if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_TARGETING) != 0) {
						fsfx_triggersfx(FSFX_TARGET_SELECTED_SLOT, FSFX_UNPOSITIONED_OBJECT);
						g_playerFlightState.currentTargetObjectIdx = target;
						g_playerFlightState.missileLockState = 0;
						g_playerFlightState.craft->warheadLockTicks = 0;
					} else {
						g_msgArgTable[0] = XW_MSG_SUBSYSTEM_TARGETING_COMPUTER;
						g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
						msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
					}
				}
			}
			break;
		}
		case USER_KEY_STORE_TARGET_1:
		case USER_KEY_STORE_TARGET_2:
		case USER_KEY_STORE_TARGET_3:
		case USER_KEY_STORE_TARGET_4: {
			uint16_t digit = g_currentActionKey - USER_KEY_STORE_TARGET_1 + '5';
			g_currentActionKey = digit;
			if (g_playerFlightState.currentTargetObjectIdx != XW_PLAYER_NO_TARGET)
				g_playerFlightState.savedTargetRefs[digit - '5'] = g_playerFlightState.currentTargetObjectIdx;
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			break;
		}
		case 'h':
			if (g_missionRuntimeState.provingGroundsActive != 0) {
				user_checkreplaycamera();
				g_missionRuntimeState.flightExitRequested = 1;
				g_missionRuntimeState.flightExitReason = MISSION_GOAL_EVALUATION_EXIT_REASON;
			} else if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_HYPERDRIVE) == 0) {
				g_msgArgTable[0] = XW_MSG_SUBSYSTEM_HYPERDRIVE;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			} else {
				msg_clearmessagequeue();
				msg_messageprintf(XW_MSG_HYPERSPACE_JUMP_PREPARING);
				g_hyperspaceflag = ANIM_HYPERSPACE_ALIGN;
				g_hyperspaceAbortAndCollisionsAllowed = 1;
				g_flightCamera.externalViewActive = 0;
				g_flightCamera.manualControlActive = 0;
				g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
				panelrts_setnewpilotview(0);
				g_flightCamera.hudAimX = 0;
				g_flightCamera.hudAimY = 0;
				g_playerHyperspaceElapsedTicks = 0;
				fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			}
			break;
		case USER_KEY_DETAIL: {
			uint8_t preset = ++g_flightGraphicsDetailPreset;
			if (preset >= USER_GRAPHICS_DETAIL_PRESET_COUNT) {
				preset = 0;
				g_flightGraphicsDetailPreset = 0;
			}
			user_setdetaillevel(preset);
			nullsub_SharedNoOp();
			g_msgArgTable[0] = g_flightGraphicsDetailPreset + XW_MSG_DETAIL_LOWEST;
			msg_messageprintf(XW_MSG_GRAPHICS_DETAIL_LEVEL);
			fsfx_triggersfx(FSFX_CONTROL_ACKNOWLEDGE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			break;
		}
		case USER_KEY_EJECT:
			if (g_missionRuntimeState.provingGroundsActive != 0) {
				user_checkreplaycamera();
				g_missionRuntimeState.flightExitRequested = 1;
				g_missionRuntimeState.flightExitReason = MISSION_GOAL_EVALUATION_EXIT_REASON;
			} else if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_2) != 0) {
				if (g_hyperspaceflag == 0 && g_replayviewmode == 0) {
					uint16_t craftType;
					uint16_t tumble;
					uint16_t maxTumble;
					if (user_isrescued(g_playerFlightState.objectIndex) != 0) {
						g_missionRuntimeState.flightExitReason = USER_EXIT_RESCUED;
						fediskio_updatepilotrecord(g_playerFlightState.objectIndex, 0, 1);
					} else {
						g_missionRuntimeState.flightExitReason = USER_EXIT_LOST;
						fediskio_updatepilotrecord(g_playerFlightState.objectIndex, 1, 1);
					}
					user_ejectcamera();
					craftType = g_playerFlightState.craft->craftTypeIndex;
					tumble = (math2_getrandom() & USER_EJECT_TUMBLE_MASK) + USER_EJECT_MIN_TUMBLE;
					maxTumble = g_craftTypeDefs[craftType].maxTumbleAngle;
					while (tumble > maxTumble)
						tumble >>= 1;
					g_playerFlightState.object->rollImpulseRate = tumble;
					g_playerFlightState.craft->objectKind = XW_CRAFT_OBJECT_KIND_3;
					g_playerFlightState.object->lifetimeTicks =
						XW_SIMULATION_TICKS_PER_SECOND *
						((math2_getrandom() & USER_EJECT_LIFETIME_MASK) + USER_EJECT_MIN_SECONDS);
				}
			} else {
				g_msgArgTable[0] = XW_MSG_SUBSYSTEM_AUTO_EJECTION;
				g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
				msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
			}
			break;
		case USER_KEY_FRAME_RATE:
			g_showSimStepScale ^= 1;
			break;
		case USER_KEY_MUSIC:
			if (g_flightAudioMode != 0) {
				g_savedShellPreferences.preferences.musicEnabled =
					g_savedShellPreferences.preferences.musicEnabled == 0;
				g_flightMusicEnabled = g_savedShellPreferences.preferences.musicEnabled;
				ShellPreferences_Save();
			}
			break;
		case USER_KEY_SOUND:
			if (g_flightAudioMode != 0) {
				if (g_flightSfxGroupUnmuted != 0) {
					lolevel_ImSetGroupVol(XW_SOUND_GROUP_ALL, 0);
					g_flightSfxGroupUnmuted = 0;
				} else {
					int16_t volume = 0;
					if (g_flightSfxVolume != 0)
						volume = XW_SHELL_VOLUME_STEP * (int8_t)g_flightSfxVolume - 1;
					lolevel_ImSetGroupVol(XW_SOUND_GROUP_ALL, volume);
					g_flightSfxGroupUnmuted = 1;
				}
			}
			break;
		case 'm':
			if (g_missionRuntimeState.provingGroundsActive == 0) {
				if (g_replayviewmode != 0) {
					user_RestoreRepairPriorities();
					g_replayUiEventConsumed = 1;
				} else {
					user_PrepareFlightStatusSnapshot();
					if (replayio_copytosave("+savegame.rpy") != 0 && replayio_savereplaybuffer() != 0) {
						g_missionRuntimeState.flightExitRequested = 1;
						g_missionRuntimeState.flightExitReason = USER_EXIT_MAP;
					} else
						msg_messageprintf(XW_MSG_FILE_ERROR);
				}
			}
			break;
		case 'd':
			if (g_missionRuntimeState.provingGroundsActive == 0) {
				if (g_replayviewmode != 0) {
					user_RestoreRepairPriorities();
					g_replayUiEventConsumed = 1;
				} else {
					user_PrepareFlightStatusSnapshot();
					if (replayio_copytosave("+savegame.rpy") != 0 && replayio_savereplaybuffer() != 0) {
						g_missionRuntimeState.flightExitRequested = 1;
						g_missionRuntimeState.flightExitReason = USER_EXIT_DAMAGE;
					} else
						msg_messageprintf(XW_MSG_FILE_ERROR);
				}
			}
			break;
		case 'b':
			if (g_missionRuntimeState.provingGroundsActive == 0) {
				if (g_replayviewmode != 0) {
					user_RestoreRepairPriorities();
					g_replayUiEventConsumed = 1;
				} else {
					user_PrepareFlightStatusSnapshot();
					if (replayio_copytosave("+savegame.rpy") != 0 && replayio_savereplaybuffer() != 0) {
						g_missionRuntimeState.flightExitRequested = 1;
						g_missionRuntimeState.flightExitReason = USER_EXIT_BRIEFING;
					} else
						msg_messageprintf(XW_MSG_FILE_ERROR);
				}
			}
			break;
		case USER_KEY_ESCAPE:
			if (g_replayviewmode != 0) {
#ifdef XW_MODERN
				if (!XwReplayFormat_RequireInputRecords(2)) {
					XwPort_Fail(XwReplayFormat_Error());
					return;
				}
#endif
				g_replayUiEventConsumed = 1;
				g_unlimitedWeaponsEnabled = g_ReplayInputPointer[0];
				g_flightInvulnerabilityEnabled = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
				g_flightDebrisPreference = g_ReplayInputPointer[0];
				g_flightBackdropsPreference = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
				g_flightStarshipDetail = g_ReplayInputPointer[0];
				g_flightHighDetailStarfield = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
				g_flightStarfighterDetail = g_ReplayInputPointer[0];
				g_flightDeathStarDetail = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
#ifdef XW_MODERN
				g_ReplayInputPointer += REPLAY_INPUT_RECORD_SIZE - REPLAY_LEGACY_INPUT_RECORD_SIZE;
#endif
				user_nextreplaycount();
				g_flightCraftCollisionsEnabled = g_ReplayInputPointer[0];
				g_flightMarkingsPreference = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
				g_flightMusicEnabled = g_ReplayInputPointer[0];
				g_flightMusicVolume = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
				g_flightSfxEnabled = g_ReplayInputPointer[0];
				g_flightSfxVolume = g_ReplayInputPointer[1];
				g_ReplayInputPointer += 2;
				g_flightEngineGlowPreference = g_ReplayInputPointer[1];
				g_flightDigitalSoundEnabled = g_ReplayInputPointer[0] & 1;
				g_flightVoiceEnabled = (g_ReplayInputPointer[0] & 2) != 0;
				g_ReplayInputPointer += 2;
#ifdef XW_MODERN
				g_ReplayInputPointer += REPLAY_INPUT_RECORD_SIZE - REPLAY_LEGACY_INPUT_RECORD_SIZE;
#endif
				user_nextreplaycount();
				user_ApplyPreferences();
			} else if (replayio_copytosave("+savegame.rpy") != 0 && replayio_savereplaybuffer() != 0) {
				g_missionRuntimeState.flightExitRequested = 1;
				g_missionRuntimeState.flightExitReason = USER_EXIT_OPTIONS;
			} else {
				msg_messageprintf(XW_MSG_FILE_ERROR);
			}
			break;
		case 'H':
			if (user_checkradio()) {
				CraftData* craft = g_curCraft;
				if (craft->aiCurrentPlanId != PAI_PLAN_RETURN_HOME &&
					craft->aiCurrentPlanId != PAI_PLAN_STARSHIP_RETURN_HOME) {
					craft->aiCurrentPlanId =
						g_objectTable[g_playerFlightState.currentTargetObjectIdx].genusId != XW_GENUS_STARSHIP
							? PAI_PLAN_RETURN_HOME
							: PAI_PLAN_STARSHIP_RETURN_HOME;
					pai_initplan(g_playerFlightState.currentTargetObjectIdx);
					craft = g_curCraft;
				}
				msg_radiomessage(craft, XW_MSG_RADIO_HEADING_HOME);
			}
			break;
		case 'W':
			if (user_checkradio()) {
				uint8_t plan = g_curCraft->aiCurrentPlanId;
				if (plan != PAI_PLAN_66 && plan != PAI_PLAN_INTO_HYPERSPACE &&
					plan != PAI_PLAN_OUT_OF_HYPERSPACE) {
					g_curCraft->aiSavedPlanId = plan;
					if (g_objectTable[g_playerFlightState.currentTargetObjectIdx].genusId ==
						XW_GENUS_STARSHIP)
						g_curCraft->aiCurrentPlanId = PAI_PLAN_67;
					else
						g_curCraft->aiCurrentPlanId = PAI_PLAN_66;
					pai_initplan(g_playerFlightState.currentTargetObjectIdx);
					msg_radiomessage(g_curCraft, XW_MSG_RADIO_WAITING_FOR_ORDERS);
				}
			}
			break;
		case 'G':
			if (user_checkradio() && g_curCraft->aiCurrentPlanId == PAI_PLAN_66) {
				g_curCraft->aiCurrentPlanId = g_curCraft->aiSavedPlanId;
				pai_initplan(g_playerFlightState.currentTargetObjectIdx);
				msg_radiomessage(g_curCraft, XW_MSG_RADIO_PROCEEDING_WITH_MISSION);
			}
			break;
		case 'E':
			if (user_checkradio()) {
				CraftData* craft = g_curCraft;
				if (craft->aiCurrentPlanId == PAI_PLAN_66) {
					craft->aiCurrentPlanId = craft->aiSavedPlanId;
					pai_initplan(g_playerFlightState.currentTargetObjectIdx);
					craft = g_curCraft;
				}
				craft->aiCandidateTargetOrSavedInterval = USER_RADIO_EVASIVE_REQUEST;
				msg_radiomessage(g_curCraft, XW_MSG_RADIO_MAKING_EVASIVE_MANEUVER);
			}
			break;
		case 'A':
			if (g_playerFlightState.currentTargetObjectIdx != XW_PLAYER_NO_TARGET)
				user_assigntarget(g_playerFlightState.currentTargetObjectIdx,
								  XW_MSG_RADIO_USING_DESIGNATED_TARGET);
			break;
		case 'C': {
			uint16_t target = user_findclosestattacker();
			if (target != XW_PLAYER_NO_TARGET)
				user_assigntarget(target, XW_MSG_RADIO_COMING_TO_YOUR_AID);
			break;
		}
		case 'I':
			user_assigntarget(USER_RADIO_IGNORE_TARGET, XW_MSG_RADIO_IGNORING_TARGET);
			break;
		case 'R':
			if (g_playerFlightState.currentTargetObjectIdx < XW_CRAFT_OBJECT_COUNT &&
				g_objectTable[g_playerFlightState.currentTargetObjectIdx].iff ==
					g_playerFlightState.object->iff) {
				g_curCraft =
					(CraftData*)g_objectTable[g_playerFlightState.currentTargetObjectIdx].instanceData;
				msg_reportmessage(g_curCraft,
								  (XwFlightMessageId)g_aiPlanStatusMessageIds[g_curCraft->aiCurrentPlanId]);
			}
			break;
		case 'e':
			user_setnewtarget(user_findclosestattacker());
			break;
		case '\r':
			if (g_playerFlightState.currentTargetObjectIdx != XW_PLAYER_NO_TARGET) {
				uint16_t engine;
				if (g_playerFlightState.currentTargetObjectIdx < XW_MISSION_OBJECT_REF_BASE) {
					if (g_playerFlightState.currentTargetObjectIdx < XW_CRAFT_OBJECT_COUNT) {
						uint16_t speed = g_objectTable[g_playerFlightState.currentTargetObjectIdx].speed;
						uint16_t power = USER_ENGINE_POWER_BASE - g_playerFlightState.craft->shieldRedirect -
										 g_playerFlightState.craft->laserRedirect;
						uint16_t maxSpeed;
						if (power < XW_CRAFT_THROTTLE_HALF)
							maxSpeed = g_playerFlightState.craft->maxSpeed +
									   math2_fraction(power << USER_ENGINE_POWER_SHIFT,
													  g_playerFlightState.craft->maxSpeed);
						else
							maxSpeed = g_playerFlightState.craft->maxSpeed -
									   math2_fraction(-(1 << USER_ENGINE_POWER_SHIFT) * power,
													  g_playerFlightState.craft->maxSpeed);
						if (speed < maxSpeed) {
							for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
								g_playerFlightState.craft->engineThrottle[engine] =
									math2_percentage(speed, maxSpeed);
							msg_messageprintf(XW_MSG_MATCHING_TARGET_SPEED);
						} else {
							for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
								g_playerFlightState.craft->engineThrottle[engine] = XW_CRAFT_THROTTLE_FULL;
							msg_messageprintf(XW_MSG_MATCHING_TARGET_SPEED_MAX_THROTTLE);
						}
					} else {
						for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
							g_playerFlightState.craft->engineThrottle[engine] = XW_CRAFT_THROTTLE_FULL;
					}
				} else {
					for (engine = 0; engine < g_playerFlightState.engineCount; ++engine)
						g_playerFlightState.craft->engineThrottle[engine] = 0;
				}
			}
			break;
		default:
			break;
	}
#ifdef XW_MODERN
	XwPlayerTiming_BeginControls();
#endif
	if (g_flightCamera.manualControlActive == 0) {
		int roll =
			(((uint16_t)math2_percentage(g_playerFlightState.craft->rollRateLimit, USER_ROLL_RATE_SCALE) >>
			  1) *
			 (int32_t)g_scaledInputYaw) >>
			USER_TURN_INPUT_SHIFT;
		int pitch =
			(((uint16_t)math2_percentage(g_playerFlightState.craft->pitchRateLimit, USER_PITCH_RATE_SCALE) >>
			  1) *
			 (int32_t)g_scaledInputPitch) >>
			USER_TURN_INPUT_SHIFT;
		uint16_t mode = 0;
		int16_t frameRoll;
		int16_t framePitch;
#ifdef XW_MODERN
		int16_t analogRoll = XwPlayerTiming_RollStep(g_xwInputRoll);
#endif
		if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) == 0) {
			pitch = 0;
			roll = 0;
		}
		if ((g_flightKeyMods & USER_TURN_MODIFIER_MASK) == USER_TURN_ROLL_MODE)
			mode = 1;
		if ((int16_t)g_playerFlightState.previousRollModifierMode == mode) {
#ifdef XW_MODERN
			if (XwFlightTiming_IsUnlocked()) {
				g_playerFlightState.smoothedTurnInput.roll = XwPlayerTiming_Slew(
					XW_PLAYER_SLEW_ROLL, g_playerFlightState.smoothedTurnInput.roll, (int16_t)roll);
				g_playerFlightState.smoothedTurnInput.pitch = XwPlayerTiming_Slew(
					XW_PLAYER_SLEW_PITCH, g_playerFlightState.smoothedTurnInput.pitch, (int16_t)pitch);
			} else
#endif
			{
				int16_t difference = roll - g_playerFlightState.smoothedTurnInput.roll;
				if (difference != 0) {
					int16_t step = difference;
					if (difference < 0)
						step = -difference;
					if (step < USER_SMOOTHING_THRESHOLD) {
						g_playerFlightState.smoothedTurnInput.roll += difference;
					} else {
						if (g_simStepScale > USER_SMOOTHING_SCALE) {
							step /= g_simStepScale;
							if (step == 0)
								step = 1;
							step *= USER_SMOOTHING_SCALE;
						}
						if (difference < 0)
							g_playerFlightState.smoothedTurnInput.roll -= step;
						else
							g_playerFlightState.smoothedTurnInput.roll += step;
					}
				}
				difference = pitch - g_playerFlightState.smoothedTurnInput.pitch;
				if (difference != 0) {
					int16_t step = difference;
					if (difference < 0)
						step = -difference;
					if (step < USER_SMOOTHING_THRESHOLD) {
						g_playerFlightState.smoothedTurnInput.pitch += difference;
					} else {
						if (g_simStepScale > USER_SMOOTHING_SCALE) {
							step /= g_simStepScale;
							if (step == 0)
								step = 1;
							step *= USER_SMOOTHING_SCALE;
						}
						if (difference < 0)
							g_playerFlightState.smoothedTurnInput.pitch -= step;
						else
							g_playerFlightState.smoothedTurnInput.pitch += step;
					}
				}
			}
		} else {
			g_playerFlightState.smoothedTurnInput.roll = 0;
			g_playerFlightState.smoothedTurnInput.pitch = 0;
		}
		g_playerFlightState.previousRollModifierMode = mode;
#ifdef XW_MODERN
		if (XwFlightTiming_IsUnlocked()) {
			frameRoll =
				(int16_t)XwPlayerTiming_Scale(XW_PLAYER_ROLL, g_playerFlightState.smoothedTurnInput.roll);
			framePitch =
				(int16_t)XwPlayerTiming_Scale(XW_PLAYER_PITCH, g_playerFlightState.smoothedTurnInput.pitch);
		} else
#endif
		{
			frameRoll = user_framerateadjust(g_playerFlightState.smoothedTurnInput.roll);
			framePitch = user_framerateadjust(g_playerFlightState.smoothedTurnInput.pitch);
		}
		if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ATTITUDE) == 0) {
			framePitch = 0;
			frameRoll = 0;
		}
		if (framePitch != 0 || frameRoll != 0) {
			user_calcdeltapitch(framePitch, -frameRoll, g_playerFlightState.objectIndex,
								g_playerFlightState.craft);
			g_playerFlightState.object->orientMatrixDirty = 1;
			g_playerFlightState.object->moveVectorDirty = 1;
			if (frameRoll != 0) {
				if (mode != 0)
					g_objectTable[g_playerFlightState.objectIndex].roll -= 2 * frameRoll;
				else
#ifdef XW_MODERN
					if (!analogRoll)
#endif
					g_objectTable[g_playerFlightState.objectIndex].roll -= frameRoll;
			}
		}
#ifdef XW_MODERN
		if (analogRoll) {
			g_playerFlightState.object->roll -= (int16_t)(2 * analogRoll);
			g_playerFlightState.object->orientMatrixDirty = 1;
			g_playerFlightState.object->moveVectorDirty = 1;
		}
	} else if (XwFlightTiming_IsUnlocked()) {
		XwPlayerTiming_ManualCamera();
#endif
	} else {
		int16_t yaw = user_framerateadjust(g_scaledInputYaw);
		int16_t modifiers;
		g_flightCamera.hudAimX += user_framerateadjust(g_scaledInputPitch);
		g_flightCamera.hudAimY += yaw;
		modifiers = g_flightKeyMods & USER_CAMERA_MODIFIER_MASK;
		if (modifiers == USER_CAMERA_ZOOM_IN || modifiers == USER_CAMERA_ZOOM_OUT) {
			int16_t step = g_flightCamera.movementStep + USER_CAMERA_STEP;
			g_flightCamera.movementStep += USER_CAMERA_STEP;
			if (g_flightCamera.movementStep > USER_CAMERA_MAX_STEP) {
				step = USER_CAMERA_MAX_STEP;
				g_flightCamera.movementStep = USER_CAMERA_MAX_STEP;
			}
			if (modifiers == USER_CAMERA_ZOOM_IN) {
				g_flightCamera.externalDistance -= user_framerateadjust(step);
				if (g_flightCamera.externalDistance < USER_CAMERA_MIN_DISTANCE)
					g_flightCamera.externalDistance = USER_CAMERA_MIN_DISTANCE;
			} else {
				g_flightCamera.externalDistance += user_framerateadjust(step);
				if (g_flightCamera.externalDistance > USER_CAMERA_MAX_DISTANCE)
					g_flightCamera.externalDistance = USER_CAMERA_MAX_DISTANCE;
			}
		} else
			g_flightCamera.movementStep = USER_CAMERA_STEP;
	}
}

// FUNCTION: XW 0x42CC70
int user_framerateadjust(int16_t step) {
	return math2_ABoverC32(step, g_elapsedTicks, XW_SIMULATION_TICKS_PER_SECOND);
}

// FUNCTION: XW 0x42CC90
void user_increasepower(uint16_t engineIndex, uint16_t step) {
	uint16_t previousThrottle = g_playerFlightState.craft->engineThrottle[engineIndex];
	g_playerFlightState.craft->engineThrottle[engineIndex] = previousThrottle + step;
	if (g_playerFlightState.craft->engineThrottle[engineIndex] < previousThrottle)
		g_playerFlightState.craft->engineThrottle[engineIndex] = XW_CRAFT_THROTTLE_FULL;
}

// FUNCTION: XW 0x42CCD0
void user_decreasepower(uint16_t engineIndex, uint16_t step) {
	uint16_t previousThrottle = g_playerFlightState.craft->engineThrottle[engineIndex];
	g_playerFlightState.craft->engineThrottle[engineIndex] = previousThrottle - step;
	if (g_playerFlightState.craft->engineThrottle[engineIndex] > previousThrottle)
		g_playerFlightState.craft->engineThrottle[engineIndex] = 0;
}

// FUNCTION: XW 0x42CD10
void user_adjustshields(uint16_t destinationBank, uint16_t sourceBank) {
	if (g_playerFlightState.craft->shieldEnergy[sourceBank] > 0) {
		int16_t destinationEnergy = g_playerFlightState.craft->shieldEnergy[destinationBank];
		int16_t destinationRoom =
			XW_SHIELD_MAX_CHARGE_MULTIPLIER *
				g_craftTypeDefs[g_playerFlightState.craftTypeIndex].nominalShieldEnergy[destinationBank] -
			destinationEnergy;
		if (destinationRoom > 0) {
			int16_t sourceEnergy = g_playerFlightState.craft->shieldEnergy[sourceBank];
			if (destinationRoom < sourceEnergy) {
				g_playerFlightState.craft->shieldEnergy[destinationBank] =
					destinationEnergy + destinationRoom;
				g_playerFlightState.craft->shieldEnergy[sourceBank] -= destinationRoom;
			} else {
				g_playerFlightState.craft->shieldEnergy[destinationBank] = destinationEnergy + sourceEnergy;
				g_playerFlightState.craft->shieldEnergy[sourceBank] = 0;
			}
		}
	}
}

// FUNCTION: XW 0x42CDC0
void user_resetview(void) {
#ifdef XW_MODERN
	XwPlayerTiming_Reset();
	XwFlightCamera_ResetChase();
#endif
	if (g_flightCamera.externalViewActive != 0) {
		uint16_t sampleIndex;
		panelrts_setnewpilotview(PANEL_VIEW_NO_COCKPIT);
		for (sampleIndex = 0; sampleIndex != sizeof(g_flightCamera.angleHistory.pitch) /
												 sizeof(g_flightCamera.angleHistory.pitch[0]);
			 ++sampleIndex) {
			g_flightCamera.angleHistory.roll[sampleIndex] = g_flightCamera.viewRoll;
			g_flightCamera.angleHistory.pitch[sampleIndex] = g_flightCamera.viewPitch;
			g_flightCamera.angleHistory.yaw[sampleIndex] = g_flightCamera.viewYaw;
		}
	} else {
		g_flightCamera.manualControlActive = 0;
		if (g_flightCamera.focusObjectRef == g_playerFlightState.objectIndex) {
			g_flightCamera.hudAimX = g_flightCamera.savedHudAimX;
			g_flightCamera.hudAimY = g_flightCamera.savedHudAimY;
			panelrts_setnewpilotview(g_flightCamera.savedHudState);
		} else {
			g_flightCamera.hudAimX = 0;
			g_flightCamera.hudAimY = 0;
			panelrts_setnewpilotview(PANEL_VIEW_NO_COCKPIT);
		}
	}
}

// FUNCTION: XW 0x42CE60
uint16_t user_picktarget(void) {
	uint16_t bestTargetRef = XW_PLAYER_NO_TARGET;
	uint16_t bestAngleScore = USER_PICK_MAX_ANGLE;
	uint16_t objectIndex;
	uint16_t staticRef;
	int staticIndex;
	for (objectIndex = 0; objectIndex < XW_OBJECT_COUNT; ++objectIndex) {
#ifdef XW_MODERN
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
			objectIndex != g_playerFlightState.objectIndex &&
			XwFlightTypes_Targetable(g_objectTable[objectIndex].objectType) &&
			user_targetincross(objectIndex, USER_PICK_MAX_ANGLE) != 0 &&
			(bestTargetRef == XW_PLAYER_NO_TARGET || g_targetAngleScore < bestAngleScore)) {
#else
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE &&
			objectIndex != g_playerFlightState.objectIndex &&
			(g_modelTypeTable[g_objectTable[objectIndex].objectType].objectFlags & MODEL_TYPE_TARGETABLE) !=
				0 &&
			user_targetincross(objectIndex, USER_PICK_MAX_ANGLE) != 0 &&
			(bestTargetRef == XW_PLAYER_NO_TARGET || g_targetAngleScore < bestAngleScore)) {
#endif
			bestTargetRef = objectIndex;
			bestAngleScore = g_targetAngleScore;
		}
	}
	for (staticIndex = 0, staticRef = XW_MISSION_OBJECT_REF_BASE;
		 (uint16_t)(staticRef - XW_MISSION_OBJECT_REF_BASE) < MISSION_OBJECT_COUNT;
		 ++staticIndex, ++staticRef) {
#ifdef XW_MODERN
		if (g_missionObjects[staticIndex].objectType != XW_OBJ_NONE &&
			XwFlightTypes_Targetable(g_missionObjects[staticIndex].objectType) &&
			user_targetincross(staticRef, USER_PICK_MAX_ANGLE) != 0 &&
			(bestTargetRef == XW_PLAYER_NO_TARGET || g_targetAngleScore < bestAngleScore)) {
#else
		if (g_missionObjects[staticIndex].objectType != XW_OBJ_NONE &&
			(g_modelTypeTable[g_missionObjects[staticIndex].objectType].objectFlags &
			 MODEL_TYPE_TARGETABLE) != 0 &&
			user_targetincross(staticRef, USER_PICK_MAX_ANGLE) != 0 &&
			(bestTargetRef == XW_PLAYER_NO_TARGET || g_targetAngleScore < bestAngleScore)) {
#endif
			bestTargetRef = staticRef;
			bestAngleScore = g_targetAngleScore;
		}
	}
	return bestTargetRef;
}

// FUNCTION: XW 0x42CF30
uint16_t user_picknexttarget(uint16_t start, int16_t step) {
	uint16_t candidate = start;
	uint16_t candidatesLeft = XW_OBJECT_COUNT + MISSION_OBJECT_COUNT - 1;
	do {
		uint16_t modelType;
		candidate += step;
		if (candidate >= INT16_MAX + 1U)
			candidate = XW_MISSION_OBJECT_REF_BASE + MISSION_OBJECT_COUNT - 1;
		else if (candidate == XW_MISSION_OBJECT_REF_BASE - 1)
			candidate = XW_OBJECT_COUNT - 1;
		else if (candidate == XW_OBJECT_COUNT)
			candidate = XW_MISSION_OBJECT_REF_BASE;
		else if (candidate == XW_MISSION_OBJECT_REF_BASE + MISSION_OBJECT_COUNT)
			candidate = 0;
		if (candidate != g_playerFlightState.objectIndex) {
			if (candidate < XW_MISSION_OBJECT_REF_BASE)
				modelType = g_objectTable[candidate].objectType;
			else
				modelType = g_missionObjects[candidate - XW_MISSION_OBJECT_REF_BASE].objectType;
#ifdef XW_MODERN
			if (modelType != XW_OBJ_NONE && XwFlightTypes_Targetable(modelType)) {
#else
			if (modelType != XW_OBJ_NONE &&
				(g_modelTypeTable[modelType].objectFlags & MODEL_TYPE_TARGETABLE) != 0) {
#endif
				if (candidate >= XW_CRAFT_OBJECT_COUNT)
					return candidate;
				if (g_objectTable[candidate].genusId != XW_GENUS_EXPLOSION_EFFECT) {
					if (g_objectTable[candidate].familyId != XW_OBJECT_FAMILY_CRAFT)
						return candidate;
					g_curCraft = g_objectTable[candidate].instanceData;
					if (g_curCraft->objectKind != XW_CRAFT_OBJECT_KIND_3 &&
						g_curCraft->objectKind != XW_CRAFT_OBJECT_KIND_4)
						return candidate;
				}
			}
		}
	} while (candidatesLeft-- != 0);
	return XW_PLAYER_NO_TARGET;
}

// FUNCTION: XW 0x42D030
int16_t user_targetincross(uint16_t candidateObjRef, uint16_t maxAngleScore) {
	ObjectRecord* playerObject;
	int16_t deltaX, deltaY, deltaZ;
	int32_t forwardDepth;
	int32_t termX, termY, termZ;
	int32_t magnitude;
	uint64_t numerator;
	int32_t projectedSide, projectedUp;
	if (candidateObjRef < XW_MISSION_OBJECT_REF_BASE) {
		pai_roughdistancebetween(candidateObjRef, g_playerFlightState.objectIndex);
		playerObject = g_playerFlightState.object;
		if ((int32_t)g_targetRangeScore < USER_CROSS_FAR_RANGE) {
#ifdef XW_MODERN
			deltaX = (int16_t)((int32_t)((uint32_t)g_objectTable[candidateObjRef].worldX -
										 (uint32_t)playerObject->worldX) >>
							   USER_CROSS_NEAR_SHIFT);
#else
			deltaX = (g_objectTable[candidateObjRef].worldX - playerObject->worldX) >> USER_CROSS_NEAR_SHIFT;
#endif
#ifdef XW_MODERN
			deltaY = (int16_t)((int32_t)((uint32_t)g_objectTable[candidateObjRef].worldY -
										 (uint32_t)playerObject->worldY) >>
							   USER_CROSS_NEAR_SHIFT);
#else
			deltaY = (g_objectTable[candidateObjRef].worldY - playerObject->worldY) >> USER_CROSS_NEAR_SHIFT;
#endif
#ifdef XW_MODERN
			deltaZ = (int16_t)((int32_t)((uint32_t)g_objectTable[candidateObjRef].worldZ -
										 (uint32_t)playerObject->worldZ) >>
							   USER_CROSS_NEAR_SHIFT);
#else
			deltaZ = (g_objectTable[candidateObjRef].worldZ - playerObject->worldZ) >> USER_CROSS_NEAR_SHIFT;
#endif
		} else {
#ifdef XW_MODERN
			deltaX = (int16_t)((int32_t)((uint32_t)g_objectTable[candidateObjRef].worldX -
										 (uint32_t)playerObject->worldX) >>
							   USER_CROSS_FAR_SHIFT);
#else
			deltaX = (g_objectTable[candidateObjRef].worldX - playerObject->worldX) >> USER_CROSS_FAR_SHIFT;
#endif
#ifdef XW_MODERN
			deltaY = (int16_t)((int32_t)((uint32_t)g_objectTable[candidateObjRef].worldY -
										 (uint32_t)playerObject->worldY) >>
							   USER_CROSS_FAR_SHIFT);
#else
			deltaY = (g_objectTable[candidateObjRef].worldY - playerObject->worldY) >> USER_CROSS_FAR_SHIFT;
#endif
#ifdef XW_MODERN
			deltaZ = (int16_t)((int32_t)((uint32_t)g_objectTable[candidateObjRef].worldZ -
										 (uint32_t)playerObject->worldZ) >>
							   USER_CROSS_FAR_SHIFT);
#else
			deltaZ = (g_objectTable[candidateObjRef].worldZ - playerObject->worldZ) >> USER_CROSS_FAR_SHIFT;
#endif
		}
	} else {
		playerObject = g_playerFlightState.object;
		deltaX = g_missionObjects[candidateObjRef - XW_MISSION_OBJECT_REF_BASE].worldX -
				 (playerObject->worldX >> USER_CROSS_FAR_SHIFT);
		deltaY = g_missionObjects[candidateObjRef - XW_MISSION_OBJECT_REF_BASE].worldY -
				 (playerObject->worldY >> USER_CROSS_FAR_SHIFT);
		deltaZ = g_missionObjects[candidateObjRef - XW_MISSION_OBJECT_REF_BASE].worldZ -
				 (playerObject->worldZ >> USER_CROSS_FAR_SHIFT);
	}
	if (playerObject->orientMatrixDirty != 0) {
		fview_calcrotatemove(playerObject->pitch, playerObject->yaw, playerObject);
		fview_calcrotateorient(g_playerFlightState.object->roll, 0, g_playerFlightState.object);
		playerObject = g_playerFlightState.object;
	}
	termX = (int32_t)(((int64_t)playerObject->cachedForwardX * deltaX) >> USER_CROSS_BASIS_SHIFT);
	termY =
		(int32_t)(((int64_t)g_playerFlightState.object->cachedForwardY * deltaY) >> USER_CROSS_BASIS_SHIFT);
	termZ =
		(int32_t)(((int64_t)g_playerFlightState.object->cachedForwardZ * deltaZ) >> USER_CROSS_BASIS_SHIFT);
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		termX = (int16_t)termX;
		termY = (int16_t)termY;
		termZ = (int16_t)termZ;
	}
#endif
	forwardDepth = termZ + termY + termX;
	if (forwardDepth < 0)
		return 0;
	if (forwardDepth > USER_CROSS_MAX_DEPTH)
		return 0;
	termX = (int32_t)(((int64_t)g_playerFlightState.object->cachedSideX * deltaX) >> USER_CROSS_BASIS_SHIFT);
	termY = (int32_t)(((int64_t)g_playerFlightState.object->cachedSideY * deltaY) >> USER_CROSS_BASIS_SHIFT);
	termZ = (int32_t)(((int64_t)g_playerFlightState.object->cachedSideZ * deltaZ) >> USER_CROSS_BASIS_SHIFT);
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		termX = (int16_t)termX;
		termY = (int16_t)termY;
		termZ = (int16_t)termZ;
	}
#endif
	magnitude = termZ + termY + termX;
	if (magnitude < 0)
		magnitude = -magnitude;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		int16_t side = (int16_t)(Dos94_TRANSFM2_getscreenx(magnitude, forwardDepth) - (g_flightVpWidth >> 1));
		int32_t up = (int16_t)(((int32_t)(int16_t)playerObject->cachedUpX * (int16_t)deltaX) >> 15);
		int16_t projected;
		up += (int16_t)(((int32_t)(int16_t)playerObject->cachedUpY * (int16_t)deltaY) >> 15);
		up += (int16_t)(((int32_t)(int16_t)playerObject->cachedUpZ * (int16_t)deltaZ) >> 15);
		if (up < 0)
			up = -up;
		projected =
			(int16_t)(Dos94_TRANSFM2_getscreeny(up, forwardDepth) - (g_flightVpHeight >> 1) - g_projOffsetY);
		g_targetAngleScore = (uint16_t)(side + projected);
		return maxAngleScore >= g_targetAngleScore;
	}
#endif
	numerator = ((uint64_t)(uint32_t)magnitude << USER_CROSS_PROJECTION_SHIFT) + USER_CROSS_PROJECTION_BIAS;
	if ((uint32_t)(numerator >> USER_CROSS_HIGH_WORD_SHIFT) < (uint32_t)forwardDepth)
		projectedSide = (int32_t)(numerator / (uint32_t)forwardDepth);
	else
		projectedSide = USER_CROSS_PROJECTION_OVERFLOW;
	if (projectedSide > USER_CROSS_SIDE_LIMIT)
		return 0;
	termX = (int32_t)(((int64_t)g_playerFlightState.object->cachedUpX * deltaX) >> USER_CROSS_BASIS_SHIFT);
	termY = (int32_t)(((int64_t)g_playerFlightState.object->cachedUpY * deltaY) >> USER_CROSS_BASIS_SHIFT);
	termZ = (int32_t)(((int64_t)g_playerFlightState.object->cachedUpZ * deltaZ) >> USER_CROSS_BASIS_SHIFT);
	magnitude = termZ + termY + termX;
	if (magnitude < 0)
		magnitude = -magnitude;
	numerator = ((uint64_t)(uint32_t)magnitude << USER_CROSS_PROJECTION_SHIFT) + USER_CROSS_PROJECTION_BIAS;
	if ((uint32_t)(numerator >> USER_CROSS_HIGH_WORD_SHIFT) < (uint32_t)forwardDepth)
		projectedUp = (int32_t)(numerator / (uint32_t)forwardDepth);
	else
		projectedUp = USER_CROSS_PROJECTION_OVERFLOW;
	if (projectedUp > USER_CROSS_UP_LIMIT)
		return 0;
	g_targetAngleScore =
		(int16_t)projectedSide + ((USER_CROSS_UP_WEIGHT * projectedUp) >> USER_CROSS_WEIGHT_SHIFT);
	return maxAngleScore >= g_targetAngleScore;
}

// FUNCTION: XW 0x42D380
void user_setnewtarget(uint16_t newTargetObjIdx) {
	if ((g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_TARGETING) != 0) {
		if (newTargetObjIdx != XW_PLAYER_NO_TARGET &&
			newTargetObjIdx != g_playerFlightState.currentTargetObjectIdx) {
			fsfx_triggersfx(FSFX_TARGET_SELECTED_SLOT, FSFX_UNPOSITIONED_OBJECT);
			g_playerFlightState.currentTargetObjectIdx = newTargetObjIdx;
			g_playerFlightState.missileLockState = 0;
			g_playerFlightState.craft->warheadLockTicks = 0;
			if ((g_playerFlightState.craft->activeHudFeatureMask & USER_HUD_TARGETING_FEATURE) != 0 &&
				g_flightCamera.hudStateLive == USER_TARGET_ANNOUNCEMENT_HUD_STATE &&
				newTargetObjIdx < XW_CRAFT_OBJECT_COUNT) {
				const CraftData* targetCraft = g_objectTable[newTargetObjIdx].instanceData;
				msg_addmessageptr(0, g_craftTypeDefs[targetCraft->craftTypeIndex].name);
				if (g_missionFlightGroups[targetCraft->flightGroupIndex].numberOfCraft > 1) {
					g_msgArgTable[1] = targetCraft->craftIndexInFlightGroup + 1;
					if (user_CanRevealCraftIdentity(newTargetObjIdx, targetCraft) != 0)
						msg_addmessageptr(2, g_missionFlightGroups[targetCraft->flightGroupIndex].name);
					else
						g_msgArgTable[2] = XW_MSG_UNIDENTIFIED_CRAFT_NAME;
					g_msgArgTable[3] = XW_MSG_CRAFT_TARGETED;
					msg_messageprintf(XW_MSG_TARGET_EVENT_NUMBERED);
				} else {
					if (user_CanRevealCraftIdentity(newTargetObjIdx, targetCraft) != 0)
						msg_addmessageptr(1, g_missionFlightGroups[targetCraft->flightGroupIndex].name);
					else
						g_msgArgTable[1] = XW_MSG_UNIDENTIFIED_CRAFT_NAME;
					g_msgArgTable[2] = XW_MSG_CRAFT_TARGETED;
					msg_messageprintf(XW_MSG_TARGET_EVENT_UNNUMBERED);
				}
			}
		}
	} else {
		g_msgArgTable[0] = XW_MSG_SUBSYSTEM_TARGETING_COMPUTER;
		g_msgArgTable[1] = XW_MSG_SUBSYSTEM_DAMAGED_AND_INOPERATIVE;
		msg_messageprintf(XW_MSG_SUBSYSTEM_STATUS);
	}
}

// FUNCTION: XW 0x42D540
int16_t user_CanRevealCraftIdentity(uint16_t objectIndex, const struct CraftData* craft) {
	if (craft->isInspected != 0) {
		return 1;
	}
	return g_objectTable[objectIndex].genusId == XW_GENUS_STARFIGHTER;
}

// FUNCTION: XW 0x42D580
void user_calcdeltapitch(int16_t pitchDelta, int16_t yawDelta, uint16_t objectIndex,
						 struct CraftData* craft) {
#ifdef XW_MODERN
	ObjectRecord* object = &g_objectTable[objectIndex];
	XwOrientationAngles current = { object->yaw, object->pitch, object->roll };
	XwOrientationAngles updated = XwOrientation_ApplyPitchYaw(
		current, pitchDelta,
		(g_flightKeyMods & USER_TURN_MODIFIER_MASK) == USER_TURN_ROLL_MODE ? 0 : yawDelta);
	/* Publish a coherent orientation before the per-object simulation step,
	 * including the craft's commanded pitch, as in OpenXvT. */
	craft->pitch = object->pitch = (int16_t)updated.pitch;
	object->yaw = (int16_t)updated.yaw;
	object->roll = (int16_t)updated.roll;
#else
	int objectSlot = objectIndex;
	int16_t newPitch;
	int newYaw;
	int16_t sinYaw;
	int16_t cosYaw;
	int16_t sinPitch;
	int16_t cosPitch;
	int16_t negativeSinYaw;
	int16_t negativeSinPitch;
	int cosPitchSinYaw;
	int sinPitchSinYaw;
	int cosPitchCosYaw;
	int sinPitchCosYaw;
	int rotatedX;
	int rotatedY;
	int rotatedZ;
	int dot;
	if (g_objectTable[objectSlot].orientMatrixDirty != 0) {
		fview_calcrotatemove(g_objectTable[objectSlot].pitch, g_objectTable[objectSlot].yaw,
							 &g_objectTable[objectSlot]);
		fview_calcrotateorient(g_objectTable[objectSlot].roll, 0, &g_objectTable[objectSlot]);
	}
	g_curMatR2_X = -g_objectTable[objectSlot].cachedForwardX;
	g_curMatR2_Y = -g_objectTable[objectSlot].cachedForwardY;
	g_curMatR2_Z = -g_objectTable[objectSlot].cachedForwardZ;
	g_curMatR1_X = g_objectTable[objectSlot].cachedUpX;
	g_curMatR1_Y = g_objectTable[objectSlot].cachedUpY;
	g_curMatR1_Z = g_objectTable[objectSlot].cachedUpZ;
	g_curMatR0_X = g_objectTable[objectSlot].cachedSideX;
	g_curMatR0_Y = g_objectTable[objectSlot].cachedSideY;
	g_curMatR0_Z = g_objectTable[objectSlot].cachedSideZ;
	fview_transformaxes(g_curMatR0_X, g_curMatR0_Y, g_curMatR0_Z, pitchDelta);
	if ((g_flightKeyMods & USER_TURN_MODIFIER_MASK) != USER_TURN_ROLL_MODE)
		fview_transformaxes(g_curMatR1_X, g_curMatR1_Y, g_curMatR1_Z, yawDelta);
	newPitch = trig2_w_arccos(-g_curMatR2_Z);
	craft->pitch = newPitch;
	newYaw = -trig2_arctan(g_curMatR2_X, -g_curMatR2_Y);
	cosYaw = (int16_t)trig2_getsignedcos((int16_t)newYaw);
	sinYaw = (int16_t)trig2_getsignedsin((int16_t)newYaw);
	cosPitch = (int16_t)trig2_getsignedcos(newPitch);
	sinPitch = (int16_t)trig2_getsignedsin(newPitch);
	cosPitchSinYaw = (int)(((int64_t)cosPitch * sinYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	sinPitchSinYaw = (int)(((int64_t)sinPitch * sinYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	negativeSinYaw = (int16_t)-sinYaw;
	cosPitchCosYaw = (int)(((int64_t)cosPitch * cosYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	sinPitchCosYaw = (int)(((int64_t)sinPitch * cosYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	negativeSinPitch = (int16_t)-sinPitch;
	dot = cosYaw * g_curMatR0_X + negativeSinYaw * g_curMatR0_Y;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedX = dot >> FVIEW_MATRIX_FRACTION_BITS;
	dot = (int16_t)cosPitchSinYaw * g_curMatR0_X + (int16_t)cosPitchCosYaw * g_curMatR0_Y +
		  negativeSinPitch * g_curMatR0_Z;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedY = dot >> FVIEW_MATRIX_FRACTION_BITS;
	dot = (int16_t)sinPitchSinYaw * g_curMatR0_X + (int16_t)sinPitchCosYaw * g_curMatR0_Y +
		  cosPitch * g_curMatR0_Z;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedZ = dot >> FVIEW_MATRIX_FRACTION_BITS;
	g_curMatR0_X = (int16_t)rotatedX;
	g_curMatR0_Y = (int16_t)rotatedY;
	g_curMatR0_Z = (int16_t)rotatedZ;
	dot = cosYaw * g_curMatR1_X + negativeSinYaw * g_curMatR1_Y;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedX = dot >> FVIEW_MATRIX_FRACTION_BITS;
	dot = (int16_t)cosPitchSinYaw * g_curMatR1_X + (int16_t)cosPitchCosYaw * g_curMatR1_Y +
		  negativeSinPitch * g_curMatR1_Z;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedY = dot >> FVIEW_MATRIX_FRACTION_BITS;
	dot = (int16_t)sinPitchSinYaw * g_curMatR1_X + (int16_t)sinPitchCosYaw * g_curMatR1_Y +
		  cosPitch * g_curMatR1_Z;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedZ = dot >> FVIEW_MATRIX_FRACTION_BITS;
	g_curMatR1_X = (int16_t)rotatedX;
	g_curMatR1_Y = (int16_t)rotatedY;
	g_curMatR1_Z = (int16_t)rotatedZ;
	dot = cosYaw * g_curMatR2_X + negativeSinYaw * g_curMatR2_Y;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedX = dot >> FVIEW_MATRIX_FRACTION_BITS;
	dot = (int16_t)cosPitchSinYaw * g_curMatR2_X + (int16_t)cosPitchCosYaw * g_curMatR2_Y +
		  negativeSinPitch * g_curMatR2_Z;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedY = dot >> FVIEW_MATRIX_FRACTION_BITS;
	dot = (int16_t)sinPitchSinYaw * g_curMatR2_X + (int16_t)sinPitchCosYaw * g_curMatR2_Y +
		  cosPitch * g_curMatR2_Z;
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	rotatedZ = dot >> FVIEW_MATRIX_FRACTION_BITS;
	g_curMatR2_X = (int16_t)rotatedX;
	g_curMatR2_Y = (int16_t)rotatedY;
	g_curMatR2_Z = (int16_t)rotatedZ;
	{
		int newRoll = -trig2_arctan(g_curMatR0_Y, g_curMatR0_X);
		g_objectTable[objectSlot].yaw = (int16_t)newYaw;
		g_objectTable[objectSlot].roll = (int16_t)newRoll;
	}
#endif
}

// FUNCTION: XW 0x42DB00
void user_setdetaillevel(uint16_t preset) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_user_setdetaillevel(preset);
		return;
	}
#endif
	g_craftExplosionSpawnThreshold = g_craftExplosionSpawnThresholdByGraphicsDetailPreset[preset];
	g_starshipDetail = g_starshipDetailByPreset[preset];
	g_starDensity = g_starDensityByGraphicsDetailPreset[preset];
	if (g_deathStarSurfaceModeActive == 0) {
		g_backdropsEnabled = (uint8_t)g_backdropsEnabledByGraphicsDetailPreset[preset];
		g_debrisEnabled = (uint8_t)g_debrisEnabledByGraphicsDetailPreset[preset];
	} else {
		g_backdropsEnabled = 0;
		g_debrisEnabled = 0;
	}
	g_shipDetailValue = g_shipDetailValueByPreset[preset];
	g_shipDetailPolyCount = g_shipDetailPolyCountByPreset[preset];
	g_drawMarkingsFlag = g_drawMarkingsByPreset[preset];
	g_deathStarDetailLevel = g_deathStarDetailLevelByPreset[preset];
	g_surfaceObjectDetailLimit = g_surfaceObjectDetailLimitByPreset[preset];
	g_trenchObjectDetailLimit = g_trenchObjectDetailLimitByPreset[preset];
	g_hyperspaceEffectObjectCount = g_hyperspaceEffectObjectCountByPreset[preset];
	g_gouraudEnableMask = g_gouraudEnableMaskByPreset[preset];
	g_transformLightDirectionToObjectSpace = 1;
}

// FUNCTION: XW 0x42DBE0
void user_ApplyPreferences(void) {
	unsigned int brightness;
	g_starshipDetail = g_starshipDetailByDetail[g_flightStarshipDetail];
#ifdef XW_MODERN
	g_craftExplosionSpawnThreshold =
		(g_flightStarshipDetail + USER_EXPLOSION_DETAIL_BIAS) * (1 << USER_EXPLOSION_DETAIL_SHIFT);
#else
	g_craftExplosionSpawnThreshold = (g_flightStarshipDetail + USER_EXPLOSION_DETAIL_BIAS)
									 << USER_EXPLOSION_DETAIL_SHIFT;
#endif
	if (g_flightHighDetailStarfield != 0) {
		g_starDensity = USER_HIGH_DETAIL_STAR_DENSITY;
		g_hyperspaceEffectObjectCount = USER_HIGH_DETAIL_HYPERSPACE_OBJECTS;
	} else {
		g_starDensity = USER_LOW_DETAIL_STAR_DENSITY;
		g_hyperspaceEffectObjectCount = USER_LOW_DETAIL_HYPERSPACE_OBJECTS;
	}
	if (g_deathStarSurfaceModeActive == 0) {
		g_backdropsEnabled = g_flightBackdropsPreference;
		g_debrisEnabled = g_flightDebrisPreference;
	} else {
		g_backdropsEnabled = 0;
		g_debrisEnabled = 0;
	}
	g_shipDetailValue = g_shipDetailValueByDetail[g_flightStarfighterDetail];
	g_shipDetailPolyCount = g_flightStarfighterDetail + USER_OBJECT_DETAIL_BIAS;
	g_drawMarkingsFlag = g_flightMarkingsPreference;
	g_deathStarDetailLevel = g_deathStarDetailLevelByDetail[g_flightDeathStarDetail];
	g_surfaceObjectDetailLimit = g_flightDeathStarDetail + USER_OBJECT_DETAIL_BIAS;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		g_trenchObjectDetailLimit = g_surfaceObjectDetailLimit;
	else
#endif
		g_trenchObjectDetailLimit = g_flightDeathStarDetail / USER_TRENCH_DETAIL_DIVISOR +
									g_flightDeathStarDetail + USER_TRENCH_DETAIL_BIAS;
	g_engineGlowEnabled = g_flightEngineGlowPreference;
	if (g_flightAudioMode != 0) {
		int volume;
		if (g_flightSfxVolume != 0)
			volume = XW_SHELL_VOLUME_STEP * (int8_t)g_flightSfxVolume - 1;
		else
			volume = 0;
		lolevel_ImSetGroupVol(XW_SOUND_GROUP_ALL, volume);
	}
	g_flightSfxGroupUnmuted = 1;
	if (g_flightAudioMode != 0) {
		int volume;
		if (g_flightMusicVolume != 0)
			volume = XW_SHELL_VOLUME_STEP * (int8_t)g_flightMusicVolume - 1;
		else
			volume = 0;
		lolevel_ImSetGroupVol(XW_SOUND_GROUP_MUSIC, volume);
	}
	g_flightMusicPlaybackEnabled = 1;
	g_transformLightDirectionToObjectSpace = 1;
	g_missionCheatOptionsUsed |= (int8_t)((g_flightCraftCollisionsEnabled ^ 1) |
										  g_flightInvulnerabilityEnabled | g_unlimitedWeaponsEnabled);
	brightness = g_flightBrightnessSetting;
	if (brightness > USER_BRIGHTNESS_MAX)
		brightness = USER_BRIGHTNESS_MAX;
	g_flightBrightnessScaleQ8 = (brightness + USER_BRIGHTNESS_BIAS) * USER_BRIGHTNESS_STEP;
}

// FUNCTION: XW 0x42DD90
int16_t user_checkradio(void) {
	if (g_playerFlightState.currentTargetObjectIdx == XW_PLAYER_NO_TARGET ||
		g_playerFlightState.currentTargetObjectIdx >= XW_RADIO_CRAFT_SLOT_LIMIT ||
		g_objectTable[g_playerFlightState.currentTargetObjectIdx].iff !=
			g_objectTable[g_playerFlightState.objectIndex].iff ||
		g_objectTable[g_playerFlightState.currentTargetObjectIdx].genusId == XW_GENUS_STARSHIP ||
		g_objectTable[g_playerFlightState.currentTargetObjectIdx].genusId == XW_GENUS_FREIGHTER) {
		return 0;
	}
	g_curCraft = g_objectTable[g_playerFlightState.currentTargetObjectIdx].instanceData;
	return g_curCraft->workingSubsystems != 0;
}

// FUNCTION: XW 0x42DE00
void user_PrepareFlightStatusSnapshot(void) {
	uint16_t objectIndex;
	for (objectIndex = 0; objectIndex < XW_CRAFT_OBJECT_COUNT; ++objectIndex) {
		if (g_objectTable[objectIndex].objectType != XW_OBJ_NONE) {
			pai_distancebetween(objectIndex, g_playerFlightState.objectIndex);
			g_trig2PolarDistance = (int)((unsigned int)g_trig2PolarDistance * USER_SNAPSHOT_DISTANCE_SCALE);
			g_missionRuntimeState.snapshotCraftDisplayDistances[objectIndex] =
				(uint16_t)(g_trig2PolarDistance >> USER_SNAPSHOT_DISTANCE_SHIFT) /
				USER_SNAPSHOT_DISTANCE_DIVISOR;
			g_missionRuntimeState.snapshotCraftStatusCodes[objectIndex] = panel_getcraftstatus(objectIndex);
		}
	}
	memcpy(g_playerSubsystemRepairPriority, g_playerFlightState.savedSubsystemRepairPriority,
		   sizeof(g_playerSubsystemRepairPriority));
}

// FUNCTION: XW 0x42DEA0
void user_RestoreRepairPriorities(void) {
	uint8_t* input;
	unsigned int pairsRemaining;
#ifdef XW_MODERN
	if (!XwReplayFormat_RequireInputRecords(1)) {
		XwPort_Fail(XwReplayFormat_Error());
		return;
	}
#endif
	input = g_ReplayInputPointer;
	for (pairsRemaining = XW_PLAYER_SUBSYSTEM_COUNT / sizeof(uint16_t); pairsRemaining != 0;
		 --pairsRemaining) {
		unsigned int index = XW_PLAYER_SUBSYSTEM_COUNT - pairsRemaining * sizeof(uint16_t);
		uint16_t priorities;
		priorities = input[index] | (uint16_t)input[index + 1] << CHAR_BIT;
		g_playerFlightState.savedSubsystemRepairPriority[index] = priorities >> CHAR_BIT;
		g_playerFlightState.savedSubsystemRepairPriority[index + 1] = priorities;
		g_ReplayInputPointer = &input[index + sizeof(priorities)];
	}
#ifdef XW_MODERN
	g_ReplayInputPointer = input + REPLAY_INPUT_RECORD_SIZE;
#endif
	user_nextreplaycount();
}

// FUNCTION: XW 0x42DEE0
void user_assigntarget(uint16_t targetObjIdx, XwFlightMessageId commandTextId) {
	uint16_t wingmanObjectIndex;
	if (targetObjIdx < XW_CRAFT_OBJECT_COUNT) {
		if (g_objectTable[targetObjIdx].iff == g_playerFlightState.object->iff)
			return;
#ifdef XW_MODERN
	} else if (!XwFlightTypes_Dos() && targetObjIdx != PAI_TARGET_NONE) {
#else
	} else if (targetObjIdx != PAI_TARGET_NONE) {
#endif
		return;
	}
	for (wingmanObjectIndex = 0; wingmanObjectIndex < XW_CRAFT_OBJECT_COUNT; ++wingmanObjectIndex) {
		if (wingmanObjectIndex != g_playerFlightState.objectIndex &&
			g_objectTable[wingmanObjectIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[wingmanObjectIndex].iff == g_playerFlightState.object->iff) {
			CraftData* wingmanCraft = g_objectTable[wingmanObjectIndex].instanceData;
			if (wingmanCraft->flightGroupIndex == g_playerFlightState.craft->flightGroupIndex) {
				if (wingmanCraft->aiCurrentPlanId == PAI_PLAN_66) {
					wingmanCraft->aiCurrentPlanId = wingmanCraft->aiSavedPlanId;
					pai_initplan(wingmanObjectIndex);
				}
				wingmanCraft->aiCandidateTargetOrSavedInterval = targetObjIdx;
				msg_radiomessage(wingmanCraft, commandTextId);
			}
		}
	}
}

// FUNCTION: XW 0x42DF90
int16_t user_findclosestattacker(void) {
	unsigned int nearestRangeScore = UINT_MAX;
	uint16_t nearestAttackerIndex = XW_PLAYER_NO_TARGET;
	uint16_t candidateIndex;
	for (candidateIndex = 0; candidateIndex < XW_CRAFT_OBJECT_COUNT; ++candidateIndex) {
		if (candidateIndex != g_playerFlightState.objectIndex &&
			g_objectTable[candidateIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[candidateIndex].iff != g_playerFlightState.object->iff) {
			CraftData* candidateCraft = g_objectTable[candidateIndex].instanceData;
			if (candidateCraft->aiTargetRef == g_playerFlightState.objectIndex &&
#ifdef XW_MODERN
				(XwFlightTypes_Dos() || candidateCraft->workingSubsystems != 0)) {
#else
				candidateCraft->workingSubsystems != 0) {
#endif
				pai_roughdistancebetween(candidateIndex, g_playerFlightState.objectIndex);
				if (g_targetRangeScore < nearestRangeScore) {
					nearestRangeScore = g_targetRangeScore;
					nearestAttackerIndex = candidateIndex;
				}
			}
		}
	}
	return nearestAttackerIndex;
}

// FUNCTION: XW 0x42E010
int16_t user_isrescued(uint16_t subjectObjectIndex) {
	unsigned int nearestFriendlyDistance;
	unsigned int nearestHostileDistance;
	uint16_t candidateObjectIndex;
	if ((g_missionHeader.missionRuleFlags & MISSION_RULE_FORCE_RESCUE) != 0) {
		return 1;
	}
	nearestFriendlyDistance = USER_RESCUE_MAX_DISTANCE;
	nearestHostileDistance = USER_RESCUE_MAX_DISTANCE;
	for (candidateObjectIndex = 0; candidateObjectIndex < XW_CRAFT_OBJECT_COUNT; ++candidateObjectIndex) {
		if (candidateObjectIndex != subjectObjectIndex &&
			g_objectTable[candidateObjectIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[candidateObjectIndex].familyId == XW_OBJECT_FAMILY_CRAFT &&
			g_objectTable[candidateObjectIndex].genusId != XW_GENUS_STARFIGHTER) {
			if (g_objectTable[candidateObjectIndex].iff == USER_RESCUE_FRIENDLY_IFF) {
				pai_distancebetween(subjectObjectIndex, candidateObjectIndex);
				if ((unsigned int)g_trig2PolarDistance < nearestFriendlyDistance) {
					nearestFriendlyDistance = g_trig2PolarDistance;
				}
			} else if (g_objectTable[candidateObjectIndex].iff == USER_RESCUE_HOSTILE_IFF) {
				pai_distancebetween(subjectObjectIndex, candidateObjectIndex);
				if ((unsigned int)g_trig2PolarDistance < nearestHostileDistance) {
					nearestHostileDistance = g_trig2PolarDistance;
				}
			}
		}
	}
	return (nearestFriendlyDistance >> USER_RESCUE_FRIENDLY_DISTANCE_SHIFT) < nearestHostileDistance;
}

// FUNCTION: XW 0x42E0B0
void user_checkreplaycamera(void) {
	if (g_ReplayRecording != 0) {
		if (replayio_spoolreplayinput() == 0)
			g_ReplayFrameCount -= g_ReplayBufferIndex;
		g_ReplayBufferIndex = 0;
		g_ReplayRecording = 0;
		msg_messageprintf(XW_MSG_REPLAY_CAMERA_OFF);
		calcframerate = 0;
	}
}

// FUNCTION: XW 0x42E100
void user_ejectcamera(void) {
	g_hyperspaceflag = 0;
	user_checkreplaycamera();
	g_flightCamera.focusObjectRef = XW_OBJECT_SLOT_UNAVAILABLE;
	g_flightCamera.worldPosition.x = g_playerFlightState.object->prevWorldX;
	g_flightCamera.worldPosition.y = g_playerFlightState.object->prevWorldY;
	g_flightCamera.worldPosition.z = g_playerFlightState.object->prevWorldZ;
	g_flightCamera.externalViewActive = 1;
	g_flightCamera.manualControlActive = 1;
	g_flightCamera.hudAimY = 0;
	g_flightCamera.hudAimX = 0;
	msg_clearmessagequeue();
	if (g_playerFlightState.craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_2) {
		panelrts_setnewpilotview(PANEL_VIEW_NO_COCKPIT);
		msg_messageprintf(XW_MSG_PLAYER_EJECTED_SAFELY);
	} else {
		panelrts_setnewpilotview(0);
		msg_messageprintf(XW_MSG_PLAYER_DIED);
	}
	g_playerFlightState.hudSuppressed = 1;
}
