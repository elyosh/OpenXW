#ifndef XW_DOS94_SPECIAL_DRAW_H
#define XW_DOS94_SPECIAL_DRAW_H
#include "xw/flight/death_star.h"
#include "xw_dos94/render/display.h"

typedef struct Dos94SurfaceDraw {
	Dos94EyePoint stepX, stepY, halfX, halfY;
	int16_t steps[3][3][16];
} Dos94SurfaceDraw;

static inline Dos94EyePoint Dos94World_Add(Dos94EyePoint a, Dos94EyePoint b) {
	return (Dos94EyePoint) { (int32_t)((uint32_t)a.x + (uint32_t)b.x),
							 (int32_t)((uint32_t)a.y + (uint32_t)b.y),
							 (int32_t)((uint32_t)a.z + (uint32_t)b.z) };
}

static inline Dos94EyePoint Dos94World_Sub(Dos94EyePoint a, Dos94EyePoint b) {
	return (Dos94EyePoint) { (int32_t)((uint32_t)a.x - (uint32_t)b.x),
							 (int32_t)((uint32_t)a.y - (uint32_t)b.y),
							 (int32_t)((uint32_t)a.z - (uint32_t)b.z) };
}

static inline Dos94EyePoint Dos94World_Half(Dos94EyePoint p, unsigned shift) {
	return (Dos94EyePoint) { p.x >> shift, p.y >> shift, p.z >> shift };
}

static inline Dos94EyePoint Dos94World_Scale(Dos94EyePoint p, uint32_t scale) {
	return (Dos94EyePoint) { (int32_t)((uint32_t)p.x * scale), (int32_t)((uint32_t)p.y * scale),
							 (int32_t)((uint32_t)p.z * scale) };
}

Dos94EyePoint Dos94World_Eye(Dos94EyePoint relative);
Dos94EyePoint Dos94World_Step(const Dos94SurfaceDraw* state, unsigned axis, unsigned count);
void Dos94World_Position(Dos94EyePoint world);
bool Dos94World_Visible(Dos94EyePoint eye, uint16_t extent);
void Dos94World_DrawModel(uint16_t model, uint16_t component, Dos94EyePoint eye);
const XwSurfaceCellDamageState* Dos94World_Damage(uint16_t start, uint16_t key);
void Dos94_DeathStar_DrawTrench(Dos94SurfaceDraw* state);
#endif
