#include "xw/frontend/scenes/sabotage.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/sabotage_middle_task.h"
#include "xw_runtime/runtime/sabotage_task.h"
#endif

#include <landru/actback.h>
#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4FA160
Film* g_sabotageFilm = NULL;

// GLOBAL: XW 0x4FA168
Actor* g_sabotageBackgroundSaveActor = NULL;

// GLOBAL: XW 0x4FA16C
Actor* g_sabotageClearActor = NULL;

// GLOBAL: XW 0x4FA170
int16_t g_sabotageHalfHeightBackground = 0;

// GLOBAL: XW 0x4FA174
LandruHandle g_sabotageBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FA178
Rect g_sabotageMiddleDirtyRect = { 0 };

// GLOBAL: XW 0x4FA180
Film* g_sabotageMiddleFilm = NULL;

// GLOBAL: XW 0x4FA184
Actor* g_sabotageMiddleBackgroundActor = NULL;

// GLOBAL: XW 0x4FA188
Actor* g_sabotageMiddleScrollActor = NULL;

// GLOBAL: XW 0x4FA18C
Actor* g_sabotageMiddleViewActor = NULL;

// GLOBAL: XW 0x4FA190
LandruHandle g_sabotageMiddleBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4FA194
int16_t g_sabotageMiddleFullRedrawFrames = 0;

// GLOBAL: XW 0x4FA198
int16_t g_sabotageMiddlePreviousX = 0;

// GLOBAL: XW 0x4FA19C
Sound* g_sabotageMusic = NULL;

// GLOBAL: XW 0x4FA1A0
Film* g_sabotageMusicFilm = NULL;

// GLOBAL: XW 0x4FA1A4
Sound* g_sabotageMiddleMusic = NULL;

// GLOBAL: XW 0x4FA1A8
Film* g_sabotageMiddleMusicFilm = NULL;

// GLOBAL: XW 0x4FA1B0
Sound* g_sabotageMiddleSpeech33 = NULL;

// GLOBAL: XW 0x4FA1B4
Sound* g_sabotageMiddleSpeech10 = NULL;

// GLOBAL: XW 0x4FA1B8
Sound* g_sabotageMiddleSpeech3 = NULL;

// GLOBAL: XW 0x4FA1BC
Sound* g_sabotageMiddleSpeech17 = NULL;

