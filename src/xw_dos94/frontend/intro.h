#ifndef XW_DOS94_FRONTEND_INTRO_H
#define XW_DOS94_FRONTEND_INTRO_H
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell.h"

#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_Logo640_Play(struct XwShellContext* shell);
void Dos94_Logo640_end_View(int time);
int16_t Dos94_Logo640_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Logo640_StampBackground(Actor* actor);
int16_t Dos94_Logo640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh);
XwShellSceneResult Dos94_Intro2_Attack(struct XwShellContext* shell);
void Dos94_Intro2_end_View(int time);
void Dos94_Intro2_user_Music(Sound* sound, int time);
XwShellSceneResult Dos94_Brdg1_Play(struct XwShellContext* shell);
void Dos94_Brdg1_end_View(int unusedTime);
int16_t Dos94_Brdg1_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Brdg1_StampBackground(Actor* actor);
int16_t Dos94_Brdg1_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
									int16_t unusedY, int16_t refresh);
XwShellSceneResult Dos94_Brdg2_Play(struct XwShellContext* shell);
void Dos94_Brdg2_end_View(int unusedTime);
int16_t Dos94_Brdg2_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Brdg2_StampBackground(Actor* actor);
int16_t Dos94_Brdg2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
									int16_t unusedY, int16_t refresh);
void Dos94_Intro1_Opening(XwShellContext* shell);
void Dos94_Intro1_ReleaseBackground(void);
XwShellSceneResult Dos94_title_Title(struct XwShellContext* shell);
void Dos94_title_end_View(int unusedTime);
int16_t Dos94_title_draw_Back(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							  int16_t unusedY, int16_t refresh);
void Dos94_title_user_Music(Sound* sound, int time);
XwShellSceneResult Dos94_XLogo_XLogo(struct XwShellContext* shellContext);
void Dos94_XLogo_end_View(int time);
#ifdef __cplusplus
}
#endif
#endif
