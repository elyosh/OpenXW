#include "xw/flight/fview.h"
#ifdef XW_MODERN
#include "xw_dos94/render/view.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/snapshot/render_camera.h"
#endif

#include "xw/flight/object/object.h"
#include "xw/flight/player/user.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"

#include <stdlib.h>

// GLOBAL: XW 0x62B5E8
int g_fviewSideY_Q15 = 0;

// GLOBAL: XW 0x62B5EC
int g_fviewSideX_Q15 = 0;

// GLOBAL: XW 0x62B5F0
int g_craftMoveZ = 0;

// GLOBAL: XW 0x62B5F4
int g_craftMoveY = 0;

// GLOBAL: XW 0x62B5F8
int g_fviewSideZ_Q15 = 0;

// GLOBAL: XW 0x62B600
int g_craftMoveX = 0;

// GLOBAL: XW 0x62B6B8
int g_fviewUpY_Q15 = 0;

// GLOBAL: XW 0x62B6BC
int g_fviewUpX_Q15 = 0;

// GLOBAL: XW 0x62B6C4
int g_fviewUpZ_Q15 = 0;

// GLOBAL: XW 0x62B920
int g_fviewForwardZ_Q15 = 0;

// GLOBAL: XW 0x62B924
int g_fviewForwardY_Q15 = 0;

// GLOBAL: XW 0x62B928
int g_fviewForwardX_Q15 = 0;

// GLOBAL: XW 0x62B92C
int g_modelLightDirectionX = 0;

// GLOBAL: XW 0x62B934
int g_modelLightDirectionY = 0;

// GLOBAL: XW 0x62B940
int g_modelLightDirectionZ = 0;

// GLOBAL: XW 0x62BA98
int g_objViewMat_R2_X = 0;

// GLOBAL: XW 0x62BA9C
int g_objViewMat_R1_Z = 0;

// GLOBAL: XW 0x62BAA0
int g_objViewMat_R1_Y = 0;

// GLOBAL: XW 0x62BAA4
int g_objViewMat_R2_Z = 0;

// GLOBAL: XW 0x62BAA8
int g_objViewMat_R2_Y = 0;

// GLOBAL: XW 0x62BAAC
int g_objViewMat_R0_X = 0;

// GLOBAL: XW 0x62BAB0
int g_objViewMat_R1_X = 0;

// GLOBAL: XW 0x62BAB4
int g_objViewMat_R0_Z = 0;

// GLOBAL: XW 0x62BAB8
int g_objViewMat_R0_Y = 0;

// GLOBAL: XW 0x62D130
int g_curMatR0_Y = 0;

// GLOBAL: XW 0x62D134
int g_curMatR0_Z = 0;

// GLOBAL: XW 0x62D13C
int g_curMatR0_X = 0;

// GLOBAL: XW 0x62D144
int g_curMatR1_Y = 0;

// GLOBAL: XW 0x62D148
int g_curMatR1_Z = 0;

// GLOBAL: XW 0x62D150
int g_curMatR1_X = 0;

// GLOBAL: XW 0x637328
int g_curMatR2_X = 0;

// GLOBAL: XW 0x63732C
int g_curMatR2_Y = 0;

// GLOBAL: XW 0x637334
int g_curMatR2_Z = 0;

// GLOBAL: XW 0x63736C
int g_objectLightDirectionZ = 0;

// GLOBAL: XW 0x637384
int g_objectLightDirectionX = 0;

// GLOBAL: XW 0x637388
int g_objectLightDirectionY = 0;

// FUNCTION: XW 0x40D100
void fview_newcalcview(int16_t viewRoll, int16_t viewPitch, int16_t viewYaw, int16_t viewAngleD,
					   int16_t hudAimX, int16_t hudAimY, struct ObjectRecord* objRecord) {
	int savedAimAxisX_Q15;
	int savedAimAxisY_Q15;
	int savedAimAxisZ_Q15;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_fview_newcalcview(viewRoll, viewPitch, viewYaw, viewAngleD, hudAimX, hudAimY, objRecord);
		XwRenderCamera_Build(viewRoll, viewPitch, viewYaw, viewAngleD, hudAimX, hudAimY);
		return;
	}
#endif
	fview_calcrotatemove(viewPitch, viewYaw, objRecord);
	fview_calcrotateorient(viewRoll, viewAngleD, objRecord);
#ifdef XW_MODERN
	g_curMatR2_X = (int32_t)(0u - (uint32_t)g_curMatR2_X);
