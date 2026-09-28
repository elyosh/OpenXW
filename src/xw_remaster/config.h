#ifndef XW_REMASTER_CONFIG_H
#define XW_REMASTER_CONFIG_H
#include "xw_runtime/config/video_config.h"
#include <aeron/render.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Apply a published configuration generation at the host-frame boundary. */
int XwRemasterConfig_Sync(void);
/* Capability-adjusted parameters; actual presentation mode is XwRemaster_Status(). */
const XwRenderSettings* XwRemasterConfig_Effective(void);
uint64_t XwRemasterConfig_Generation(void);
AeronSampler* XwRemasterConfig_MeshSampler(void);
void XwRemasterConfig_Shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