// FUNCTION: XW 0x45FF10
XwShellSceneResult Sabotage_Sabotage(struct XwShellContext* shellContext) {
	ResFile* resourceFile;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	if (shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_1) {
		xfade_AddTimedText("Rebel agents on approach to Star Destroyer Invincible.",
						   SABOTAGE_APPROACH_CAPTION_START, SABOTAGE_APPROACH_CAPTION_END,
						   SABOTAGE_CAPTION_FONT, SABOTAGE_CAPTION_X, SABOTAGE_APPROACH_CAPTION_Y,
						   SABOTAGE_CAPTION_COLOR);
		xfade_AddTimedText("Security codes approved.  Shuttle landing in Main Bay.",
						   SABOTAGE_LANDING_CAPTION_START, SABOTAGE_LANDING_CAPTION_END,
						   SABOTAGE_CAPTION_FONT, SABOTAGE_CAPTION_X, SABOTAGE_LANDING_CAPTION_Y,
						   SABOTAGE_CAPTION_COLOR);
	}
	resourceFile = xres_Open_Resource("sab1.lfd");
	xrect_Set_Rect(&frame, 0, 0, SABOTAGE_BACKGROUND_WIDTH, SABOTAGE_BACKGROUND_HEIGHT);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_sabotageClearActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, SABOTAGE_CLEAR_Z);
	xactor_Set_Actor_Draw_Function(g_sabotageClearActor, Sabotage_draw_Clear);
	g_sabotageHalfHeightBackground = 0;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_DESTROY_STAR_DESTROYER_1:
			if ((uint16_t)xio_Is_System_Slower_Than(SABOTAGE_FILM_SPEED_THRESHOLD) != 0) {
				g_sabotageBackgroundSaveActor =
					xactback_Alloc_Back_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, SABOTAGE_SAVE_Z);
				xactback_Alloc_Back_Actor_Buffer(g_sabotageBackgroundSaveActor,
												 SABOTAGE_HALF_BACKGROUND_BYTES);
				g_sabotageFilm = xfilm_Res_Callback_Film(resourceFile, "sab1_s", &frame, 0, 0, 0,
														 Sabotage_film_Slow_Callback);
			} else {
				g_sabotageBackgroundHandle =
					xmemhdl_Alloc_Clear_Handle(SABOTAGE_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
				g_sabotageFilm =
					xfilm_Res_Callback_Film(resourceFile, "sab1_f", &frame, 0, 0, 0, Sabotage_film_Callback);
			}
			break;
		case XW_SCENE_DESTROY_STAR_DESTROYER_3:
			if (Shared_ReturnZero() != 0) {
				g_sabotageBackgroundHandle =
					xmemhdl_Alloc_Clear_Handle(SABOTAGE_HALF_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
				g_sabotageHalfHeightBackground = 1;
				g_sabotageFilm =
					xfilm_Res_Callback_Film(resourceFile, "sab3_l", &frame, 0, 0, 0, Sabotage_film_Callback);
			} else {
				g_sabotageBackgroundHandle =
					xmemhdl_Alloc_Clear_Handle(SABOTAGE_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
				if ((uint16_t)xio_Is_System_Slower_Than(SABOTAGE_FILM_SPEED_THRESHOLD) != 0)
					g_sabotageFilm = xfilm_Res_Callback_Film(resourceFile, "sab3_s", &frame, 0, 0, 0,
															 Sabotage_film_Callback);
				else
					g_sabotageFilm = xfilm_Res_Callback_Film(resourceFile, "sab3_f", &frame, 0, 0, 0,
															 Sabotage_film_Callback);
			}
			break;
	}
	xfilm_Set_Film_Def_Palette(g_sabotageFilm, shellContext->standardPalette);
	xview_Set_View_Update_Function(Sabotage_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Sabotage_OpenMusic(resourceFile, g_sabotageFilm);
	Sabotage_LoadSoundEffects();
#ifdef XW_MODERN
	XwSabotage_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Sabotage_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	if ((uint16_t)xio_Is_System_Slower_Than(SABOTAGE_FILM_SPEED_THRESHOLD) == 0 ||
		shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_3)
		xmemhdl_Free_Handle(g_sabotageBackgroundHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4601F0
void Sabotage_end_View(int time) {
	int16_t exitScene;
	(void)time;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_DESTROY_STAR_DESTROYER_1:
			if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DESTROY_STAR_DESTROYER_2,
										  shipext_Get_Pending_Medal_Scene(),
										  g_sabotageFilm->cur_cel == g_sabotageFilm->cels) != 0)
				xerror_Set_Landru_Exit(exitScene);
			break;
		case XW_SCENE_DESTROY_STAR_DESTROYER_3:
			if (shellext_Check_Scene_Exit(&exitScene, shipext_Get_Pending_Medal_Scene(),
										  shipext_Get_Pending_Medal_Scene(),
										  g_sabotageFilm->cur_cel == g_sabotageFilm->cels) != 0)
				xerror_Set_Landru_Exit(exitScene);
			break;
	}
}

// FUNCTION: XW 0x460290
int16_t Sabotage_film_Callback(Film* film, FilmObject* object) {
	int16_t discardRecord = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Sabotage_film_Actor_To_Background(actor);
			if (actor->var2 == SABOTAGE_BACKGROUND_DRAW_TILED) {
				xactor_Set_Actor_Draw_Function(actor, Sabotage_draw_TiledBackground);
			}
			discardRecord = actor->var2 == 0;
		}
		if (actor->var1 == SABOTAGE_ACTOR_SOUND) {
			xactor_Set_Actor_User_Function(actor, Sabotage_user_Sound);
		}
	}
	return discardRecord;
}

