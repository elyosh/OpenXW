#include "xw/frontend/scenes/rescue64.h"
#include "xw_dos94/frontend/recovery_scenes.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"

#ifdef XW_MODERN
#include "xw_runtime/integration/landru_sound.h"
#include "xw_runtime/runtime/rescue64_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x541e58. */
XwShellSceneResult Dos94_Rescue64_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Rect frame;
	if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED) {
		xfade_AddTimedText("We've found him!", RESCUE64_FIRST_CAPTION_START, RESCUE64_FIRST_CAPTION_END, 0,
						   RESCUE64_CAPTION_X, RESCUE64_CAPTION_Y, 14);
		xfade_AddTimedText("He's OK.", RESCUE64_SECOND_CAPTION_START, RESCUE64_SECOND_CAPTION_END, 0,
						   RESCUE64_CAPTION_X, RESCUE64_CAPTION_Y, 14);
	} else {
		xfade_AddTimedText("There's the rebel!", RESCUE64_FIRST_CAPTION_START, RESCUE64_FIRST_CAPTION_END, 0,
						   RESCUE64_CAPTION_X, RESCUE64_CAPTION_Y, 14);
		xfade_AddTimedText("Bring him in.", RESCUE64_SECOND_CAPTION_START, RESCUE64_SECOND_CAPTION_END, 0,
						   RESCUE64_CAPTURE_SECOND_X, RESCUE64_CAPTION_Y, 14);
	}
	sceneResource = xres_Open_Resource("rescue.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(RESCUE64_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		Film* slowFilm;
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_rescue64BackgroundHandle = xmemhdl_Alloc_Clear_Handle(320 * 200, LANDRU_MEMORY_RESOURCE);
		if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED)
			slowFilm =
				xfilm_Res_Callback_Film(sceneResource, "rescr_s", &frame, 0, 0, 0, Rescue64_film_Callback);
		else
			slowFilm =
				xfilm_Res_Callback_Film(sceneResource, "resci_s", &frame, 0, 0, 0, Rescue64_film_Callback);
		g_rescue64Film = slowFilm;
		g_rescue64BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, RESCUE64_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_rescue64BackgroundActor, Rescue64_user_Background);
		xactor_Set_Actor_Draw_Function(g_rescue64BackgroundActor, Rescue64_draw_Background);
		g_rescue64CloseActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, RESCUE64_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_rescue64CloseActor, Rescue64_user_Close);
		xactor_Set_Actor_Draw_Function(g_rescue64CloseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		Film* fastFilm;
		if (shellext_Get_Cur_Scene() == XW_SCENE_PILOT_RESCUED)
			fastFilm =
				xfilm_Res_Callback_Film(sceneResource, "rescr_f", &frame, 0, 0, 0, Rescue64_film_Callback);
		else
			fastFilm =
				xfilm_Res_Callback_Film(sceneResource, "resci_f", &frame, 0, 0, 0, Rescue64_film_Callback);
		g_rescue64Film = fastFilm;
	}
	xfilm_Set_Film_Def_Palette(g_rescue64Film, shell->standardPalette);
	xview_Set_View_Update_Function(Rescue64_end_View);
	Rescue64_OpenMusic(sceneResource, g_rescue64Film);
	XwRescue64_RunView(sceneResource);
}
