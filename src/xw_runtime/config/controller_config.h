#ifndef XW_RUNTIME_CONFIG_CONTROLLER_CONFIG_H
#define XW_RUNTIME_CONFIG_CONTROLLER_CONFIG_H
#include "aeron/config_file.h"
#include "xw_runtime/input/controller_options.h"
bool XwControllerConfig_Parse(const AeronConfigFile* document, XwControllerOptions* out, char* error,
							  size_t capacity);
bool XwControllerConfig_Write(AeronConfigFile* document, const XwControllerOptions* options,
							  AeronConfigError* error);
bool XwControllerConfig_ReadProfile(const AeronConfigFile* document, const char* path,
									AeronControllerKind kind, XwControllerProfile* profile, char* error,
									size_t capacity);
#endif
