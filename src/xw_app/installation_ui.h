#ifndef XW_APP_INSTALLATION_UI_H
#define XW_APP_INSTALLATION_UI_H
#include "xw_app/installation.h"
#include <aeron/scene/ui.h>
#include <aeron/scene/ui_file_picker.h>
uint32_t XwInstallation_PathRow(AeronUiContext* ui, XwGameVersion version, char* path, size_t capacity,
								uint32_t flags);
/* version is borrowed until the picker is closed. */
int XwInstallation_OpenPicker(AeronUiFilePicker* picker, XwGameVersion* version, const char* path,
							  char* error, size_t capacity);
#endif
