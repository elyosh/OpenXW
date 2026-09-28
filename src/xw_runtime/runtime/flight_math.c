#include "xw_runtime/runtime/flight_math.h"

#include "xw/flight/fview.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw_runtime/runtime/flight_types.h"

/* DOS clears the multiplication high word before adding/subtracting world Y. */
void XwFlightMath_MoveHyperspace(int rate) {
	int delta = rate * g_elapsedTicks;
	if (XwFlightTypes_Dos())
		delta = rate < 0 ? -(int)(uint16_t)-delta : (uint16_t)delta;
	g_playerFlightState.object->worldY =
		(int32_t)((uint32_t)g_playerFlightState.object->worldY + (uint32_t)delta);
}

/* DOS narrows each Q15 component before multiplying by the prediction horizon. */
int32_t XwFlightMath_PredictionStep(int16_t distance, int32_t direction) {
	int32_t step = (int32_t)(((int64_t)distance * direction) >> FVIEW_MATRIX_FRACTION_BITS);
	if (XwFlightTypes_Dos())
		return (int16_t)step;
	return step;
}
