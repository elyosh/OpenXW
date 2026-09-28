#include "xw_runtime/storage/pilot_file.h"
#include "xw/frontend/register.h"
#include "xw_runtime/storage/file_io.h"
#include <landru/file.h>
#include <string.h>

int XwPilot_Read(AeronFile* file, REGISTER_PilotFileRecord* record) {
	const size_t nativeSize = sizeof(*record);
	const size_t tourOffset = offsetof(REGISTER_PilotFileRecord, tour_status);
	uint8_t bytes[sizeof(*record) + 1];
	int32_t fileSize = XwFile_Length(file);
	/* The briefing launch path writes two copies; readers use the first. */
	int dos = fileSize == sizeof(bytes) || fileSize == 2 * sizeof(bytes);
	if (!dos && fileSize != nativeSize && fileSize != 2 * nativeSize)
		return 0;
	if (!xfile_Read_Data_From_File(file, bytes, dos ? sizeof(bytes) : nativeSize))
		return 0;
	/* DOS94 reserves 87 bytes at 0x289, Windows 86. All fields from tour_status
	 * through the final ejection counter have the same layout after this gap. */
	if (dos)
		memmove(bytes + tourOffset, bytes + tourOffset + 1, nativeSize - tourOffset);
	memcpy(record, bytes, nativeSize);
	return 1;
}
