/* Soundtrack selection boundary follows OpenTIE's music policy. */
#include "xw_runtime/audio/music_policy.h"
#include "xw/audio/cdaudio.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/midi_backend.h"
#include "xw_runtime/storage/storage.h"
#include <aeron/compat/host.h>
#include <aeron/log.h>

static bool cd_available;

bool XwMusicPolicy_UsesImuse(void) { return XwMidiBackend_UsesImuse(); }

void XwMusicPolicy_Init(bool available) {
	cd_available = available;
	const AeronWinmmCdAudioDesc cd = { XwStorage_InstallationVfs(XW_GAME_VERSION_98), AERON_VFS_ROOT_ASSET,
									   "MUSIC" };
	if (!AeronWinmm_ConfigureCdAudio(&cd))
		XwMusicPolicy_DisableCd("Cannot configure the CD music directory");
}

bool XwMusicPolicy_CdAvailable(void) { return cd_available && !XwMusicPolicy_UsesImuse(); }

void XwMusicPolicy_DisableCd(const char* reason) {
	if (cd_available)
		Aeron_LogWarn("xw.music", "%s; CD music disabled for this session", reason);
	cd_available = false;
}

int XwMusicPolicy_FailCd(const char* message) {
	CDAudio_CloseDevice();
	g_musicCdTrackCount = 0;
	XwMusicPolicy_DisableCd(message);
	return 0;
}
