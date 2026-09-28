#ifndef XW_DOS94_FRONTEND_CEREMONY_H
#define XW_DOS94_FRONTEND_CEREMONY_H
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_Flet640_Play(struct XwShellContext* shell);
int16_t Dos94_Flet640_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Flet640_film_Actor_To_Background(Actor* actor);
int16_t Dos94_Flet640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh);
void Dos94_Flet640_CloseMusic(void);
XwShellSceneResult Dos94_Awards640_Play(struct XwShellContext* shell);
int16_t Dos94_Awards640_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Awards640_ActorToScreenAndBackground(Actor* actor);
int16_t Dos94_Awards640_draw_Background(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
										int16_t unusedY, int16_t refresh);
void Dos94_Awards640_CloseMusic(void);
#ifdef __cplusplus
}
#endif
#endif
