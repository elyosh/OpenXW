#include "xw/frontend/scenes/med640.h"
#include "xw_dos94/frontend/recovery_scenes.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/med640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/memhdl.h>
#include <landru/timer.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x5021ec. */
XwShellSceneResult Dos94_Med640_Play(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource("medical.lfd");
	Rect frame;
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_med640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(64000, LANDRU_MEMORY_RESOURCE);
	g_med640Film =
		xfilm_Res_Callback_Film(sceneResource, "medic_f", &frame, 0, 0, 0, Dos94_Med640_film_Callback);
	xfilm_Set_Film_Def_Palette(g_med640Film, shell->standardPalette);
	g_med640BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, MED640_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_med640BackgroundActor, Med640_user_Background);
	xactor_Set_Actor_Draw_Function(g_med640BackgroundActor, Dos94_Med640_draw_Background);
	g_med640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, MED640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_med640CloseActor, Med640_user_Close);
	xactor_Set_Actor_Draw_Function(g_med640CloseActor, XwCutscene_DrawCloseOnRefresh);
	xview_Set_View_Update_Function(Med640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xtimer_Set_FrameRatePreset(LANDRU_FRAME_PRESET_1);
	Med640_OpenMusic(sceneResource, g_med640Film);
	XwMed640_RunView(sceneResource);
}

/* DOS94 0x50248a. */
int16_t Dos94_Med640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Dos94_Med640_film_Actor_To_Background(actor);
			consumeObject = 1;
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Med640_user_SoundCue);
		} else {
			if (actor->var2 != 0) {
				xactor_Set_Actor_Flag1(actor);
			}
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
				xactor_Set_Actor_User_Function(actor, Med640_user_DirtyActor);
			}
		}
	}
	return consumeObject;
}

/* DOS94 0x502526. */
int16_t Dos94_Med640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_med640BackgroundHandle);
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
	xmemhdl_Unlock_Handle(g_med640BackgroundHandle);
	return drawResult;
}

/* DOS94 0x50267a. */
int16_t Dos94_Med640_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									 int16_t refresh) {
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	backgroundPixels = xmemhdl_Lock_Handle(g_med640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_med640PreviousDirtyRect,
								  g_med640PreviousDirtyRect.left, g_med640PreviousDirtyRect.top, 320, 200);
	xmemhdl_Unlock_Handle(g_med640BackgroundHandle);
	return 1;
}

/* DOS94 0x502950. */
void Dos94_Med640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid("rescue");
		g_med640MusicState.sound = music;
		if (music != NULL) {
			soundext_SetPriority((intptr_t)music, 0);
			soundext_FadeVolume(g_med640MusicState.sound, 0, MED640_MUSIC_FADE_DURATION);
		}
	}
}
