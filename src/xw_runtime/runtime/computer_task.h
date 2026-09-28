#ifndef XW_RUNTIME_RUNTIME_COMPUTER_TASK_H
#define XW_RUNTIME_RUNTIME_COMPUTER_TASK_H

#include "xw/landru_config.h"

#include <landru/dialog.h>

#ifdef __cplusplus
extern "C" {
#endif

void XwComputer_ApplyBindingReset(int16_t accepted, void* context);

/* Owns the confirmation inputs until dismissal or cancellation. */
void XwComputer_ScheduleBindingResetDialog(Input* dialog, DialogSubResultHandler complete, void* context);

#ifdef __cplusplus
}
#endif

#endif
