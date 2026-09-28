#include "xw_runtime/audio/frontend_music.h"
#include "xw/audio/soundext.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/audio/music_policy.h"
#include "xw_runtime/storage/storage.h"
#include <aeron/log.h>
#include <ctype.h>
#include <landru/fourcc.h>
#include <stdio.h>
#include <string.h>

static uint32_t little32(const uint8_t* p) {
	return p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool name_equal(const uint8_t* entry, const char* name) {
	for (int i = 0; i < 8; ++i) {
		if (tolower(entry[i]) != tolower((unsigned char)name[i]))
			return false;
		if (!entry[i])
			return true;
	}
	return name[8] == 0;
}

Sound* XwFrontendMusic_Load(ResFile* resource, const char* name) {
	if (!XwMusicPolicy_UsesImuse())
		return resource ? xsound_Res_Sound_Data(resource, FOURCC_GMID, name, gmidiSound) : NULL;
	if (!resource || !name || strlen(name) > 8)
		return NULL;
	const char* base = resource->filename;
	for (const char* p = base; *p; ++p)
		if (*p == '/' || *p == '\\')
			base = p + 1;
	char path[128];
	snprintf(path, sizeof path, "RESOURCE/%s", base);
	AeronFile* f = XwStorage_OpenInstallation(XW_GAME_VERSION_94, path);
	if (!f) {
		Aeron_LogError("xw.music", "Missing music archive %s", path);
		return NULL;
	}
	uint8_t header[16];
	size_t read;
	Sound* sound = NULL;
	while (AeronVfs_Read(f, header, sizeof header, &read) && read == sizeof header) {
		uint32_t size = little32(header + 12);
		if (size > 16u * 1024 * 1024)
			break;
		if ((((uint32_t)header[0] << 24) | ((uint32_t)header[1] << 16) | ((uint32_t)header[2] << 8) |
			 header[3]) == XwMidiBackend_ResourceType() &&
			name_equal(header + 4, name)) {
			LandruHandle handle = xmemhdl_Alloc_Handle(size, LANDRU_MEMORY_RESOURCE);
			if (!handle)
				break;
			void* bytes = xmemhdl_Lock_Handle(handle);
			bool ok = AeronVfs_Read(f, bytes, size, &read) && read == size &&
					  imuse_xwing_validate_sound(bytes, size);
			xmemhdl_Unlock_Handle(handle);
			if (ok)
				sound = xsound_Alloc_Sound(handle, 0, size);
			if (!sound)
				xmemhdl_Free_Handle(handle);
			else {
				xsound_Set_Sound_Name(sound, XwMidiBackend_ResourceType(), name);
				sound->type = gmidiSound;
				sound->id = (intptr_t)sound;
				xsound_Discard_Sound_Data(sound);
			}
			break;
		}
		if (!AeronVfs_Seek(f, size, 1))
			break;
	}
	AeronVfs_Close(f);
	if (!sound)
		Aeron_LogError("xw.music", "Missing or invalid selected music %s/%s", path, name);
	return sound;
}

void XwFrontendMusic_Release(Sound* sound) {
	imuse_t* im = g_dos94Imuse;
	if (!im || !sound)
		return;
	imuse_forget_sound(im, (intptr_t)sound);
	if (g_soundTriggerContext.sound == sound) {
		g_soundTriggerContext.sound = NULL;
		g_soundTriggerContext.marker = 0;
	}
}