#else
	g_curMatR2_X = -g_curMatR2_X;
#endif
#ifdef XW_MODERN
	g_curMatR2_Y = (int32_t)(0u - (uint32_t)g_curMatR2_Y);
#else
	g_curMatR2_Y = -g_curMatR2_Y;
#endif
#ifdef XW_MODERN
	g_curMatR2_Z = (int32_t)(0u - (uint32_t)g_curMatR2_Z);
#else
	g_curMatR2_Z = -g_curMatR2_Z;
#endif
#ifdef XW_MODERN
	g_curMatR1_X = (int32_t)(0u - (uint32_t)g_curMatR1_X);
#else
	g_curMatR1_X = -g_curMatR1_X;
#endif
#ifdef XW_MODERN
	g_curMatR1_Y = (int32_t)(0u - (uint32_t)g_curMatR1_Y);
#else
	g_curMatR1_Y = -g_curMatR1_Y;
#endif
#ifdef XW_MODERN
	g_curMatR1_Z = (int32_t)(0u - (uint32_t)g_curMatR1_Z);
#else
	g_curMatR1_Z = -g_curMatR1_Z;
#endif
	savedAimAxisX_Q15 = g_curMatR1_X;
	savedAimAxisY_Q15 = g_curMatR1_Y;
	savedAimAxisZ_Q15 = g_curMatR1_Z;
	fview_transformaxes(g_curMatR0_X, g_curMatR0_Y, g_curMatR0_Z, hudAimX);
	fview_transformaxes(savedAimAxisX_Q15, savedAimAxisY_Q15, savedAimAxisZ_Q15, hudAimY);
	g_camMatR0_X = g_curMatR0_X;
	g_camMatR0_Y = g_curMatR0_Y;
	g_camMatR0_Z = g_curMatR0_Z;
	g_camMatR1_X = g_curMatR1_X;
	g_camMatR1_Y = g_curMatR1_Y;
	g_camMatR1_Z = g_curMatR1_Z;
	g_camMatR2_X = g_curMatR2_X;
	g_camMatR2_Y = g_curMatR2_Y;
	g_camMatR2_Z = g_curMatR2_Z;
#ifdef XW_MODERN
	XwRenderCamera_Build(viewRoll, viewPitch, viewYaw, viewAngleD, hudAimX, hudAimY);
#endif
}

// FUNCTION: XW 0x40D220
void fview_newcalcrotate(int16_t roll, int16_t pitch, int16_t yaw, int16_t rollOffset,
						 struct ObjectRecord* object) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_fview_newcalcrotate(roll, pitch, yaw, rollOffset, object);
		return;
	}
#endif
	if (object == NULL) {
		fview_calcrotatemove(pitch, yaw, NULL);
		fview_calcrotateorient(roll, rollOffset, NULL);
	} else if (object->orientMatrixDirty != 0) {
		fview_calcrotatemove(pitch, yaw, object);
		fview_calcrotateorient(roll, rollOffset, object);
	} else {
		g_fviewForwardX_Q15 = object->cachedForwardX;
		g_fviewForwardY_Q15 = object->cachedForwardY;
		g_fviewForwardZ_Q15 = object->cachedForwardZ;
		g_fviewSideX_Q15 = object->cachedSideX;
		g_fviewSideY_Q15 = object->cachedSideY;
		g_fviewSideZ_Q15 = object->cachedSideZ;
		g_fviewUpX_Q15 = object->cachedUpX;
		g_fviewUpY_Q15 = object->cachedUpY;
		g_fviewUpZ_Q15 = object->cachedUpZ;
		g_curMatR2_X = -g_fviewForwardX_Q15;
		g_curMatR2_Y = -g_fviewForwardY_Q15;
		g_curMatR2_Z = -g_fviewForwardZ_Q15;
		g_curMatR0_X = g_fviewSideX_Q15;
		g_curMatR0_Y = g_fviewSideY_Q15;
		g_curMatR0_Z = g_fviewSideZ_Q15;
		g_curMatR1_X = g_fviewUpX_Q15;
		g_curMatR1_Y = g_fviewUpY_Q15;
		g_curMatR1_Z = g_fviewUpZ_Q15;
	}
	fview_calcrotworldeye();
}

// FUNCTION: XW 0x40D340
void fview_calcrotatemove(int16_t pitch, int16_t yaw, struct ObjectRecord* object) {
	int16_t adjustedPitch;
	int16_t negativeYaw;
	int32_t cosYaw;
	int32_t cosPitch;
	int32_t sinYaw;
	int32_t sinPitch;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_fview_calcrotatemove(pitch, yaw, object);
		return;
	}
