#ifndef XW_DOS94_VIEW_H
#define XW_DOS94_VIEW_H
#include <stdint.h>
struct ObjectRecord;
int Dos94_trig2_arctan(int32_t first, int32_t second);
void Dos94_fview_newcalcview(int16_t roll, int16_t pitch, int16_t yaw, int16_t offset, int16_t aimX,
							 int16_t aimY, struct ObjectRecord* object);
void Dos94_fview_newcalcrotate(int16_t roll, int16_t pitch, int16_t yaw, int16_t offset,
							   struct ObjectRecord* object);
void Dos94_fview_calcrotatemove(int16_t pitch, int16_t yaw, struct ObjectRecord* object);
void Dos94_fview_transformaxes(int x, int y, int z, int16_t angle);
#endif
