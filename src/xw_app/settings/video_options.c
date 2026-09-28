/* Host-output apply/rollback follows OpenXvT; game surfaces keep their own lifetime. */
#include "xw_app/settings/video_options.h"
#include <aeron/aeron.h>
#include <stdio.h>

static bool Apply(const XwSettings* settings) {
	const XwPresentationSettings* p = &settings->presentation;
	if (!Aeron_SetFullscreen(settings->fullscreen) || !Aeron_SetPresentationVsyncDivisor(p->vsync_divisor) ||
		!Aeron_SetOutputHdr(p->hdr_output))
		return false;
#ifndef __APPLE__
	Aeron_SetOutputSdrContentGamma(p->sdr_gamma < 0 ? 2.2f : p->sdr_gamma);
	Aeron_SetOutputPaperWhiteNits(p->paper_white_nits);
#endif
	return true;
}

bool XwVideoOptions_Apply(const XwSettings* previous, const XwSettings* requested, char* error,
						  size_t capacity) {
	if (Apply(requested))
		return true;
	if (!Apply(previous)) {
		snprintf(error, capacity, "Cannot restore host display settings.");
		Aeron_RequestFatalRendererError(error);
	} else
		snprintf(error, capacity, "Cannot apply host display settings; previous settings restored.");
	return false;
}
