#include "xw/frontend/scenes/intro2.h"
#include "xw_dos94/frontend/intro.h"

#include "xw/audio/lolevel.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/intro2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x4c2fdc. */
XwShellSceneResult Dos94_Intro2_Attack(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("intro.lfd");
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	if ((uint16_t)xio_Is_System_Slower_Than(INTRO2_BACKGROUND_CACHE_SPEED) != 0) {
		g_intro2BackgroundBuffers[0] =
			xmemhdl_Alloc_Clear_Handle(INTRO2_LOWER_BUFFER_BYTES, LANDRU_MEMORY_RESOURCE);
		g_intro2BackgroundBuffers[1] =
			xmemhdl_Alloc_Clear_Handle(INTRO2_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
		xrect_Copy_Rect(&g_intro2BackgroundRedrawRect, &frame);
		xrect_Set_Rect(&g_intro2DirtyRect, 0, 0, 0, 0);
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		g_intro2Film =
			xfilm_Res_Callback_Film(sceneResource, "intro2_s", &frame, 0, 0, 0, Intro2_film_Callback);
		xfilm_Set_Film_Def_Palette(g_intro2Film, shell->standardPalette);
		g_intro2BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, INTRO2_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_intro2BackgroundActor, Intro2_user_Background);
		xactor_Set_Actor_Draw_Function(g_intro2BackgroundActor, Intro2_draw_Background);
		g_intro2EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, INTRO2_ERASE_Z);
		xactor_Set_Actor_User_Function(g_intro2EraseActor, Intro2_user_Erase);
		xactor_Set_Actor_Draw_Function(g_intro2EraseActor, Cutscene_DrawConditionalErase);
	} else {
		g_intro2Film =
			xfilm_Res_Callback_Film(sceneResource, "intro2_f", &frame, 0, 0, 0, Intro2_film_Callback);
		xfilm_Set_Film_Def_Palette(g_intro2Film, shell->standardPalette);
	}
	xview_Set_View_Update_Function(Dos94_Intro2_end_View);
	Intro2_OpenMusic(sceneResource, g_intro2Film);
	if (ShellPreferences_GetMusicEnabled())
		xsound_Set_Sound_User_Function(g_intro2MusicState.sound, Dos94_Intro2_user_Music);
	XwIntro2_RunView(sceneResource);
}

/* DOS94 0x4c328e. */
void Dos94_Intro2_end_View(int time) {
	int16_t exitScene;

	(void)time;
	if (shellext_Check_Scene_Exit(&exitScene, 15, XW_SCENE_REGISTER_INITIAL,
								  g_intro2Film->cur_cel == g_intro2Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
	xrect_Set_Rect(&g_intro2DirtyRect, 0, 0, 0, 0);
}

/* DOS94 0x4c38f2. */
void Dos94_Intro2_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	switch (g_intro2MusicState.film->cur_cel) {
		case INTRO2_MUSIC_FIRST_CONTROL_CEL:
			soundext_SetHook(g_intro2MusicState.sound, XW_SOUND_CONTROL_DIRECT, INTRO2_MUSIC_FIRST_CONTROL,
							 0);
			break;
		case INTRO2_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_FadeVolume(g_intro2MusicState.sound, INTRO2_MUSIC_QUIET_VOLUME,
									INTRO2_MUSIC_QUIET_DURATION);
			break;
		case INTRO2_MUSIC_SECOND_CONTROL_CEL:
			soundext_SetHook(g_intro2MusicState.sound, XW_SOUND_CONTROL_DIRECT, INTRO2_MUSIC_SECOND_CONTROL,
							 0);
			break;
		case INTRO2_MUSIC_THIRD_CONTROL_CEL:
			soundext_SetHook(g_intro2MusicState.sound, XW_SOUND_CONTROL_DIRECT, INTRO2_MUSIC_THIRD_CONTROL,
							 0);
			break;
		case INTRO2_MUSIC_LOUD_CEL:
			soundext_FadeVolume(g_intro2MusicState.sound, 128, INTRO2_MUSIC_LOUD_DURATION);
			break;
	}
}
