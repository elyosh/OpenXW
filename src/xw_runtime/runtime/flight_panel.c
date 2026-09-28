#include "xw_runtime/runtime/flight_panel.h"
#include "xw/flight/hud/panel.h"
#include "xw_runtime/runtime/flight_types.h"

/* Fixed flight layouts, independent of the selected frontend resolution. */
const XwFlightPanelLayout* XwFlightPanel_Layout(void) {
	static const XwFlightPanelLayout windows = {
		.width = PANEL_CMD_WIDTH,
		.height = PANEL_CMD_HEIGHT,
		.titleHeight = PANEL_CMD_TITLE_HEIGHT,
		.footerBottom = PANEL_CMD_FOOTER_BOTTOM,
		.statusBottom = PANEL_CMD_STATUS_BOTTOM,
		.distanceLeft = PANEL_CMD_DISTANCE_LEFT,
		.distanceRight = PANEL_CMD_DISTANCE_RIGHT,
		.cargoLeft = PANEL_CMD_CARGO_LEFT,
		.cargoTop = PANEL_CMD_CARGO_TOP,
		.warheadLeft = PANEL_CMD_WARHEAD_LEFT,
		.warheadTop = PANEL_CMD_WARHEAD_TOP,
		.reticleTop = PANEL_CMD_RETICLE_TOP,
		.detailLeft = PANEL_CMD_DETAIL_LEFT,
		.variantLeft = PANEL_CMD_VARIANT_LEFT,
		.variantTop = PANEL_CMD_VARIANT_TOP,
		.lockLeft = PANEL_CMD_LOCK_LEFT,
		.lockTop = PANEL_CMD_LOCK_TOP,
		.radarCenterX = PANEL_RADAR_TARGET_CENTER_X,
		.radarCenterY = PANEL_RADAR_TARGET_CENTER_Y,
		.radarYLimit = PANEL_RADAR_TARGET_Y_LIMIT,
		.distanceWholeX = PANEL_DISTANCE_WHOLE_X,
		.distanceFractionX = PANEL_DISTANCE_FRACTION_X,
		.distanceY = PANEL_DISTANCE_Y,
		.clockSecondsX = PANEL_CLOCK_SECONDS_X_OFFSET,
		.redirectStepY = PANEL_REDIRECT_POWER_Y_STEP,
		.engineStepY = PANEL_ENGINE_POWER_Y_STEP,
	};
	static const XwFlightPanelLayout dos = {
		.width = 66,
		.height = 38,
		.titleHeight = 6,
		.footerBottom = 43,
		.statusBottom = 44,
		.distanceLeft = 12,
		.distanceRight = 32,
		.cargoLeft = 14,
		.cargoTop = 32,
		.warheadLeft = 13,
		.warheadTop = 32,
		.reticleTop = 5,
		.detailLeft = 34,
		.variantLeft = 57,
		.variantTop = 21,
		.lockLeft = 22,
		.lockTop = 15,
		.radarCenterX = 33,
		.radarCenterY = 19,
		.radarYLimit = 13,
		.distanceWholeX = 12,
		.distanceFractionX = 22,
		.distanceY = 38,
		.clockSecondsX = 10,
		.redirectStepY = 4,
		.engineStepY = 2,
	};
	if (XwFlightTypes_Dos())
		return &dos;
	return &windows;
}
