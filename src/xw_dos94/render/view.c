#include "xw_dos94/render/view.h"
#include "xw/flight/fview.h"
#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw_dos94/render/display.h"
#include "xw_dos94/render/dos93_math.h"
#include "xw_dos94/render/fview.h"
#include <string.h>

int Dos94_trig2_arctan(int32_t first, int32_t second) {
	return Dos94Assets_Version() == XW_GAME_VERSION_93 ? Dos93_trig2_arctan(first, second)
													   : trig2_arctan(first, second);
}

static int16_t narrow_dot(uint32_t value) {
	int16_t high = (int16_t)(value >> 16);
	if (high >= 0x4000)
		high = 0x3FFF;
	if (high <= -0x4000)
		high = -0x3FFF;
	return (int16_t)((uint16_t)high * 2u + ((value >> 15) & 1));
}

/* DOS94 FVIEW_transformaxes: Rodrigues coefficients keep per-product truncation. */
static void transform_axes(int16_t matrix[3][3], const int16_t inputAxis[3], int16_t angle) {
	if (!angle)
		return;
	int16_t axis[3];
	memcpy(axis, inputAxis, sizeof axis);
	int16_t sine = Dos94_trig2_getsignedsin(angle), cosine = Dos94_trig2_getsignedcos(angle);
	int16_t rotation[3][3];
	for (unsigned row = 0; row < 3; ++row)
		for (unsigned col = 0; col < 3; ++col) {
			int32_t product = (int32_t)axis[row] * axis[col];
			int32_t term;
			if (row == col)
				term = cosine;
			else {
				unsigned third = 3 - row - col;
				term = ((int32_t)sine * axis[third]) >> 15;
				if ((row + 1) % 3 != col)
					term = -term;
			}
			uint32_t value = (uint32_t)((product >> 15) * (cosine >= 0 ? 32767 - cosine : -cosine));
			if (cosine < 0)
				value += (uint32_t)product;
			value += (uint32_t)term << 15;
			rotation[row][col] = narrow_dot(value);
		}
	for (unsigned row = 0; row < 3; ++row) {
		int16_t out[3];
		for (unsigned col = 0; col < 3; ++col) {
			int16_t column[3] = { rotation[0][col], rotation[1][col], rotation[2][col] };
			out[col] = Dos94_math2_dot3Q15(matrix[row], column);
		}
		memcpy(matrix[row], out, sizeof out);
	}
}

static void build_move(int16_t matrix[3][3], int16_t pitch, int16_t yaw, ObjectRecord* object) {
	int16_t sy = Dos94_trig2_getsignedsin((uint16_t)-yaw), cy = Dos94_trig2_getsignedcos((uint16_t)-yaw);
	int16_t sp = Dos94_trig2_getsignedsin((uint16_t)(-0x4000 - pitch)),
			cp = Dos94_trig2_getsignedcos((uint16_t)(-0x4000 - pitch));
	matrix[0][0] = cy;
	matrix[0][1] = sy;
	matrix[0][2] = 0;
	matrix[2][0] = (int16_t)(((int32_t)cp * -sy) >> 15);
	matrix[2][1] = (int16_t)(((int32_t)cp * cy) >> 15);
	matrix[2][2] = sp;
	matrix[1][0] = (int16_t)-(((int32_t)sp * sy) >> 15);
	matrix[1][1] = (int16_t)-(((int32_t)sp * -cy) >> 15);
	matrix[1][2] = (int16_t)-cp;
	g_craftMoveX = (int16_t)-matrix[2][0];
	g_craftMoveZ = (int16_t)-matrix[2][1];
	g_craftMoveY = (int16_t)-matrix[2][2];
	if (object) {
		object->moveX = g_craftMoveX;
		object->moveY = g_craftMoveZ;
		object->moveZ = g_craftMoveY;
		object->moveVectorDirty = 0;
	}
}

static void build_axes(int16_t matrix[3][3], int16_t roll, int16_t pitch, int16_t yaw, int16_t offset,
					   ObjectRecord* object) {
	build_move(matrix, pitch, yaw, object);
	transform_axes(matrix, matrix[1], offset);
	transform_axes(matrix, matrix[2], roll);
	if (object) {
		object->cachedSideX = matrix[0][0];
		object->cachedSideY = matrix[0][1];
		object->cachedSideZ = matrix[0][2];
		object->cachedUpX = matrix[1][0];
		object->cachedUpY = matrix[1][1];
		object->cachedUpZ = matrix[1][2];
		object->cachedForwardX = (int16_t)-matrix[2][0];
		object->cachedForwardY = (int16_t)-matrix[2][1];
		object->cachedForwardZ = (int16_t)-matrix[2][2];
		object->orientMatrixDirty = 0;
	}
}

static void publish_axes(const int16_t matrix[3][3]) {
	g_curMatR0_X = matrix[0][0];
	g_curMatR0_Y = matrix[0][1];
	g_curMatR0_Z = matrix[0][2];
	g_curMatR1_X = matrix[1][0];
	g_curMatR1_Y = matrix[1][1];
	g_curMatR1_Z = matrix[1][2];
	g_curMatR2_X = matrix[2][0];
	g_curMatR2_Y = matrix[2][1];
	g_curMatR2_Z = matrix[2][2];
}

