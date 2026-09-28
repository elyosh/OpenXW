#ifndef XW_DOS94_DRAWPOL_H
#define XW_DOS94_DRAWPOL_H
#include "xw_dos94/render/projection.h"

enum { DOS_COMPONENT_ORDER_CAPACITY = 32 };

typedef struct Dos94DrawState {
	Dos94Transform camera, object;
	int16_t craftBasis[3][3], worldLight[3], objectLight[3];
	uint16_t sphereRadius, target;
	uint8_t gateTint;
	Dos94Lighting lighting;
	uint8_t faceVisibility[256], faceMark[256];
	uint16_t markSources[128];
	/* DOS93 may generate more entries than the resident component count consumes. */
	uint16_t componentOrder[DOS_COMPONENT_ORDER_CAPACITY];
	bool closeToObject;
} Dos94DrawState;

int Dos94_drawpol_drawpolyobject(const Dos94MeshView* mesh, Dos94EyePoint origin);
void Dos94_DRAWPOL_drawmarkings(const Dos94MeshView* mesh, const Dos94FaceView* face, uint8_t markId,
								uint16_t source, uint8_t faceId);
void Dos94_DRAWPOL_drawsurfacepoly(const Dos94EyePoint points[4], uint16_t color);
#endif
