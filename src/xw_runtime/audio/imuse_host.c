/* Native callbacks: sized MIDI resources, logging and output rendering. */
#include "xw_runtime/audio/imuse_host.h"
#include "xw/landru_config.h"
#include "xw_dos94/audio/fcallbk.h"
#include "xw_dos94/audio/fmusic.h"
#include "xw_dos94/audio/gamesnd.h"
#include "xw_runtime/audio/flight_music.h"
#include "xw_runtime/audio/imuse_session.h"
#include <aeron/log.h>
#include <landru/sound.h>

imuse_t* g_dos94Imuse;

static size_t sound_size(intptr_t id) {
	if (id >= 500 && id < 625)
		return g_flightMusicResources[id - 500].size;
	for (Sound* s = xsound_Ask_Sound_List(); s; s = s->next)
		if ((intptr_t)s == id)
			return s->size;
	return 0;
}

static void log_message(void* user, ImuseLogLevel level, const char* message) {
	(void)user;
	static const AeronLogLevel levels[] = { AERON_LOG_TRACE, AERON_LOG_INFO, AERON_LOG_WARN,
											AERON_LOG_ERROR };
	Aeron_LogMessage(levels[(unsigned)level < 4 ? level : 3], "xw.imuse", "%s", message);
}

const ImuseHost g_xwImuseHost = { .getSoundPtrFunc = Dos94_gamesnd_GetSoundAddr,
								  .getSoundSizeFunc = sound_size,
								  .logFunc = log_message };

void XwImuseHost_Render(void* user, int16_t* frames, size_t count) { imuse_mix_s16(user, frames, count); }

void XwImuseHost_MusicMarker(int marker, intptr_t a0, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4,
							 intptr_t a5, intptr_t a6, intptr_t a7, intptr_t a8, intptr_t a9) {
	(void)marker;
	(void)a0;
	(void)a1;
	(void)a2;
	(void)a3;
	(void)a4;
	(void)a5;
	(void)a6;
	(void)a7;
	(void)a8;
	(void)a9;
	Dos94_fcallbk_CbDoCallback();
}
