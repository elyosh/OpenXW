#include "xw/flight/shell_flight.h"

#include "xw/flight/fediskio.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/xw.h"
#include "xw/frontend/inflight_ui.h"
#include "xw/frontend/register.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/frontend/xmain.h"
#include "xw/render/rtsvga2.h"
#include "xw/util/shared.h"
#include "xw_runtime/compat/legacy_memory.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/shell_flight_task.h"
#endif

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/pal.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <landru/view.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// GLOBAL: XW 0x5BE9F0
int g_shellFlightAudioMode1Requested = 0;

// GLOBAL: XW 0x5BE9F4
int g_shellFlightAudioMode2Requested = 0;

// GLOBAL: XW 0x5BECC8
int g_shellFlightPhaseState = 0;

// GLOBAL: XW 0x5E7260
void* g_legacyShellAuxiliary16K = NULL;

// GLOBAL: XW 0x5E7280
uint16_t g_pilotSlotSkillValues[XW_FLIGHT_PILOT_SLOT_COUNT] = { 0 };

// GLOBAL: XW 0x5E7300
XwLegacyMemoryConfig g_legacyMemoryConfig = { { 0 }, { 0 }, { NULL }, { 0 }, { 0 }, { 0 } };

// GLOBAL: XW 0x5E7358
void* g_legacyShellAuxiliary32K = NULL;

// GLOBAL: XW 0x5E735C
unsigned int g_legacyShellAuxiliary32KSize = 0;

// GLOBAL: XW 0x5E7360
unsigned int g_legacyShellBlockUsableSize = 0;

// GLOBAL: XW 0x5E7380
char g_PilotSlotNames[XW_FLIGHT_PILOT_SLOT_COUNT][XW_FLIGHT_PILOT_NAME_CAPACITY] = { { 0 } };

// GLOBAL: XW 0x5E7500
uint8_t g_pilotSlotFlightGroupIndices[XW_FLIGHT_PILOT_SLOT_COUNT] = { 0 };

// GLOBAL: XW 0x5E7510
void* g_legacyShellBlock = NULL;

// GLOBAL: XW 0x5E7520
uint8_t g_pilotSlotCraftIndices[XW_FLIGHT_PILOT_SLOT_COUNT] = { 0 };

// GLOBAL: XW 0x62B4E8
int g_flightAudioMode = 0;

// GLOBAL: XW 0x62D100
uint8_t g_PilotObjectRefs[XW_FLIGHT_PILOT_SLOT_COUNT] = { 0 };

// FUNCTION: XW 0x47E3C0
void ShellFlight_Run(int argumentCount, const char* const* arguments) {
#ifdef XW_MODERN
	(void)argumentCount;
	(void)arguments;
	XwShellFlight_Begin();
#else
	int16_t missionType = XW_SCENE_EXIT_SHELL;
	XwShellSceneId nextScene;
	XwShellSceneId flightScene;
	(void)argumentCount;
	(void)arguments;
	if (ShellFlight_AllocateLegacyMemory(XW_LEGACY_MEMORY_SELECTOR) != 0) {
		ShellFlight_Initialize();
		nextScene = XW_SCENE_STARTUP_LOGO;
		do {
			if (shellext_GetTransitionsEnabled() == 0)
				nextScene = shellext_Convert_Transition(nextScene, 0);
			ShellFlight_ResetSceneState(nextScene);
			flightScene = xmain_main(nextScene, &g_legacyMemoryConfig);
			if (flightScene == XW_SCENE_EXIT_SHELL)
				break;
			switch (flightScene) {
				case XW_SCENE_FLIGHT_PROVING_GROUNDS:
				case XW_SCENE_FLIGHT_COMBAT:
				case XW_SCENE_FLIGHT_TOUR:
					if (flightScene == XW_SCENE_FLIGHT_PROVING_GROUNDS) {
						int pilotSlot;
						uint16_t skillValue;
						int16_t trainingShip = shipext_Get_Train_Ship();
						g_missionRuntimeState.provingGroundsSelectedCraft = trainingShip + 1;
						skillValue = g_RegisterShellPilot.skillValue;
						for (pilotSlot = 0; pilotSlot < XW_FLIGHT_PILOT_SLOT_COUNT; ++pilotSlot) {
							if (pilotSlot != 0)
								g_PilotSlotNames[pilotSlot][0] = 0;
							else {
								g_pilotSlotFlightGroupIndices[0] = 0;
								g_pilotSlotCraftIndices[0] = 0;
								g_pilotSlotSkillValues[0] = skillValue;
								strcpy(g_PilotSlotNames[0], g_RegisterShellPilot.name);
							}
						}
					}
					g_missionRuntimeState.mode = flightScene - XW_SCENE_FLIGHT_PROVING_GROUNDS;
					if (flightScene == XW_SCENE_FLIGHT_TOUR)
						g_missionRuntimeState.mode = FEDISKIO_MISSION_MODE_TOUR;
					if (flightScene == XW_SCENE_FLIGHT_COMBAT && shipext_Is_Combat_Ship_Tour() != 0)
						g_missionRuntimeState.mode = FEDISKIO_MISSION_MODE_COMBAT_TOUR;
					shipext_Get_Mission_Path(g_currentMissionFile, g_shellMissionName, 1);
					g_flightEntryMode = FLIGHT_ENTRY_NEW_MISSION;
					g_inflightMapPreferencesInitialized = 0;
					missionType = flightScene;
					break;
				case XW_SCENE_FLIGHT_REPLAY:
					g_flightEntryMode = FLIGHT_ENTRY_REPLAY_VIEWER;
					missionType = XW_SCENE_FLIGHT_REPLAY;
					break;
				case XW_SCENE_FLIGHT_RESUME:
					g_flightEntryMode = FLIGHT_ENTRY_RESUME_SAVED;
					break;
			}

			ShellFlight_ResetLegacyBlock();
			g_shellFlightPhaseState = 0;
			Xw_simulator(g_flightEntryMode);
			g_shellFlightPhaseState = 1;
			nullsub_SharedNoOp();
			nextScene =
				shipext_Mission_Exit(missionType, g_missionRuntimeState.flightExitReason,
									 g_missionRuntimeState.newMedal, g_missionRuntimeState.tourCutsceneIndex);
		} while (nextScene != XW_SCENE_EXIT_SHELL);
		ShellFlight_Shutdown();
	}
#endif
}

