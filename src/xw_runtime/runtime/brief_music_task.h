#ifndef XW_RUNTIME_RUNTIME_BRIEF_MUSIC_TASK_H
#define XW_RUNTIME_RUNTIME_BRIEF_MUSIC_TASK_H

#include "xw/landru_config.h"

#include <landru/film.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return nonzero when startup is suspended. The briefing owner must yield and retain
 * music resources until the child completes the remainder of OpenMusic. */
int XwBriefMusic_WaitForTick(Film* film);

#ifdef __cplusplus
}
#endif

#endif