#endif
	adjustedPitch = -TRIG2_QUARTER_TURN - pitch;
	negativeYaw = -yaw;
	cosYaw = (int16_t)trig2_getsignedcos(negativeYaw);
	cosPitch = (int16_t)trig2_getsignedcos(adjustedPitch);
	sinYaw = (int16_t)trig2_getsignedsin(negativeYaw);
	sinPitch = (int16_t)trig2_getsignedsin(adjustedPitch);
	g_curMatR0_X = cosYaw;
	g_curMatR0_Y = sinYaw;
	g_curMatR0_Z = 0;
	g_curMatR2_X = (int32_t)((uint64_t)((int64_t)cosPitch * -sinYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	g_curMatR2_Y = (int32_t)((uint64_t)((int64_t)cosPitch * cosYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	g_curMatR2_Z = sinPitch;
	g_curMatR1_X = -(int32_t)((uint64_t)((int64_t)sinPitch * sinYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	g_curMatR1_Y = -(int32_t)((uint64_t)((int64_t)sinPitch * -cosYaw) >> FVIEW_MATRIX_FRACTION_BITS);
	g_curMatR1_Z = -cosPitch;
	g_craftMoveX = -g_curMatR2_X;
	g_craftMoveZ = -g_curMatR2_Y;
	g_craftMoveY = -g_curMatR2_Z;
	if (object != NULL) {
		object->moveX = g_craftMoveX;
		object->moveY = g_craftMoveZ;
		object->moveZ = g_craftMoveY;
		object->moveVectorDirty = 0;
	}
}

// FUNCTION: XW 0x40D490
void fview_calcrotateorient(int16_t roll, int16_t rollOffset, struct ObjectRecord* object) {
	fview_transformaxes(g_curMatR1_X, g_curMatR1_Y, g_curMatR1_Z, rollOffset);
	fview_transformaxes(g_curMatR2_X, g_curMatR2_Y, g_curMatR2_Z, roll);
#ifdef XW_MODERN
	g_fviewForwardX_Q15 = (int32_t)(0u - (uint32_t)g_curMatR2_X);
#else
	g_fviewForwardX_Q15 = -g_curMatR2_X;
#endif
#ifdef XW_MODERN
	g_fviewForwardY_Q15 = (int32_t)(0u - (uint32_t)g_curMatR2_Y);
#else
	g_fviewForwardY_Q15 = -g_curMatR2_Y;
#endif
#ifdef XW_MODERN
	g_fviewForwardZ_Q15 = (int32_t)(0u - (uint32_t)g_curMatR2_Z);
#else
	g_fviewForwardZ_Q15 = -g_curMatR2_Z;
#endif
	g_fviewSideX_Q15 = g_curMatR0_X;
	g_fviewSideY_Q15 = g_curMatR0_Y;
	g_fviewSideZ_Q15 = g_curMatR0_Z;
	g_fviewUpX_Q15 = g_curMatR1_X;
	g_fviewUpY_Q15 = g_curMatR1_Y;
	g_fviewUpZ_Q15 = g_curMatR1_Z;
	if (object != NULL) {
		object->cachedForwardX = g_fviewForwardX_Q15;
		object->cachedForwardY = g_fviewForwardY_Q15;
		object->cachedForwardZ = g_fviewForwardZ_Q15;
		object->cachedSideX = g_fviewSideX_Q15;
		object->cachedSideY = g_fviewSideY_Q15;
		object->cachedSideZ = g_fviewSideZ_Q15;
		object->cachedUpX = g_fviewUpX_Q15;
		object->cachedUpY = g_fviewUpY_Q15;
		object->cachedUpZ = g_fviewUpZ_Q15;
		object->orientMatrixDirty = 0;
	}
}

// FUNCTION: XW 0x40D5B0
void fview_calcrotworldeye(void) {
	int32_t dot;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)g_camMatR0_X +
					(uint32_t)g_curMatR0_Y * (uint32_t)g_camMatR0_Y +
					(uint32_t)g_curMatR0_Z * (uint32_t)g_camMatR0_Z);
#else
	dot = g_curMatR0_X * g_camMatR0_X;
	dot += g_curMatR0_Y * g_camMatR0_Y;
	dot += g_curMatR0_Z * g_camMatR0_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R0_X = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)g_camMatR1_X +
					(uint32_t)g_curMatR0_Y * (uint32_t)g_camMatR1_Y +
					(uint32_t)g_curMatR0_Z * (uint32_t)g_camMatR1_Z);
#else
	dot = g_curMatR0_X * g_camMatR1_X;
	dot += g_curMatR0_Y * g_camMatR1_Y;
	dot += g_curMatR0_Z * g_camMatR1_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R0_Y = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)g_camMatR2_X +
					(uint32_t)g_curMatR0_Y * (uint32_t)g_camMatR2_Y +
					(uint32_t)g_curMatR0_Z * (uint32_t)g_camMatR2_Z);
