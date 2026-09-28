#ifndef XW_DOS94_FRONTEND_RECOVERY_SCENES_H
#define XW_DOS94_FRONTEND_RECOVERY_SCENES_H
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_Med640_Play(struct XwShellContext* shell);
int16_t Dos94_Med640_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Med640_film_Actor_To_Background(Actor* actor);
int16_t Dos94_Med640_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									 int16_t refresh);
void Dos94_Med640_CloseMusic(void);
XwShellSceneResult Dos94_Tort640_Play(struct XwShellContext* shell);
int16_t Dos94_Tort640_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Tort640_film_Actor_To_Background(Actor* actor);
int16_t Dos94_Tort640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh);
void Dos94_Tort640_CloseMusic(void);
XwShellSceneResult Dos94_Rescue64_Play(struct XwShellContext* shell);
XwShellSceneResult Dos94_Death640_Play(struct XwShellContext* shell);
void Dos94_Death640_end_View(int time);
int16_t Dos94_Death640_film_InteriorCallback(Film* film, FilmObject* object);
int16_t Dos94_Death640_film_Actor_To_Background(Actor* actor);
int16_t Dos94_Death640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									   int16_t unusedX, int16_t unusedY, int16_t refresh);
void Dos94_Death640_CloseMusic(void);
void Dos94_DsBay(XwShellContext* shell);
#ifdef __cplusplus
}
#endif
#endif
