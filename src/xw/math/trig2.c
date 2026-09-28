#include "xw/math/trig2.h"

#include <math.h>
#include <stdlib.h>

// GLOBAL: XW 0x4C34F0
const float g_trigRadiansPerAngle16 = 0.00009587372187525034f;

// GLOBAL: XW 0x4C34F4
const float g_trigSignedQ15Magnitude = 32767.0f;

// GLOBAL: XW 0x4DDFA0
uint16_t g_sinTable[TRIG2_SINE_TABLE_COUNT] = {
	0,     402,   804,   1206,  1608,  2010,  2412,  2814,  3216,  3617,  4019,  4420,  4821,  5222,  5623,
	6023,  6424,  6824,  7224,  7623,  8022,  8421,  8820,  9218,  9616,  10014, 10411, 10808, 11204, 11600,
	11996, 12391, 12785, 13180, 13573, 13966, 14359, 14751, 15143, 15534, 15924, 16314, 16703, 17091, 17479,
	17867, 18253, 18639, 19024, 19409, 19792, 20175, 20557, 20939, 21320, 21699, 22078, 22457, 22834, 23210,
	23586, 23961, 24335, 24708, 25080, 25451, 25821, 26190, 26558, 26925, 27291, 27656, 28020, 28383, 28745,
	29106, 29466, 29824, 30182, 30538, 30893, 31248, 31600, 31952, 32303, 32652, 33000, 33347, 33692, 34037,
	34380, 34721, 35062, 35401, 35738, 36075, 36410, 36744, 37076, 37407, 37736, 38064, 38391, 38716, 39040,
	39362, 39683, 40002, 40320, 40636, 40951, 41264, 41576, 41886, 42194, 42501, 42806, 43110, 43412, 43713,
	44011, 44308, 44604, 44898, 45190, 45480, 45769, 46056, 46341, 46624, 46906, 47186, 47464, 47741, 48015,
	48288, 48559, 48828, 49095, 49361, 49624, 49886, 50146, 50404, 50660, 50914, 51166, 51417, 51665, 51911,
	52156, 52398, 52639, 52878, 53114, 53349, 53581, 53812, 54040, 54267, 54491, 54714, 54934, 55152, 55368,
	55582, 55794, 56004, 56212, 56418, 56621, 56823, 57022, 57219, 57414, 57607, 57798, 57986, 58172, 58356,
	58538, 58718, 58896, 59071, 59244, 59415, 59583, 59750, 59914, 60075, 60235, 60392, 60547, 60700, 60851,
	60999, 61145, 61288, 61429, 61568, 61705, 61839, 61971, 62101, 62228, 62353, 62476, 62596, 62714, 62830,
	62943, 63054, 63162, 63268, 63372, 63473, 63572, 63668, 63763, 63854, 63944, 64031, 64115, 64197, 64277,
	64354, 64429, 64501, 64571, 64639, 64704, 64766, 64827, 64884, 64940, 64993, 65043, 65091, 65137, 65180,
	65220, 65259, 65294, 65328, 65358, 65387, 65413, 65436, 65457, 65476, 65492, 65505, 65516, 65525, 65531,
	65533, 65534, 65533, 65531, 65525, 65516, 65505, 65492, 65476, 65457, 65436, 65413, 65387, 65358, 65328,
	65294, 65259, 65220, 65180, 65137, 65091, 65043, 64993, 64940, 64884, 64827, 64766, 64704, 64639, 64571,
	64501, 64429, 64354, 64277, 64197, 64115, 64031, 63944, 63854, 63763, 63668, 63572, 63473, 63372, 63268,
	63162, 63054, 62943, 62830, 62714, 62596, 62476, 62353, 62228, 62101, 61971, 61839, 61705, 61568, 61429,
	61288, 61145, 60999, 60851, 60700, 60547, 60392, 60235, 60075, 59914, 59750, 59583, 59415, 59244, 59071,
	58896, 58718, 58538, 58356, 58172, 57986, 57798, 57607, 57414, 57219, 57022, 56823, 56621, 56418, 56212,
	56004, 55794, 55582, 55368, 55152, 54934, 54714, 54491, 54267, 54040, 53812, 53581, 53349, 53114, 52878,
	52639, 52398, 52156, 51911, 51665, 51417, 51166, 50914, 50660, 50404, 50146, 49886, 49624, 49361, 49095,
	48828, 48559, 48288, 48015, 47741, 47464, 47186, 46906, 46624, 46341, 46056, 45769, 45480, 45190, 44898,
	44604, 44308, 44011, 43713, 43412, 43110, 42806, 42501, 42194, 41886, 41576, 41264, 40951, 40636, 40320,
	40002, 39683, 39362, 39040, 38716, 38391, 38064, 37736, 37407, 37076, 36744, 36410, 36075, 35738, 35401,
	35062, 34721, 34380, 34037, 33692, 33347, 33000, 32652, 32303, 31952, 31600, 31248, 30893, 30538, 30182,
	29824, 29466, 29106, 28745, 28383, 28020, 27656, 27291, 26925, 26558, 26190, 25821, 25451, 25080, 24708,
	24335, 23961, 23586, 23210, 22834, 22457, 22078, 21699, 21320, 20939, 20557, 20175, 19792, 19409, 19024,
	18639, 18253, 17867, 17479, 17091, 16703, 16314, 15924, 15534, 15143, 14751, 14359, 13966, 13573, 13180,
	12785, 12391, 11996, 11600, 11204, 10808, 10411, 10014, 9616,  9218,  8820,  8421,  8022,  7623,  7224,
	6824,  6424,  6023,  5623,  5222,  4821,  4420,  4019,  3617,  3216,  2814,  2412,  2010,  1608,  1206,
	804,   402,   0,
};

