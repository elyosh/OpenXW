#ifndef XW_UTIL_SHARED_H
#define XW_UTIL_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum { XW_SHARED_EMPTY_STRING_CAPACITY = 8 };

extern char g_sharedEmptyString[XW_SHARED_EMPTY_STRING_CAPACITY];

/* Declarations follow ascending original IDB address. */

/* 0x46CBD0 */
void j_nullsub_2(void);

/* 0x485100 */
int Shared_ReturnZero32(void);

/* 0x49E560 */
int16_t Shared_ReturnZero(void);

/* 0x49E570 */
int16_t Shared_ReturnOne(void);

/* 0x49E5C0 */
void nullsub_SharedNoOp(void);

#ifdef __cplusplus
}
#endif

#endif