#else
	dot = g_curMatR0_X * g_camMatR2_X;
	dot += g_curMatR0_Y * g_camMatR2_Y;
	dot += g_curMatR0_Z * g_camMatR2_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R0_Z = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)g_camMatR0_X +
					(uint32_t)g_curMatR2_Y * (uint32_t)g_camMatR0_Y +
					(uint32_t)g_curMatR2_Z * (uint32_t)g_camMatR0_Z);
#else
	dot = g_curMatR2_X * g_camMatR0_X;
	dot += g_curMatR2_Y * g_camMatR0_Y;
	dot += g_curMatR2_Z * g_camMatR0_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R1_X = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)g_camMatR1_X +
					(uint32_t)g_curMatR2_Y * (uint32_t)g_camMatR1_Y +
					(uint32_t)g_curMatR2_Z * (uint32_t)g_camMatR1_Z);
#else
	dot = g_curMatR2_X * g_camMatR1_X;
	dot += g_curMatR2_Y * g_camMatR1_Y;
	dot += g_curMatR2_Z * g_camMatR1_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R1_Y = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)g_camMatR2_X +
					(uint32_t)g_curMatR2_Y * (uint32_t)g_camMatR2_Y +
					(uint32_t)g_curMatR2_Z * (uint32_t)g_camMatR2_Z);
#else
	dot = g_curMatR2_X * g_camMatR2_X;
	dot += g_curMatR2_Y * g_camMatR2_Y;
	dot += g_curMatR2_Z * g_camMatR2_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R1_Z = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)g_camMatR0_X +
					(uint32_t)g_curMatR1_Y * (uint32_t)g_camMatR0_Y +
					(uint32_t)g_curMatR1_Z * (uint32_t)g_camMatR0_Z);
#else
	dot = g_curMatR1_X * g_camMatR0_X;
	dot += g_curMatR1_Y * g_camMatR0_Y;
	dot += g_curMatR1_Z * g_camMatR0_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R2_X = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)g_camMatR1_X +
					(uint32_t)g_curMatR1_Y * (uint32_t)g_camMatR1_Y +
					(uint32_t)g_curMatR1_Z * (uint32_t)g_camMatR1_Z);
#else
	dot = g_curMatR1_X * g_camMatR1_X;
	dot += g_curMatR1_Y * g_camMatR1_Y;
	dot += g_curMatR1_Z * g_camMatR1_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R2_Y = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)g_camMatR2_X +
					(uint32_t)g_curMatR1_Y * (uint32_t)g_camMatR2_Y +
					(uint32_t)g_curMatR1_Z * (uint32_t)g_camMatR2_Z);
#else
	dot = g_curMatR1_X * g_camMatR2_X;
	dot += g_curMatR1_Y * g_camMatR2_Y;
	dot += g_curMatR1_Z * g_camMatR2_Z;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	g_objViewMat_R2_Z = dot >> FVIEW_MATRIX_FRACTION_BITS;
	if (g_transformLightDirectionToObjectSpace != 0) {
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)g_modelLightDirectionX +
						(uint32_t)g_curMatR0_Y * (uint32_t)g_modelLightDirectionY +
						(uint32_t)g_curMatR0_Z * (uint32_t)g_modelLightDirectionZ);
#else
		dot = g_curMatR0_X * g_modelLightDirectionX;
		dot += g_curMatR0_Y * g_modelLightDirectionY;
		dot += g_curMatR0_Z * g_modelLightDirectionZ;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		g_objectLightDirectionX = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)g_modelLightDirectionX +
						(uint32_t)g_curMatR2_Y * (uint32_t)g_modelLightDirectionY +
						(uint32_t)g_curMatR2_Z * (uint32_t)g_modelLightDirectionZ);
