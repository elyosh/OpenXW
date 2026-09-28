#include "xw/frontend/scenes/ds_bay.h"
#include "xw_dos94/frontend/recovery_scenes.h"
#include "xw_runtime/runtime/scene_view_task.h"
#include <landru/view.h>
static ResFile* resource;

static void release_scene(void) {
	xview_Clear_View_Update_Function();
	xres_Close_Resource(resource);
	resource = NULL;
}

/* DOS94 0x4010d0. */
void Dos94_DsBay(XwShellContext* shell) {
	resource = xres_Open_Resource("dsbay.lfd");
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_dsbayFilm = xfilm_Res_Film(resource, "dsbay_f", &frame, 0, 0, 0);
	xfilm_Set_Film_Def_Palette(g_dsbayFilm, shell->standardPalette);
	xview_Set_View_Update_Function(DsBay_end_View);
	DsBay_OpenMusic(resource, g_dsbayFilm);
	XwScene_RunView(NULL, release_scene);
}
