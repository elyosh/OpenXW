#ifndef XW_DOS94_FRONTEND_AWARDS_H
#define XW_DOS94_FRONTEND_AWARDS_H
#include "xw/frontend/award_box.h"
#include "xw/frontend/uniform.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_Uniform_Uniform(struct XwShellContext* shellContext);
void Dos94_Uniform_end_View(int time);
int16_t Dos94_Uniform_film_Callback(Film* film, FilmObject* object);
int16_t Dos94_Uniform_iupdate_Awards(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									 int rightEvent, int16_t x, int16_t y);
void Dos94_Uniform_idraw_Awards(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_Uniform_user_Award(Actor* actor, int time);
void Dos94_Uniform_CloseMusic(void);
XwShellSceneResult Dos94_AwardBox_AwardBox(struct XwShellContext* shell);
void Dos94_AwardBox_end_View(int time);
int16_t Dos94_AwardBox_film_Callback(Film* film, FilmObject* filmObject);
int16_t Dos94_AwardBox_iupdate_Awards(Input* input, Rect* frame, Rect* clip, int16_t key, int leftEvent,
									  int rightEvent, int16_t x, int16_t y);
void Dos94_AwardBox_idraw_Awards(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_AwardBox_user_AwardState(Actor* actor, int time);
#ifdef __cplusplus
}
#endif
#endif
