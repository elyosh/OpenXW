#include "xw_app/application.h"

#include "aeron/main.h"

#include <stdio.h>

int main(int argc, char* argv[]) {
	XwLaunchOptions options;
	int valid = XwLaunchOptions_Parse(argc, argv, &options);
	if (!valid || options.show_help) {
		fprintf(valid ? stdout : stderr,
				"OpenXW\n"
				"Usage: OpenXW [options]\n"
				"  --xw93-data <path>          Extracted X-Wing 1993 B-Wing installation folder\n"
				"  --xw94-data <path>          X-Wing Collector's CD-ROM (1994) folder or image\n"
				"  --xw98-data <path>          X-Wing 1998 installation folder\n"
				"  --setup                     Choose and save your game folders\n"
				"  --skip-intro                Skip the startup sequence\n"
				"  --save-config               Save the supplied game folders\n"
				"  --reset-config              Explicitly replace config.yaml with defaults\n"
				"  --check-installation       Validate/setup from CLI and exit without a window\n"
				"  --resource-root <directory> Override packaged resources\n"
				"  --help                      Show this help\n");
		return valid ? 0 : 2;
	}
	return XwApplication_Run(&options);
}
