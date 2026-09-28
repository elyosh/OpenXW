#ifndef XW_DOS94_FRONTEND_HANGAR3_H
#define XW_DOS94_FRONTEND_HANGAR3_H
#include "xw/frontend/scenes/hangar3.h"
#ifdef __cplusplus
extern "C" {
#endif
int16_t Dos94_Hangar3_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Hangar3_StampBackground(Actor* actor);
int16_t Dos94_Hangar3_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh);
void Dos94_Hangar3(XwShellContext* shell);
#ifdef __cplusplus
}
#endif
#endif
