#ifndef XW_RUNTIME_RUNTIME_HANGAR3_VIEW_TASK_H
#define XW_RUNTIME_RUNTIME_HANGAR3_VIEW_TASK_H
#include "xw/landru_config.h"
#include <landru/res.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Retain the resource through view and music cleanup. The shell owner yields
 * until completion, then reads the Landru exit code. */
void XwHangar3_RunView(ResFile* resourceFile, int manageSceneAudio);
#ifdef __cplusplus
}
#endif
#endif
