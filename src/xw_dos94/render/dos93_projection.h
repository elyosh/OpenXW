#ifndef XW_DOS93_PROJECTION_H
#define XW_DOS93_PROJECTION_H

#include "xw_dos94/render/projection.h"

int Dos93_TRANSFM2_checkfaceorientation(Dos94MeshProjection* projection, const Dos94FaceView* face);
unsigned Dos93_DRAWPOL_checknormal(const Dos94Transform* transform, const Dos94FaceView* face,
								   const Dos94EyePoint* eyeVertices, uint16_t vertexCount);

#endif
