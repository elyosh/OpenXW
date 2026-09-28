#include "xw_runtime/snapshot/render_camera.h"
#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/fview.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/xw.h"
#include "xw/math/transfm2.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"
#include "xw/render/renderer.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_objects.h"

#include <math.h>
#include <string.h>

/* OpenXWA's render-only camera shadow. The source tag prevents a temporary
 * or independently written Q15 camera from borrowing another view's basis. */
typedef struct RenderCamera {
	double rows[9];
	int32_t source[9];
	int valid;
} RenderCamera;

static RenderCamera g_camera;

static void CopySource(int32_t rows[9]) {
	const int32_t source[9] = { g_camMatR0_X, g_camMatR0_Y, g_camMatR0_Z, g_camMatR1_X, g_camMatR1_Y,
								g_camMatR1_Z, g_camMatR2_X, g_camMatR2_Y, g_camMatR2_Z };
	memcpy(rows, source, sizeof source);
}

/* Mirror of FVIEW_calcrotatemove, using the same full-turn angle convention. */
static void RotMove(double m[9], int16_t pitch, int16_t yaw) {
	const double scale = 6.283185307179586 / 65536.0;
	double a = (double)(uint16_t)(0xc000 - pitch) * scale;
	double b = (double)(uint16_t)(int16_t)-yaw * scale;
	double cb = cos(b), sb = sin(b), ca = cos(a), sa = sin(a);
	m[0] = cb;
	m[1] = sb;
	m[2] = 0;
	m[3] = -sb * sa;
	m[4] = cb * sa;
	m[5] = -ca;
	m[6] = -sb * ca;
	m[7] = cb * ca;
	m[8] = sa;
}

/* OpenXWA's double-precision mirror of FVIEW_transformaxes. */
static void RotateAxes(double m[9], double ax, double ay, double az, int16_t angle) {
	if (!angle)
		return;
	const double scale = 6.283185307179586 / 65536.0;
	double c = cos((double)(uint16_t)angle * scale);
	double s = sin((double)(uint16_t)angle * scale);
	double omc = 1 - c;
	double r00 = c + omc * ax * ax;
	double r01 = az * s + omc * ay * ax;
	double r02 = -ay * s + omc * az * ax;
	double r10 = -az * s + omc * ay * ax;
	double r11 = c + omc * ay * ay;
	double r12 = ax * s + omc * az * ay;
	double r20 = ay * s + omc * az * ax;
	double r21 = -ax * s + omc * az * ay;
	double r22 = c + omc * az * az;
	for (int i = 0; i < 9; i += 3) {
		double x = m[i], y = m[i + 1], z = m[i + 2];
		m[i] = r20 * z + r10 * y + r00 * x;
		m[i + 1] = r21 * z + r11 * y + r01 * x;
		m[i + 2] = r22 * z + r12 * y + r02 * x;
	}
}

void XwRenderCamera_Build(int16_t roll, int16_t pitch, int16_t yaw, int16_t angle_d, int16_t aim_x,
						  int16_t aim_y) {
	double* m = g_camera.rows;
	RotMove(m, pitch, yaw);
	RotateAxes(m, m[3], m[4], m[5], angle_d);
	RotateAxes(m, m[6], m[7], m[8], roll);
	for (int i = 3; i < 9; ++i)
		m[i] = -m[i];
	/* FVIEW saves the yaw axis before applying the HUD pitch offset. */
	double axis[3] = { m[3], m[4], m[5] };
	RotateAxes(m, m[0], m[1], m[2], aim_x);
	RotateAxes(m, axis[0], axis[1], axis[2], aim_y);
	CopySource(g_camera.source);
	g_camera.valid = 1;
}

void XwRenderCamera_CopyRows(float rows[9]) {
	int32_t source[9];
	CopySource(source);
	int precise = g_camera.valid && !memcmp(source, g_camera.source, sizeof source);
	for (int i = 0; i < 9; ++i)
		rows[i] = precise ? (float)g_camera.rows[i] : source[i] * (1.0f / 32768.0f);
}

bool XwRenderCamera_Capture(XwSnapCamera* out) {
	*out = (XwSnapCamera) { .world_pos = { g_flightCamera.worldPosition.x, g_flightCamera.worldPosition.y,
										   g_flightCamera.worldPosition.z },
							.viewport = { g_flightVpX, g_flightVpY, g_flightVpWidth, g_flightVpHeight },
							.center_x = g_flightVpCenterX,
							.center_y = g_flightVpCenterY,
							.projection_offset_y = g_projOffsetY,
							.focal_x = XwFlightTypes_Dos() ? 256 : g_projScaleInt,
							.aspect_y_q16 = XwFlightTypes_Dos() ? 0xE8BA : g_projAspectY,
							.screen_width = g_flightScreenWidth,
							.screen_height = g_flightScreenHeight,
							.player = XwRenderObjects_Id(g_playerFlightState.objectIndex),
							.focus = XwRenderObjects_Id(g_flightCamera.focusObjectRef),
							.hud_state = g_flightCamera.hudStateLive,
							.replay_mode = g_replayviewmode,
							.external = g_flightCamera.externalViewActive != 0 };
	out->legacy_render_convention = XwFlightTypes_Dos() ? XW_SNAP_CLASSIC_DOS
									: g_useHardware3D   ? XW_SNAP_CLASSIC_WINDOWS_HARDWARE
														: XW_SNAP_CLASSIC_WINDOWS_SOFTWARE;
	XwRenderCamera_CopyRows(out->rows);
	return out->viewport.width > 0 && out->viewport.height > 0 && out->screen_width && out->screen_height;
}

void XwRenderCamera_Appearance(XwSnapAppearance* out) {
	/* The effective DOS palette is published independently at the host boundary. */
	out->direction_q15[0] = g_modelLightDirectionX;
	out->direction_q15[1] = g_modelLightDirectionY;
	out->direction_q15[2] = g_modelLightDirectionZ;
	out->brightness_q8 = g_flightBrightnessScaleQ8;
	out->local_lights_level = g_localLightsLevel;
	out->ship_detail_value = g_shipDetailValue;
	out->ship_detail_polygons = g_shipDetailPolyCount;
	out->starship_detail = g_starshipDetail;
	out->surface_detail_level = g_deathStarDetailLevel;
	out->surface_object_limit = g_surfaceObjectDetailLimit;
	out->trench_object_limit = g_trenchObjectDetailLimit;
	out->target_highlight = g_targetHighlightObjectAndBlinkBits;
	out->graphics_detail = g_flightGraphicsDetailPreset;
	out->directional_enabled =
		(XwFlightTypes_Dos() ? g_transformLightDirectionToObjectSpace : g_dirLightingEnabled) != 0;
	out->gouraud_enabled = (g_gouraudEnableMask & 0x40) != 0;
	out->markings_enabled = g_drawMarkingsFlag != 0;
	out->engine_glow_enabled = g_engineGlowEnabled != 0;
	out->debris_enabled = g_debrisEnabled != 0;
	out->backdrops_enabled = g_backdropsEnabled != 0;
}
