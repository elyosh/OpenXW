#ifndef XW_RUNTIME_RUNTIME_TOURDESK_VIEW_TASK_H
#define XW_RUNTIME_RUNTIME_TOURDESK_VIEW_TASK_H

#include "xw/landru_config.h"
#include <landru/res.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Retain the resource through the view and its cleanup. The shell owner yields
 * until completion, then reads the Landru exit code. */
void XwTourDesk_RunView(ResFile* deskResource, ResFile* extraTextResource);

#ifdef __cplusplus
}
#endif
#endif
