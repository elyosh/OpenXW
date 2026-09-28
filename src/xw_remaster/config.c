/* Requested/effective capability policy adapted from OpenXvT f643323. */
#include "xw_remaster/config.h"
#include "xw_runtime/config/config.h"
#include <aeron/aeron.h>
#include <string.h>

static XwRenderSettings effective;
static uint64_t generation, document_generation;
static int initialized;
static AeronSampler* mesh_sampler;

static int SupportedSamples(int samples) {
	while (samples > 1 && (!Aeron_TextureFormatSupportsSampleCount(AERON_TEXTURE_FORMAT_RGBA16_FLOAT,
																   (AeronSampleCount)samples) ||
						   !Aeron_TextureFormatSupportsSampleCount(AERON_TEXTURE_FORMAT_D32_FLOAT,
																   (AeronSampleCount)samples)))
		samples /= 2;
	return samples;
}

int XwRemasterConfig_Sync(void) {
	const XwSettings* settings = XwConfig_Settings();
	if (!settings)
		return 0;
	if (initialized && document_generation == XwConfig_Generation())
		return 1;
	XwRenderSettings next = settings->render;
	next.msaa_samples = next.temporal_mode == AERON_TEMPORAL_OFF ? SupportedSamples(next.msaa_samples) : 1;
	next.dos_msaa_samples = SupportedSamples(next.dos_msaa_samples);
	if (!initialized || next.anisotropic != effective.anisotropic ||
		next.max_anisotropy != effective.max_anisotropy) {
		AeronSampler* sampler =
			next.anisotropic
				? Aeron_CreateSampler(&(AeronSamplerDesc) { .min_filter = AERON_FILTER_LINEAR,
															.mag_filter = AERON_FILTER_LINEAR,
															.mip_filter = AERON_FILTER_LINEAR,
															.address_u = AERON_ADDRESS_CLAMP_TO_EDGE,
															.address_v = AERON_ADDRESS_CLAMP_TO_EDGE,
															.address_w = AERON_ADDRESS_CLAMP_TO_EDGE,
															.max_lod = 1000,
															.enable_anisotropy = 1,
															.max_anisotropy = next.max_anisotropy })
				: NULL;
		if (next.anisotropic && !sampler)
			return 0;
		Aeron_DestroySampler(mesh_sampler);
		mesh_sampler = sampler;
	}
	if (!initialized || memcmp(&next, &effective, sizeof next))
		++generation;
	effective = next;
	document_generation = XwConfig_Generation();
	initialized = 1;
	return 1;
}

const XwRenderSettings* XwRemasterConfig_Effective(void) { return initialized ? &effective : NULL; }

uint64_t XwRemasterConfig_Generation(void) { return generation; }

AeronSampler* XwRemasterConfig_MeshSampler(void) { return mesh_sampler; }

void XwRemasterConfig_Shutdown(void) {
	Aeron_DestroySampler(mesh_sampler);
	mesh_sampler = NULL;
	memset(&effective, 0, sizeof effective);
	generation = document_generation = 0;
	initialized = 0;
}
