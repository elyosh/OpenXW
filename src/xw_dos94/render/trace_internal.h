#ifndef XW_DOS94_TRACE_INTERNAL_H
#define XW_DOS94_TRACE_INTERNAL_H
#include "xw_dos94/render/trace2.h"
#include "xw_dos94/render/transfm2.h"

typedef struct Dos94TraceSlope {
	uint32_t whole;
	uint16_t fraction;
} Dos94TraceSlope;

typedef struct Dos94TraceLine {
	Dos94ScreenPoint first, second;
	Dos94TraceSlope slope;
	uint16_t light1, light2, tag;
	int16_t lightIncX, lightIncY;
	int8_t xDirection, yDirection;
	uint8_t startY, endY;
	int32_t unclippedX;
	int16_t startX;
	uint16_t entryDistance;
} Dos94TraceLine;

Dos94TraceSlope Dos94_math2_calcEdgeSlope(uint32_t numerator, uint32_t denominator);
int Dos94_TRACE2_ydomclipy(Dos94TraceLine* line);
int Dos94_TRACE2_xdomclipy(Dos94TraceLine* line);
void Dos94_TRACE2_ydomedge(Dos94TraceLine* line);
void Dos94_TRACE2_xdomedge(Dos94TraceLine* line);
void Dos94Trace_Outside(Dos94TraceLine* line, bool right, bool yDominant);
#endif
