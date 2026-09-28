#ifndef XW_RUNTIME_PREFERENCE_APPLY_H
#define XW_RUNTIME_PREFERENCE_APPLY_H
#include <stdbool.h>
void XwPreferences_InitRuntime(void);
void XwPreferences_LoadRuntime(void);
void XwPreferences_ApplyPending(void);
void XwPreferences_BeginMission(void);
void XwPreferences_RestoreFlightResolution(void);
bool XwPreferences_RendererPending(int resolution);
/* At an input-frame boundary; true consumes a replay-compatible options event. */
bool XwPreferences_ApplyGameplay(void);
void XwPreferences_RecordReplayOptions(void);
#endif