// GLOBAL: XW 0x4DE6A8
const uint16_t g_trig2ArctanTable[TRIG2_ARCTAN_TABLE_COUNT] = {
	0,    41,   81,   122,  163,  204,  244,  285,  326,  367,  407,  448,  489,  529,  570,  610,  651,
	692,  732,  773,  813,  854,  894,  935,  975,  1015, 1056, 1096, 1136, 1177, 1217, 1257, 1297, 1337,
	1377, 1417, 1457, 1497, 1537, 1577, 1617, 1656, 1696, 1736, 1775, 1815, 1854, 1894, 1933, 1973, 2012,
	2051, 2090, 2129, 2168, 2207, 2246, 2285, 2324, 2363, 2401, 2440, 2478, 2517, 2555, 2593, 2632, 2670,
	2708, 2746, 2784, 2822, 2860, 2897, 2935, 2973, 3010, 3047, 3085, 3122, 3159, 3196, 3233, 3270, 3307,
	3344, 3380, 3417, 3453, 3490, 3526, 3562, 3598, 3634, 3670, 3706, 3742, 3778, 3813, 3849, 3884, 3920,
	3955, 3990, 4025, 4060, 4095, 4129, 4164, 4199, 4233, 4267, 4302, 4336, 4370, 4404, 4438, 4471, 4505,
	4539, 4572, 4605, 4639, 4672, 4705, 4738, 4771, 4803, 4836, 4869, 4901, 4933, 4966, 4998, 5030, 5062,
	5093, 5125, 5157, 5188, 5220, 5251, 5282, 5313, 5344, 5375, 5406, 5437, 5467, 5498, 5528, 5558, 5589,
	5619, 5649, 5679, 5708, 5738, 5768, 5797, 5826, 5856, 5885, 5914, 5943, 5972, 6000, 6029, 6057, 6086,
	6114, 6142, 6171, 6199, 6226, 6254, 6282, 6310, 6337, 6365, 6392, 6419, 6446, 6473, 6500, 6527, 6554,
	6580, 6607, 6633, 6660, 6686, 6712, 6738, 6764, 6790, 6815, 6841, 6867, 6892, 6917, 6943, 6968, 6993,
	7018, 7043, 7067, 7092, 7117, 7141, 7166, 7190, 7214, 7238, 7262, 7286, 7310, 7334, 7358, 7381, 7405,
	7428, 7451, 7474, 7498, 7521, 7544, 7566, 7589, 7612, 7634, 7657, 7679, 7702, 7724, 7746, 7768, 7790,
	7812, 7834, 7856, 7877, 7899, 7920, 7942, 7963, 7984, 8005, 8026, 8047, 8068, 8089, 8110, 8130, 8151,
	8172, 8192, 8192,
};

