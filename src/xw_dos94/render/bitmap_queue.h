#ifndef XW_DOS94_BITMAP_QUEUE_H
#define XW_DOS94_BITMAP_QUEUE_H
#include "xw_dos94/render/transfm2.h"

typedef struct Dos94BitmapDrawEntry {
	uint16_t objectRef, packedBitmapId, scaleFactor;
	int16_t screenX, screenY;
	int32_t eyeZ;
	int16_t angle;
} Dos94BitmapDrawEntry;

enum { DOS94_BITMAP_QUEUE_CAPACITY = 32 };

typedef struct Dos94BitmapQueue {
	Dos94BitmapDrawEntry entries[DOS94_BITMAP_QUEUE_CAPACITY];
	Dos94EyePoint world, eye;
	int16_t count;
	bool sortPending;
} Dos94BitmapQueue;

void Dos94BitmapQueue_Begin(Dos94BitmapQueue* queue);
void Dos94_ANIM_add_bitmap_draw(Dos94BitmapQueue* queue, const Dos94BitmapDrawEntry* entry);
bool Dos94BitmapQueue_Next(Dos94BitmapQueue* queue, Dos94BitmapDrawEntry* entry);
uint16_t Dos94_ROTSCALE_calcscale(int32_t depth, uint16_t boundHalfWidth, uint16_t factor);
#endif
