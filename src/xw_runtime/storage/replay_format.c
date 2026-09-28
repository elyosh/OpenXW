#include "xw_runtime/storage/replay_format.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/replay/replayio.h"
#include "xw/flight/xw.h"
#include "xw_dos94/flight/hyperspace.h"
#include "xw_dos94/render/world.h"
#include "xw_runtime/input/flight_controls.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/storage/file_io.h"
#include "xw_runtime/storage/replay_snapshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	HEADER_SIZE = 24,
	FORMAT_VERSION = 2,
	FLAG_CLASSIC = 1,
	FLAG_LEGACY_STATE = 2,
	FLAG_UNLOCKED = 4,
	FLAG_LASER_CONVERGENCE = 8,
	MAX_INPUT_BYTES = UINT16_MAX * 1024u
};

typedef struct ReplayFile {
	uint8_t* allocation;
	uint8_t* snapshot;
	uint8_t* input;
	size_t snapshotSize;
	uint32_t records;
	uint16_t seed, flags, recordBytes;
	XwReplayLayout layout;
	XwFlightUpdateRate rate;
	XwGameVersion version;
} ReplayFile;

static char lastError[256];

const char* XwReplayFormat_Error(void) { return lastError; }

static bool fail(const char* path, const char* reason) {
	snprintf(lastError, sizeof lastError, "%.100s: %s", path, reason);
	return false;
}

bool XwReplayFormat_RequireInputRecords(unsigned count) {
	return (g_ReplayPlaybackFrameIndex <= g_ReplayFrameCount &&
			count <= g_ReplayFrameCount - g_ReplayPlaybackFrameIndex) ||
		   fail("Flight recording", "truncated input or options records");
}

static uint16_t word(const uint8_t* p) { return p[0] | (uint16_t)p[1] << 8; }

static uint32_t dword(const uint8_t* p) { return word(p) | (uint32_t)word(p + 2) << 16; }

static void putword(uint8_t* p, uint16_t value) {
	p[0] = (uint8_t)value;
	p[1] = (uint8_t)(value >> 8);
}

static void putdword(uint8_t* p, uint32_t value) {
	putword(p, (uint16_t)value);
	putword(p + 2, (uint16_t)(value >> 16));
}

static XwGameVersion flight_version(void) {
	return XwProfile_HasActiveFlight() ? XwProfile_ActiveFlight()->version
									   : XwProfile_MissionFlight()->version;
}

static void header(uint8_t* bytes, XwReplayKind kind, const ReplayFile* file) {
	memcpy(bytes, "OXWR", 4);
	putword(bytes + 4, FORMAT_VERSION);
	bytes[6] = (uint8_t)kind;
	bytes[7] = (uint8_t)(XwGameVersion_Year(file->version) % 100);
	putdword(bytes + 8, (uint32_t)file->snapshotSize);
	putdword(bytes + 12, file->records);
	putword(bytes + 16, file->seed);
	putword(bytes + 18, kind == XW_REPLAY_FILM ? REPLAY_INPUT_RECORD_SIZE : 0);
	putword(bytes + 20, file->flags);
	putword(bytes + 22, HEADER_SIZE);
}

