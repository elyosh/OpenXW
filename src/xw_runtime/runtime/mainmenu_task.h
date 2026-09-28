#ifndef XW_RUNTIME_RUNTIME_MAINMENU_TASK_H
#define XW_RUNTIME_RUNTIME_MAINMENU_TASK_H

#include "xw/landru_config.h"
#include <landru/res.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Retain the resource through the view and its cleanup. The shell owner yields
 * until completion, then reads the Landru exit code. */
void XwMainMenu_RunView(ResFile* resource);

void XwMainMenu_GetDate(char* date, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
