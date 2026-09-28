#ifndef XW_MATH_MATH3D_H
#define XW_MATH_MATH3D_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* Declarations follow ascending original IDB address. */

/* 0x47DA10 */
float Math3D_Dot3(const float* lhs, const float* rhs);

/* 0x47DA40 */
void Math3D_RotateVec3(float* vecInOut, const float* matrix3x3);

/* 0x47DAB0 */
double Math3D_RotateVec3X(const float* vec, const float* matrix3x3);

/* 0x47DAE0 */
double Math3D_RotateVec3Y(const float* vec, const float* matrix3x3);

/* 0x47DB10 */
double Math3D_RotateVec3Z(const float* vec, const float* matrix3x3);

/* 0x47DB40 */
void Math3D_MulMatrix3x3(float* lhsInOut, const float* rhs);

/* 0x47DCC0 */
void Math3D_MulMatrix3x3T(float* lhsInOut, const float* rhs);

/* 0x47DE40 */
void Math3D_BuildAxisAngleMatrix(float* matrix3x3Out, const float* axisAngle);

#ifdef __cplusplus
}
#endif

#endif
