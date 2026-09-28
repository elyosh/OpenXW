#ifndef XW_REMASTER_HYPERSPACE_H
#define XW_REMASTER_HYPERSPACE_H
#include "xw_remaster/flight.h"
#ifdef __cplusplus
extern "C" {
#endif
bool XwHyperspace_Active(const XwRenderSnapshot* snapshot);
int XwHyperspace_Prepare(AeronCommandBuffer* cmd, const XwRenderSnapshot* snapshot, AeronScene3D* scene,
						 const XwRenderView* view);
void XwHyperspace_Draw(AeronCommandBuffer* cmd, AeronRenderPass* pass, int width, int height, void* user);
void XwHyperspace_Shutdown(void);

typedef struct XwHyperLighting {
	AeronTexture* texture;
	AeronSampler* sampler;
	float direction[3], color[3];
} XwHyperLighting;

/* Only the scene prepared for this tunnel may consume its environment. */
int XwHyperspace_Lighting(AeronScene3D* scene, XwHyperLighting* out);
#ifdef __cplusplus
}
#endif
#endif