#else
		dot = g_curMatR2_X * g_modelLightDirectionX;
		dot += g_curMatR2_Y * g_modelLightDirectionY;
		dot += g_curMatR2_Z * g_modelLightDirectionZ;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		g_objectLightDirectionY = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)g_modelLightDirectionX +
						(uint32_t)g_curMatR1_Y * (uint32_t)g_modelLightDirectionY +
						(uint32_t)g_curMatR1_Z * (uint32_t)g_modelLightDirectionZ);
#else
		dot = g_curMatR1_X * g_modelLightDirectionX;
		dot += g_curMatR1_Y * g_modelLightDirectionY;
		dot += g_curMatR1_Z * g_modelLightDirectionZ;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		g_objectLightDirectionZ = dot >> FVIEW_MATRIX_FRACTION_BITS;
	} else {
		g_objectLightDirectionX = g_modelLightDirectionX;
		g_objectLightDirectionY = g_modelLightDirectionY;
		g_objectLightDirectionZ = g_modelLightDirectionZ;
	}
}

// FUNCTION: XW 0x40DA10
void fview_transformaxes(int axisX_Q15, int axisY_Q15, int axisZ_Q15, int16_t angle) {
	int cosAngle_Q15;
	int sinAngle_Q15;
	int rot00_Q15;
	int rot01_Q15;
	int rot02_Q15;
	int rot10_Q15;
	int rot11_Q15;
	int rot12_Q15;
	int rot20_Q15;
	int rot21_Q15;
	int rot22_Q15;
	int dot;
	int axisProduct;
	int sineTerm;
	int newX;
	int newY;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_fview_transformaxes(axisX_Q15, axisY_Q15, axisZ_Q15, angle);
		return;
	}
#endif
	if (angle != 0) {
		cosAngle_Q15 = trig2_getsignedcos(angle);
		sinAngle_Q15 = trig2_getsignedsin(angle);
		if (cosAngle_Q15 >= 0) {
			int oneMinusCos_Q15 = FVIEW_MATRIX_ONE - cosAngle_Q15;
			sineTerm = cosAngle_Q15;
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisX_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisX_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot00_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisZ_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisY_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisY_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot01_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisY_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			sineTerm = (int32_t)(0u - (uint32_t)sineTerm);
#else
			sineTerm = -sineTerm;
#endif
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot02_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisZ_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			sineTerm = (int32_t)(0u - (uint32_t)sineTerm);
#else
			sineTerm = -sineTerm;
#endif
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisY_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisY_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot10_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = cosAngle_Q15;
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisY_Q15 * (uint32_t)axisY_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisY_Q15 * axisY_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot11_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisX_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisY_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisY_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot12_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisY_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot20_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisX_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			sineTerm = (int32_t)(0u - (uint32_t)sineTerm);
#else
			sineTerm = -sineTerm;
#endif
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisY_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisY_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot21_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = cosAngle_Q15;
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisZ_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)oneMinusCos_Q15 +
						  ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisZ_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * oneMinusCos_Q15;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot22_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
		} else {
			int negCosAngle_Q15 = -cosAngle_Q15;
			sineTerm = cosAngle_Q15;
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisX_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisX_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot00_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisZ_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisY_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisY_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot01_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisY_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			sineTerm = (int32_t)(0u - (uint32_t)sineTerm);
#else
			sineTerm = -sineTerm;
#endif
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot02_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisZ_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			sineTerm = (int32_t)(0u - (uint32_t)sineTerm);
#else
			sineTerm = -sineTerm;
#endif
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisY_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisY_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot10_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = cosAngle_Q15;
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisY_Q15 * (uint32_t)axisY_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisY_Q15 * axisY_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot11_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisX_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisY_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisY_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot12_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisY_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisX_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisX_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot20_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = (int32_t)(((int64_t)sinAngle_Q15 * axisX_Q15) >> FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			sineTerm = (int32_t)(0u - (uint32_t)sineTerm);
#else
			sineTerm = -sineTerm;
#endif
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisY_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisY_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot21_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
			sineTerm = cosAngle_Q15;
#ifdef XW_MODERN
			axisProduct = (int32_t)((uint32_t)axisZ_Q15 * (uint32_t)axisZ_Q15);
			dot =
				(int32_t)((uint32_t)(axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * (uint32_t)negCosAngle_Q15 +
						  (uint32_t)axisProduct + ((uint32_t)sineTerm << FVIEW_MATRIX_FRACTION_BITS));
#else
			axisProduct = axisZ_Q15 * axisZ_Q15;
			dot = (axisProduct >> FVIEW_MATRIX_FRACTION_BITS) * negCosAngle_Q15;
			dot += axisProduct;
			dot += sineTerm << FVIEW_MATRIX_FRACTION_BITS;
#endif
			if (dot >= FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MAX;
			if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
				dot = FVIEW_DOT_CLAMP_MIN;
			rot22_Q15 = dot >> FVIEW_MATRIX_FRACTION_BITS;
		}
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)rot00_Q15 +
						(uint32_t)g_curMatR0_Y * (uint32_t)rot10_Q15 +
						(uint32_t)g_curMatR0_Z * (uint32_t)rot20_Q15);
#else
		dot = g_curMatR0_X * rot00_Q15 + g_curMatR0_Y * rot10_Q15 + g_curMatR0_Z * rot20_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		newX = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)rot01_Q15 +
						(uint32_t)g_curMatR0_Y * (uint32_t)rot11_Q15 +
						(uint32_t)g_curMatR0_Z * (uint32_t)rot21_Q15);
