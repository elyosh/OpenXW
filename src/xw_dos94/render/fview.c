#include "xw_dos94/render/fview.h"
#include "xw/math/trig2.h"
#include "xw_runtime/storage/dos94_assets.h"
#include <string.h>

/* DOS94 0x6A740F/0x6A75F7. The interpolated unsigned sine table is shared with
 * Windows; its floating-point signed-sine entry is not used here. */
int16_t Dos94_trig2_getsignedsin(uint16_t angle) {
	uint16_t sample = (uint16_t)trig2_calcsineofangle(angle);
	int16_t value = (int16_t)((sample >> 1) & (sample | 0xFFFE));
	return angle & 0x8000 ? (int16_t)-value : value;
}

int16_t Dos94_trig2_getsignedcos(uint16_t angle) {
	return Dos94_trig2_getsignedsin((uint16_t)(angle + 0x4000));
}

static int16_t clamp_dot(uint32_t sum) {
	int16_t high = (int16_t)(sum >> 16);
	if (high >= 0x4000)
		high = 0x3FFF;
	if (high <= -0x4000)
		high = -0x3FFF;
	/* Saturation replaces the high word only, retaining the rounding bit. */
	return (int16_t)((uint16_t)high * 2u + ((sum >> 15) & 1u));
}

/* DOS94 0x6A867F/0x6A8640. */
int16_t Dos94_math2_dot2Q15(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	return clamp_dot((uint32_t)((int32_t)x1 * x2) + (uint32_t)((int32_t)y1 * y2));
}

int16_t Dos94_math2_dot3Q15(const int16_t a[3], const int16_t b[3]) {
	return clamp_dot((uint32_t)((int32_t)a[0] * b[0]) + (uint32_t)((int32_t)a[1] * b[1]) +
					 (uint32_t)((int32_t)a[2] * b[2]));
}

/* The original backup is single-level and contains no light-vector state. */
static int16_t savedMatrix[3][3];
static Dos94EyePoint savedOrigin;

static void save_rotation(const Dos94Transform* t) {
	memcpy(savedMatrix, t->matrix, sizeof savedMatrix);
	savedOrigin = t->origin;
}

static void rotate_columns(Dos94Transform* t, unsigned second, int16_t sine, int16_t cosine) {
	for (unsigned row = 0; row < 3; ++row) {
		t->matrix[row][0] =
			Dos94_math2_dot2Q15(cosine, (int16_t)-sine, savedMatrix[row][0], savedMatrix[row][second]);
		t->matrix[row][second] =
			Dos94_math2_dot2Q15(sine, cosine, savedMatrix[row][0], savedMatrix[row][second]);
	}
}

/* DOS94 0x684444. */
void Dos94_FVIEW_sfoilrotation(Dos94Transform* t, int16_t angle) {
	save_rotation(t);
	rotate_columns(t, 2, Dos94_trig2_getsignedsin(angle), Dos94_trig2_getsignedcos(angle));
}

/* DOS94 0x684558. */
void Dos94_FVIEW_corvettegunrotation(Dos94Transform* t, int16_t angle) {
	save_rotation(t);
	rotate_columns(t, 1, Dos94_trig2_getsignedsin(angle), Dos94_trig2_getsignedcos(angle));
}

static void translate_pivot(Dos94Transform* t, int16_t x, int16_t z) {
	const int16_t pivot[3] = { x, 0, z };
	int16_t dx = Dos94_math2_dot3Q15(pivot, t->matrix[0]);
	int16_t dy = Dos94_math2_dot3Q15(pivot, t->matrix[1]);
	int16_t dz = Dos94_math2_dot3Q15(pivot, t->matrix[2]);
	t->origin.x = (int32_t)((uint32_t)t->origin.x + (uint32_t)(int32_t)dx);
	t->origin.y = (int32_t)((uint32_t)t->origin.y + (uint32_t)(int32_t)dy);
	t->origin.z = (int32_t)((uint32_t)t->origin.z + (uint32_t)(int32_t)dz);
}

/* DOS94 0x68466C. Component 5 translates without rotating its matrix. */
void Dos94_FVIEW_bwingrotation(Dos94Transform* t, int16_t angle, uint16_t part) {
	save_rotation(t);
	bool dos93 = Dos94Assets_Version() == XW_GAME_VERSION_93;
	if ((uint16_t)angle == 0x4000) {
		int16_t closedX = dos93 ? 95 : 92;
		int16_t pivotX = part == 5 ? -closedX : part == 4 ? closedX : 0;
		int16_t pivotZ = (part == 4 || part == 5) ? (dos93 ? -11 : -8) : 0;
		translate_pivot(t, (int16_t)(pivotZ - 170), (int16_t)(170 - pivotX));
		if (part != 5)
			for (unsigned row = 0; row < 3; ++row) {
				t->matrix[row][0] = (int16_t)-(part == 4 ? savedMatrix[row][0] : savedMatrix[row][2]);
				t->matrix[row][2] = part == 4 ? (int16_t)-savedMatrix[row][2] : savedMatrix[row][0];
			}
		return;
	}
	int16_t sine = Dos94_trig2_getsignedsin(angle), cosine = Dos94_trig2_getsignedcos(angle);
	int16_t pivotZ = dos93 ? 53 : 50;
	int16_t x = 0, z = 0;
	if (part == 5) {
		x = (int16_t)(Dos94_math2_dot2Q15(cosine, (int16_t)-sine, 42, pivotZ) - 42);
		z = (int16_t)(Dos94_math2_dot2Q15(sine, cosine, 42, pivotZ) - pivotZ);
	} else if (part == 4) {
		x = (int16_t)(Dos94_math2_dot2Q15(cosine, sine, -42, pivotZ) + 42);
		z = (int16_t)(Dos94_math2_dot2Q15((int16_t)-sine, cosine, -42, pivotZ) - pivotZ);
	}
	z = (int16_t)(z - 170);
	int16_t rotatedX = Dos94_math2_dot2Q15(cosine, sine, x, z);
	int16_t rotatedZ = Dos94_math2_dot2Q15((int16_t)-sine, cosine, x, z);
	translate_pivot(t, rotatedX, (int16_t)(rotatedZ + 170));
	if (part == 5)
		return;
	if (part == 4) {
		sine = Dos94_trig2_getsignedsin((uint16_t)(2 * (uint16_t)angle));
		cosine = Dos94_trig2_getsignedcos((uint16_t)(2 * (uint16_t)angle));
	}
	rotate_columns(t, 2, sine, cosine);
}

/* DOS94 0x6849FE. */
void Dos94_FVIEW_restorerotation(Dos94Transform* t) {
	memcpy(t->matrix, savedMatrix, sizeof savedMatrix);
	t->origin = savedOrigin;
}
