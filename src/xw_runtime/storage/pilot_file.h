#ifndef XW_RUNTIME_STORAGE_PILOT_FILE_H
#define XW_RUNTIME_STORAGE_PILOT_FILE_H

#include <aeron/vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct REGISTER_PilotFileRecord;

/* Read a DOS94 or Windows pilot at the start of an open file into the shared Windows layout.
 * Return 1 on success, 0 without changing the destination on invalid size or read failure. */
int XwPilot_Read(AeronFile* file, struct REGISTER_PilotFileRecord* record);

#ifdef __cplusplus
}
#endif
#endif
