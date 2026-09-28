#include "xw/frontend/scenes/flet640.h"
#include "xw_dos94/frontend/ceremony.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/scenes/ds_land.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/flet640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>

/* DOS94 0x540000. */
XwShellSceneResult Dos94_Flet640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	switch (shellext_Get_Cur_Scene()) {
		case 82:
			xfade_AddTimedText("Arriving at the Calamari Cruiser Defiance.", 4, 60, 0, 20, 36, 16);
			break;
		case 85:
			xfade_AddTimedText("Arriving at Rebel Fleet Medical Ship, the Salvation.", 4, 60, 0, 20, 16, 16);
			break;
		case 86:
			xfade_AddTimedText("Arriving at the Executor.  Flagship of the Empire!", 4, 60, 0, 20, 180, 16);
			break;
		case 188:
			xfade_AddTimedText("Imperial Fleet on course for Tatooine.", 4, 60, 0, 50, 180, 16);
			break;
	}
	resourceFile = xres_Open_Resource("fleets.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	if ((uint16_t)xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_flet640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(320 * 200, LANDRU_MEMORY_RESOURCE);

		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_RESCUE_ARRIVE_SALVATION:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640RebelSlowFilmName, &frame, 0,
														0, 0, Dos94_Flet640_film_Callback);
				break;
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, "totour_s", &frame, 0, 0, 0,
														Dos94_Flet640_film_Callback);
				break;
			case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640ImperialSlowFilmName, &frame,
														0, 0, 0, Dos94_Flet640_film_Callback);
				break;
			case XW_SCENE_RECOVER_PLANS_FLEET:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640PlansSlowFilmName, &frame, 0,
														0, 0, Dos94_Flet640_film_Callback);
				break;
		}
		g_flet640BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, FLET640_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_flet640BackgroundActor, Flet640_user_Background);
		xactor_Set_Actor_Draw_Function(g_flet640BackgroundActor, Dos94_Flet640_draw_Background);
		g_flet640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, FLET640_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_flet640CloseActor, Flet640_user_Close);
		xactor_Set_Actor_Draw_Function(g_flet640CloseActor, XwCutscene_DrawCloseOnRefresh);
	} else {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_RESCUE_ARRIVE_SALVATION:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640RebelFastFilmName, &frame, 0,
														0, 0, Dos94_Flet640_film_Callback);
				break;
			case XW_SCENE_TOUR_ARRIVE_DEFIANCE:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, "totour_f", &frame, 0, 0, 0,
														Dos94_Flet640_film_Callback);
				break;
			case XW_SCENE_CAPTURE_ARRIVE_EXECUTOR:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640ImperialFastFilmName, &frame,
														0, 0, 0, Dos94_Flet640_film_Callback);
				break;
			case XW_SCENE_RECOVER_PLANS_FLEET:
				g_flet640Film = xfilm_Res_Callback_Film(resourceFile, g_flet640PlansFastFilmName, &frame, 0,
														0, 0, Dos94_Flet640_film_Callback);
				break;
		}
	}
	xfilm_Set_Film_Def_Palette(g_flet640Film, shell->standardPalette);
	xview_Set_View_Update_Function(Flet640_end_View);
	Flet640_OpenMusic(resourceFile, g_flet640Film);
	DsLand_LoadSoundEffects(resourceFile, g_flet640Film);
	XwFlet640_RunView(resourceFile);
}

/* DOS94 0x540430. */
int16_t Dos94_Flet640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				if (xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					xactor_Set_Actor_User_Function(actor, Flet640_user_DirtyBounds);
				}
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				if (xio_Is_System_Slower_Than(FLET640_BACKGROUND_CACHE_SPEED_THRESHOLD) != 0) {
					Dos94_Flet640_film_Actor_To_Background(actor);
					consumeObject = 1;
				}
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, DsLand_user_Sound);
				break;
		}
	}
	return consumeObject;
}

/* DOS94 0x5404e4. */
int16_t Dos94_Flet640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_flet640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						320, 200, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_flet640BackgroundHandle);
	return drawResult;
}

/* DOS94 0x540638. */
int16_t Dos94_Flet640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_flet640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_flet640PreviousDirtyRect,
								  g_flet640PreviousDirtyRect.left, g_flet640PreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_flet640BackgroundHandle);
	return 1;
}

/* DOS94 0x540938. */
void Dos94_Flet640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		int scene = shellext_Get_Cur_Scene();
		if (scene == XW_SCENE_TOUR_ARRIVE_DEFIANCE) {
			Sound* music = xsound_Find_Gmid("plans");
			if (music != NULL) {
				soundext_FadeVolume(music, 0, FLET640_MUSIC_FADE_DURATION);

				soundext_SetPriority((intptr_t)music, FLET640_MUSIC_CLOSE_PRIORITY);
			}
		}
	}
}
