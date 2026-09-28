#include "xw/math/math.h"

#ifndef XW_MODERN
#include <float.h>
#endif

// FUNCTION: XW 0x47EE90
void Math_SetFpuSinglePrecisionMode(void) {
#ifdef XW_MODERN
	/* Supported modern hosts use native arithmetic rather than x87 precision control. */
#else
	enum {
		FPU_PRECISION_CONTROL_MASK = 0x00030000,
		FPU_SINGLE_PRECISION = 0x00020000,
	};

	_control87(FPU_SINGLE_PRECISION, FPU_PRECISION_CONTROL_MASK);
#endif
}

// FUNCTION: XW 0x47EEC0
void Math_SetFpuExtendedPrecisionMode(void) {
#ifdef XW_MODERN
	/* Supported modern hosts use native arithmetic rather than x87 precision control. */
#else
	enum {
		FPU_PRECISION_CONTROL_MASK = 0x00030000,
		FPU_EXTENDED_PRECISION = 0x00000000,
	};

	_control87(FPU_EXTENDED_PRECISION, FPU_PRECISION_CONTROL_MASK);
#endif
}
