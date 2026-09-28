#ifndef XW_DOS94_SKY_H
#define XW_DOS94_SKY_H
#include <stdint.h>

typedef struct Dos94Sky {
	int16_t steps[3][3][16], jitter[64][3];
	uint16_t previous[768], previousCount;
	uint8_t present[8000];
} Dos94Sky;

void Dos94_backdrp2_backdrop(void);
void Dos94_RTSVGA2_drawstars(void);
#endif
