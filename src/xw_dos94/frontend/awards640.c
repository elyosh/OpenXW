#include "xw/frontend/scenes/awards640.h"
#include "xw_dos94/frontend/ceremony.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#ifdef XW_MODERN
#include "xw_runtime/runtime/awards640_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/font.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>

/* DOS94 0x5c2d00. */
XwShellSceneResult Dos94_Awards640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	resourceFile = xres_Open_Resource("awards.lfd");
	if (shellext_Get_Cur_Scene() == 240)
		xfade_AddTimedText("Congratulations on behalf of the Rebel Alliance.", 4, 50, 0, 20, 8, 14);
	xrect_Set_Rect(&frame, 0, 0, 320, 200);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	g_awards640PreviousScrollY = 0;
	g_awards640FullRefreshCountdown = 0;
	g_awards640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(320 * 200, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_AWARD_CEREMONY_1:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award1_s", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award1_f", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			break;
		case XW_SCENE_AWARD_CEREMONY_2:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award2_s", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award2_f", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			break;
		case XW_SCENE_AWARD_CEREMONY_3:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award3_s", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "award3_f", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_1:
			g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal1_f", &frame, 0, 0, 0,
													  Dos94_Awards640_film_Callback);
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_2:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal2_s", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal2_f", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			break;
		case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
			if ((uint16_t)xio_Is_System_Slower_Than(AWARDS640_SPEED_THRESHOLD) != 0)
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal3_s", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			else
				g_awards640Film = xfilm_Res_Callback_Film(resourceFile, "medal3_f", &frame, 0, 0, 0,
														  Dos94_Awards640_film_Callback);
			break;
	}
	xfilm_Set_Film_Def_Palette(g_awards640Film, shell->standardPalette);
	g_awards640BackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, AWARDS640_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_awards640BackgroundActor, Dos94_Awards640_draw_Background);
	g_awards640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, AWARDS640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_awards640CloseActor, Awards640_user_Close);
	xactor_Set_Actor_Draw_Function(g_awards640CloseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_awards640DirtyRect);
	xview_Set_View_Update_Function(Awards640_end_View);
	Awards640_OpenMusic(resourceFile, g_awards640Film);
	XwAwards640_RunView(resourceFile);
}

/* DOS94 0x5c3144. */
int16_t Dos94_Awards640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Awards640_user_Actor);
				break;
			case AWARDS640_ACTOR_SCROLL:
				g_awards640ScrollActor = actor;
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				if (actor->var2 == AWARDS640_STAMP_MEDAL) {
					int16_t medalIndex = shipext_Get_Pending_Medal_Index();
					if (medalIndex < AWARDS640_MEDAL_LINEAR_END) {
						if (medalIndex < AWARDS640_MEDAL_LINEAR_FIRST) {
							xactor_Set_Actor_State(actor, AWARDS640_MEDAL_DEFAULT_STATE, 0);
						} else {
							xactor_Set_Actor_State(actor, medalIndex + AWARDS640_MEDAL_STATE_OFFSET, 0);
						}
					} else {
						switch (medalIndex) {
							case AWARDS640_MEDAL_SPECIAL_7:
								xactor_Set_Actor_State(actor, AWARDS640_MEDAL_7_STATE, 0);
								break;
							case AWARDS640_MEDAL_SPECIAL_8:
								xactor_Set_Actor_State(actor, AWARDS640_MEDAL_8_STATE, 0);
								break;
							case AWARDS640_MEDAL_SPECIAL_9:
								xactor_Set_Actor_State(actor, AWARDS640_MEDAL_9_STATE, 0);
								break;
						}
					}
				}
				Dos94_Awards640_ActorToScreenAndBackground(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Awards640_user_Sound);
				break;
		}
	}
	return consumeObject;
}

/* DOS94 0x5c324c. */
int16_t Dos94_Awards640_ActorToScreenAndBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	int16_t drawResult = 0;
	int backgroundYOffset = 0;
	int16_t pass;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (pass = 0; pass < AWARDS640_DRAW_PASS_COUNT; ++pass) {
		if (pass != 0) {
			uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_awards640BackgroundHandle);
			xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth,
								&previousHeight, 320, 200, 0);
		}
		if (actor->draw != NULL) {
			drawResult =
				actor->draw(actor, &canvasBounds, &canvasBounds, actor->x, actor->y + backgroundYOffset, 1);
		}
		if (pass != 0) {
			xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
			xmemhdl_Unlock_Handle(g_awards640BackgroundHandle);
		}
		switch ((int16_t)shellext_Get_Cur_Scene()) {
			case XW_SCENE_AWARD_CEREMONY_1:
				backgroundYOffset -= 200;
				break;
			case XW_SCENE_AWARD_CEREMONY_3:
			case XW_SCENE_VICTORY_MEDAL_CEREMONY_3:
				backgroundYOffset += 200;
				break;
		}
	}
	return drawResult;
}

/* DOS94 0x5c3360. */
int16_t Dos94_Awards640_draw_Background(Actor* actor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
										int16_t unusedY, int16_t refresh) {
	Rect scrollBounds;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	if (g_awards640ScrollActor != NULL &&
		(g_awards640PreviousScrollY != g_awards640ScrollActor->y || g_awards640ScrollActor->var2 != 0)) {
		uint8_t* screenPixels = xcanvas_Get_Screen_Buffer();
		int16_t deltaY;
		uint8_t* backgroundPixels;
		int16_t scene;
		xcanvas_Get_Drawing_Canvas_Bounds(&scrollBounds);
		if (shellext_Get_Cur_Scene() == XW_SCENE_AWARD_CEREMONY_1)
			scrollBounds.bottom = g_awards640PreviousScrollY + 200;
		else
			scrollBounds.top = g_awards640PreviousScrollY;
		deltaY = g_awards640ScrollActor->y - g_awards640PreviousScrollY;
		if (actor->var2 == 0)
			xcanvas_Scroll_Clipped_Buffer(screenPixels, &scrollBounds, 0, deltaY,
										  xcanvas_Get_Current_Canvas_Bitmap()->w, 200);
		xcanvas_Get_Drawing_Canvas_Bounds(&scrollBounds);
		backgroundPixels = xmemhdl_Lock_Handle(g_awards640BackgroundHandle);
		scene = shellext_Get_Cur_Scene();
		if (scene == XW_SCENE_AWARD_CEREMONY_1)
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &scrollBounds, 0,
										  deltaY + g_awards640PreviousScrollY + 200, 320, 200);
		else
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &scrollBounds, 0,
										  deltaY + g_awards640PreviousScrollY - 200, 320, 200);
		xmemhdl_Unlock_Handle(g_awards640BackgroundHandle);
		g_awards640PreviousScrollY = g_awards640ScrollActor->y;
	}
	return 1;
}

/* DOS94 0x5c3770. */
void Dos94_Awards640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && shellext_Get_Cur_Scene() == XW_SCENE_AWARD_CEREMONY_3) {
		Sound* music = xsound_Find_Gmid("recruits");
		g_awards640Music = music;
		if (music != NULL) {
			soundext_FadeVolume(music, 0, AWARDS640_MUSIC_FADE_DURATION);

			soundext_SetPriority((intptr_t)music, 0);
		}
	}
}