// FUNCTION: XW 0x460300
int16_t Sabotage_film_Slow_Callback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		int16_t role;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		role = actor->var1;
		if (role >= SABOTAGE_ACTOR_NON_REFRESHABLE_FIRST) {
			xactor_Non_Refreshable_Actor(actor);
			xactor_Set_Actor_User_Function(actor, Sabotage_user_SlowActor);
		} else if (role == SABOTAGE_ACTOR_SLOW) {
			xactor_Set_Actor_User_Function(actor, Sabotage_user_SlowActor);
		}
		if (actor->var1 == SABOTAGE_ACTOR_SOUND) {
			xactor_Set_Actor_User_Function(actor, Sabotage_user_Sound);
		}
	}
	return 0;
}

// FUNCTION: XW 0x460370
int16_t Sabotage_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	int16_t backgroundHeight;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_sabotageBackgroundHandle);
	backgroundHeight =
		g_sabotageHalfHeightBackground != 0 ? SABOTAGE_HALF_BACKGROUND_HEIGHT : SABOTAGE_BACKGROUND_HEIGHT;
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						SABOTAGE_BACKGROUND_WIDTH, backgroundHeight, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_sabotageBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x460460
void Sabotage_user_Sound(Actor* actor, int time) {
	int16_t action = actor->var2;
	(void)time;
	if (action != 0) {
		Sabotage_HandleSoundAction(action);
	}
}

// FUNCTION: XW 0x460480
void Sabotage_user_SlowActor(Actor* actor, int time) {
	(void)time;
	switch (actor->var1) {
		case SABOTAGE_ACTOR_SLOW:
			if ((uint16_t)xactor_Is_Actor_Visible(actor) != 0) {
				Rect actorRect;
				xactor_Get_Actor_Rect(actor, &actorRect);
				if (g_sabotageFilm->cur_cel < g_sabotageFilm->cels - 1) {
					xactback_Set_Back_Actor_Rect(g_sabotageBackgroundSaveActor, &actorRect);
				} else {
					xactback_Stop_Back_Actor(g_sabotageBackgroundSaveActor);
				}
			}
			break;
		case SABOTAGE_ACTOR_NON_REFRESHABLE_FIRST:
			if (actor->var2 == 1) {
				xactor_Refresh_Actor(actor);
			}
			break;
		case SABOTAGE_ACTOR_CLEAR_CONTROL:
			if (actor->var2 == 1) {
				xactor_Refresh_Actor(actor);
			}
			g_sabotageClearActor->var1 = actor->var2;
			break;
	}
}

