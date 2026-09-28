#ifndef XW_DOS94_PANEL_H
#define XW_DOS94_PANEL_H
#include "xw/flight/hud/panel.h"
#include <stdbool.h>
void Dos94_panel_loadpaneldata(void);
void Dos94_panel_forcenewviewdir(uint16_t);
void Dos94_panel_dosetnewpilotview(uint16_t);
void Dos94_panel_updatethrottle(void);
bool Dos94_panel_getradareye(uint16_t objectRef, int32_t* x, int32_t* y, int32_t* depth);
void Dos94_math2_getradarcoord(int32_t x, int32_t y, uint32_t depth);
void Dos94Panel_Free(void);
void Dos94Panel_Replay(bool standalone);
void Dos94Panel_RestoreCraftSprites(void);
#endif
