#ifndef XW_RUNTIME_RUNTIME_LEAVE1_TASK_H
#define XW_RUNTIME_RUNTIME_LEAVE1_TASK_H

#include "xw/landru_config.h"
#include <landru/res.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Retain the resource through the view and its cleanup. The shell owner yields
 * until completion, then reads the Landru exit code. */
void XwLeave1_RunView(ResFile* sceneResource, ResFile* yavinResource);

#ifdef __cplusplus
}
#endif
#endif
