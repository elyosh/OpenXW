#ifndef XW_FRONTEND_XMAIN_H
#define XW_FRONTEND_XMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwLegacyMemoryConfig;
/* Declarations follow ascending original IDB address. */

/* 0x47DF80 */
XwShellSceneResult xmain_main(XwShellSceneId scene, struct XwLegacyMemoryConfig* memory);

#ifdef __cplusplus
}
#endif

#endif
