#ifndef XW_REMASTER_SKY_H
#define XW_REMASTER_SKY_H
#include "xw_remaster/flight.h"
#ifdef __cplusplus
extern "C" {
#endif
int XwSky_Prepare(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwRenderSnapshot* snapshot,
				  const XwRenderView* view);
void XwSky_Shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
