#ifndef XW_DOS94_RASTER_H
#define XW_DOS94_RASTER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
	DOS94_SPAN_RUN_CAPACITY = 6000,
	/* DOS93: 256 pages * 42 three-byte records; DOS94: 320 * 31. */
	DOS_EDGE_EVENTS = 10752,
	DOS_NO_EVENT = UINT16_MAX
};

/* Encoded length 0..255 represents 1..256 pixels in the persistent run cache. */
typedef struct Dos94SpanRun {
	uint8_t lengthMinusOne, color;
} Dos94SpanRun;

typedef enum Dos94EventKind { DOS_EVENT_POINT, DOS_EVENT_VERTICAL, DOS_EVENT_CLIPPED } Dos94EventKind;

typedef enum Dos94ClipKind {
	DOS_CLIP_Y_EXIT,
	DOS_CLIP_X_Q7,
	DOS_CLIP_Y_OUTSIDE,
	DOS_CLIP_X_INTEGER
} Dos94ClipKind;

typedef struct Dos94ClippedState {
	Dos94ClipKind kind;
	bool right, increase;
	uint16_t step, fraction, rows;
	uint32_t position;
} Dos94ClippedState;

typedef struct Dos94EdgeEvent {
	uint16_t next, x, light, rows;
	uint8_t object, edge, lightFraction;
	Dos94EventKind kind;
	Dos94ClippedState clip;
} Dos94EdgeEvent;

typedef struct Dos94RasterEdge {
	uint8_t face1, face2;
	uint16_t halfLightIncY;
} Dos94RasterEdge;

typedef struct Dos94RasterObject {
	int16_t bounds[6];
	uint16_t parent, component, faceCount;
	uint8_t material[256], modelFace[256], behind[5];
	bool gouraud[256];
	Dos94RasterEdge edges[256];
} Dos94RasterObject;

typedef struct Dos94RasterFlat {
	int16_t x, y, z;
	uint16_t parent;
	uint8_t component, color;
} Dos94RasterFlat;

typedef struct Dos94RasterMark {
	uint8_t object, face, materials[16], layers[16], count;
	bool gouraud[16];
} Dos94RasterMark;

typedef struct Dos94Raster {
	Dos94EdgeEvent events[DOS_EDGE_EVENTS];
	uint16_t rowHeads[200], rowTails[200], rowPageBytes[200];
	uint16_t eventCount, pageCount, pageLimit, pageBytes, objectLimit, flatLimit, markFirst;
	bool dos93, failed;
	uint16_t nextObject, parentObject, objectNumber, edgeIndex, flatObjectNumber, numMarks;
	Dos94RasterObject objects[128];
	Dos94RasterObject* objectById[128];
	Dos94RasterFlat flat[128];
	Dos94RasterMark marks[256];
	uint8_t backgroundColor;
	int16_t lightIncX, lightIncY, lineLightIncY, lineLight1, lineLight2;
	uint16_t lastEdge, layer;
	uint16_t runRead, runWrite, frameRuns;
	uint8_t materialColors[624], markColorOffset[64], targetMapping[39];
} Dos94Raster;

void Dos94Raster_Init(Dos94Raster* raster);
extern const uint8_t Dos94_trainingGateColors[4];
void Dos94_XTRANS2_initxtrans(void);
void Dos94_xtrans2_clearruntable(void);
void Dos94_XTRANS2_beginoutput(void);
void Dos94_XTRANS2_endoutput(void);
void Dos94_TRACE2_enterevent(uint16_t row, uint16_t tag, uint16_t x, uint8_t light);
void Dos94_TRACE2_entervertedge(uint16_t top, uint16_t count, uint16_t x, uint16_t light, uint16_t tag);
void Dos94_TRACE2_enterclipped(uint16_t row, uint16_t tag, uint16_t light, Dos94ClippedState state);
void Dos94_XTRANS2_outputCachedSpan(uint16_t row, uint16_t start, uint16_t end, uint8_t color);
/* Resolved edge coordinates/light; the sweep selects the opposite edge first. */
void Dos94_XTRANS2_outputGouraudSpan(uint16_t row, uint16_t start, uint16_t end, uint8_t material,
									 uint16_t leftX, uint8_t leftLight, uint16_t rightX, uint8_t rightLight);
void Dos94_DRAWPOL_setmarkingcolors(uint16_t mode);

static inline uint16_t Dos94_read16(const uint8_t* bytes, size_t offset) {
	return (uint16_t)(bytes[offset] | ((uint16_t)bytes[offset + 1] << 8));
}

static inline uint32_t Dos94_read32(const uint8_t* bytes, size_t offset) {
	return Dos94_read16(bytes, offset) | ((uint32_t)Dos94_read16(bytes, offset + 2) << 16);
}

static inline void Dos94_write16(uint8_t* bytes, size_t offset, uint16_t value) {
	bytes[offset] = (uint8_t)value;
	bytes[offset + 1] = (uint8_t)(value >> 8);
}

static inline void Dos94_write32(uint8_t* bytes, size_t offset, uint32_t value) {
	Dos94_write16(bytes, offset, (uint16_t)value);
	Dos94_write16(bytes, offset + 2, (uint16_t)(value >> 16));
}
#endif