// FUNCTION: XW 0x460540
int16_t Sabotage_draw_TiledBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh) {
	Rect sourceRect;
	int16_t baseX;
	int16_t baseY;
	int16_t tileX;
	int16_t tileY;
	int tileHeight;
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)frame;
	(void)clip;
	if (refresh == 0) {
		return 0;
	}
	baseX = x;
	baseY = y;
	if (x >= 0) {
		baseX -= SABOTAGE_BACKGROUND_WIDTH;
	}
	if (y >= 0) {
		baseY -= SABOTAGE_BACKGROUND_HEIGHT;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, SABOTAGE_BACKGROUND_WIDTH, SABOTAGE_BACKGROUND_HEIGHT);
	backgroundPixels = xmemhdl_Lock_Handle(g_sabotageBackgroundHandle);
	tileHeight =
		g_sabotageHalfHeightBackground != 0 ? SABOTAGE_HALF_BACKGROUND_HEIGHT : SABOTAGE_BACKGROUND_HEIGHT;
	for (tileY = 0; tileY < SABOTAGE_TILE_COVERAGE_HEIGHT; tileY += tileHeight) {
		for (tileX = 0; tileX < SABOTAGE_TILE_COVERAGE_WIDTH; tileX += SABOTAGE_BACKGROUND_WIDTH) {
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, tileX + baseX, tileY + baseY,
										  SABOTAGE_BACKGROUND_WIDTH, tileHeight);
		}
	}
	xmemhdl_Unlock_Handle(g_sabotageBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x460630
int16_t Sabotage_draw_Clear(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	if (g_sabotageFilm->cur_cel == g_sabotageFilm->cels || actor->var1 != 0) {
		xcanvas_Erase_Canvas();
	}
	return 1;
}

// FUNCTION: XW 0x460660
XwShellSceneResult Sabotage_MiddleScene(struct XwShellContext* shell) {
	ResFile* sceneResource;
	Film* sceneFilm;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	xfade_AddTimedText("Is that it?", SABOTAGE_MIDDLE_QUESTION_START, SABOTAGE_MIDDLE_QUESTION_END,
					   SABOTAGE_MIDDLE_CAPTION_FONT, SABOTAGE_MIDDLE_QUESTION_X, SABOTAGE_MIDDLE_QUESTION_Y,
					   SABOTAGE_MIDDLE_QUESTION_COLOR);
	xfade_AddTimedText("Yes, that's the last crate.", SABOTAGE_MIDDLE_ANSWER_START,
					   SABOTAGE_MIDDLE_ANSWER_END, SABOTAGE_MIDDLE_CAPTION_FONT, SABOTAGE_MIDDLE_ANSWER_X,
					   SABOTAGE_MIDDLE_ANSWER_Y, SABOTAGE_MIDDLE_ANSWER_COLOR);
	xfade_AddTimedText("Alright, lift off.", SABOTAGE_MIDDLE_LIFTOFF_START, SABOTAGE_MIDDLE_LIFTOFF_END,
					   SABOTAGE_MIDDLE_CAPTION_FONT, SABOTAGE_MIDDLE_LIFTOFF_X, SABOTAGE_MIDDLE_LIFTOFF_Y,
					   SABOTAGE_MIDDLE_LIFTOFF_COLOR);
	xfade_AddTimedText("Hidden among the crates, the Rebels have", SABOTAGE_MIDDLE_NARRATION_FIRST_START,
					   SABOTAGE_MIDDLE_NARRATION_FIRST_END, SABOTAGE_MIDDLE_CAPTION_FONT,
					   SABOTAGE_MIDDLE_NARRATION_FIRST_X, SABOTAGE_MIDDLE_NARRATION_FIRST_Y,
					   SABOTAGE_MIDDLE_NARRATION_FIRST_COLOR);
	xfade_AddTimedText("planted a powerful explosive device.", SABOTAGE_MIDDLE_NARRATION_SECOND_START,
					   SABOTAGE_MIDDLE_NARRATION_SECOND_END, SABOTAGE_MIDDLE_CAPTION_FONT,
					   SABOTAGE_MIDDLE_NARRATION_SECOND_X, SABOTAGE_MIDDLE_NARRATION_SECOND_Y,
					   SABOTAGE_MIDDLE_NARRATION_SECOND_COLOR);
	sceneResource = xres_Open_Resource("sab2.lfd");
	xrect_Set_Rect(&frame, 0, 0, SABOTAGE_MIDDLE_BACKGROUND_WIDTH, SABOTAGE_MIDDLE_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	g_sabotageMiddlePreviousX = 0;
	g_sabotageMiddleFullRedrawFrames = 0;
	g_sabotageMiddleBackgroundHandle =
		xmemhdl_Alloc_Clear_Handle(SABOTAGE_MIDDLE_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	if ((uint16_t)xio_Is_System_Slower_Than(SABOTAGE_MIDDLE_FILM_SPEED_THRESHOLD) != 0)
		sceneFilm =
			xfilm_Res_Callback_Film(sceneResource, "sab2_s", &frame, 0, 0, 0, Sabotage_middle_FilmCallback);
	else
		sceneFilm =
			xfilm_Res_Callback_Film(sceneResource, "sab2_f", &frame, 0, 0, 0, Sabotage_middle_FilmCallback);
	g_sabotageMiddleFilm = sceneFilm;
	xfilm_Set_Film_Def_Palette(sceneFilm, shell->standardPalette);
	g_sabotageMiddleBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, SABOTAGE_MIDDLE_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_sabotageMiddleBackgroundActor, Sabotage_middle_DrawBackground);
	g_sabotageMiddleViewActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, SABOTAGE_MIDDLE_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_sabotageMiddleViewActor, Sabotage_middle_UpdateViewFrame);
	xactor_Set_Actor_Draw_Function(g_sabotageMiddleViewActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_sabotageMiddleDirtyRect);
	xview_Set_View_Update_Function(Sabotage_middle_EndView);
	Sabotage_middle_OpenMusic(sceneResource, g_sabotageMiddleFilm);
	Sabotage_middle_LoadSounds();
#ifdef XW_MODERN
	XwSabotageMiddle_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_sabotageMiddleBackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4608E0
void Sabotage_middle_EndView(int time) {
	int16_t exitScene;
	(void)time;
	if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_DESTROY_STAR_DESTROYER_3,
								  shipext_Get_Pending_Tour_Cutscene(),
								  g_sabotageMiddleFilm->cur_cel == g_sabotageMiddleFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x460920
int16_t Sabotage_middle_FilmCallback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Sabotage_middle_AccumulateDirtyRect);
				break;
			case SABOTAGE_MIDDLE_ACTOR_SCROLL:
				g_sabotageMiddleScrollActor = actor;
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Sabotage_middle_StampBackground(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Sabotage_middle_UserSound);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4609F0
int16_t Sabotage_middle_StampBackground(Actor* actor) {
	int16_t previousHeight;
	int16_t previousWidth;
	uint8_t* previousPixels;
	Rect canvasBounds;
	Rect previousClip;
	int16_t drawResult = 0;
	int16_t xOffset = 0;
	int16_t canvasIndex;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (canvasIndex = 0; canvasIndex < SABOTAGE_MIDDLE_CANVAS_COUNT; ++canvasIndex) {
		if (canvasIndex != 0) {
			uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_sabotageMiddleBackgroundHandle);
			xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth,
								&previousHeight, SABOTAGE_MIDDLE_BACKGROUND_WIDTH,
								SABOTAGE_MIDDLE_BACKGROUND_HEIGHT, 0);
		}
		if (actor->draw) {
			drawResult = actor->draw(actor, &canvasBounds, &canvasBounds, actor->x + xOffset, actor->y, 1);
		}
		if (canvasIndex != 0) {
			xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
			xmemhdl_Unlock_Handle(g_sabotageMiddleBackgroundHandle);
		}
		xOffset -= SABOTAGE_MIDDLE_BACKGROUND_WIDTH;
	}
	return drawResult;
}