#else
		dot = g_curMatR0_X * rot01_Q15 + g_curMatR0_Y * rot11_Q15 + g_curMatR0_Z * rot21_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		newY = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR0_X * (uint32_t)rot02_Q15 +
						(uint32_t)g_curMatR0_Y * (uint32_t)rot12_Q15 +
						(uint32_t)g_curMatR0_Z * (uint32_t)rot22_Q15);
#else
		dot = g_curMatR0_X * rot02_Q15 + g_curMatR0_Y * rot12_Q15 + g_curMatR0_Z * rot22_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		dot = dot >> FVIEW_MATRIX_FRACTION_BITS;
		g_curMatR0_X = newX;
		g_curMatR0_Y = newY;
		g_curMatR0_Z = dot;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)rot00_Q15 +
						(uint32_t)g_curMatR1_Y * (uint32_t)rot10_Q15 +
						(uint32_t)g_curMatR1_Z * (uint32_t)rot20_Q15);
#else
		dot = g_curMatR1_X * rot00_Q15 + g_curMatR1_Y * rot10_Q15 + g_curMatR1_Z * rot20_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		newX = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)rot01_Q15 +
						(uint32_t)g_curMatR1_Y * (uint32_t)rot11_Q15 +
						(uint32_t)g_curMatR1_Z * (uint32_t)rot21_Q15);
#else
		dot = g_curMatR1_X * rot01_Q15 + g_curMatR1_Y * rot11_Q15 + g_curMatR1_Z * rot21_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		newY = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR1_X * (uint32_t)rot02_Q15 +
						(uint32_t)g_curMatR1_Y * (uint32_t)rot12_Q15 +
						(uint32_t)g_curMatR1_Z * (uint32_t)rot22_Q15);
#else
		dot = g_curMatR1_X * rot02_Q15 + g_curMatR1_Y * rot12_Q15 + g_curMatR1_Z * rot22_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		dot = dot >> FVIEW_MATRIX_FRACTION_BITS;
		g_curMatR1_X = newX;
		g_curMatR1_Y = newY;
		g_curMatR1_Z = dot;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)rot00_Q15 +
						(uint32_t)g_curMatR2_Y * (uint32_t)rot10_Q15 +
						(uint32_t)g_curMatR2_Z * (uint32_t)rot20_Q15);
#else
		dot = g_curMatR2_X * rot00_Q15 + g_curMatR2_Y * rot10_Q15 + g_curMatR2_Z * rot20_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		newX = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)rot01_Q15 +
						(uint32_t)g_curMatR2_Y * (uint32_t)rot11_Q15 +
						(uint32_t)g_curMatR2_Z * (uint32_t)rot21_Q15);
#else
		dot = g_curMatR2_X * rot01_Q15 + g_curMatR2_Y * rot11_Q15 + g_curMatR2_Z * rot21_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		newY = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_curMatR2_X * (uint32_t)rot02_Q15 +
						(uint32_t)g_curMatR2_Y * (uint32_t)rot12_Q15 +
						(uint32_t)g_curMatR2_Z * (uint32_t)rot22_Q15);
#else
		dot = g_curMatR2_X * rot02_Q15 + g_curMatR2_Y * rot12_Q15 + g_curMatR2_Z * rot22_Q15;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		dot = dot >> FVIEW_MATRIX_FRACTION_BITS;
		g_curMatR2_X = newX;
		g_curMatR2_Y = newY;
		g_curMatR2_Z = dot;
	}
}
