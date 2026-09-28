#ifndef XW_APP_HOST_CONFIG_H
#define XW_APP_HOST_CONFIG_H

#include "aeron/aeron.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XwLaunchOptions {
	const char* resource_root;
	const char* xw93_data;
	const char* xw94_data;
	const char* xw98_data;
	int show_help;
	int setup;
	int save_config;
	int reset_config;
	int check_installation;
	int skip_intro;
} XwLaunchOptions;

/* Launch strings borrow argv for the lifetime of the application. */
int XwLaunchOptions_Parse(int argc, char* argv[], XwLaunchOptions* options);
void XwHostConfig_InitAeron(const XwLaunchOptions* options, AeronConfig* config);
int XwHostConfig_ResolveResourceRoot(const XwLaunchOptions* options, char* out, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
