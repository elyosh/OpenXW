#ifndef XW_KEYBOARD_CONFIG_H
#define XW_KEYBOARD_CONFIG_H
#include "aeron/config_file.h"
#include "xw_runtime/input/keyboard_mapping.h"

bool XwKeyboardConfig_Read(const AeronConfigFile* document, XwKeyboardBindings* profile, char* error,
						   size_t capacity);
bool XwKeyboardConfig_Write(AeronConfigFile* document, const XwKeyboardBindings* profile,
							AeronConfigError* error);
/* Resolve user precedence into the temporary merged document without changing user overrides. */
bool XwKeyboardConfig_Resolve(const XwKeyboardBindings* defaults, const AeronConfigFile* user,
							  AeronConfigFile* merged, char* error, size_t capacity);
#endif
