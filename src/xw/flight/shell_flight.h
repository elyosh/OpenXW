#ifndef XW_FLIGHT_SHELL_FLIGHT_H
#define XW_FLIGHT_SHELL_FLIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/compiler.h>

struct XwLegacyMemoryConfig;

enum { XW_LEGACY_MEMORY_SELECTOR = 1, XW_SHELL_FATAL_EXIT_CODE = -1 };

enum { XW_FLIGHT_PILOT_SLOT_COUNT = 16, XW_FLIGHT_PILOT_NAME_CAPACITY = 24 };

enum {
	XW_LEGACY_AUXILIARY_16K_SIZE = 16 * 1024,
	XW_LEGACY_AUXILIARY_32K_SIZE = 32 * 1024,
	XW_LEGACY_BLOCK_SIZE = 4 * 1024 * 1024,
	XW_LEGACY_BLOCK_USABLE_SIZE = 3 * 1024 * 1024
};

enum { XW_SHELL_FLIGHT_AUDIO_MODE_2 = 2 };

extern int g_shellFlightAudioMode1Requested;
extern int g_shellFlightAudioMode2Requested;
extern int g_shellFlightPhaseState;
extern void* g_legacyShellAuxiliary16K;
extern uint16_t g_pilotSlotSkillValues[XW_FLIGHT_PILOT_SLOT_COUNT];
extern char g_PilotSlotNames[XW_FLIGHT_PILOT_SLOT_COUNT][XW_FLIGHT_PILOT_NAME_CAPACITY];
extern uint8_t g_PilotObjectRefs[XW_FLIGHT_PILOT_SLOT_COUNT];
extern uint8_t g_pilotSlotFlightGroupIndices[XW_FLIGHT_PILOT_SLOT_COUNT];
extern uint8_t g_pilotSlotCraftIndices[XW_FLIGHT_PILOT_SLOT_COUNT];
extern struct XwLegacyMemoryConfig g_legacyMemoryConfig;
extern void* g_legacyShellAuxiliary32K;
extern unsigned int g_legacyShellAuxiliary32KSize;
extern unsigned int g_legacyShellBlockUsableSize;
extern void* g_legacyShellBlock;

extern int g_flightAudioMode;

/* Declarations follow ascending original IDB address. */

/* 0x47E3C0 */
void ShellFlight_Run(int argumentCount, const char* const* arguments);

/* 0x47E590 */
void ShellFlight_Initialize(void);

/* 0x47E660 */
void ShellFlight_Shutdown(void);

/* 0x47E670 */
void ShellFlight_ResetSceneState(int16_t scene);

/* 0x47E6E0 */
void ShellFlight_ResetLegacyBlock(void);

/* 0x47E700 */
void XW_NORETURN ShellFlight_FatalExit(const char* message);

/* 0x49E3C0 */
int16_t ShellFlight_AllocateLegacyMemory(int selector);

/* 0x49E4B0 */
void ShellFlight_FreeLegacyMemory(int selector);

/* 0x49E5D0 */
void XW_NORETURN ShellFlight_ExitProcess(const char* unusedMessage, int16_t exitCode);

#ifdef __cplusplus
}
#endif

#endif
