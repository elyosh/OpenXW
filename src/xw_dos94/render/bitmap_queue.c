#include "xw_dos94/render/bitmap_queue.h"

void Dos94BitmapQueue_Begin(Dos94BitmapQueue* queue) {
	queue->count = 0;
	queue->sortPending = true;
}

/* DOS94 0x6954F6: a full queue overwrites world XYZ and the low word of eye X.
 * Keep those side effects explicit; the incoming entry is never drawn. */
void Dos94_ANIM_add_bitmap_draw(Dos94BitmapQueue* q, const Dos94BitmapDrawEntry* e) {
	if (q->count < 0 || q->count > DOS94_BITMAP_QUEUE_CAPACITY)
		return;
	if (q->count == DOS94_BITMAP_QUEUE_CAPACITY) {
		q->world.x = (int32_t)(e->objectRef | ((uint32_t)e->packedBitmapId << 16));
		q->world.y = (int32_t)(e->scaleFactor | ((uint32_t)(uint16_t)e->screenX << 16));
		q->world.z = (int32_t)((uint16_t)e->screenY | ((uint32_t)(uint16_t)e->eyeZ << 16));
		q->eye.x = (int32_t)(((uint32_t)q->eye.x & 0xFFFF0000u) | (uint16_t)e->angle);
		return;
	}
	q->entries[q->count++] = *e;
}

/* DOS94 0x695546: one bubble pass per draw, preserving equal-depth order.
 * The caller consumes each returned tail immediately through ANIM_draw_bitmap. */
bool Dos94BitmapQueue_Next(Dos94BitmapQueue* q, Dos94BitmapDrawEntry* out) {
	if (q->count < 0)
		return false;
	int16_t before = q->count--;
	if (!before)
		return false;
	if (q->sortPending) {
		q->sortPending = false;
		for (int i = 0; i < q->count; ++i) {
			if (q->entries[i].eyeZ > q->entries[i + 1].eyeZ) {
				Dos94BitmapDrawEntry temporary = q->entries[i];
				q->entries[i] = q->entries[i + 1];
				q->entries[i + 1] = temporary;
				q->sortPending = true;
			}
		}
	}
	*out = q->entries[q->count];
	return true;
}

/* DOS94 0x6A7260: division precedes multiplication; no absolute-depth conversion. */
uint16_t Dos94_ROTSCALE_calcscale(int32_t depth, uint16_t boundHalfWidth, uint16_t factor) {
	uint16_t denominator = (uint16_t)((uint32_t)depth >> 8);
	if ((uint32_t)depth >> 24)
		denominator |= 0xFF00;
	if (!denominator)
		denominator = 1;
	uint32_t product = (uint32_t)(boundHalfWidth / denominator) * factor;
	uint16_t scale = (uint16_t)(product >> 8);
	return scale < 1024 ? scale : 1024;
}