/* DOS94 0x683F80. */
void Dos94_fview_newcalcview(int16_t roll, int16_t pitch, int16_t yaw, int16_t offset, int16_t aimX,
							 int16_t aimY, ObjectRecord* object) {
	int16_t matrix[3][3];
	build_axes(matrix, roll, pitch, yaw, offset, object);
	g_fviewSideX_Q15 = matrix[0][0];
	g_fviewSideY_Q15 = matrix[0][1];
	g_fviewSideZ_Q15 = matrix[0][2];
	g_fviewUpX_Q15 = matrix[1][0];
	g_fviewUpY_Q15 = matrix[1][1];
	g_fviewUpZ_Q15 = matrix[1][2];
	g_fviewForwardX_Q15 = (int16_t)-matrix[2][0];
	g_fviewForwardY_Q15 = (int16_t)-matrix[2][1];
	g_fviewForwardZ_Q15 = (int16_t)-matrix[2][2];
	for (unsigned i = 0; i < 3; ++i) {
		matrix[1][i] = (int16_t)-matrix[1][i];
		matrix[2][i] = (int16_t)-matrix[2][i];
	}
	int16_t axis[3];
	memcpy(axis, matrix[1], sizeof axis);
	transform_axes(matrix, matrix[0], aimX);
	transform_axes(matrix, axis, aimY);
	publish_axes(matrix);
	g_camMatR0_X = matrix[0][0];
	g_camMatR0_Y = matrix[0][1];
	g_camMatR0_Z = matrix[0][2];
	g_camMatR1_X = matrix[1][0];
	g_camMatR1_Y = matrix[1][1];
	g_camMatR1_Z = matrix[1][2];
	g_camMatR2_X = matrix[2][0];
	g_camMatR2_Y = matrix[2][1];
	g_camMatR2_Z = matrix[2][2];
	if (Dos94_display)
		memcpy(Dos94_display->draw.camera.matrix, matrix, sizeof matrix);
}

/* DOS94 0x684038. */
void Dos94_fview_newcalcrotate(int16_t roll, int16_t pitch, int16_t yaw, int16_t offset,
							   ObjectRecord* object) {
	int16_t matrix[3][3];
	if (!object || object->orientMatrixDirty)
		build_axes(matrix, roll, pitch, yaw, offset, object);
	else {
		int16_t cached[3][3] = { { object->cachedSideX, object->cachedSideY, object->cachedSideZ },
								 { object->cachedUpX, object->cachedUpY, object->cachedUpZ },
								 { (int16_t)-object->cachedForwardX, (int16_t)-object->cachedForwardY,
								   (int16_t)-object->cachedForwardZ } };
		memcpy(matrix, cached, sizeof matrix);
	}
	g_fviewSideX_Q15 = matrix[0][0];
	g_fviewSideY_Q15 = matrix[0][1];
	g_fviewSideZ_Q15 = matrix[0][2];
	g_fviewUpX_Q15 = matrix[1][0];
	g_fviewUpY_Q15 = matrix[1][1];
	g_fviewUpZ_Q15 = matrix[1][2];
	g_fviewForwardX_Q15 = (int16_t)-matrix[2][0];
	g_fviewForwardY_Q15 = (int16_t)-matrix[2][1];
	g_fviewForwardZ_Q15 = (int16_t)-matrix[2][2];
	publish_axes(matrix);
	Dos94DrawState* d = &Dos94_display->draw;
	for (unsigned i = 0; i < 3; ++i) {
		d->craftBasis[i][0] = matrix[0][i];
		d->craftBasis[i][1] = (int16_t)-matrix[2][i];
		d->craftBasis[i][2] = matrix[1][i];
		int16_t* camera = d->camera.matrix[i];
		d->object.matrix[i][0] = Dos94_math2_dot3Q15(matrix[0], camera);
		d->object.matrix[i][1] = Dos94_math2_dot3Q15(matrix[2], camera);
		d->object.matrix[i][2] = Dos94_math2_dot3Q15(matrix[1], camera);
	}
	g_objViewMat_R0_X = d->object.matrix[0][0];
	g_objViewMat_R0_Y = d->object.matrix[1][0];
	g_objViewMat_R0_Z = d->object.matrix[2][0];
	g_objViewMat_R1_X = d->object.matrix[0][1];
	g_objViewMat_R1_Y = d->object.matrix[1][1];
	g_objViewMat_R1_Z = d->object.matrix[2][1];
	g_objViewMat_R2_X = d->object.matrix[0][2];
	g_objViewMat_R2_Y = d->object.matrix[1][2];
	g_objViewMat_R2_Z = d->object.matrix[2][2];
	int16_t light[3] = { (int16_t)g_modelLightDirectionX, (int16_t)g_modelLightDirectionY,
						 (int16_t)g_modelLightDirectionZ };
	for (unsigned i = 0; i < 3; ++i)
		d->objectLight[i] = g_transformLightDirectionToObjectSpace ? Dos94_math2_dot3Q15(matrix[i == 0   ? 0
																								: i == 1 ? 2
																										 : 1],
																						 light)
																   : light[i];
	memcpy(d->worldLight, light, sizeof light);
	g_objectLightDirectionX = d->objectLight[0];
	g_objectLightDirectionY = d->objectLight[1];
	g_objectLightDirectionZ = d->objectLight[2];
}

/* DOS94 0x6840E6. */
void Dos94_fview_calcrotatemove(int16_t pitch, int16_t yaw, ObjectRecord* object) {
	int16_t matrix[3][3];
	build_move(matrix, pitch, yaw, object);
	publish_axes(matrix);
}

/* DOS94 0x684A60. */
void Dos94_fview_transformaxes(int x, int y, int z, int16_t angle) {
	int16_t matrix[3][3] = { { g_curMatR0_X, g_curMatR0_Y, g_curMatR0_Z },
							 { g_curMatR1_X, g_curMatR1_Y, g_curMatR1_Z },
							 { g_curMatR2_X, g_curMatR2_Y, g_curMatR2_Z } };
	int16_t axis[3] = { (int16_t)x, (int16_t)y, (int16_t)z };
	transform_axes(matrix, axis, angle);
	publish_axes(matrix);
}
