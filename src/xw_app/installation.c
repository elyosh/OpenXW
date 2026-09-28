/* Versioned installation ownership follows OpenTIE's installation service. */
#include "xw_app/installation.h"
#include "xw_runtime/storage/file_io.h"
#include <SDL3/SDL_filesystem.h>
#include <aeron/audio_decode.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int ProbeFile(AeronVfs* vfs, const char* path, char* error, size_t capacity) {
	char resolved[XW_PATH_CAPACITY];
	AeronFile* file = XwStorage_OpenAssetVfs(vfs, path, resolved, sizeof resolved);
	if (!file) {
		snprintf(error, capacity, "Required game file '%s' is missing or inaccessible.", path);
		return 0;
	}
	uint8_t byte;
	int success = AeronVfs_GetSize(file) > 0 && XwFile_Read(&byte, 1, 1, file) == 1;
	int close_failed = XwFile_Close(file) != 0;
	if (!success || close_failed) {
		snprintf(error, capacity, "Required game file '%s' is empty or unreadable.", resolved);
		return 0;
	}
	return 1;
}

static int ProbeDirectory(AeronVfs* vfs, const char* path, char* error, size_t capacity) {
	char resolved[XW_PATH_CAPACITY];
	if (XwStorage_ResolveAssetDirectory(vfs, path, resolved, sizeof resolved) == 1)
		return 1;
	snprintf(error, capacity, "Required game directory '%s' is missing or inaccessible.", path);
	return 0;
}

static int ProbeModel(AeronVfs* vfs, char* error, size_t capacity) {
	const char* path = "ivfiles/SPEC640.LST";
	char resolved[XW_PATH_CAPACITY], line[XW_PATH_CAPACITY];
	AeronFile* file = XwStorage_OpenAssetVfs(vfs, path, resolved, sizeof resolved);
	if (!file) {
		snprintf(error, capacity, "Cannot read game file '%s'.", path);
		return 0;
	}
	int success = 0;
	snprintf(error, capacity, "Game file '%s' contains no ship models. Select a complete game installation.",
			 path);
	while (XwFile_Gets(line, sizeof line, file)) {
		size_t length = strcspn(line, "\r\n\x1a");
		if (length == sizeof line - 1) {
			snprintf(error, capacity, "Game file '%s' appears damaged. Select a complete game installation.",
					 path);
			break;
		}
		int end = line[length] == '\x1a';
		line[length] = 0;
		const char* extension = strrchr(line, '.');
		if (extension && strlen(extension) == 4 && tolower((unsigned char)extension[1]) == 'o' &&
			tolower((unsigned char)extension[2]) == 'p' && tolower((unsigned char)extension[3]) == 't') {
			success = ProbeFile(vfs, line, error, capacity);
			break;
		}
		if (end)
			break;
	}
	if (AeronVfs_HasError(file)) {
		snprintf(error, capacity, "Cannot read game file '%s'.", path);
		success = 0;
	}
	if (XwFile_Close(file) != 0) {
		snprintf(error, capacity, "Could not finish reading game file '%s'.", path);
		success = 0;
	}
	return success;
}

static int ProbeInstallation(AeronVfs* vfs, char* error, size_t capacity) {
	static const char* files[] = { "X-Wing Data/RESOURCE/MAINMENU.LFD", "X-Wing Data/RESOURCE/XWING.LFD",
								   "X-Wing Data/RESOURCE/BLAST.LFD",    "X-Wing Data/CP640/XWINGP.PNL",
								   "X-Wing Data/CP640/YWINGP.PNL",      "X-Wing Data/CP640/AWINGP.PNL",
								   "X-Wing Data/CP640/BWINGP.PNL",      "X-Wing Data/CP640/PARTS.PNL",
								   "XwingCD/MUSIC/MAINMENU.WAV",        "XwingCD/TALK/ACKBAR/ACKBAR.P02" };
	static const char* lists[] = { "SPEC", "SPEC2", "SPEC3", "DSTAR" };
	if (!AeronVfs_SetRootOptions(vfs, AERON_VFS_ROOT_ASSET, AERON_VFS_ROOT_OPTION_CASE_INSENSITIVE_LOOKUP))
		return 0;
	for (size_t i = 0; i < sizeof files / sizeof files[0]; ++i)
		if (!ProbeFile(vfs, files[i], error, capacity))
			return 0;
	if (!ProbeDirectory(vfs, "X-Wing Data/MISSION", error, capacity) ||
		!ProbeDirectory(vfs, "X-Wing Data/CLASSIC", error, capacity))
		return 0;
	for (size_t i = 0; i < 4; ++i) {
		for (int resolution = 320; resolution <= 640; resolution += 320) {
			char path[64];
			snprintf(path, sizeof path, "ivfiles/%s%d.LST", lists[i], resolution);
			if (!ProbeFile(vfs, path, error, capacity))
				return 0;
		}
	}
	return ProbeModel(vfs, error, capacity);
}

