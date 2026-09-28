#ifndef XW_UTIL_TESTDRV_H
#define XW_UTIL_TESTDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
	TESTDRV_PROBE_PATH_CAPACITY = 80,
	TESTDRV_DRIVE_LIST_CAPACITY = 260,
	TESTDRV_DRIVE_ROOT_BYTES = 4,
	TESTDRV_CDROM = 5
};

/* Declarations follow ascending original IDB address. */

/* 0x4AA380 */
uint8_t testdrv_Get_TIE_CD_Drive(const char* filename);

/* 0x4AA4A0 */
int testdrv_Get_XWing_CD_Drive(void);

#ifdef __cplusplus
}
#endif

#endif
