#ifndef XW_DOS94_FVIEW_H
#define XW_DOS94_FVIEW_H
#include "xw_dos94/render/transfm2.h"
int16_t Dos94_trig2_getsignedsin(uint16_t angle);
int16_t Dos94_trig2_getsignedcos(uint16_t angle);
int16_t Dos94_math2_dot2Q15(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
int16_t Dos94_math2_dot3Q15(const int16_t a[3], const int16_t b[3]);
void Dos94_FVIEW_sfoilrotation(Dos94Transform* transform, int16_t angle);
void Dos94_FVIEW_corvettegunrotation(Dos94Transform* transform, int16_t angle);
void Dos94_FVIEW_bwingrotation(Dos94Transform* transform, int16_t angle, uint16_t part);
void Dos94_FVIEW_restorerotation(Dos94Transform* transform);
#endif
