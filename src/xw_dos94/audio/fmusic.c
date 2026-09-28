#include "xw_dos94/audio/fmusic.h"
#include "xw_runtime/audio/flight_music.h"
#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/storage/storage.h"
#include <aeron/log.h>
#include <ctype.h>
#include <imuse/xwing.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: DOS94 0x191F6
uint16_t g_dos94MusicTrackCount;

// FUNCTION: DOS94 0x13240
XwMusicSoundId Dos94_fmusic_fmLoadSound(const char* name) {
	if (!name)
		return 0;
	for (unsigned i = 0; i < g_dos94MusicTrackCount; ++i)
		if (!strcmp(g_flightMusicResources[i].name, name))
			return (XwMusicSoundId)(500 + i);
	Aeron_LogError("xw.music", "Missing flight cue: %s", name);
	return 0;
}

// FUNCTION: DOS94 0x132D0
void* Dos94_fmusic_GetPagedSound(uint32_t trackIndex) {
	/* Native allocations retain every catalog entry; DOS used two paging windows. */
	return trackIndex < g_dos94MusicTrackCount ? g_flightMusicResources[trackIndex].data : NULL;
}

// FUNCTION: DOS94 0x1356A
uint16_t Dos94_fmusic_loadmusic(const char* path) {
	uint32_t type = XwMidiBackend_ResourceType();
	AeronFile* file = XwStorage_OpenInstallation(XW_GAME_VERSION_94, path);
	if (!file)
		return 0;
	uint8_t header[16];
	size_t read;
	int count = 0;
	bool valid = true;
	while (AeronVfs_Read(file, header, 16, &read) && read == 16) {
		uint32_t size =
			header[12] | (uint32_t)header[13] << 8 | (uint32_t)header[14] << 16 | (uint32_t)header[15] << 24;
		if (size > 16u * 1024 * 1024) {
			valid = false;
			break;
		}
		if ((((uint32_t)header[0] << 24) | ((uint32_t)header[1] << 16) | ((uint32_t)header[2] << 8) |
			 header[3]) != type) {
			if (!AeronVfs_Seek(file, size, 1)) {
				valid = false;
				break;
			}
			continue;
		}
		if (count >= 125) {
			valid = false;
			break;
		}
		XwFlightMusicResource* c = &g_flightMusicResources[count++];
		c->data = malloc(size);
		c->size = size;
		if (!c->data || !AeronVfs_Read(file, c->data, size, &read) || read != size ||
			!imuse_xwing_validate_sound(c->data, size)) {
			valid = false;
			break;
		}
		for (int j = 0; j < 8; ++j)
			c->name[j] = (char)toupper(header[4 + j]);
	}
	AeronVfs_Close(file);
	g_dos94MusicTrackCount = (uint16_t)count;
	return valid ? g_dos94MusicTrackCount : 0;
}
