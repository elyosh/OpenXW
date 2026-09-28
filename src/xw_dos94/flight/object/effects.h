#ifndef XW_DOS94_EFFECTS_H
#define XW_DOS94_EFFECTS_H
#include "xw/flight/object/craft.h"
#include "xw/flight/object/object.h"
int Dos94_starship_damagecomponent(uint16_t victim, int16_t component, uint16_t damage);
uint16_t Dos94_starship_makestarshipcompexplo(ObjectRecord* object, uint16_t component, uint16_t scale,
											  uint16_t vertex);
void Dos94_starship_createstarshipexplo(uint16_t object);
void Dos94_starship_createstarshipexplo__partial(uint16_t object);
void Dos94_anim_updateanimation(void);
/* Explicit trailing words written by the exhausted DOS ion-disable loop. */
extern uint16_t Dos94_ionExhaustedHealth, Dos94_ionExhaustedRepairTimer;
#endif