// FUNCTION: XW 0x460AD0
void Sabotage_middle_UserSound(Actor* actor, int time) {
	int16_t action = actor->var2;
	(void)time;
	if (action != 0)
		Sabotage_middle_HandleSoundAction(action);
}

// FUNCTION: XW 0x460AF0
int16_t Sabotage_middle_DrawBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									   int16_t refresh) {
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	if (g_sabotageMiddlePreviousX != g_sabotageMiddleScrollActor->x ||
		g_sabotageMiddleScrollActor->var2 != 0) {
		Rect copyRect;
		uint8_t* screenPixels = xcanvas_Get_Screen_Buffer();
		const uint8_t* backgroundPixels;
		xcanvas_Get_Drawing_Canvas_Bounds(&copyRect);
		copyRect.left = g_sabotageMiddlePreviousX;
#ifdef XW_MODERN
		xcanvas_Scroll_Clipped_Buffer(
			screenPixels, &copyRect, g_sabotageMiddleScrollActor->x - g_sabotageMiddlePreviousX, 0,
			xcanvas_Get_Current_Canvas_Bitmap()->w, SABOTAGE_MIDDLE_BACKGROUND_HEIGHT);
#else
		xcanvas_Scroll_Clipped_Buffer(screenPixels, &copyRect,
									  g_sabotageMiddleScrollActor->x - g_sabotageMiddlePreviousX, 0,
									  SABOTAGE_MIDDLE_BACKGROUND_WIDTH, SABOTAGE_MIDDLE_BACKGROUND_HEIGHT);
#endif
		xcanvas_Get_Drawing_Canvas_Bounds(&copyRect);
		backgroundPixels = xmemhdl_Lock_Handle(g_sabotageMiddleBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &copyRect,
									  g_sabotageMiddleScrollActor->x + SABOTAGE_MIDDLE_BACKGROUND_WIDTH, 0,
									  SABOTAGE_MIDDLE_BACKGROUND_WIDTH, SABOTAGE_MIDDLE_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_sabotageMiddleBackgroundHandle);
		g_sabotageMiddlePreviousX = g_sabotageMiddleScrollActor->x;
	}
	return 1;
}