static bool decode(const char* path, uint8_t* bytes, size_t size, XwReplayKind kind, XwGameVersion expected,
				   ReplayFile* file) {
	size_t offset;
	if (size >= 4 && !memcmp(bytes, "OXWR", 4)) {
		if (size < HEADER_SIZE)
			return fail(path, "truncated recording header");
		file->recordBytes = REPLAY_INPUT_RECORD_SIZE;
		if (word(bytes + 4) != FORMAT_VERSION || bytes[6] != kind || word(bytes + 22) != HEADER_SIZE ||
			word(bytes + 18) != (kind == XW_REPLAY_FILM ? REPLAY_INPUT_RECORD_SIZE : 0) ||
			(word(bytes + 20) & ~(FLAG_CLASSIC | FLAG_LEGACY_STATE | FLAG_UNLOCKED | FLAG_LASER_CONVERGENCE)))
			return fail(path, "unsupported or invalid recording header");
		switch (bytes[7]) {
			case 93:
				file->version = XW_GAME_VERSION_93;
				break;
			case 94:
				file->version = XW_GAME_VERSION_94;
				break;
			case 98:
				file->version = XW_GAME_VERSION_98;
				break;
			default:
				return fail(path, "unsupported flight version in recording");
		}
		file->snapshotSize = dword(bytes + 8);
		file->records = dword(bytes + 12);
		file->seed = word(bytes + 16);
		file->flags = word(bytes + 20);
		if ((file->flags & FLAG_LEGACY_STATE) && (file->flags & FLAG_UNLOCKED))
			return fail(path, "Windows recordings require native timing");
		if ((file->flags & FLAG_LEGACY_STATE) && (file->flags & FLAG_LASER_CONVERGENCE))
			return fail(path, "Windows recordings require parallel cannon fire");
		file->layout = file->flags & FLAG_LEGACY_STATE ? XW_REPLAY_LAYOUT_LEGACY : XW_REPLAY_LAYOUT_CURRENT;
		file->rate =
			file->flags & FLAG_UNLOCKED ? XW_FLIGHT_UPDATE_RATE_UNLOCKED : XW_FLIGHT_UPDATE_RATE_NATIVE;
		offset = HEADER_SIZE;
	} else {
		/* Headerless Windows recordings have no flight identity. */
		file->version = XW_GAME_VERSION_98;
		file->recordBytes = REPLAY_LEGACY_INPUT_RECORD_SIZE;
		file->layout = XW_REPLAY_LAYOUT_LEGACY;
		file->rate = XW_FLIGHT_UPDATE_RATE_NATIVE;
		file->flags = FLAG_LEGACY_STATE | (XwProfile_MissionClassic() ? FLAG_CLASSIC : 0);
		file->snapshotSize = XwReplaySnapshot_Size(XW_GAME_VERSION_98, XW_REPLAY_LAYOUT_LEGACY);
		offset = kind == XW_REPLAY_FILM ? 6 : 0;
		if (size < offset)
			return fail(path, "truncated recording");
		file->records = kind == XW_REPLAY_FILM ? dword(bytes) : 0;
		file->seed = kind == XW_REPLAY_FILM ? word(bytes + 4) : 0;
	}
	if (file->version != expected) {
		char detail[96];
		int year = XwGameVersion_Year(file->version);
		snprintf(detail, sizeof detail, "recorded for X-Wing %d; select xw%d flight to open it", year,
				 year % 100);
		return fail(path, detail);
	}
	bool legacy = (file->flags & FLAG_LEGACY_STATE) != 0;
	if ((legacy && file->version != XW_GAME_VERSION_98) ||
		file->snapshotSize != XwReplaySnapshot_Size(file->version, file->layout) ||
		file->records > MAX_INPUT_BYTES / file->recordBytes ||
		(kind == XW_REPLAY_CHECKPOINT && (file->records || file->seed)) ||
		(kind == XW_REPLAY_FILM && !file->records) ||
		size != offset + file->snapshotSize + (size_t)file->records * file->recordBytes)
		return fail(path, "invalid recording length or state layout");
	file->snapshot = bytes + offset;
	file->input = file->snapshot + file->snapshotSize;
	if (!XwReplaySnapshot_Check(file->snapshot, file->snapshotSize, file->version, file->layout, file->rate))
		return fail(path, "invalid checkpoint state or references");
	return true;
}

