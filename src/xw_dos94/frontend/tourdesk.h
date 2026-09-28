#ifndef XW_DOS94_FRONTEND_TOURDESK_H
#define XW_DOS94_FRONTEND_TOURDESK_H
#include "xw/frontend/tourdesk.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_tourdesk_TourDesk(struct XwShellContext* shellContext);
void Dos94_tourdesk_user_Door(Actor* actor, int time);
void Dos94_tourdesk_iuser_TourDesk(Input* input, int context);
void Dos94_tourdesk_idraw_ActionHint(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_tourdesk_iuser_SelectionButton(Input* input, int context);
void Dos94_tourdesk_idraw_SelectionButton(PushButton* button, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_tourdesk_idraw_TourText(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_tourdesk_CloseMusic(void);
#ifdef __cplusplus
}
#endif
#endif
