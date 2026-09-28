#ifndef XW_DOS94_FRONTEND_CREDITS_H
#define XW_DOS94_FRONTEND_CREDITS_H
#include "xw/frontend/scenes/credits.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_credits_Credits(struct XwShellContext* shell);
void Dos94_credits_end_View(int time);
int16_t Dos94_credits_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_credits_Credit_Actor_To_Buffer(Actor* actor);
int16_t Dos94_credits_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh);
void Dos94_credits_user_Credit(Actor* actor, int unusedTime);
int16_t Dos94_credits_draw_Credit(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								  int16_t unusedY, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
