#ifndef XW_DOS94_TRACE2_H
#define XW_DOS94_TRACE2_H
#include "xw_dos94/render/projection.h"
#include "xw_dos94/render/raster.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Dos94TraceXEdge {
	uint16_t xQ7, slopeQ7, light, tag;
	int16_t lightIncY;
	uint8_t startY, rowCount;
	bool skipHalfStep;
} Dos94TraceXEdge;

void Dos94_TRACE2_xdownleft(Dos94TraceXEdge edge);
void Dos94_TRACE2_xdownright(Dos94TraceXEdge edge);
void Dos94_TRACE2_xupleft(Dos94TraceXEdge edge);
void Dos94_TRACE2_xupright(Dos94TraceXEdge edge);

typedef struct Dos94TraceYEdge {
	uint16_t startYQ8, spanYQ8, x, slopeQ8, light, tag;
	int16_t lightIncX, lightIncY;
} Dos94TraceYEdge;

void Dos94_TRACE2_ydownleft(Dos94TraceYEdge edge);
void Dos94_TRACE2_ydownright(Dos94TraceYEdge edge);
void Dos94_TRACE2_yupleft(Dos94TraceYEdge edge);
void Dos94_TRACE2_yupright(Dos94TraceYEdge edge);
void Dos94_TRACE2_drawface(Dos94MeshProjection* projection, Dos94Lighting* lighting,
						   const Dos94FaceView* face, Dos94RasterObject* object, uint8_t faceId);
bool Dos94_TRACE2_drawscreencoords(const Dos94ScreenPoint* points, uint16_t count,
								   const Dos94ScreenBounds* bounds, uint8_t color);
void Dos94_DRAWLN2_tracelineedges(Dos94ScreenPoint* first, Dos94ScreenPoint* second, uint16_t thickness,
								  int16_t light1, int16_t light2, uint16_t tag);
#endif
