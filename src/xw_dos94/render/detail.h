#ifndef XW_DOS94_DETAIL_H
#define XW_DOS94_DETAIL_H
#include "xw_dos94/assets/models.h"
void Dos94_user_setdetaillevel(uint16_t preset);
const Dos94MeshView* Dos94_DRAW_getdetailptr(const Dos94Lod* lods, uint16_t count, int32_t depth);
const Dos94Lod* Dos94_DRAW_getcomponentptr(uint16_t model, uint16_t component, uint16_t* count);
extern uint16_t Dos94_solidindex;
#endif
