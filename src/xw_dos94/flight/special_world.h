#ifndef XW_DOS94_SPECIAL_WORLD_H
#define XW_DOS94_SPECIAL_WORLD_H
#include "xw/flight/death_star.h"
#include "xw_dos94/assets/models.h"
#include <stdbool.h>
/* Select resident placement/health/pattern tables with the active flight owner. */
void Dos94World_Select(bool dos);
const Dos94MeshView* Dos94World_Bounds(uint16_t model, uint16_t component, XwBounds16* bounds);
uint16_t Dos94World_Hit(const Dos94MeshView* mesh, int32_t x, int32_t y, int32_t z, bool forced);
int32_t Dos94World_ClampDot(int32_t value);
uint16_t Dos94World_TrenchKey(int16_t cellY);
/* Pure authored order shared by classic traversal and the semantic renderer. */
void Dos94_gate_Order(uint16_t type, int16_t side, int16_t forward, int16_t up, unsigned shift,
					  uint16_t* order);
void Dos94_gate_DrawCourseObject(uint16_t index);
int Dos94_gate_TestGatePlaneCollision(uint16_t type);
int Dos94_gate_PointsOnSameSide(uint16_t index, const int32_t first[3], const int32_t second[3]);
void Dos94_DeathStar_DrawSurfaceAndTrench(void);
void Dos94_gate_updatebonuspoints(void);
void Dos94_gate_trainingupdatecrt(int16_t x, int16_t y);
extern const uint8_t Dos94_trenchSurfaceColors[18];
#endif
