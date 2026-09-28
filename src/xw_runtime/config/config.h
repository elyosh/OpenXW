#ifndef XW_RUNTIME_CONFIG_H
#define XW_RUNTIME_CONFIG_H
#include "xw_runtime/config/settings.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Owner-thread only. Views remain valid until a successful update or shutdown. */
int XwConfig_Load(AeronVfs* vfs, char* error, size_t capacity);
int XwConfig_Replace(char* error, size_t capacity);
void XwConfig_Shutdown(void);
const AeronConfigFile* XwConfig_UserDocument(void);
const AeronConfigFile* XwConfig_ResolvedDocument(void);
const XwSettings* XwConfig_Settings(void);
const XwSettings* XwConfig_DefaultSettings(void);
uint64_t XwConfig_Generation(void);
int XwConfig_CanReplace(void);
/* Parse and optionally persist a candidate before publishing any changes. */
int XwConfig_UpdateUser(const AeronConfigFile* candidate, int save, char* error, size_t capacity);
int XwConfig_SetInstallations(const char* xw93, const char* xw94, const char* xw98, const char* arrangement,
							  const char* backend, const char* sc55_rom_directory, const char* mt32_control,
							  const char* mt32_pcm, int save, char* error, size_t capacity);
int XwConfig_SetSkipIntro(int enabled, char* error, size_t capacity);
int XwConfig_Apply(XwSavedShellPreferences* game, char* error, size_t capacity);
int XwConfig_Write(const XwSavedShellPreferences* game, char* error, size_t capacity);
bool XwConfig_SetController(const XwControllerOptions* options, char* error, size_t capacity);
bool XwConfig_SetKeyboard(const XwKeyboardBindings* bindings, char* error, size_t capacity);
bool XwConfig_RestoreKeyboard(char* error, size_t capacity);
int XwConfig_Save(char* error, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
