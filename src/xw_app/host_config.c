#include "xw_app/host_config.h"
#include "xw_app/window_icon.h"

#include <stdio.h>
#include <string.h>

int XwLaunchOptions_Parse(int argc, char* argv[], XwLaunchOptions* options) {
	memset(options, 0, sizeof(*options));
	for (int index = 1; index < argc; ++index) {
		const char* argument = argv[index];
		const char* equals = strchr(argument, '=');
		size_t length = equals ? (size_t)(equals - argument) : strlen(argument);
		const char** destination = NULL;

		struct {
			const char* name;
			const char** value;
		} paths[] = { { "--xw93-data", &options->xw93_data },
					  { "--resource-root", &options->resource_root },
					  { "--xw94-data", &options->xw94_data },
					  { "--xw98-data", &options->xw98_data } };

		struct {
			const char* name;
			int* value;
		} flags[] = { { "--help", &options->show_help },
					  { "-h", &options->show_help },
					  { "--setup", &options->setup },
					  { "--skip-intro", &options->skip_intro },
					  { "--save-config", &options->save_config },
					  { "--reset-config", &options->reset_config },
					  { "--check-installation", &options->check_installation } };

		int handled = 0;
		for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); ++i) {
			if (!strcmp(argument, flags[i].name)) {
				*flags[i].value = 1;
				handled = 1;
				break;
			}
		}
		if (handled)
			continue;
		for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
			if (strlen(paths[i].name) == length && !strncmp(argument, paths[i].name, length)) {
				destination = paths[i].value;
				break;
			}
		}
		if (!destination) {
			fprintf(stderr, "OpenXW: unknown argument: %s\n", argument);
			return 0;
		}
		const char* value = equals ? equals + 1 : ++index < argc ? argv[index] : NULL;
		if (*destination || !value || !value[0] || value[0] == '-') {
			fprintf(stderr, "OpenXW: %.*s requires one nonempty argument\n", (int)length, argument);
			return 0;
		}
		*destination = value;
	}
	if (options->setup && options->check_installation) {
		fprintf(stderr, "OpenXW: --setup cannot be combined with --check-installation.\n");
		return 0;
	}
	return 1;
}

void XwHostConfig_InitAeron(const XwLaunchOptions* options, AeronConfig* config) {
	memset(config, 0, sizeof(*config));
	config->org_name = "TotallyOpen";
	config->app_name = "OpenXW";
	config->window_title = "OpenXW";
	config->window_icon_bmp = xw_window_icon_bmp;
	config->window_icon_bmp_size = sizeof xw_window_icon_bmp;
	config->resource_root = options ? options->resource_root : NULL;
	config->resource_path = "resources";
	config->shader_path = "shaders";
	config->logical_width = 640;
	config->logical_height = 480;
	config->presentation_mode = AERON_PRESENTATION_ASPECT_FIT;
	config->clear_color_enabled = 1;
	config->clear_color_rgba[3] = 1.0f;
}

int XwHostConfig_ResolveResourceRoot(const XwLaunchOptions* options, char* out, size_t capacity) {
	AeronConfig config;
	XwHostConfig_InitAeron(options, &config);
	if (config.resource_root && config.resource_root[0]) {
		if (strlen(config.resource_root) >= capacity)
			return 0;
		strcpy(out, config.resource_root);
		return 1;
	}
	return Aeron_ApplicationPath(config.resource_path, out, capacity);
}
