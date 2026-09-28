#ifndef XW_RUNTIME_LANDRU_ADAPTER_H
#define XW_RUNTIME_LANDRU_ADAPTER_H
#include <landru/res.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Borrow storage for the host lifetime; destroy tasks/modules before Shutdown. */
bool XwLandru_Init(void);
void XwLandru_Shutdown(void);
bool XwLandru_OpenVideo(void);
void XwLandru_CloseVideo(void);
void XwLandru_SetLogicalViewport(int low_resolution, int width, int height);
/* The retained resource filename preserves mission routing on lazy reopen. */
ResFile* XwLandru_OpenMissionResource(const char* path);
#ifdef __cplusplus
}
#endif
#endif
