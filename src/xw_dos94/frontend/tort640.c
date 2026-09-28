#include "xw/frontend/scenes/tort640.h"
#include "xw_dos94/frontend/recovery_scenes.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/b1b640.h"
#include "xw/frontend/scenes/cutscene.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/tort640_task.h"
#endif
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>
#include <string.h>

/* DOS94 0x5c0edc. */
XwShellSceneResult Dos94_Tort640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	int first = xio_Is_System_Slower_Than(2) ? 30 : 70;
	xfade_AddTimedText("Now we will discuss the position", first, first + 70, 0, 60, 8, 15);
	xfade_AddTimedText("of the secret rebel base!", first, first + 70, 0, 60, 18, 15);
	resourceFile = xres_Open_Resource("torture.lfd");
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	g_tort640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(320 * 200, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	if ((uint16_t)xio_Is_System_Slower_Than(TORT640_SPEED_THRESHOLD) != 0) {
		g_tort640ScrollActor = NULL;
		g_tort640VerticalOffset = 0;
		g_tort640Film =
			xfilm_Res_Callback_Film(resourceFile, "trfilm_s", &frame, 0, 0, 0, Dos94_Tort640_film_Callback);
	} else {
		g_tort640VerticalOffset = TORT640_FAST_VERTICAL_OFFSET;
		g_tort640Film =
			xfilm_Res_Callback_Film(resourceFile, "trfilm_f", &frame, 0, 0, 0, Dos94_Tort640_film_Callback);
	}
	xfilm_Set_Film_Def_Palette(g_tort640Film, shell->standardPalette);
	g_tort640BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TORT640_BACKGROUND_Z);
	xactor_Set_Actor_User_Function(g_tort640BackgroundActor, Tort640_user_Background);
	xactor_Set_Actor_Draw_Function(g_tort640BackgroundActor, Dos94_Tort640_draw_Background);
	g_tort640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, TORT640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_tort640CloseActor, Tort640_user_Close);
	xactor_Set_Actor_Draw_Function(g_tort640CloseActor, Cutscene_DrawConditionalErase);
	xview_Set_View_Update_Function(Tort640_end_View);
	Tort640_OpenMusic(resourceFile, g_tort640Film);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	XwTort640_RunView(resourceFile);
}

/* DOS94 0x5c122e. */
int16_t Dos94_Tort640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumed = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = (Actor*)object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Tort640_user_DirtyActor);
				if (actor->var2 == TORT640_SCROLL_ACTOR) {
					g_tort640ScrollActor = actor;
				}
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Dos94_Tort640_film_Actor_To_Background(actor);
				consumed = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Tort640_user_SoundCue);
				break;
		}
	}
	return consumed;
}

/* DOS94 0x5c12e2. */
int16_t Dos94_Tort640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_tort640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						320, 200, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult =
			actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y - g_tort640VerticalOffset, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_tort640BackgroundHandle);
	return drawResult;
}

/* DOS94 0x5c1452. */
int16_t Dos94_Tort640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
									  int16_t unusedX, int16_t unusedY, int16_t refresh) {
	Rect sourceRect;
	int16_t destinationX;
	int16_t destinationY;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, 320, 200);
	if (g_tort640ScrollActor != NULL) {
		xrect_Offset_Rect(&sourceRect, 0, g_tort640ScrollActor->y + g_tort640VerticalOffset);
	}
	xrect_Clip_Rect(&sourceRect, &g_tort640PreviousDirtyRect);
	destinationX = sourceRect.left;
	destinationY = sourceRect.top;
	if (g_tort640ScrollActor != NULL) {
		xrect_Offset_Rect(&sourceRect, 0, -(g_tort640ScrollActor->y + g_tort640VerticalOffset));
	}
	backgroundPixels = (const uint8_t*)xmemhdl_Lock_Handle(g_tort640BackgroundHandle);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, destinationX, destinationY, 320, 200);
	xmemhdl_Unlock_Handle(g_tort640BackgroundHandle);
	return 1;
}

/* DOS94 0x5c176c. */
void Dos94_Tort640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		Sound* music = xsound_Find_Gmid("torture");
		g_tort640MusicState.sound = music;
		if (music != NULL) {
			soundext_SetPriority((intptr_t)music, 0);
			soundext_FadeVolume(g_tort640MusicState.sound, 0, TORT640_MUSIC_FADE_DURATION);
		}
	}
}
