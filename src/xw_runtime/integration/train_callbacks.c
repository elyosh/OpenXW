#include "xw_runtime/integration/train_callbacks.h"
#include "xw/frontend/train.h"

void XwTrain_DrawNavigationButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	train_idraw_NavigationButton((PushButton*)input, frame, clip, refresh);
}
