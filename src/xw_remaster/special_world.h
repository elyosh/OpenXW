#ifndef XW_REMASTER_SPECIAL_WORLD_H
#define XW_REMASTER_SPECIAL_WORLD_H
#include "xw_remaster/flight.h"
#include "xw_runtime/snapshot/render_world.h"

bool XwSpecialWorld_Prepare(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwRenderSnapshot* snapshot,
							const XwPreparedFlight* frame, bool reset);
void XwSpecialWorld_Shutdown(void);
#endif
