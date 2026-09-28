#include "xw_runtime/integration/input_callbacks.h"
#include "xw/util/shared.h"

int16_t XwInput_UpdateNoOp(Input* input, Rect* frame, Rect* clip, int16_t key, InputMouseEvent leftEvent,
						   InputMouseEvent rightEvent, int16_t x, int16_t y) {
	(void)input;
	(void)frame;
	(void)clip;
	(void)key;
	(void)leftEvent;
	(void)rightEvent;
	(void)x;
	(void)y;
	return Shared_ReturnZero();
}

void XwInput_UserNoOp(Input* input, int time) {
	(void)input;
	(void)time;
	nullsub_SharedNoOp();
}
