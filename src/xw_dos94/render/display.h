#ifndef XW_DOS94_DISPLAY_H
#define XW_DOS94_DISPLAY_H
#include "xw/render/flight_palette.h"
#include "xw_dos94/render/bitmap_queue.h"
#include "xw_dos94/render/drawpol.h"
#include "xw_dos94/render/projection.h"
#include "xw_dos94/render/raster.h"
#include "xw_dos94/render/rotscale.h"
#include "xw_dos94/render/sky.h"
#include "xw_dos94/render/sweep.h"
#include <aeron/aeron.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
	DOS94_SCREEN_BYTES = 64000,
	DOS94_WORK_BYTES = 0xED80,
	DOS94_SPRITE_BYTES = 0xA316,
	DOS94_MASK_BYTES = 3456
};

/* Mission-owned display buffers and renderer workspaces. */
typedef struct Dos94Display {
	Dos94Raster raster;
	Dos94Sky sky;
	Dos94Rotation rotation;
	Dos94DrawState draw;
	Dos94Sweep sweep;
	Dos94MeshProjection projection;
	Dos94BitmapQueue bitmaps;
	uint8_t screen[DOS94_SCREEN_BYTES], published[DOS94_SCREEN_BYTES];
	uint8_t logical[DOS94_WORK_BYTES], mask[DOS94_MASK_BYTES];
	uint8_t sprites[DOS94_SPRITE_BYTES];
	Dos94SpanRun runs[DOS94_SPAN_RUN_CAPACITY];
	uint8_t tiny[1030], micro[760];
	RgbTriplet stored[256], effective[256];
	AeronPaletteEntry palette[256];
	uint16_t rowStride, byteArea;
	uint32_t viewportGeneration, generation;
	bool fullUpdate, ready;
} Dos94Display;

extern Dos94Display* Dos94_display;
bool Dos94Display_Init(char* error, size_t capacity);
void Dos94Display_Free(void);
/* Owner-thread host clock, independent of mission simulation and audio playback. */
void Dos94Display_ResetPaletteCycle(void);
void Dos94Display_AdvancePalette(int32_t deltaUs);
void Dos94Display_Publish(void);
void Dos94Display_Submit(void);
void Dos94Display_BindCallbacks(void);
void Dos94Display_Restore(void);
void Dos94_SetFlightViewport(uint16_t width, uint16_t height, int unused, unsigned int byteOffset);
void Dos94_LOGBUF2_clearbuffer(uint8_t color);
void Dos94_festring_setfontsize(uint8_t tier);
void Dos94_rtsvga2_blankVGA(void);
void Dos94_rtsvga2_unblankVGA(void);
void Dos94_rtsvga2_buildpaletteVGA(const RgbTriplet*, uint16_t start, uint16_t count);
void Dos94_rtsvga2_savepaletteVGA(RgbTriplet*);
void Dos94_rtsvga2_restorepaletteVGA(const RgbTriplet*);
int Dos94_rtsvga2_calcpositionVGA(uint16_t x, uint16_t y);
void Dos94_rtsvga2_drawshapeVGA(const uint8_t*, int16_t x, int16_t y, int16_t transparent, int16_t mirror);
/* Sized entry point for resources; callbacks receive only previously bounded streams. */
bool Dos94Sprite_Size(const uint8_t* data, size_t available, size_t* size);
void Dos94_rtsvga2_outcharVGA(char);
void Dos94_rtsvga2_clearwindowVGA(void);
void Dos94_rtsvga2_fillboxVGA(uint16_t, uint16_t, uint16_t, uint16_t);
void Dos94_rtsvga2_saveboxVGA(uint8_t*, uint16_t, uint16_t, uint16_t, uint16_t);
void Dos94_rtsvga2_restoreboxVGA(const uint8_t*, uint16_t, uint16_t, uint16_t, uint16_t);
void Dos94_rtsvga2_drawdotVGA(uint16_t, uint16_t, uint8_t);
struct XwRadarBlip;
void Dos94_rtsvga2_drawblips(struct XwRadarBlip*, int);
void Dos94_rtsvga2_removeblips(struct XwRadarBlip*, int);
void Dos94_rtsvga2_drawbracket(void);
void Dos94_rtsvga2_removebracket(void);
void Dos94_rtsvga2_drawcross(uint16_t, uint16_t, uint8_t);
void Dos94_rtsvga2_removecross(uint16_t, uint16_t);
#endif
