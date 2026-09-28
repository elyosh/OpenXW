#ifndef XW_REMASTER_FLIGHT_PIPELINE_H
#define XW_REMASTER_FLIGHT_PIPELINE_H
#include "xw_remaster/flight.h"

bool XwFlightPipeline_PrepareResources(AeronScene3D* scene, int width, int height);
bool XwFlightPipeline_Begin(AeronScene3D* scene, const XwRenderSnapshot* snapshot,
							const XwPreparedFlight* frame, bool reset);
void XwFlightPipeline_Post(AeronScene3D* scene, float shutter, int motion);
int XwFlightPipeline_Resolve(AeronCommandBuffer* cmd, AeronTexture* color, int width, int height, int bloom);
/* Enable only for effective, fully opaque modern flight ownership. */
int XwFlightPipeline_SetDirect(int enabled, int width, int height);
int XwFlightPipeline_SubmitDirect(void);
int XwFlightPipeline_Retain(AeronCommandBuffer* cmd);
int XwFlightPipeline_NeedsRetain(void);
/* World postprocessing, bloom and tonemapping, then display-authored HUD in retained/direct output. */
bool XwFlightPipeline_Finish(AeronCommandBuffer* cmd, AeronScene3D* scene, const XwPreparedFlight* frame);
/* Only successful GPU submission publishes the recorded output. */
void XwFlightPipeline_Commit(bool submitted);
AeronTexture* XwFlightPipeline_Output(void);
void XwFlightPipeline_Invalidate(void);
void XwFlightPipeline_Shutdown(void);
#endif
