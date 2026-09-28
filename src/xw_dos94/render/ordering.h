#ifndef XW_DOS94_ORDERING_H
#define XW_DOS94_ORDERING_H
#include <stdbool.h>
#include <stdint.h>
uint16_t Dos94_DRAW_polydepthsort(uint8_t faceA, uint16_t idA, uint16_t parentA, uint16_t componentA,
								  uint8_t faceB, uint16_t idB, uint16_t parentB, uint16_t componentB);
uint16_t Dos94_xtrans2_getinfront(uint16_t a, uint16_t b, uint8_t faceA, uint8_t faceB, bool trenchCover);
#endif
