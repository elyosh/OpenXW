#ifndef XW_DOS94_FRONTEND_PILOT_LOG_H
#define XW_DOS94_FRONTEND_PILOT_LOG_H
#include "xw/frontend/pilot_log.h"
#ifdef __cplusplus
extern "C" {
#endif
XwShellSceneResult Dos94_PilotLog_Show(struct XwShellContext* shell);
void Dos94_PilotLog_idraw_Page(Input* unusedInput, Rect* frame, Rect* unusedClip, int16_t refresh);
void Dos94_PilotLog_DrawTrainingPage(const Rect* frame);
void Dos94_PilotLog_DrawHistoricPage(const Rect* frame);
void Dos94_PilotLog_DrawBonusPage(const Rect* frame);
void Dos94_PilotLog_DrawCombatStatistics(const Rect* frame);
void Dos94_PilotLog_DrawTourPage(const Rect* frame);
void Dos94_PilotLog_iuser_Navigation(Input* input, int unusedTime);
void Dos94_PilotLog_idraw_Navigation(Input* input, Rect* frame, Rect* clip, int16_t refresh);
void Dos94_PilotLog_CloseMusic(void);
#ifdef __cplusplus
}
#endif
#endif
