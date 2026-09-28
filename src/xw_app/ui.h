#ifndef XW_APP_UI_H
#define XW_APP_UI_H
#include "aeron/aeron.h"
#include "aeron/scene/font_atlas.h"
#include "aeron/scene/ui.h"
#include <stdbool.h>

typedef struct XwAppUi {
	AeronUiContext* context;
	AeronFontAtlas font;
} XwAppUi;

bool XwAppUi_Init(XwAppUi* ui, const char* font, char* error, size_t capacity);
void XwAppUi_Shutdown(XwAppUi* ui);
AeronUiContext* XwAppUi_Context(XwAppUi* ui);
#endif
