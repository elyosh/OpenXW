#include "xw/frontend/tooltip.h"

#include "xw/render/shade.h"
#include "xw/util/shared.h"

#include <landru/memhdl.h>
#include <landru/pal.h>

// FUNCTION: XW 0x4698A0
void Tooltip_BuildScreenPaletteRemap(void) {
	Palette* palette = xpal_Get_Screen_Palette();
	const struct RgbTriplet* colors = xmemhdl_Lock_Handle(palette->colors);
	shade_Set_Shaded_Palette(colors, TOOLTIP_SHADE_BLEND_FACTOR, 0, 0, 0);
	nullsub_SharedNoOp();
}
