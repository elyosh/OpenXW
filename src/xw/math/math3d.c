#include "xw/math/math3d.h"

#include <math.h>
#include <stdlib.h>

// FUNCTION: XW 0x47DA10
float Math3D_Dot3(const float* lhs, const float* rhs) {
	return lhs[1] * rhs[1] + lhs[2] * rhs[2] + rhs[0] * lhs[0];
}

// FUNCTION: XW 0x47DA40
void Math3D_RotateVec3(float* vecInOut, const float* matrix3x3) {
	float x = vecInOut[0];
	float y = vecInOut[1];
	float z = vecInOut[2];
	float zCoefficient = matrix3x3[6];
	vecInOut[0] = x * matrix3x3[0] + zCoefficient * z + matrix3x3[3] * y;
	vecInOut[1] = matrix3x3[4] * y + matrix3x3[7] * z + matrix3x3[1] * x;
	vecInOut[2] = matrix3x3[8] * z + matrix3x3[5] * y + matrix3x3[2] * x;
}

// FUNCTION: XW 0x47DAB0
double Math3D_RotateVec3X(const float* vec, const float* matrix3x3) {
	float y = vec[1];
	return vec[2] * matrix3x3[6] + y * matrix3x3[3] + matrix3x3[0] * vec[0];
}

// FUNCTION: XW 0x47DAE0
double Math3D_RotateVec3Y(const float* vec, const float* matrix3x3) {
	float z = vec[2];
	return vec[1] * matrix3x3[4] + z * matrix3x3[7] + matrix3x3[1] * vec[0];
}

// FUNCTION: XW 0x47DB10
double Math3D_RotateVec3Z(const float* vec, const float* matrix3x3) {
	float yCoefficient = matrix3x3[5];
	return matrix3x3[8] * vec[2] + yCoefficient * vec[1] + matrix3x3[2] * vec[0];
}

// FUNCTION: XW 0x47DB40
void Math3D_MulMatrix3x3(float* lhsInOut, const float* rhs) {
	float r00;
	float r01;
	float r02;
	float r10;
	float r11;
	float r12;
	float r20;
	float r21;
	float r22;

	r00 = lhsInOut[1] * rhs[3] + lhsInOut[2] * rhs[6] + rhs[0] * lhsInOut[0];
	r01 = lhsInOut[0] * rhs[1] + rhs[4] * lhsInOut[1] + rhs[7] * lhsInOut[2];
	r02 = lhsInOut[0] * rhs[2] + lhsInOut[1] * rhs[5] + lhsInOut[2] * rhs[8];
	r10 = rhs[0] * lhsInOut[3] + rhs[6] * lhsInOut[5] + rhs[3] * lhsInOut[4];
	r11 = rhs[7] * lhsInOut[5] + rhs[1] * lhsInOut[3] + rhs[4] * lhsInOut[4];
	r12 = lhsInOut[5] * rhs[8] + lhsInOut[3] * rhs[2] + lhsInOut[4] * rhs[5];
	r20 = rhs[0] * lhsInOut[6] + lhsInOut[8] * rhs[6] + lhsInOut[7] * rhs[3];
	r21 = lhsInOut[7] * rhs[4] + lhsInOut[8] * rhs[7] + lhsInOut[6] * rhs[1];
	r22 = lhsInOut[6] * rhs[2] + lhsInOut[7] * rhs[5] + lhsInOut[8] * rhs[8];
	lhsInOut[0] = r00;
	lhsInOut[1] = r01;
	lhsInOut[2] = r02;
	lhsInOut[3] = r10;
	lhsInOut[4] = r11;
	lhsInOut[5] = r12;
	lhsInOut[6] = r20;
	lhsInOut[7] = r21;
	lhsInOut[8] = r22;
}

// FUNCTION: XW 0x47DCC0
void Math3D_MulMatrix3x3T(float* lhsInOut, const float* rhs) {
	float r00;
	float r01;
	float r02;
	float r10;
	float r11;
	float r12;
	float r20;
	float r21;
	float r22;

	r00 = lhsInOut[0] * rhs[0] + lhsInOut[3] * rhs[3] + lhsInOut[6] * rhs[6];
	r01 = lhsInOut[1] * rhs[0] + lhsInOut[4] * rhs[3] + lhsInOut[7] * rhs[6];
	r02 = lhsInOut[2] * rhs[0] + lhsInOut[5] * rhs[3] + lhsInOut[8] * rhs[6];
	r10 = lhsInOut[0] * rhs[1] + lhsInOut[3] * rhs[4] + lhsInOut[6] * rhs[7];
	r11 = lhsInOut[1] * rhs[1] + lhsInOut[4] * rhs[4] + lhsInOut[7] * rhs[7];
	r12 = lhsInOut[2] * rhs[1] + lhsInOut[5] * rhs[4] + lhsInOut[8] * rhs[7];
	r20 = lhsInOut[0] * rhs[2] + lhsInOut[3] * rhs[5] + lhsInOut[6] * rhs[8];
	r21 = lhsInOut[1] * rhs[2] + lhsInOut[4] * rhs[5] + lhsInOut[7] * rhs[8];
	r22 = lhsInOut[2] * rhs[2] + lhsInOut[5] * rhs[5] + lhsInOut[8] * rhs[8];
	lhsInOut[0] = r00;
	lhsInOut[1] = r01;
	lhsInOut[2] = r02;
	lhsInOut[3] = r10;
	lhsInOut[4] = r11;
	lhsInOut[5] = r12;
	lhsInOut[6] = r20;
	lhsInOut[7] = r21;
	lhsInOut[8] = r22;
}

// FUNCTION: XW 0x47DE40
void Math3D_BuildAxisAngleMatrix(float* matrix3x3Out, const float* axisAngle) {
	float axisX = axisAngle[0];
	float axisY = axisAngle[1];
	float axisZ = axisAngle[2];
	float cosAngle = cos(axisAngle[3]);
	float sinAngle = sin(axisAngle[3]);
	float oneMinusCos = 1.0f - cosAngle;
	float xxTerm = oneMinusCos * axisX * axisX;
	float yyTerm = axisY * (oneMinusCos * axisY);
	float zzTerm = axisZ * (oneMinusCos * axisZ);
	float xyTerm = (oneMinusCos * axisY) * axisX;
	float yzTerm = (oneMinusCos * axisZ) * axisY;
	float xzTerm = (oneMinusCos * axisZ) * axisX;
	float sinX = sinAngle * axisX;
	float sinY = sinAngle * axisY;
	float sinZ = sinAngle * axisZ;

	matrix3x3Out[1] = xyTerm + sinZ;
	matrix3x3Out[0] = xxTerm + cosAngle;
	matrix3x3Out[2] = xzTerm - sinY;
	matrix3x3Out[3] = xyTerm - sinZ;
	matrix3x3Out[4] = yyTerm + cosAngle;
	matrix3x3Out[5] = yzTerm + sinX;
	matrix3x3Out[6] = xzTerm + sinY;
	matrix3x3Out[7] = yzTerm - sinX;
	matrix3x3Out[8] = zzTerm + cosAngle;
}