// FUNCTION: XW 0x47E590
void ShellFlight_Initialize(void) {
	time_t seedTime;
	seedTime = time(&seedTime);
	srand((uint16_t)seedTime);
	mouse_exists_gbl = Shared_ReturnOne();
	joy_exists_gbl = Shared_ReturnOne();
	joy_callibrate_gbl = 0;
	g_joystickPollingSuppressed = 0;
	g_mouseButtons = 0;
	g_keyMods = 0;
	g_flightJoystickY = 0;
	g_flightJoystickX = 0;
	g_joystickAvailable = joy_exists_gbl;
	g_joystickDetectResultWord = joy_exists_gbl;
	g_flightMouseInputEnabled = mouse_exists_gbl;
	g_flightMouseY = 0;
	g_flightMouseX = 0;
	FlightDisplay_LockSurface();
	rtsvga2_initgraphVGA();
	FlightDisplay_UnlockSurface();
	nullsub_SharedNoOp();
	if (g_shellFlightAudioMode2Requested != 0)
		g_flightAudioMode = XW_SHELL_FLIGHT_AUDIO_MODE_2;
	else
		g_flightAudioMode = g_shellFlightAudioMode1Requested != 0;
}

// FUNCTION: XW 0x47E660
void ShellFlight_Shutdown(void) { ShellFlight_FreeLegacyMemory(XW_LEGACY_MEMORY_SELECTOR); }

// FUNCTION: XW 0x47E670
void ShellFlight_ResetSceneState(int16_t scene) {
	int sceneId;
	xactor_ResetSceneState();
	xfilm_ResetSceneState();
	xfont_ResetSceneList();
	xpal_ResetScenePointers();
	xres_ResetSceneList();
	xview_ResetLegacySceneState();
	xsound_ResetSceneList();
	g_legacyMemoryConfig.blockBases[0] = g_legacyShellBlock;
	if (g_legacyShellBlock != NULL) {
		g_legacyMemoryConfig.blockSizes[0] = g_legacyShellBlockUsableSize;
	}
	sceneId = scene;
	if (sceneId >= XW_SCENE_INFLIGHT_MAP && sceneId <= XW_SCENE_INFLIGHT_DAMAGE_CONTROL) {
		g_legacyMemoryConfig.blockBases[2] = NULL;
		g_legacyMemoryConfig.blockSizes[2] = 0;
		g_legacyMemoryConfig.blockBases[3] = NULL;
		g_legacyMemoryConfig.blockSizes[3] = 0;
	}
}

