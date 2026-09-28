#ifndef XW_INSTALLATION_PAGE_H
#define XW_INSTALLATION_PAGE_H
#include "xw_app/ui.h"
bool XwInstallationPage_Init(char* error, size_t capacity);
void XwInstallationPage_Shutdown(void);
void XwInstallationPage_CancelPicker(void);
bool XwInstallationPage_PickerOpen(void);
void XwInstallationPage_OpenSc55Picker(const char* path);
void XwInstallationPage_OpenMt32Picker(bool control, const char* path);
void XwInstallationPage_Draw(AeronUiContext* ui, const AeronInputSnapshot* input);
void XwInstallationPage_DrawPicker(AeronUiContext* ui);
bool XwInstallationPage_Validate(char* error, size_t capacity);
#endif
