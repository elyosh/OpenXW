#ifdef XW_MODERN
#include "xw_dos94/audio/fsfx.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/player_engine.h"
#endif
#include "xw/audio/fsfx.h"
#include "xw/audio/lolevel.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw_dos94/assets/models.h"
#include "xw_dos94/flight/hyperspace.h"

/* DOS94 0x695CFE: the two star endpoints are encoded signed words. */
static void set_endpoints(uint16_t end) {
	Dos94MeshView mesh;
	if (!Dos94Models_Hyperstar(&mesh))
		return;
	uint8_t* data = Dos94Assets_Data(mesh.payload);
	unsigned first = mesh.vertices + 2, second = first + 6;
	data[first] = 0x10;
	data[first + 1] = 0x7E;
	data[second] = (uint8_t)end;
	data[second + 1] = (uint8_t)(end >> 8);
}

void Dos94Hyperspace_Init(void) {
	g_hyperspaceStreakLength = 0x7E00;
	set_endpoints(0x7E00);
}

void Dos94Hyperspace_Restore(void) {
	uint16_t end = (uint16_t)g_hyperspaceStreakLength;
	if ((g_hyperspaceflag == ANIM_HYPERSPACE_DEPART &&
		 g_playerHyperspaceElapsedTicks >= ANIM_HYPERSPACE_DEPART_MOVE_TICK) ||
		g_hyperspaceflag == ANIM_HYPERSPACE_EXTERNAL ||
		(g_hyperspaceflag == ANIM_HYPERSPACE_RETURN &&
		 g_playerHyperspaceElapsedTicks < ANIM_HYPERSPACE_RETURN_FADE_TICK))
		end = 0x8200;
	set_endpoints(end);
}

void Dos94Hyperspace_Streak(bool returning) {
	if (!returning && g_playerHyperspaceElapsedTicks >= ANIM_HYPERSPACE_DEPART_MOVE_TICK) {
		/* This phase fixes the geometry without writing the saved length word. */
		set_endpoints(0x8200);
		return;
	}
	Dos94MeshView mesh;
	uint16_t end;
	if (!Dos94Models_Hyperstar(&mesh) || !Dos94Models_ReadWord(mesh.payload, mesh.vertices + 8, &end))
		return;
	uint16_t delta = (uint16_t)(ANIM_HYPERSPACE_STEP * g_elapsedTicks);
	end = returning ? (uint16_t)(end + delta) : (uint16_t)(end - delta);
	/* Skip the reserved encoded-coordinate interval, including backlink words. */
	if (end > 0x7E00 && end < 0x8200)
		end = 0x8200;
	g_hyperspaceStreakLength = end;
	set_endpoints(end);
}

void Dos94Hyperspace_SfoilSound(void) {
	if (g_flightAudioMode &&
		imuse_get_param(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT, IMUSE_PARAM_SOUND_PLAY_COUNT) <= 0)
		fsfx_triggersfx(FSFX_SFOIL_MOVEMENT_SLOT, FSFX_UNPOSITIONED_OBJECT);
}
