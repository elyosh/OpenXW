#ifndef XW_REMASTER_OPT_MESH_H
#define XW_REMASTER_OPT_MESH_H
#include "aeron/asset/flight_model.h"
#include "aeron/vfs.h"
#include "xw_runtime/config/video_config.h"
#include "xw_runtime/snapshot/render_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

bool XwRemasterOptMesh_Init(AeronVfs* vfs, char* error, size_t capacity);
void XwRemasterOptMesh_Shutdown(void);
bool XwRemasterOptMesh_Build(const XwRenderSource* source, const XwModelSettings* settings,
							 AeronFlightModel* out, char* error, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
