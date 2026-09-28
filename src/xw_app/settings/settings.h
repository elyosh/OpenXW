#ifndef XW_APP_SETTINGS_H
#define XW_APP_SETTINGS_H
#include "xw_app/ui.h"
#include "xw_runtime/config/settings.h"
bool XwSettingsMenu_Init(XwAppUi* ui, char* error, size_t capacity);
void XwSettingsMenu_Shutdown(void);
void XwSettingsMenu_FlushForExit(void);
bool XwSettingsMenu_Open(void);
bool XwSettingsMenu_CapturesKeyboard(void);
void XwSettingsMenu_Show(void);
void XwSettingsMenu_RequestClose(void);
void XwSettingsMenu_ReportError(const char* error);
bool XwSettingsMenu_BeginFrame(const AeronInputSnapshot* input);
bool XwSettingsMenu_ConsumeRuntimeRequest(void);
void XwSettingsMenu_Frame(const AeronInputSnapshot* input, float seconds);
XwSettings* XwSettingsMenu_Draft(void);
float XwSettingsMenu_ScrollHeight(void);
#endif
