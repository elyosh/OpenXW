#ifndef XW_REMASTER_GATE_LIGHTS_H
#define XW_REMASTER_GATE_LIGHTS_H
#include <aeron/scene/scene3d.h>
#include <aeron/vfs.h>
#include <stdbool.h>

typedef struct XwGateLightModel XwGateLightModel;

bool XwGateLights_Init(AeronVfs* vfs, char* error, size_t capacity);
/* Definitions are borrowed until the OPT asset cache shuts down. */
const XwGateLightModel* XwGateLights_Model(const char* path);
void XwGateLights_Submit(AeronScene3D* scene, const XwGateLightModel* model, unsigned component,
						 unsigned variant, const float transform[16]);
void XwGateLights_Shutdown(void);
#endif
