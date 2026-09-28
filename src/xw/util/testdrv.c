#include "xw/util/testdrv.h"

#include "xw/assets/file.h"
#include "xw_runtime/platform/legacy_hardware.h"

#include <stdlib.h>
#include <string.h>

// FUNCTION: XW 0x4AA380
uint8_t testdrv_Get_TIE_CD_Drive(const char* filename) {
#ifdef XW_MODERN
	return File_FindMountedCdDrive(filename);
#else
	char probePath[TESTDRV_PROBE_PATH_CAPACITY];
	char driveList[TESTDRV_DRIVE_LIST_CAPACITY];
	int driveOffset;
	int foundFile = 0;
	int driveListBytes = GetLogicalDriveStringsA(0, NULL);
	GetLogicalDriveStringsA(driveListBytes, driveList);
	for (driveOffset = 0; driveOffset < driveListBytes; driveOffset += TESTDRV_DRIVE_ROOT_BYTES) {
		if (GetDriveTypeA(&driveList[driveOffset]) == TESTDRV_CDROM) {
			XwFile* probeStream;
			strcpy(probePath, &driveList[driveOffset]);
			if (*filename == ':' || *filename == '\\')
				strcat(probePath, filename + 1);
			else
				strcat(probePath, filename);
			probeStream = File_RawOpenText(probePath, "r");
			if (probeStream != NULL) {
				File_RawClose(probeStream);
				foundFile = 1;
				break;
			}
		}
	}
	if (foundFile)
		return driveList[driveOffset];
	return 0;
#endif
}

// FUNCTION: XW 0x4AA4A0
int testdrv_Get_XWing_CD_Drive(void) {
	return testdrv_Get_TIE_CD_Drive("XwingCD\\X-Wing Data\\resource\\mainmenu.lfd");
}
