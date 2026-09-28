/* Pure X-Wing transforms from FVIEW_sfoilrotation/corvettegunrotation/bwingrotation.
 * Pivot displacements are in world units; mesh half/enlarged scaling is applied later. */
#include "xw_remaster/dos_ship.h"
#include <math.h>

static void Translate(float pose[16], float x, float z) {
	for (unsigned row = 0; row < 3; ++row)
		pose[4 * row + 3] += pose[4 * row] * x + pose[4 * row + 2] * z;
}

static void Rotate(float pose[16], unsigned second, float sine, float cosine) {
	for (unsigned row = 0; row < 3; ++row) {
		float x = pose[4 * row], y = pose[4 * row + second];
		pose[4 * row] = cosine * x - sine * y;
		pose[4 * row + second] = sine * x + cosine * y;
	}
}

void XwDosShip_Articulate(float pose[16], unsigned model, unsigned part, float rotation, uint8_t version) {
	if (!rotation || (model != 1 && model != 15 && model != 118))
		return;
	float angle = rotation * (6.283185307179586f / 256.0f);
	float sine = sinf(angle), cosine = cosf(angle);
	if (model != 118) {
		Rotate(pose, model == 1 ? 2 : 1, sine, cosine);
		return;
	}
	if (rotation == 64) {
		float closed_x = version == 93 ? 95 : 92;
		float x = part == 5 ? -closed_x : part == 4 ? closed_x : 0;
		float z = part == 4 || part == 5 ? (version == 93 ? -11 : -8) : 0;
		Translate(pose, z - 170, 170 - x);
		if (part != 5)
			Rotate(pose, 2, part == 4 ? 0 : 1, part == 4 ? -1 : 0);
		return;
	}
	float pivot_z = version == 93 ? 53 : 50, x = 0, z = 0;
	if (part == 5) {
		x = cosine * 42 - sine * pivot_z - 42;
		z = sine * 42 + cosine * pivot_z - pivot_z;
	} else if (part == 4) {
		x = cosine * -42 + sine * pivot_z + 42;
		z = sine * 42 + cosine * pivot_z - pivot_z;
	}
	z -= 170;
	Translate(pose, cosine * x + sine * z, -sine * x + cosine * z + 170);
	if (part == 5)
		return;
	if (part == 4) {
		sine = sinf(2 * angle);
		cosine = cosf(2 * angle);
	}
	Rotate(pose, 2, sine, cosine);
}
