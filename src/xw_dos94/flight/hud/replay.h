#ifndef XW_DOS94_REPLAY_H
#define XW_DOS94_REPLAY_H
#include <stdbool.h>
#include <stdint.h>
extern char Dos94_panelStatusStrings[11][9];
void Dos94_replay_drawreplaybutton(uint16_t button);
void Dos94_replay_outputclipname(void);
void Dos94Replay_StatusBounds(bool tracked);
void Dos94Replay_ProgressBounds(void);
#endif
