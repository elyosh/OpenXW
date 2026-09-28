#include "xw_runtime/integration/brief_callbacks.h"
#include "xw/frontend/brief.h"

void XwBrief_DrawPlaybackButton(Input* input, Rect* frame, Rect* clip, int16_t refresh) {
	brief_DrawPlaybackButton((PushButton*)input, frame, clip, refresh);
}
