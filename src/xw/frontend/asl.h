#ifndef XW_FRONTEND_ASL_H
#define XW_FRONTEND_ASL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int16_t g_aslActive;

struct XwLegacyMemoryConfig;

/* 0x46C9C0 */
void asl_Open_ASL(struct XwLegacyMemoryConfig* memory);

/* 0x46CA10 */
void asl_Close_ASL(void);

/* 0x46CAB0 */
void asl_ConfigureSystemDefaults(void);

#ifdef __cplusplus
}
#endif

#endif