// FUNCTION: XW 0x460BE0
void Sabotage_middle_AccumulateDirtyRect(Actor* actor, int time) {
	Rect actorRect;
	(void)time;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Enclose_Rect(&g_sabotageMiddleDirtyRect, &actorRect);
	}
}

// FUNCTION: XW 0x460C30
void Sabotage_middle_UpdateViewFrame(Actor* actor, int time) {
	Rect frame;
	(void)time;
	if (g_sabotageMiddleFilm->cur_cel == g_sabotageMiddleFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		if (g_sabotageMiddleScrollActor->xv != 0 || g_sabotageMiddleScrollActor->yv != 0 ||
			g_sabotageMiddleScrollActor->xvf != 0 || g_sabotageMiddleScrollActor->yvf != 0)
			g_sabotageMiddleFullRedrawFrames = SABOTAGE_MIDDLE_FULL_REDRAW_FRAMES;
		if (g_sabotageMiddleFullRedrawFrames != 0 || g_sabotageMiddleScrollActor->var2 != 0) {
			xcanvas_Get_Drawing_Canvas_Bounds(&frame);
			if (g_sabotageMiddleFullRedrawFrames != 0)
				--g_sabotageMiddleFullRedrawFrames;
		} else {
			xrect_Copy_Rect(&frame, &g_sabotageMiddleDirtyRect);
			xcanvas_Clip_Rect_To_Canvas(&frame);
		}
		actor->var1 = 0;
	}
	xrect_Clear_Rect(&g_sabotageMiddleDirtyRect);
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(SABOTAGE_MIDDLE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(SABOTAGE_MIDDLE_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x460D30
void Sabotage_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_sabotageMusicFilm = film;
		g_sabotageMusic = xsound_Find_Gmid("sabotage");
		if (g_sabotageMusic == NULL) {
			ResFile* resourceFile = xres_Open_Resource("sb1music.lfd");
			g_sabotageMusic = xsound_Res_Music(resourceFile, "sabotage");
			xres_Close_Resource(resourceFile);
			soundext_Start_Resource_Sound(g_sabotageMusic);
			if (shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_3) {
				soundext_ScanMidi(g_sabotageMusic, 0, SABOTAGE_FINAL_MUSIC_START_BEAT, 0);
			}
		}
		xsound_Set_Sound_Keep(g_sabotageMusic);
		xsound_Set_Sound_User_Function(g_sabotageMusic, Sabotage_user_Music);
	}
}

// FUNCTION: XW 0x460DE0
void Sabotage_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 &&
		shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_3) {
		Sound* music = xsound_Find_Gmid("sabotage");
		g_sabotageMusic = music;
		if (music != NULL) {
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(g_sabotageMusic, 0);
#else
			soundext_SetPriority(0, 0);
#endif
			soundext_FadeVolume(g_sabotageMusic, 0, SABOTAGE_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x460E30
void Sabotage_user_Music(Sound* sound, int time) {
	int currentCel = g_sabotageMusicFilm->cur_cel;
	(void)sound;
	(void)time;
	if (shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_3 &&
		currentCel == SABOTAGE_MUSIC_CONTROL_CEL) {
		soundext_SetHook(g_sabotageMusic, XW_SOUND_CONTROL_DIRECT, SABOTAGE_MUSIC_CONTROL_VALUE, 0);
	}
}

// FUNCTION: XW 0x460E70
void Sabotage_LoadSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_2, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_4, 0, NULL, 0, 0);
		if (shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_3) {
			soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_BIG, 0, NULL, 0, 1);
		}
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x460EC0
void Sabotage_HandleSoundAction(int16_t action) {
	switch (action) {
		case SABOTAGE_SOUND_SHUTTLE_2:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_2);
			}
			break;
		case SABOTAGE_SOUND_SHUTTLE_4:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_4);
			}
			break;
		case SABOTAGE_SOUND_EXPLOSION:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_BIG);
			}
			break;
	}
}

