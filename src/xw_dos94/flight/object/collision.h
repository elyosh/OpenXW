#ifndef XW_DOS94_COLLISION_H
#define XW_DOS94_COLLISION_H
#include "xw_dos94/assets/models.h"
/* Coordinates are model axes; component results are one-based. */
uint16_t Dos94_COLLIDE_checkhitpolygons(const Dos94MeshView* mesh, const int16_t start[3],
										const int16_t end[3], uint8_t shift);
int16_t Dos94_starship_checkstarshiphit(uint16_t source, uint16_t target);
int16_t Dos94_static_laserstaticcollide(uint16_t source, uint16_t target);
void Dos94_collide_hitoffsets(uint16_t fraction);
#endif
