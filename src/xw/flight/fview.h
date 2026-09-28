#ifndef XW_FLIGHT_FVIEW_H
#define XW_FLIGHT_FVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct ObjectRecord;

enum {
	FVIEW_MATRIX_FRACTION_BITS = 15,
	FVIEW_MATRIX_ONE = 0x7FFF,
	FVIEW_DOT_CLAMP_LIMIT = 0x40000000,
	FVIEW_DOT_CLAMP_MAX = 0x3FFFFFFF,
	FVIEW_DOT_CLAMP_MIN = -0x3FFF0000
};

extern int g_fviewForwardX_Q15;
extern int g_fviewForwardY_Q15;
extern int g_fviewForwardZ_Q15;
extern int g_fviewSideX_Q15;
extern int g_fviewSideY_Q15;
extern int g_fviewSideZ_Q15;
extern int g_fviewUpX_Q15;
extern int g_fviewUpY_Q15;
extern int g_fviewUpZ_Q15;

extern int g_craftMoveZ;
extern int g_craftMoveY;
extern int g_craftMoveX;
extern int g_curMatR0_Y;
extern int g_curMatR0_Z;
extern int g_curMatR0_X;
extern int g_curMatR1_Y;
extern int g_curMatR1_Z;
extern int g_curMatR1_X;
extern int g_curMatR2_X;
extern int g_curMatR2_Y;
extern int g_curMatR2_Z;

extern int g_objViewMat_R0_X;
extern int g_objViewMat_R0_Y;
extern int g_objViewMat_R0_Z;
extern int g_objViewMat_R1_X;
extern int g_objViewMat_R1_Y;
extern int g_objViewMat_R1_Z;

extern int g_modelLightDirectionX;
extern int g_modelLightDirectionY;
extern int g_modelLightDirectionZ;
extern int g_objectLightDirectionX;
extern int g_objectLightDirectionY;
extern int g_objectLightDirectionZ;
extern int g_objViewMat_R2_X;
extern int g_objViewMat_R2_Y;
extern int g_objViewMat_R2_Z;

/* Declarations follow ascending original IDB address. */

/* 0x40D100 */
void fview_newcalcview(int16_t viewRoll, int16_t viewPitch, int16_t viewYaw, int16_t viewAngleD,
					   int16_t hudAimX, int16_t hudAimY, struct ObjectRecord* objRecord);

/* 0x40D220 */
void fview_newcalcrotate(int16_t roll, int16_t pitch, int16_t yaw, int16_t rollOffset,
						 struct ObjectRecord* object);

/* 0x40D340 */
void fview_calcrotatemove(int16_t pitch, int16_t yaw, struct ObjectRecord* object);

/* 0x40D490 */
void fview_calcrotateorient(int16_t roll, int16_t rollOffset, struct ObjectRecord* object);

/* 0x40D5B0 */
void fview_calcrotworldeye(void);

/* 0x40DA10 */
void fview_transformaxes(int axisX_Q15, int axisY_Q15, int axisZ_Q15, int16_t angle);

#ifdef __cplusplus
}
#endif

#endif