// GLOBAL: XW 0x4DE8B0
const uint16_t g_trig2RadiusCorrectionTable[TRIG2_RADIUS_CORRECTION_COUNT] = {
	0,     0,     2,     4,     8,     12,    18,    24,    32,    40,    50,    60,    72,    84,    98,
	112,   128,   144,   162,   180,   200,   220,   242,   264,   287,   312,   337,   363,   391,   419,
	448,   479,   510,   542,   575,   610,   645,   681,   718,   756,   795,   835,   876,   918,   961,
	1005,  1050,  1095,  1142,  1190,  1238,  1288,  1338,  1390,  1442,  1495,  1550,  1605,  1661,  1718,
	1776,  1835,  1895,  1955,  2017,  2079,  2143,  2207,  2273,  2339,  2406,  2474,  2543,  2612,  2683,
	2755,  2827,  2900,  2974,  3049,  3125,  3202,  3280,  3358,  3438,  3518,  3599,  3681,  3764,  3847,
	3932,  4017,  4103,  4190,  4278,  4367,  4456,  4547,  4638,  4730,  4822,  4916,  5010,  5105,  5201,
	5298,  5396,  5494,  5593,  5693,  5794,  5895,  5997,  6100,  6204,  6309,  6414,  6520,  6627,  6734,
	6843,  6952,  7061,  7172,  7283,  7395,  7508,  7621,  7735,  7850,  7966,  8082,  8199,  8317,  8435,
	8554,  8674,  8794,  8915,  9037,  9160,  9283,  9407,  9531,  9656,  9782,  9909,  10036, 10164, 10292,
	10421, 10551, 10681, 10812, 10944, 11076, 11209, 11343, 11477, 11612, 11747, 11883, 12019, 12157, 12294,
	12433, 12572, 12711, 12852, 12992, 13134, 13276, 13418, 13561, 13705, 13849, 13994, 14139, 14285, 14431,
	14578, 14726, 14874, 15022, 15171, 15321, 15471, 15622, 15773, 15925, 16077, 16230, 16384, 16537, 16692,
	16847, 17002, 17158, 17314, 17471, 17629, 17786, 17945, 18104, 18263, 18423, 18583, 18744, 18905, 19066,
	19229, 19391, 19554, 19718, 19882, 20046, 20211, 20376, 20542, 20708, 20875, 21042, 21209, 21377, 21546,
	21714, 21884, 22053, 22223, 22394, 22565, 22736, 22908, 23080, 23252, 23425, 23599, 23772, 23946, 24121,
	24296, 24471, 24647, 24823, 24999, 25176, 25353, 25531, 25709, 25887, 26066, 26245, 26424, 26604, 26784,
	26964, 27146,
};

// GLOBAL: XW 0x5B8A50
int g_moveDeltaX = 0;

// GLOBAL: XW 0x5B8A54
int16_t g_trig2Yaw = 0;

// GLOBAL: XW 0x5B8A58
int g_trig2PolarDistance = 0;

// GLOBAL: XW 0x5B8A5C
int trig2_xoffset = 0;

// GLOBAL: XW 0x5B8A60
int g_moveDeltaY = 0;

// GLOBAL: XW 0x5B8A64
uint16_t trig2_phi = 0;

// GLOBAL: XW 0x5B8A68
int trig2_rho = 0;

// GLOBAL: XW 0x5B8A6C
int trig2_yoffset = 0;

// GLOBAL: XW 0x5B8A70
uint16_t g_trig2ArctanAxesSwapped = 0;

// GLOBAL: XW 0x5B8A72
uint16_t g_trig2NegativeX = 0;

// GLOBAL: XW 0x5B8A74
uint16_t g_trig2NegativeY = 0;

