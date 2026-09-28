#ifndef XW_RUNTIME_RUNTIME_HANGAR3_TASK_H
#define XW_RUNTIME_RUNTIME_HANGAR3_TASK_H

#include "xw/landru_config.h"

#include <landru/sound.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Wait until exactly one transition instance is active. The scene owner must
 * yield after Hangar3_CloseMusic and retain its resources until resumption. */
void XwHangar3_WaitForTransition(Sound* sound);

#ifdef __cplusplus
}
#endif

#endif
