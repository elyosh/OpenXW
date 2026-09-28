#ifndef XW_RUNTIME_PORT_H
#define XW_RUNTIME_PORT_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum XwSettingsPage {
	XW_SETTINGS_GAME,
	XW_SETTINGS_VIDEO,
	XW_SETTINGS_AUDIO,
	XW_SETTINGS_CONTROLLER,
	XW_SETTINGS_KEYBOARD,
	XW_SETTINGS_MOUSE
} XwSettingsPage;

void XwPort_RequestSettingsPage(XwSettingsPage page);
XwSettingsPage XwPort_SettingsPage(void);
/* Main-thread owner. Aeron, configuration and storage outlive the port. */
/* Pass the installation probe result; optional CD music never blocks startup. */
int XwPort_Init(int cd_music_available);
void XwPort_SetSkipIntro(int skip);
int XwPort_SkipIntro(void);
void XwPort_Tick(int32_t delta_us);
void XwPort_RebaseClock(void);
int XwPort_SettingsOpen(void);
void XwPort_SetSettingsOpen(int open);
void XwPort_RequestSettings(void);
int XwPort_ConsumeSettingsRequest(void);
int XwPort_ShouldQuit(void);
int XwPort_GetExitCode(void);
void XwPort_Fail(const char* message);
uint64_t XwPort_NextWakeDelayUs(void);
void XwPort_Shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
