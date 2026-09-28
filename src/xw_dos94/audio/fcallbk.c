#include "xw_dos94/audio/fcallbk.h"
#include "xw_dos94/audio/fscript.h"
#include "xw_dos94/audio/hilevel.h"
#include "xw_runtime/audio/imuse_host.h"
#include "xw_runtime/audio/imuse_session.h"

// FUNCTION: DOS94 0x1276A
void Dos94_fcallbk_CbDoCallback(void) {
	XwMusicSoundId next = g_musicSequenceSoundId ? g_musicSequenceSoundId : g_musicNextSoundId;
	if (!g_musicEventDispatchGate || !next)
		return;
	Dos94_hilevel_ImStartMusic(next);
	imuse_share_parts(g_dos94Imuse, g_musicCurrentSoundId, next);
	for (intptr_t id = 500; id < 625; ++id)
		imuse_clear_trigger(g_dos94Imuse, id, -1, -1);
	ImuseCmd command = { .opcode = (intptr_t)XwImuseHost_MusicMarker };
	imuse_set_trigger(g_dos94Imuse, next, 1, &command);
	g_musicPreviousSoundId = g_musicCurrentSoundId;
	g_musicCurrentSoundId = next;
	if (g_musicSequenceSoundId) {
		g_musicSequenceSoundId = 0;
		g_musicCurrentSequence = 9;
	} else
		g_musicNextSoundId = 0;
}
