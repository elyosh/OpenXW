#ifndef XW_REMASTER_FLIGHT_SCENE_H
#define XW_REMASTER_FLIGHT_SCENE_H
#include "xw_remaster/flight.h"

typedef struct XwFlightOutput {
	AeronTexture* texture; /* Borrowed SDR-linear/scRGB result; NULL for direct-only output. */
	XwSnapViewKey key;
	int width, height;
	bool direct;
	uint64_t revision;           /* Advances only after a newly rendered output submits. */
	AeronColorSpace color_space; /* DOS display-authored color bypasses cinematic tonemapping. */
} XwFlightOutput;

/* Prepare resident GPU infrastructure before admitting the first flight frame. */
bool XwFlightScene_PrepareResources(uint8_t version, int width, int height);
bool XwFlightScene_Frame(const XwRenderSnapshot* snapshot, const XwPreparedFlight* frame, bool direct);
const XwFlightOutput* XwFlightScene_Output(void);
/* Keep GPU storage for an idle/resize handoff, but revoke its current-view identity. */
void XwFlightScene_Invalidate(void);
void XwFlightScene_Shutdown(void);
#endif