// FUNCTION: XW 0x460F10
void Sabotage_middle_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_sabotageMiddleMusicFilm = film;
		g_sabotageMiddleMusic = xsound_Find_Gmid("sabotage");
		if (g_sabotageMiddleMusic == NULL) {
			ResFile* musicResource = xres_Open_Resource("sb1music.lfd");
			g_sabotageMiddleMusic = xsound_Res_Music(musicResource, "sabotage");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_sabotageMiddleMusic);
			soundext_ScanMidi(g_sabotageMiddleMusic, 0, SABOTAGE_MIDDLE_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_sabotageMiddleMusic);
		xsound_Set_Sound_User_Function(g_sabotageMiddleMusic, Sabotage_middle_UserMusic);
	}
}

// FUNCTION: XW 0x460FC0
void Sabotage_middle_UserMusic(Sound* sound, int time) {
	int currentCel = g_sabotageMiddleMusicFilm->cur_cel;
	(void)sound;
	(void)time;
	if (shellext_Get_Cur_Scene() == XW_SCENE_DESTROY_STAR_DESTROYER_2 &&
		currentCel == SABOTAGE_MIDDLE_MUSIC_CONTROL_CEL) {
		soundext_SetHook(g_sabotageMiddleMusic, XW_SOUND_CONTROL_DIRECT, 1, 0);
	}
}

// FUNCTION: XW 0x461000
void Sabotage_middle_LoadSounds(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_RAMP, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_BEEP_4_SECONDARY, 0, NULL, 0, 0);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		g_sabotageMiddleSpeech33 = soundext_LoadSpeech(XW_SHELL_SPEECH_THAT_IT, 0, NULL, 0);
		g_sabotageMiddleSpeech10 = soundext_LoadSpeech(XW_SHELL_SPEECH_CRATE, 0, NULL, 0);
		g_sabotageMiddleSpeech3 = soundext_LoadSpeech(XW_SHELL_SPEECH_ALL_RIGHT, 0, NULL, 0);
		g_sabotageMiddleSpeech17 = soundext_LoadSpeech(XW_SHELL_SPEECH_LIFT_OFF, 0, NULL, 0);
	}
}

// FUNCTION: XW 0x461090
void Sabotage_middle_HandleSoundAction(int16_t action) {
	switch (action) {
		case SABOTAGE_MIDDLE_SOUND_RAMP:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_RAMP);
			break;
		case SABOTAGE_MIDDLE_SOUND_BEEP:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_4_SECONDARY);
			break;
		case SABOTAGE_MIDDLE_SOUND_STOP_RAMP:
			if (ShellPreferences_GetSfxEnabled() != 0 && ShellPreferences_GetSfxEnabled() == 0)
				soundext_Stop_SFX(XW_SHELL_SFX_RAMP);
			break;
		case SABOTAGE_MIDDLE_SOUND_THAT_IT:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_sabotageMiddleSpeech33);
			break;
		case SABOTAGE_MIDDLE_SOUND_CRATE:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_sabotageMiddleSpeech10);
			break;
		case SABOTAGE_MIDDLE_SOUND_ALL_RIGHT:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_sabotageMiddleSpeech3);
			break;
		case SABOTAGE_MIDDLE_SOUND_LIFT_OFF:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_sabotageMiddleSpeech17);
			break;
	}
}
