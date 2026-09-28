#ifndef XW_RUNTIME_INTEGRATION_TRAIN_CALLBACKS_H
#define XW_RUNTIME_INTEGRATION_TRAIN_CALLBACKS_H
#include "xw/landru_config.h"
#include <landru/input.h>
#ifdef __cplusplus
extern "C" {
#endif
void XwTrain_DrawNavigationButton(Input* input, Rect* frame, Rect* clip, int16_t refresh);
#ifdef __cplusplus
}
#endif
#endif