// FUNCTION: XW 0x47E6E0
void ShellFlight_ResetLegacyBlock(void) {
	g_legacyMemoryConfig.blockBases[0] = g_legacyShellBlock;
	if (g_legacyShellBlock != NULL) {
		g_legacyMemoryConfig.blockSizes[0] = g_legacyShellBlockUsableSize;
	}
}

// FUNCTION: XW 0x47E700
void XW_NORETURN ShellFlight_FatalExit(const char* message) {
	ShellFlight_Shutdown();
	ShellFlight_ExitProcess(message, XW_SHELL_FATAL_EXIT_CODE);
}

// FUNCTION: XW 0x49E3C0
int16_t ShellFlight_AllocateLegacyMemory(int selector) {
	if (selector == XW_LEGACY_MEMORY_SELECTOR) {
		int index;
		g_legacyShellAuxiliary16K = malloc(XW_LEGACY_AUXILIARY_16K_SIZE);
		if (g_legacyShellAuxiliary16K != NULL) {
			memset(g_legacyShellAuxiliary16K, 0, XW_LEGACY_AUXILIARY_16K_SIZE);
			for (index = 0; index < (int)(sizeof(g_legacyMemoryConfig.field_00) /
										  sizeof(g_legacyMemoryConfig.field_00[0]));
				 ++index) {
				g_legacyMemoryConfig.field_00[index] = 0;
				g_legacyMemoryConfig.field_10[index] = 0;
				g_legacyMemoryConfig.blockBases[index] = NULL;
				g_legacyMemoryConfig.blockSizes[index] = 0;
				g_legacyMemoryConfig.field_40[index] = 0;
				g_legacyMemoryConfig.field_48[index] = 0;
				g_legacyShellAuxiliary32K = NULL;
				g_legacyShellAuxiliary32KSize = 0;
			}
			g_legacyShellAuxiliary32K = malloc(XW_LEGACY_AUXILIARY_32K_SIZE);
			if (g_legacyShellAuxiliary32K != NULL) {
				memset(g_legacyShellAuxiliary32K, 0, XW_LEGACY_AUXILIARY_32K_SIZE);
				g_legacyShellAuxiliary32KSize = XW_LEGACY_AUXILIARY_32K_SIZE;
				g_legacyShellBlock = malloc(XW_LEGACY_BLOCK_SIZE);
				if (g_legacyShellBlock != NULL) {
					memset(g_legacyShellBlock, 0, XW_LEGACY_BLOCK_SIZE);
					g_legacyShellBlockUsableSize = XW_LEGACY_BLOCK_USABLE_SIZE;
					g_legacyMemoryConfig.blockBases[0] = g_legacyShellBlock;
					g_legacyMemoryConfig.blockSizes[0] = XW_LEGACY_BLOCK_SIZE;
					return XwLegacyMemory_AllocationResult(g_legacyShellBlock);
				}
				return 0;
			}
		}
		return 0;
	}
	return (int16_t)selector;
}

// FUNCTION: XW 0x49E4B0
void ShellFlight_FreeLegacyMemory(int selector) {
	if (selector == XW_LEGACY_MEMORY_SELECTOR) {
		int index;
		if (g_legacyShellAuxiliary16K != NULL) {
			free(g_legacyShellAuxiliary16K);
		}
		if (g_legacyShellAuxiliary32K != NULL) {
			free(g_legacyShellAuxiliary32K);
		}
		if (g_legacyShellBlock != NULL)
			free(g_legacyShellBlock);
#ifdef XW_MODERN
		g_legacyShellBlock = NULL;
		g_legacyShellAuxiliary16K = NULL;
#endif
		for (index = 0;
			 index < (int)(sizeof(g_legacyMemoryConfig.field_00) / sizeof(g_legacyMemoryConfig.field_00[0]));
			 ++index) {
			g_legacyMemoryConfig.field_00[index] = 0;
			g_legacyMemoryConfig.field_10[index] = 0;
			g_legacyMemoryConfig.blockBases[index] = NULL;
			g_legacyMemoryConfig.blockSizes[index] = 0;
			g_legacyMemoryConfig.field_40[index] = 0;
			g_legacyMemoryConfig.field_48[index] = 0;
			g_legacyShellAuxiliary32K = NULL;
			g_legacyShellAuxiliary32KSize = 0;
		}
	}
}

// FUNCTION: XW 0x49E5D0
void XW_NORETURN ShellFlight_ExitProcess(const char* unusedMessage, int16_t exitCode) {
	(void)unusedMessage;
	exit(exitCode);
}
