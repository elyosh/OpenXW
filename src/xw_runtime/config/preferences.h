#ifndef XW_RUNTIME_CONFIG_PREFERENCES_H
#define XW_RUNTIME_CONFIG_PREFERENCES_H
#include "xw/frontend/shell_preferences.h"
#include <aeron/config_file.h>

#ifdef __cplusplus
extern "C" {
#endif

void XwPreferences_Defaults(XwSavedShellPreferences* out);
int XwPreferences_Parse(const AeronConfigFile* document, XwSavedShellPreferences* out, char* error,
						size_t capacity);
int XwPreferences_WriteDocument(AeronConfigFile* document, const XwSavedShellPreferences* value,
								const XwSavedShellPreferences* defaults, char* error, size_t capacity);
/* Failed recovered void saves retain the entire pending record until a successful retry. */
void XwPreferences_Save(const XwSavedShellPreferences* value);
int XwPreferences_RetrySave(char* error, size_t capacity);
int XwPreferences_HasPendingSave(void);
const char* XwPreferences_SaveError(void);
int XwPreferences_ConsumeSaveError(char* error, size_t capacity);
void XwPreferences_Flush(void);
void XwPreferences_ResetPending(void);
#ifdef __cplusplus
}
#endif
#endif
