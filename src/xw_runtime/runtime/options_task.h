#ifndef XW_RUNTIME_RUNTIME_OPTIONS_TASK_H
#define XW_RUNTIME_RUNTIME_OPTIONS_TASK_H

#include "xw/landru_config.h"
#include <landru/dialog.h>
#include <landru/pal.h>

#ifdef __cplusplus
extern "C" {
#endif

void XwOptions_ScheduleDialog(Input* dialog, Palette* savedPalette, uint8_t savedMasterVolume,
							  int16_t savedKeyButtons, int savedDoublePixels, DialogSubResultHandler complete,
							  void* context);

void XwOptions_RequestSettings(DialogSubResultHandler complete, void* context);
void XwOptions_SettingsClosed(void);
void XwOptions_CancelSettings(void);
void XwOptions_SavePreviousExit(int16_t previousExit);
void XwOptions_AfterCalibration(int16_t result, void* context);
void XwOptions_SaveAndFinish(int16_t accepted, void* context);

#ifdef __cplusplus
}
#endif
#endif
