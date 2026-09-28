#ifndef XW_RUNTIME_RUNTIME_TITLE_TASK_H
#define XW_RUNTIME_RUNTIME_TITLE_TASK_H

#include "xw/landru_config.h"

#include <landru/sound.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Push a child wait when one instance remains. The scene owner must yield
 * after calling title_CloseMusic, keeping its sounds alive until resumption. */
void XwTitle_WaitForMusic(Sound* sound);

#ifdef __cplusplus
}
#endif

#endif
