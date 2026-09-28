#ifndef XW_RUNTIME_FLIGHT_PANEL_H
#define XW_RUNTIME_FLIGHT_PANEL_H
#include <stdint.h>

/* Only the cockpit metrics that differ between DOS and Windows flight. */
typedef struct XwFlightPanelLayout {
	int16_t width;
	int16_t height;
	int16_t titleHeight;
	int16_t footerBottom;
	int16_t statusBottom;
	int16_t distanceLeft;
	int16_t distanceRight;
	int16_t cargoLeft;
	int16_t cargoTop;
	int16_t warheadLeft;
	int16_t warheadTop;
	int16_t reticleTop;
	int16_t detailLeft;
	int16_t variantLeft;
	int16_t variantTop;
	int16_t lockLeft;
	int16_t lockTop;
	int16_t radarCenterX;
	int16_t radarCenterY;
	int16_t radarYLimit;
	int16_t distanceWholeX;
	int16_t distanceFractionX;
	int16_t distanceY;
	int16_t clockSecondsX;
	int16_t redirectStepY;
	int16_t engineStepY;
} XwFlightPanelLayout;

const XwFlightPanelLayout* XwFlightPanel_Layout(void);
#endif