// GLOBAL: XW 0x5B8A76
uint16_t trig2_signz = 0;

// GLOBAL: XW 0x5B8A78
int g_moveDeltaZ = 0;

// GLOBAL: XW 0x5B8A7C
int trig2_zoffset = 0;

// GLOBAL: XW 0x5B8A80
unsigned int g_trig2MajorAxisMagnitude = 0;

// GLOBAL: XW 0x5B8A84
uint16_t trig2_theta = 0;

// GLOBAL: XW 0x5B8A86
int16_t g_trig2Pitch = 0;

// GLOBAL: XW 0x5B8A88
uint16_t g_trig2PlaneAngle = 0;

// FUNCTION: XW 0x4A9C90
int64_t trig2_getsignedsin(int16_t angle) {
	return (int64_t)(sin((double)angle * g_trigRadiansPerAngle16) * g_trigSignedQ15Magnitude);
}

// FUNCTION: XW 0x4A9CB0
int trig2_calcsineofangle(unsigned int angle) {
	uint16_t index = (uint16_t)((angle >> TRIG2_SINE_INDEX_SHIFT) & TRIG2_SINE_INDEX_MASK);
	uint16_t baseSample = g_sinTable[index];
	uint16_t magnitude = (uint16_t)(g_sinTable[index + 1] - baseSample);
	int16_t sampleDelta = (int16_t)magnitude;
	unsigned int fraction;
	int interpolatedDelta;
	if (sampleDelta < 0)
		magnitude = (uint16_t)-magnitude;
	fraction = (uint16_t)(angle << TRIG2_INTERPOLATION_FRACTION_SHIFT);
	interpolatedDelta = ((unsigned int)magnitude * fraction) >> TRIG2_PRODUCT_FRACTION_BITS;
	if (sampleDelta < 0)
		interpolatedDelta = -interpolatedDelta;
	return baseSample + interpolatedDelta;
}

// FUNCTION: XW 0x4A9D10
int16_t trig2_w_arccos(int cosineQ15) { return trig2_arccos(cosineQ15); }

// FUNCTION: XW 0x4A9D20
int16_t trig2_arccos(int cosineQ15) {
	int16_t signedCosine = (int16_t)cosineQ15;
	int16_t sampleIndex;
	int16_t remainingSamples = TRIG2_SINE_PEAK_INDEX;
	unsigned int magnitude = (unsigned int)cosineQ15;
	uint16_t doubledMagnitude;
	uint16_t lowerSample;
	uint16_t remainder;
	uint16_t angle;
	int16_t fraction;
	if (signedCosine < 0) {
		magnitude = 0u - magnitude;
	}
	doubledMagnitude = magnitude * 2;
	for (sampleIndex = TRIG2_SINE_PEAK_INDEX; remainingSamples > 0; ++sampleIndex) {
		--remainingSamples;
		if (doubledMagnitude >= g_sinTable[sampleIndex]) {
			break;
		}
	}
	if (sampleIndex == TRIG2_SINE_TABLE_COUNT - 1) {
		int quotient = (int32_t)((uint32_t)doubledMagnitude << TRIG2_PRODUCT_FRACTION_BITS) /
					   g_sinTable[sampleIndex - 1];
		angle = -((quotient >> TRIG2_ARCCOS_FRACTION_BITS) & UINT8_MAX);
		if (angle == 0) {
			angle = TRIG2_QUARTER_TURN;
		} else {
			angle >>= TRIG2_ARCCOS_ANGLE_SHIFT;
		}
		if (signedCosine < 0) {
			angle = TRIG2_ANGLE_SIGN_BIT - angle;
		}
		return angle;
	}
	lowerSample = g_sinTable[sampleIndex - 1];
	remainder = doubledMagnitude - lowerSample;
	if (remainder != 0) {
		int quotient = (int32_t)((uint32_t)remainder << TRIG2_PRODUCT_FRACTION_BITS) /
					   (uint16_t)(g_sinTable[sampleIndex - 2] - lowerSample);
		fraction = ((quotient >> TRIG2_ARCCOS_FRACTION_BITS) & UINT8_MAX);
	} else {
		fraction = 0;
	}
	angle = ((TRIG2_SINE_PEAK_INDEX - 1 - remainingSamples) << TRIG2_ARCCOS_FRACTION_BITS) - fraction;
	angle >>= TRIG2_ARCCOS_ANGLE_SHIFT;
	if (signedCosine < 0) {
		angle = TRIG2_ANGLE_SIGN_BIT - angle;
	}
	return angle;
}

