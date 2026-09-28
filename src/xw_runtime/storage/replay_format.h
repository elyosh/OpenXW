#ifndef XW_RUNTIME_REPLAY_FORMAT_H
#define XW_RUNTIME_REPLAY_FORMAT_H
#include "xw/flight/hud/msg.h"
#include "xw_runtime/runtime/profile.h"

/* OXWR v2, 24-byte LE header: magic, u16 version, u8 kind, u8 flight,
 * u32 snapshot bytes, u32 input records, u16 seed, u16 record bytes,
 * u16 flags, u16 header bytes. The 13-byte input record appends i16 roll,
 * u8 input flags (bit 0: throttle present), and u16 throttle to the original eight bytes.
 * Flags: classic=1, imported Windows state=2, unlocked=4, laser convergence=8. */
typedef enum XwReplayKind { XW_REPLAY_CHECKPOINT = 1, XW_REPLAY_FILM = 2 } XwReplayKind;

typedef struct XwReplayMetadata {
	bool classic;
	bool laser_convergence;
	XwFlightUpdateRate update_rate;
} XwReplayMetadata;

const char* XwReplayFormat_Error(void);
bool XwReplayFormat_CheckFile(const char* path, XwReplayKind kind, XwGameVersion version,
							  XwReplayMetadata* metadata);
bool XwReplayFormat_SaveCheckpoint(const char* path);
bool XwReplayFormat_LoadCheckpoint(const char* path);
XwFlightMessageId XwReplayFormat_SaveFilm(const char* path, const char* name);
bool XwReplayFormat_LoadFilm(void);
bool XwReplayFormat_RefillInput(void);
bool XwReplayFormat_SaveInputBuffer(void);
bool XwReplayFormat_LoadInputBuffer(void);
bool XwReplayFormat_RequireInputRecords(unsigned count);
#endif