static void expand_legacy_input(uint8_t* bytes, size_t records) {
	for (size_t i = records; i-- > 0;) {
		uint8_t* dest = bytes + i * REPLAY_INPUT_RECORD_SIZE;
		memmove(dest, bytes + i * REPLAY_LEGACY_INPUT_RECORD_SIZE, REPLAY_LEGACY_INPUT_RECORD_SIZE);
		memset(dest + REPLAY_LEGACY_INPUT_RECORD_SIZE, 0,
			   REPLAY_INPUT_RECORD_SIZE - REPLAY_LEGACY_INPUT_RECORD_SIZE);
	}
}

static bool read_file(const char* path, XwReplayKind kind, XwGameVersion version, bool readInput,
					  ReplayFile* file) {
	memset(file, 0, sizeof *file);
	AeronFile* stream = XwStorage_Open(path, "rb");
	int32_t length = stream ? XwFile_Length(stream) : -1;
	size_t maximum = HEADER_SIZE + XwReplaySnapshot_Size(XW_GAME_VERSION_98, XW_REPLAY_LAYOUT_CURRENT) +
					 (kind == XW_REPLAY_FILM ? MAX_INPUT_BYTES : 0);
	if (length <= 0 || (size_t)length > maximum) {
		if (stream)
			XwFile_Close(stream);
		return fail(path, "missing, empty or oversized recording");
	}
	size_t bytesToRead = (size_t)length;
	size_t stateLimit = HEADER_SIZE + XwReplaySnapshot_Size(XW_GAME_VERSION_98, XW_REPLAY_LAYOUT_CURRENT);
	if (!readInput && bytesToRead > stateLimit)
		bytesToRead = stateLimit;
	uint8_t* bytes = malloc(bytesToRead);
	bool ok = bytes && XwFile_Read(bytes, 1, bytesToRead, stream) == bytesToRead;
	if (XwFile_Close(stream))
		ok = false;
	if (!ok) {
		free(bytes);
		return fail(path, "cannot read complete recording");
	}
	if (!decode(path, bytes, (size_t)length, kind, version, file)) {
		free(bytes);
		return false;
	}
	if (readInput && kind == XW_REPLAY_FILM && file->recordBytes != REPLAY_INPUT_RECORD_SIZE) {
		size_t snapshotOffset = (size_t)(file->snapshot - bytes);
		size_t inputOffset = (size_t)(file->input - bytes);
		uint8_t* expanded = realloc(bytes, inputOffset + (size_t)file->records * REPLAY_INPUT_RECORD_SIZE);
		if (!expanded) {
			free(bytes);
			return fail(path, "cannot expand legacy flight input");
		}
		bytes = expanded;
		file->snapshot = bytes + snapshotOffset;
		file->input = bytes + inputOffset;
		expand_legacy_input(file->input, file->records);
	}
	file->allocation = bytes;
	return true;
}

bool XwReplayFormat_CheckFile(const char* path, XwReplayKind kind, XwGameVersion version,
							  XwReplayMetadata* metadata) {
	ReplayFile file;
	if (!read_file(path, kind, version, false, &file))
		return false;
	if (metadata)
		*metadata = (XwReplayMetadata) { .classic = (file.flags & FLAG_CLASSIC) != 0,
										 .laser_convergence = (file.flags & FLAG_LASER_CONVERGENCE) != 0,
										 .update_rate = file.rate };
	free(file.allocation);
	return true;
}