// FUNCTION: XW 0x4A9E30
unsigned int trig2_sinewordmult(int value, unsigned int angle) {
	unsigned int magnitude = (unsigned int)value;
	uint16_t valueSign = (uint16_t)(magnitude & TRIG2_ANGLE_SIGN_BIT);
	uint16_t productSign;
	unsigned int product;
	if (valueSign != 0) {
		magnitude = 0u - magnitude;
	}
	magnitude = (uint16_t)magnitude;
	productSign = (uint16_t)(angle & TRIG2_ANGLE_SIGN_BIT) ^ valueSign;
	product = magnitude * g_sinTable[(angle >> TRIG2_SINE_INDEX_SHIFT) & TRIG2_SINE_INDEX_MASK] +
			  TRIG2_PRODUCT_ROUNDING_BIAS;
	if (productSign != 0) {
		product = 0u - product;
	}
	return product >> TRIG2_PRODUCT_FRACTION_BITS;
}

// FUNCTION: XW 0x4A9E80
int trig2_sinedwordmult(int value, uint16_t angle) {
	unsigned int magnitude = (unsigned int)value;
	uint16_t sign = 0;
	unsigned int sineMagnitude;
	unsigned int lowProduct;
	unsigned int scaledValue;

	if (value < 0) {
		sign = TRIG2_ANGLE_SIGN_BIT;
		magnitude = 0u - magnitude;
	}
	sign ^= (uint16_t)angle;
	sineMagnitude = g_sinTable[(angle >> TRIG2_SINE_INDEX_SHIFT) & TRIG2_SINE_INDEX_MASK];
	lowProduct = (uint16_t)magnitude * sineMagnitude;
	magnitude >>= TRIG2_PRODUCT_FRACTION_BITS;
	scaledValue = magnitude * sineMagnitude;
	lowProduct += TRIG2_PRODUCT_ROUNDING_BIAS;
	scaledValue += lowProduct >> TRIG2_PRODUCT_FRACTION_BITS;
	if ((sign & TRIG2_ANGLE_SIGN_BIT) != 0) {
		return -(int)scaledValue;
	}
	return (int)scaledValue;
}

// FUNCTION: XW 0x4A9EE0
int64_t trig2_getsignedcos(int16_t angle) {
	return (int64_t)(cos((double)angle * g_trigRadiansPerAngle16) * g_trigSignedQ15Magnitude);
}

// FUNCTION: XW 0x4A9F00
unsigned int trig2_cosinewordmult(int value, int angle) {
	unsigned int magnitude = (unsigned int)value;
	uint16_t valueSign = (uint16_t)(magnitude & TRIG2_ANGLE_SIGN_BIT);
	unsigned int sineAngle;
	uint16_t productSign;
	unsigned int product;
	if (valueSign != 0) {
		magnitude = 0u - magnitude;
	}
	magnitude = (uint16_t)magnitude;
	sineAngle = (unsigned int)angle + TRIG2_QUARTER_TURN;
	productSign = (uint16_t)(sineAngle & TRIG2_ANGLE_SIGN_BIT) ^ valueSign;
	product = magnitude * g_sinTable[(sineAngle >> TRIG2_SINE_INDEX_SHIFT) & TRIG2_SINE_INDEX_MASK] +
			  TRIG2_PRODUCT_ROUNDING_BIAS;
	if (productSign != 0) {
		product = 0u - product;
	}
	return product >> TRIG2_PRODUCT_FRACTION_BITS;
}