static int ProbeCdMusic(AeronVfs* vfs) {
	static const int tracks[] = { 2, 3, 7 };
	for (size_t i = 0; i < 3; ++i) {
		char path[32];
		AeronAudioDecoderInfo info;
		snprintf(path, sizeof path, "MUSIC/Track%02d.ogg", tracks[i]);
		AeronAudioDecoder* decoder = Aeron_AudioDecoderOpen(vfs, AERON_VFS_ROOT_ASSET, path);
		if (!decoder)
			return 0;
		Aeron_AudioDecoderGetInfo(decoder, &info);
		Aeron_AudioDecoderClose(decoder);
		if (info.sample_rate <= 0 || info.channels <= 0 || info.duration_us <= 0)
			return 0;
	}
	return 1;
}

static bool NormalizePath(const char* path, char* out, size_t capacity) {
	if (!path || !*path || strlen(path) >= capacity)
		return false;
	SDL_PathInfo info;
	if (SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_FILE) {
		snprintf(out, capacity, "%s", path);
		return true;
	}
	return XwStorage_ResolveInstallation(path, out, capacity) != 0;
}

static bool MountDisc(AeronVfs* vfs, const char* path, char* error, size_t capacity) {
	SDL_PathInfo info;
	if (!SDL_GetPathInfo(path, &info))
		return false;
	char source[XW_PATH_CAPACITY];
	if (info.type == SDL_PATHTYPE_FILE)
		snprintf(source, sizeof source, "%s", path);
	else {
		static const char* const candidates[] = { "game.ins", "game.gog", "game.fil" };
		bool found = false;
		for (size_t i = 0; i < sizeof candidates / sizeof candidates[0]; ++i) {
			int n = snprintf(source, sizeof source, "%s/%s", path, candidates[i]);
			if (n < 0 || (size_t)n >= sizeof source)
				return false;
			if (SDL_GetPathInfo(source, &info) && info.type == SDL_PATHTYPE_FILE) {
				found = true;
				break;
			}
		}
		if (!found)
			return true;
	}
	if (AeronVfs_SetDiscRoot(vfs, AERON_VFS_ROOT_ASSET, source))
		return true;
	snprintf(error, capacity, "Cannot read X-Wing CD image: %s", source);
	return false;
}