bool XwReplayFormat_SaveCheckpoint(const char* path) {
	ReplayFile file = {
		.version = flight_version(),
		.layout = XW_REPLAY_LAYOUT_CURRENT,
		.recordBytes = REPLAY_INPUT_RECORD_SIZE,
		.rate = XwProfile_ActiveFlightRate(),
		.flags = (XwProfile_MissionClassic() ? FLAG_CLASSIC : 0) |
				 (XwProfile_ActiveFlightRate() == XW_FLIGHT_UPDATE_RATE_UNLOCKED ? FLAG_UNLOCKED : 0) |
				 (XwProfile_MissionLaserConvergence() ? FLAG_LASER_CONVERGENCE : 0)
	};
	file.snapshotSize = XwReplaySnapshot_Size(file.version, file.layout);
	if (!file.snapshotSize)
		return fail(path, "checkpoint support is unavailable for the selected flight version");
	uint8_t* bytes = malloc(HEADER_SIZE + file.snapshotSize);
	if (!bytes)
		return fail(path, "cannot allocate checkpoint");
	header(bytes, XW_REPLAY_CHECKPOINT, &file);
	bool ok = XwReplaySnapshot_Capture(bytes + HEADER_SIZE, file.snapshotSize, file.version) &&
			  XwStorage_WriteAtomic(path, bytes, HEADER_SIZE + file.snapshotSize);
	free(bytes);
	if (!ok)
		return fail(path, "cannot save checkpoint");
	msg_clearmessagequeue();
	return true;
}

bool XwReplayFormat_LoadCheckpoint(const char* path) {
	ReplayFile file;
	if (!read_file(path, XW_REPLAY_CHECKPOINT, flight_version(), true, &file))
		return false;
	if (((file.flags & FLAG_CLASSIC) != 0) != XwProfile_MissionClassic()) {
		free(file.allocation);
		return fail(path, "checkpoint uses a different mission content selection");
	}
	bool ok = XwReplaySnapshot_Apply(file.snapshot, file.snapshotSize, file.version, file.layout, file.rate);
	if (ok)
		ok = XwProfile_RestoreMissionLaserConvergence((file.flags & FLAG_LASER_CONVERGENCE) != 0);
	free(file.allocation);
	if (!ok)
		return fail(path, "cannot restore checkpoint");
	XwFlightControls_ResetThrottle();
	g_textureCacheFlushPending = 1;
	if (XwGameVersion_IsDos(file.version)) {
		Dos94Renderer_InvalidateAssets();
		Dos94Hyperspace_Restore();
	}
	msg_clearmessagequeue();
	return true;
}

XwFlightMessageId XwReplayFormat_SaveFilm(const char* path, const char* name) {
	ReplayFile start;
	if (!read_file(g_ReplayStartFilename, XW_REPLAY_CHECKPOINT, flight_version(), true, &start))
		return XW_MSG_FILE_ERROR;
	start.records = g_ReplayFrameCount;
	start.seed = g_ReplayRandomSeed;
	size_t inputSize = (size_t)start.records * REPLAY_INPUT_RECORD_SIZE;
	bool ok = start.records && start.records <= MAX_INPUT_BYTES / REPLAY_INPUT_RECORD_SIZE &&
			  (g_ReplaySpoolEnabled || start.records <= REPLAY_REFILL_RECORD_COUNT);
	size_t size = HEADER_SIZE + start.snapshotSize + inputSize;
	uint8_t* bytes = ok ? malloc(size) : NULL;
	if (bytes) {
		header(bytes, XW_REPLAY_FILM, &start);
		memcpy(bytes + HEADER_SIZE, start.snapshot, start.snapshotSize);
		uint8_t* input = bytes + HEADER_SIZE + start.snapshotSize;
		if (g_ReplaySpoolEnabled) {
			AeronFile* stream = XwStorage_Open("+input.spl", "rb");
			ok = stream && XwFile_Read(input, 1, inputSize, stream) == inputSize;
			if (stream && XwFile_Close(stream))
				ok = false;
		} else
			memcpy(input, g_ReplayBufferStart, inputSize);
		if (ok)
			ok = XwStorage_WriteAtomic(path, bytes, size);
	} else
		ok = false;
	free(bytes);
	free(start.allocation);
	if (!ok) {
		fail(path, "cannot save complete film");
		return XW_MSG_FILE_ERROR;
	}
	strcpy(g_ReplayClipName, name);
	return XW_MSG_REPLAY_CLIP_SAVED;
}