// FUNCTION: XW 0x4A9F60
int trig2_cosinedwordmult(int value, uint16_t angle) {
	unsigned int magnitude = (unsigned int)value;
	uint16_t sign = 0;
	uint16_t sineAngle;
	uint16_t cosineMagnitude;
	unsigned int lowProduct;
	unsigned int scaledValue;

	if (value < 0) {
		sign = TRIG2_ANGLE_SIGN_BIT;
		magnitude = 0u - magnitude;
	}
	sineAngle = (uint16_t)((unsigned int)angle + TRIG2_QUARTER_TURN);
	sign = (sign ^ sineAngle) & TRIG2_ANGLE_SIGN_BIT;
	cosineMagnitude = g_sinTable[(sineAngle >> TRIG2_SINE_INDEX_SHIFT) & TRIG2_SINE_INDEX_MASK];
	scaledValue = (magnitude >> TRIG2_PRODUCT_FRACTION_BITS) * cosineMagnitude;
	lowProduct = (magnitude & UINT16_MAX) * cosineMagnitude + TRIG2_PRODUCT_ROUNDING_BIAS;
	scaledValue += lowProduct >> TRIG2_PRODUCT_FRACTION_BITS;
	if (sign == 0) {
		return (int)scaledValue;
	}
	return -(int)scaledValue;
}

// FUNCTION: XW 0x4A9FD0
void trig2_UpdateCartesianOffsets(void) {
	trig2_zoffset = trig2_sinedwordmult(trig2_rho, trig2_phi);
	trig2_xoffset = trig2_cosinedwordmult(trig2_zoffset, trig2_theta);
	trig2_yoffset = trig2_sinedwordmult(trig2_zoffset, trig2_theta);
	trig2_zoffset = trig2_cosinedwordmult(trig2_rho, trig2_phi);
}

// FUNCTION: XW 0x4AA040
void trig2_movexyz(uint16_t distance, int16_t yaw, uint16_t pitch) {
	trig2_rho = distance;
	trig2_phi = pitch;
	trig2_theta = TRIG2_QUARTER_TURN - yaw;
	trig2_UpdateCartesianOffsets();
	g_moveDeltaX = trig2_xoffset;
	g_moveDeltaY = trig2_yoffset;
	g_moveDeltaZ = trig2_zoffset;
}

// FUNCTION: XW 0x4AA0A0
uint16_t trig2_ctop(int dx, int dy, int dz) {
	uint16_t angle;
	if (dx < 0) {
		g_trig2NegativeX = 1;
		dx = (int)(0u - (unsigned int)dx);
	} else {
		g_trig2NegativeX = 0;
	}
	trig2_xoffset = dx;
	if (dy < 0) {
		g_trig2NegativeY = 1;
		dy = (int)(0u - (unsigned int)dy);
	} else {
		g_trig2NegativeY = 0;
	}
	trig2_yoffset = dy;
	if (dz < 0) {
		trig2_signz = 1;
		dz = (int)(0u - (unsigned int)dz);
	} else {
		trig2_signz = 0;
	}
	trig2_zoffset = dz;
	trig2_calcangleplanedistance(dx, dy);
	angle = g_trig2PlaneAngle;
	if (g_trig2NegativeY != 0) {
		angle = -angle;
	}
	if (g_trig2NegativeX != 0) {
		angle = TRIG2_ANGLE_SIGN_BIT - angle;
	}
	g_trig2Yaw = TRIG2_QUARTER_TURN - angle;
	trig2_calcangleplanedistance(g_trig2PolarDistance, trig2_zoffset);
	angle = g_trig2PlaneAngle;
	if (trig2_signz != 0) {
		angle = -angle;
	}
	g_trig2Pitch = TRIG2_QUARTER_TURN - angle;
	return angle;
}

