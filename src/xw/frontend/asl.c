#include "xw/frontend/asl.h"

#include "xw/landru_config.h"
#include "xw/util/shared.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_adapter.h"
#include "xw_runtime/runtime/profile.h"
#include <aeron/log.h>
#include <landru/actanim.h>
#include <landru/actdelt.h>
#include <landru/actor.h>
#include <landru/dirty.h>
#endif
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/fade.h>
#include <landru/filedir.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/pal.h>
#include <landru/sound.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <stdlib.h>

// GLOBAL: XW 0x4FC4B4
int16_t g_aslActive = 0;

// FUNCTION: XW 0x46C9C0
void asl_Open_ASL(struct XwLegacyMemoryConfig* memory) {
#ifdef XW_MODERN
	Rect canvas_bounds;
#endif
	g_aslActive = 1;
	asl_ConfigureSystemDefaults();
	xmemhdl_ResetHandleTables(memory);
#ifdef XW_MODERN
	/* Shared Landru uses handlers where DOS dispatches ANIM and DELT tags directly. */
	xactor_Create_Actor_Module();
	xactanim_Create_Anim_Actor_Module();
	xactdelt_Create_Delta_Actor_Module();
#endif
	xsound_Open_Sound();
#ifdef XW_MODERN
	xdirty_Create_Dirty_List_Module(64);
#endif
	xcanvas_Create_Canvas_Module();
	xcanvas_InitClippingScratch();
	xio_Create_IO_Module();
#ifdef XW_MODERN
	/* Shared Landru copies completed scenes through its dirty lists, as in OpenTIE. */
	xcanvas_Get_Drawing_Canvas_Bounds(&canvas_bounds);
	xdirty_Dirty_Master_Rect(&canvas_bounds);
	xdirty_Set_Dirty_Merge();
	xdirty_Max_Dirty_List();
	/* DOS VGA background blits can change pixels outside actor dirty rectangles. */
	if (XwProfile_DosFrontend())
		xdirty_Set_Dirty_Disable();
	if (!XwLandru_OpenVideo()) {
		Aeron_FatalError("OpenXW", "Cannot create Landru video surfaces");
		exit(EXIT_FAILURE);
	}
#endif
	xview_Create_View_Module();
	xfade_Create_Fade_Module();
	xpal_Create_Palette_Module();
	xcursor_Create_Cursor_Module();
	xfiledir_Create_Directory_Module();
	xtimer_Often();
}

// FUNCTION: XW 0x46CA10
void asl_Close_ASL(void) {
	g_aslActive = 0;
	xfiledir_Destroy_Directory_Module();
	xcursor_Destroy_Cursor_Module();
	xpal_Destroy_Palette_Module();
	xfade_Destroy_Fade_Module();
	xview_Destroy_View_Module();
	nullsub_SharedNoOp();
	xcanvas_FreeClippingScratch();
#ifdef XW_MODERN
	XwLandru_CloseVideo();
	xcanvas_Destroy_Canvas_Module();
	xdirty_Destroy_Dirty_List_Module();
	xio_Destroy_IO_Module();
#endif
	j_nullsub_2();
	xsound_Close_Sound();
#ifdef XW_MODERN
	xactdelt_Destroy_Delta_Actor_Module();
	xactanim_Destroy_Anim_Actor_Module();
	xactor_Destroy_Actor_Module();
	xres_Destroy_Resource_Module();
#endif
	xmem_Close_Memory();
	nullsub_SharedNoOp();
}

// FUNCTION: XW 0x46CAB0
void asl_ConfigureSystemDefaults(void) {
	xio_Set_System_Speed(LANDRU_SYSTEM_SPEED_DEFAULT);
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
	nullsub_SharedNoOp();
}