bool XwReplayFormat_LoadFilm(void) {
	char path[REPLAY_CLIP_PATH_CAPACITY];
	snprintf(path, sizeof path, "%s.clp", g_ReplayClipName);
	ReplayFile film;
	if (!read_file(path, XW_REPLAY_FILM, flight_version(), true, &film))
		return false;
	if (((film.flags & FLAG_CLASSIC) != 0) != XwProfile_MissionClassic()) {
		free(film.allocation);
		return fail(path, "film uses a different mission content selection");
	}
	ReplayFile checkpoint = film;
	checkpoint.records = 0;
	checkpoint.seed = 0;
	uint8_t* bytes = malloc(HEADER_SIZE + film.snapshotSize);
	bool ok = bytes != NULL;
	if (ok) {
		header(bytes, XW_REPLAY_CHECKPOINT, &checkpoint);
		memcpy(bytes + HEADER_SIZE, film.snapshot, film.snapshotSize);
		ok = XwStorage_WriteAtomic(g_ReplayStartFilename, bytes, HEADER_SIZE + film.snapshotSize);
	}
	free(bytes);
	/* Spool offsets remain relative to raw input, independently of the film header. */
	if (ok && g_ReplaySpoolEnabled)
		ok = XwStorage_WriteAtomic("+input.spl", film.input, (size_t)film.records * REPLAY_INPUT_RECORD_SIZE);
	if (ok) {
		g_ReplayFrameCount = film.records;
		if (!g_ReplaySpoolEnabled) {
			if (g_ReplayFrameCount > REPLAY_REFILL_RECORD_COUNT)
				g_ReplayFrameCount = REPLAY_REFILL_RECORD_COUNT;
			memcpy(g_ReplayBufferStart, film.input, (size_t)g_ReplayFrameCount * REPLAY_INPUT_RECORD_SIZE);
		}
		g_ReplayRandomSeed = film.seed;
	}
	free(film.allocation);
	return ok || fail(path, "cannot prepare film playback");
}

bool XwReplayFormat_RefillInput(void) {
	if (g_ReplayPlaybackFrameIndex > g_ReplayFrameCount)
		return fail("input.spl", "invalid playback position");
	uint32_t count = g_ReplayFrameCount - g_ReplayPlaybackFrameIndex;
	if (count > REPLAY_REFILL_RECORD_COUNT)
		count = REPLAY_REFILL_RECORD_COUNT;
	uint8_t bytes[REPLAY_BUFFER_CAPACITY] = { 0 };
	AeronFile* file = XwStorage_Open("+input.spl", "rb");
	bool ok =
		file && g_ReplayPlaybackFrameIndex <= INT32_MAX / REPLAY_INPUT_RECORD_SIZE &&
		!XwFile_Seek(file, (int32_t)(g_ReplayPlaybackFrameIndex * REPLAY_INPUT_RECORD_SIZE), SEEK_SET) &&
		XwFile_Read(bytes, REPLAY_INPUT_RECORD_SIZE, count, file) == count;
	if (file && XwFile_Close(file))
		ok = false;
	if (!ok)
		return fail("input.spl", "cannot read complete input records");
	memcpy(g_ReplayBufferStart, bytes, sizeof bytes);
	return true;
}

bool XwReplayFormat_SaveInputBuffer(void) {
	return XwStorage_WriteAtomic("+rpybuff.tmp", g_ReplayBufferStart, REPLAY_BUFFER_CAPACITY) != 0;
}

bool XwReplayFormat_LoadInputBuffer(void) {
	uint8_t bytes[REPLAY_BUFFER_CAPACITY];
	AeronFile* file = XwStorage_Open("+rpybuff.tmp", "rb");
	bool ok = file && XwFile_Length(file) == REPLAY_BUFFER_CAPACITY &&
			  XwFile_Read(bytes, 1, sizeof bytes, file) == sizeof bytes;
	if (file && XwFile_Close(file))
		ok = false;
	if (ok)
		memcpy(g_ReplayBufferStart, bytes, sizeof bytes);
	return ok;
}
