#ifndef XW_REMASTER_H
#define XW_REMASTER_H
#include <aeron/input.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum XwRendererMode { XW_RENDERER_CLASSIC, XW_RENDERER_MODERN } XwRendererMode;

typedef struct XwRemasterStatus {
	XwRendererMode requested, effective;
	bool modern_ready, waiting_classic;
	float modern_alpha;
	bool assets_prepared, asset_failed, view_prepared, hud_prepared;
	bool scene_prepared; /* Submitted complete scene, including required special-world resources. */
} XwRemasterStatus;

/* Aeron, VFS and published settings must outlive the driver. */
int XwRemaster_Init(void);
/* Synchronize configuration before game work. Does not advance simulation. */
void XwRemaster_BeginFrame(const AeronInputSnapshot* input);
/* Advance one asset batch and prepare the snapshot view after runtime commit. */
void XwRemaster_Frame(int32_t delta_us);
XwRemasterStatus XwRemaster_Status(void);
void XwRemaster_Shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