static uint32_t ReadDword(const uint8_t* p) {
	return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* Inspect physical records, including duplicate names, without decoding geometry. */
static bool ProbeDos93Species(AeronVfs* vfs, char* error, size_t capacity) {
	char resolved[XW_PATH_CAPACITY];
	AeronFile* file = XwStorage_OpenAssetVfs(vfs, "RESOURCE/SPECIES.LFD", resolved, sizeof resolved);
	uint8_t header[16];
	int32_t length = file ? XwFile_Length(file) : -1;
	bool ok = length >= 16 && XwFile_Read(header, 1, sizeof header, file) == sizeof header &&
			  !memcmp(header, "RMAP", 4) && !(ReadDword(header + 12) & 15) &&
			  ReadDword(header + 12) <= (uint32_t)length - 16;
	uint32_t cursor = ok ? 16 + ReadDword(header + 12) : 0;
	unsigned craft = 0, bitmaps = 0;
	while (ok && cursor < (uint32_t)length) {
		ok = (uint32_t)length - cursor >= sizeof header && !XwFile_Seek(file, (int32_t)cursor, SEEK_SET) &&
			 XwFile_Read(header, 1, sizeof header, file) == sizeof header;
		if (!ok)
			break;
		cursor += sizeof header;
		uint32_t size = ReadDword(header + 12);
		if (!memcmp(header, "CRFT", 4))
			++craft;
		else if (!memcmp(header, "BTMP", 4))
			++bitmaps;
		else
			ok = false;
		ok = ok && size && size <= (uint32_t)length - cursor;
		if (ok)
			cursor += size;
	}
	if (file && XwFile_Close(file))
		ok = false;
	if (ok && craft && bitmaps)
		return true;
	snprintf(error, capacity,
			 "RESOURCE/SPECIES.LFD must contain the original 1993 CRFT/BTMP models. "
			 "Select an extracted X-Wing 1993 B-Wing installation.");
	return false;
}

/* Cockpit definitions own the list of view archives and their panel sprite bank. */
static bool ProbeDos93Cockpit(AeronVfs* vfs, const char* name, char* error, size_t capacity) {
	char path[64], resolved[XW_PATH_CAPACITY];
	snprintf(path, sizeof path, "CP/%s.INT", name);
	AeronFile* file = XwStorage_OpenAssetVfs(vfs, path, resolved, sizeof resolved);
	uint8_t data[1061];
	bool ok =
		file && XwFile_Length(file) == sizeof data && XwFile_Read(data, 1, sizeof data, file) == sizeof data;
	if (file && XwFile_Close(file))
		ok = false;
	if (!ok) {
		snprintf(error, capacity, "%s: missing or invalid cockpit definition.", path);
		return false;
	}
	for (unsigned i = 0; i < 20; ++i) {
		const uint8_t* view = data + 30 * i;
		if (view[0] != 1)
			continue;
		snprintf(path, sizeof path, "CP/%.8s.LFD", (const char*)view + 1);
		if (!ProbeFile(vfs, path, error, capacity))
			return false;
	}
	snprintf(path, sizeof path, "CP/%.8s.PNL", (const char*)data + 1050);
	return ProbeFile(vfs, path, error, capacity) != 0;
}

static bool ValidateDos93(AeronVfs* vfs, char* error, size_t capacity) {
	static const char* const files[] = { "RESOURCE/BWING.CFT", "CP/PARTS.PNL", "CP/CAMERAP.PNL",
										 "CP/CAMERA.LFD",      "CP/FILM.LFD",  "TINY.FNT",
										 "MICRO.FNT",          "VGA.PAC",      "RESOURCE/BMISSION.LFD",
										 "SHIP1.XID",          "SHIP2.XID",    "SHIP3.XID",
										 "SHIP4.XID",          "TOUR1.XID",    "TOUR2.XID",
										 "TOUR3.XID",          "TOUR5.XID" };
	static const char* const cockpits[] = { "XWING", "YWING", "AWING", "BWING" };
	char detail[1024];
	bool ok = ProbeDos93Species(vfs, detail, sizeof detail) &&
			  ProbeDirectory(vfs, "MISSION", detail, sizeof detail);
	for (size_t i = 0; ok && i < sizeof files / sizeof files[0]; ++i)
		ok = ProbeFile(vfs, files[i], detail, sizeof detail) != 0;
	for (size_t i = 0; ok && i < sizeof cockpits / sizeof cockpits[0]; ++i)
		ok = ProbeDos93Cockpit(vfs, cockpits[i], detail, sizeof detail);
	if (!ok)
		snprintf(error, capacity, "X-Wing 1993: %s", detail);
	return ok;
}

static bool ValidateDos94(AeronVfs* vfs, char* error, size_t capacity) {
	static const char* const files[] = { "RESOURCE/MAINMENU.LFD",
										 "RESOURCE/BLAST.LFD",
										 "RESOURCE/SFX.LFD",
										 "RESOURCE/SPEECH.LFD",
										 "RESOURCE/XWING.LFD",
										 "RESOURCE/SPECIES.LFD",
										 "RESOURCE/BWING.CFT",
										 "RESOURCE/ADLIB.LFD",
										 "RESOURCE/ROLAND.LFD",
										 "RESOURCE/GMIDI.LFD",
										 "RESOURCE/LGMUSIC.LFD",
										 "RESOURCE/TLMUSIC.LFD",
										 "RESOURCE/INMUSIC.LFD",
										 "CP/XWINGP.PNL",
										 "CP/YWINGP.PNL",
										 "CP/AWINGP.PNL",
										 "CP/BWINGP.PNL",
										 "TINY.FNT",
										 "MICRO.FNT",
										 "VGA.PAC",
										 "CP/PARTS.PNL",
										 "CP/CAMERAP.PNL",
										 "CP/CAMERA.LFD",
										 "CP/FILM.LFD",
										 "CP/XWING.INT",
										 "CP/YWING.INT",
										 "CP/AWING.INT",
										 "CP/BWING.INT",
										 "RESOURCE/MISSIONS.LFD" };
	char detail[1024];
	for (size_t i = 0; i < sizeof files / sizeof files[0]; ++i)
		if (!ProbeFile(vfs, files[i], detail, sizeof detail)) {
			snprintf(error, capacity, "X-Wing 1994: %s", detail);
			return false;
		}
	if (!ProbeDirectory(vfs, "MISSION", detail, sizeof detail) ||
		!ProbeDirectory(vfs, "CLASSIC", detail, sizeof detail)) {
		snprintf(error, capacity, "X-Wing 1994: %s", detail);
		return false;
	}
	return true;
}

bool XwInstallation_Open(XwInstallation* out, XwGameVersion version, const char* path, char* error,
						 size_t capacity) {
	if (!out || !XwGameVersion_Year(version))
		return false;
	char normalized[XW_PATH_CAPACITY];
	if (!NormalizePath(path, normalized, sizeof normalized)) {
		snprintf(error, capacity, "Select the complete X-Wing %d installation folder.",
				 XwGameVersion_Year(version));
		return false;
	}
	if (version == XW_GAME_VERSION_93) {
		SDL_PathInfo info;
		if (!SDL_GetPathInfo(normalized, &info) || info.type != SDL_PATHTYPE_DIRECTORY) {
			snprintf(error, capacity, "X-Wing 1993 requires an extracted B-Wing installation folder.");
			return false;
		}
	}
	AeronVfs* vfs = AeronVfs_Create(
		&(AeronVfsConfig) { .org_name = "TotallyOpen", .app_name = "OpenXW", .asset_root = normalized });
	if (!vfs) {
		snprintf(error, capacity, "Could not open this game folder. Please try again.");
		return false;
	}
	bool ok = AeronVfs_SetRootOptions(vfs, AERON_VFS_ROOT_ASSET,
									  AERON_VFS_ROOT_OPTION_CASE_INSENSITIVE_LOOKUP) != 0;
	char resolved[XW_PATH_CAPACITY];
	if (ok && version != XW_GAME_VERSION_93) {
		/* Extracted data takes precedence; GOG's DOS installation stores it on disc. */
		AeronFile* signature =
			XwStorage_OpenAssetVfs(vfs, "X-Wing Data/RESOURCE/MAINMENU.LFD", resolved, sizeof resolved);
		if (signature)
			AeronVfs_Close(signature);
		else
			ok = MountDisc(vfs, normalized, error, capacity);
	}
	if (ok && version == XW_GAME_VERSION_93)
		ok = ValidateDos93(vfs, error, capacity);
	else if (ok) {
		bool windows = XwStorage_ResolveAssetDirectory(vfs, "IVFILES", resolved, sizeof resolved) == 1 ||
					   XwStorage_ResolveAssetDirectory(vfs, "CP640", resolved, sizeof resolved) == 1;
		if (windows && version == XW_GAME_VERSION_94) {
			snprintf(error, capacity,
					 "This is X-Wing (1998). Choose your Collector's CD-ROM (1994) folder instead.");
			ok = false;
		} else if (!windows && version == XW_GAME_VERSION_98) {
			snprintf(error, capacity,
					 "This folder does not contain X-Wing (1998). Choose the folder where that version is "
					 "installed.");
			ok = false;
		} else
			ok = version == XW_GAME_VERSION_94 ? ValidateDos94(vfs, error, capacity)
											   : ProbeInstallation(vfs, error, capacity);
	}
	if (!ok) {
		AeronVfs_Destroy(vfs);
		return false;
	}
	*out = (XwInstallation) { .version = version, .vfs = vfs };
	snprintf(out->root, sizeof out->root, "%s", normalized);
	if (capacity)
		error[0] = 0;
	return true;
}

void XwInstallation_Close(XwInstallation* installation) {
	if (!installation)
		return;
	AeronVfs_Destroy(installation->vfs);
	memset(installation, 0, sizeof *installation);
}

void XwInstallation_CloseSet(XwInstallationSet* installations) {
	if (!installations)
		return;
	XwInstallation_Close(&installations->xw93);
	XwInstallation_Close(&installations->xw94);
	XwInstallation_Close(&installations->xw98);
}

const XwInstallation* XwInstallation_Get(const XwInstallationSet* installations, XwGameVersion version) {
	if (!installations)
		return NULL;
	const XwInstallation* item;
	switch (version) {
		case XW_GAME_VERSION_93:
			item = &installations->xw93;
			break;
		case XW_GAME_VERSION_94:
			item = &installations->xw94;
			break;
		case XW_GAME_VERSION_98:
			item = &installations->xw98;
			break;
		default:
			return NULL;
	}
	return item->vfs ? item : NULL;
}

bool XwInstallation_ValidatePath(XwGameVersion version, const char* path, char* resolved,
								 size_t resolved_capacity, char* error, size_t capacity) {
	XwInstallation candidate = { 0 };
	if (!XwInstallation_Open(&candidate, version, path, error, capacity))
		return false;
	bool ok = resolved && strlen(candidate.root) < resolved_capacity;
	if (ok)
		strcpy(resolved, candidate.root);
	else
		snprintf(error, capacity, "Installation path is too long.");
	XwInstallation_Close(&candidate);
	return ok;
}

int XwInstallation_CdMusicAvailable(const XwInstallation* installation) {
	return installation && installation->vfs && installation->version == XW_GAME_VERSION_98 &&
		   ProbeCdMusic(installation->vfs);
}
