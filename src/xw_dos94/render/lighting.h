#ifndef XW_DOS94_LIGHTING_H
#define XW_DOS94_LIGHTING_H
#include "xw_dos94/render/transfm2.h"

typedef struct Dos94Lighting {
	int16_t direction[3];
	int16_t vertexLight[128];
	uint16_t parent, target;
	uint8_t gateTint, markLightValue;
	bool threeD, gouraud;
} Dos94Lighting;

void Dos94Lighting_BeginMesh(Dos94Lighting* lighting);
unsigned Dos94_DRAWPOL_checknormal(const Dos94Transform* transform, const Dos94FaceView* face,
								   const Dos94EyePoint* eyeVertices, uint16_t vertexCount);
bool Dos94_DRAWPOL_vertexlight(Dos94Lighting* lighting, const Dos94MeshView* mesh, uint8_t vertex,
							   bool retainNegative, bool recalculate);
bool Dos94_DRAWPOL_getlightvalue(Dos94Lighting* lighting, const Dos94MeshView* mesh, uint16_t faceIndex,
								 uint8_t color, uint8_t* output);
Dos94ScreenPoint Dos94_TRANSFM2_facezintersect(const Dos94EyePoint* negative, const Dos94EyePoint* positive,
											   int16_t negativeLight, int16_t positiveLight, int16_t* light);
#endif
