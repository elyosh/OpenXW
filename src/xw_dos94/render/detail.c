#include "xw_dos94/render/detail.h"
#include "xw/flight/death_star.h"
#include "xw/flight/player/user.h"
#include <limits.h>

uint16_t Dos94_solidindex;

/* DOS93 0x694770 / DOS94 0x693EB2: the first eleven preset rows are identical. */
void Dos94_user_setdetaillevel(uint16_t preset) {
	static const uint16_t settings[11][4] = {
		{ 4096, 8192, 16384, 32767 },
		{ 1, 2, 3, 4 },
		{ 2, 1, 1, 1 },
		{ 0, 0, 0, 1 },
		{ 0, 0, 1, 1 },
		{ 3, 2, 1, 0 },
		{ 8, 12, 16, 16 },
		{ 0, 1, 1, 1 },
		{ 1, 2, 4, 5 },
		{ 4, 11, 16, 22 },
		{ 16, 32, 44, 60 },
	};
	if (preset >= 4)
		return;
	g_craftExplosionSpawnThreshold = settings[0][preset];
	g_starshipDetail = settings[1][preset];
	g_starDensity = settings[2][preset];
	g_backdropsEnabled = settings[3][preset];
	g_debrisEnabled = settings[4][preset];
	g_shipDetailValue = settings[5][preset];
	g_shipDetailPolyCount = settings[6][preset];
	g_drawMarkingsFlag = settings[7][preset];
	g_deathStarDetailLevel = settings[8][preset];
	g_surfaceObjectDetailLimit = settings[9][preset];
	g_hyperspaceEffectObjectCount = settings[10][preset];
	g_gouraudEnableMask = Dos94Assets_Version() == XW_GAME_VERSION_93 ? 0 : preset >= 2 ? 64 : 0;
	g_trenchObjectDetailLimit = g_surfaceObjectDetailLimit;
	if (g_deathStarSurfaceModeActive) {
		g_backdropsEnabled = 0;
		g_debrisEnabled = 0;
	}
	g_transformLightDirectionToObjectSpace = 1;
}

/* DOS94 0x699BBC: preceding LOD for -1, otherwise bounded face-count skips. */
const Dos94MeshView* Dos94_DRAW_getdetailptr(const Dos94Lod* lods, uint16_t count, int32_t depth) {
	if (!lods || !count)
		return NULL;
	unsigned index = 0;
	bool exclusive = Dos94Assets_Version() == XW_GAME_VERSION_93;
	while (index + 1 < count &&
		   (depth > lods[index].maxDepth || (exclusive && depth == lods[index].maxDepth)))
		++index;
	int16_t detail = g_shipDetailValue;
	if (detail == -1) {
		if (index)
			--index;
	} else
		for (int skip = 0; skip < detail && index + 1 < count; ++skip) {
			const Dos94MeshView* mesh = &lods[index].mesh;
			if (lods[index].maxDepth == INT32_MAX)
				break;
			if (mesh->format != 0x40 && mesh->format != 0x41) {
				const uint8_t* payload = Dos94Assets_Data(mesh->payload);
				if (!payload || mesh->offset + 4u >= mesh->payload.size)
					return NULL;
				if (payload[mesh->offset + 4] <= (uint8_t)g_shipDetailPolyCount)
					break;
			}
			++index;
		}
	return &lods[index].mesh;
}

/* DOS93 0x69BC36 / DOS94 0x699C45: publish the requested identity before
 * the CRFT accessor clamps its geometry selection to the last file component. */
const Dos94Lod* Dos94_DRAW_getcomponentptr(uint16_t model, uint16_t component, uint16_t* count) {
	Dos94_solidindex = component;
	return Dos94Models_ComponentLods(model, component, count);
}
