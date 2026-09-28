#ifndef XW_RUNTIME_STORAGE_REPLAY_SNAPSHOT_H
#define XW_RUNTIME_STORAGE_REPLAY_SNAPSHOT_H

#include "xw/assets/file.h"
#include "xw_runtime/runtime/profile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XwReplayLayout { XW_REPLAY_LAYOUT_LEGACY, XW_REPLAY_LAYOUT_CURRENT } XwReplayLayout;

/* New files share the scalar codec; references are table/slot IDs. */
size_t XwReplaySnapshot_Size(XwGameVersion version, XwReplayLayout layout);
/* Capture/apply require the matching active flight; Check also works before activation. */
bool XwReplaySnapshot_Capture(uint8_t* bytes, size_t size, XwGameVersion version);
bool XwReplaySnapshot_Check(uint8_t* bytes, size_t size, XwGameVersion version, XwReplayLayout layout,
							XwFlightUpdateRate rate);
bool XwReplaySnapshot_Apply(uint8_t* bytes, size_t size, XwGameVersion version, XwReplayLayout layout,
							XwFlightUpdateRate rate);

#ifdef __cplusplus
}
#endif

#endif
