#ifndef XW_DOS94_TRANSFM2_H
#define XW_DOS94_TRANSFM2_H

#include "xw_dos94/assets/models.h"

typedef struct Dos94EyePoint {
	int32_t x, y, z;
} Dos94EyePoint;

typedef struct Dos94ScreenPoint {
	int32_t x, y;
} Dos94ScreenPoint;

typedef struct Dos94Transform {
	int16_t matrix[3][3];
	int16_t products[3][3][16];
	Dos94EyePoint origin;
	uint8_t numEyeZPositive;
} Dos94Transform;

typedef struct Dos94ScreenBounds {
	int32_t minX, minY, maxX, maxY;
	uint16_t minXPoint, minYPoint, maxXPoint, maxYPoint;
	uint16_t sameX, sameY;
	bool initialized;
} Dos94ScreenBounds;

int32_t Dos94_transfm2_geteye(const int16_t row[3], const Dos94EyePoint* point);
void Dos94_TRANSFM2_clipobjecteyez(Dos94EyePoint* origin, const Dos94EyePoint* endpoint);
bool Dos94_TRANSFM2_geteyecoords(Dos94Transform* transform, const Dos94MeshView* mesh, unsigned shift,
								 Dos94EyePoint* output);
bool Dos94_TRANSFM2_geteyecoordsZ0(Dos94Transform* transform, const Dos94MeshView* mesh, unsigned shift,
								   Dos94EyePoint* output);
void Dos94_TRANSFM2_geteyeminmax(const Dos94Transform* transform, const int16_t bounds[6], unsigned shift,
								 int32_t output[6]);
void Dos94_TRANSFM2_getworldminmax(const int16_t matrix[3][3], const int16_t bounds[6], unsigned shift,
								   int16_t output[6]);
int32_t Dos94_TRANSFM2_getscreenx(int32_t x, uint32_t depth);
int32_t Dos94_TRANSFM2_getscreeny(int32_t y, uint32_t depth);
uint16_t Dos94_TRANSFM2_clipratio(int32_t negativeZ, int32_t positiveZ);
Dos94ScreenPoint Dos94_TRANSFM2_calczintersect(const Dos94EyePoint* negative, const Dos94EyePoint* positive);
void Dos94_TRANSFM2_resetbounds(Dos94ScreenBounds* bounds);
void Dos94_TRANSFM2_screenbounds(Dos94ScreenBounds* bounds, Dos94ScreenPoint point, uint16_t index,
								 bool clipped);
/* Output requires two slots per input vertex; no triangulation or winding change. */
uint16_t Dos94_TRANSFM2_getscreencoords(const Dos94EyePoint* input, uint8_t count, Dos94ScreenPoint* output,
										Dos94ScreenBounds* bounds);
#endif