// FUNCTION: XW 0x4AA190
void trig2_calcangleplanedistance(int absX, int absY) {
	uint16_t angle;
	uint16_t ratioIndex;
	unsigned int correction;
	unsigned int major;
	unsigned int lowProduct;
	unsigned int highProduct;
	trig2_calcarctan(absX, absY, &angle, &ratioIndex);
	g_trig2PlaneAngle = angle;
	correction = g_trig2RadiusCorrectionTable[ratioIndex];
	major = g_trig2MajorAxisMagnitude;
	lowProduct = correction * (uint16_t)major;
	lowProduct = (lowProduct + TRIG2_PRODUCT_ROUNDING_BIAS) >> TRIG2_PRODUCT_FRACTION_BITS;
	highProduct = correction * (major >> TRIG2_PRODUCT_FRACTION_BITS);
	g_trig2PolarDistance = major + highProduct + (uint16_t)lowProduct;
}

// FUNCTION: XW 0x4AA210
void trig2_calcarctan(int absX, signed int absY, uint16_t* outAngle, uint16_t* outRatioIndex) {
	unsigned int major = absX;
	unsigned int minor = absY;
	unsigned int fraction;
	uint16_t ratioIndex;
	g_trig2ArctanAxesSwapped = 0;
	if (major == minor) {
		g_trig2MajorAxisMagnitude = major;
		ratioIndex = TRIG2_ARCTAN_DIAGONAL_INDEX;
		fraction = 0;
	} else {
		if ((int)major <= (int)minor) {
			unsigned int swap = minor;
			minor = major;
			major = swap;
			g_trig2ArctanAxesSwapped = 1;
		}
		g_trig2MajorAxisMagnitude = major;
		if (major != 0) {
			if ((major & TRIG2_ARCTAN_HIGH_BYTE_MASK) == 0) {
				major <<= TRIG2_ARCTAN_NORMALIZE_SHIFT;
				minor <<= TRIG2_ARCTAN_NORMALIZE_SHIFT;
				if ((major & TRIG2_ARCTAN_HIGH_BYTE_MASK) == 0) {
					major <<= TRIG2_ARCTAN_NORMALIZE_SHIFT;
					minor <<= TRIG2_ARCTAN_NORMALIZE_SHIFT;
				}
			}
			if (major == minor) {
				fraction = minor >> TRIG2_PRODUCT_FRACTION_BITS;
				ratioIndex = TRIG2_ARCTAN_DIAGONAL_INDEX;
			} else {
				int ratio = (uint16_t)(minor / (major >> TRIG2_PRODUCT_FRACTION_BITS));
				fraction = (unsigned int)(uint8_t)ratio << TRIG2_ARCTAN_NORMALIZE_SHIFT;
				ratioIndex = ratio >> TRIG2_ARCTAN_NORMALIZE_SHIFT;
			}
		} else {
			fraction = minor >> TRIG2_PRODUCT_FRACTION_BITS;
			ratioIndex = 0;
		}
	}
	fraction &= TRIG2_ARCTAN_FRACTION_MASK;
	*outRatioIndex = ratioIndex;
	*outAngle = g_trig2ArctanTable[ratioIndex + 1];
	*outAngle = (fraction * (uint16_t)(*outAngle - g_trig2ArctanTable[*outRatioIndex])) >>
				TRIG2_PRODUCT_FRACTION_BITS;
	*outAngle += g_trig2ArctanTable[*outRatioIndex];
	if (g_trig2ArctanAxesSwapped != 0) {
		*outAngle = TRIG2_QUARTER_TURN - *outAngle;
	}
}

// FUNCTION: XW 0x4AA300
int trig2_arctan(int dy, int dx) {
	uint16_t angle;
	uint16_t ratioIndex;
	int signedAngle;
	if (dy < 0) {
		g_trig2NegativeY = 1;
		dy = (int)(0u - (unsigned int)dy);
	} else {
		g_trig2NegativeY = 0;
	}
	if (dx < 0) {
		g_trig2NegativeX = 1;
		dx = (int)(0u - (unsigned int)dx);
	} else {
		g_trig2NegativeX = 0;
	}
	trig2_calcarctan(dx, dy, &angle, &ratioIndex);
	signedAngle = angle;
	if (g_trig2NegativeY != 0) {
		signedAngle = -signedAngle;
	}
	if (g_trig2NegativeX != 0) {
		signedAngle = TRIG2_ANGLE_SIGN_BIT - signedAngle;
	}
	return signedAngle;
}
