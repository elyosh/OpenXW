#include "xw/frontend/scenes/gov2.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/gov2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/pal.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D3848
char g_gov2ResourceNames[GOV2_RESOURCE_NAME_COUNT][GOV2_RESOURCE_NAME_SIZE] = {
	"gov2.lfd", "gov2_f", "gov2_f", "gov3_s", "gov3_s", "gov4_f",
	"gov4_f",   "gov5_f", "gov5_f", "gov6_f", "gov6_f"
};

// GLOBAL: XW 0x4F7538
Rect g_gov2CurrentDirtyRect = { 0, 0, 0, 0 };

// GLOBAL: XW 0x4F7540
Actor* g_gov2BackgroundActor = NULL;

// GLOBAL: XW 0x4F7548
Rect g_gov2RestoreRect = { 0, 0, 0, 0 };

// GLOBAL: XW 0x4F7550
Actor* g_gov2ScrollActor = NULL;

// GLOBAL: XW 0x4F7554
Film* g_gov2Film = NULL;

// GLOBAL: XW 0x4F7558
Actor* g_gov2CloseActor = NULL;

// GLOBAL: XW 0x4F7560
Rect g_gov2PreviousMovingBounds = { 0, 0, 0, 0 };

// GLOBAL: XW 0x4F7568
LandruHandle g_gov2BackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F756C
int16_t g_gov2FullRefreshFrames = 0;

// GLOBAL: XW 0x4F7570
int16_t g_gov2PreviousScrollX = 0;

// GLOBAL: XW 0x4F7580
XwSceneMusicHandles g_gov2MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7588
Sound* g_gov2SpeechSounds[GOV2_SPEECH_COUNT] = { NULL, NULL, NULL, NULL, NULL };

// FUNCTION: XW 0x44B750
XwShellSceneResult Gov2_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	int16_t slowFilmSlot;
	LandruDisplay_SetLowResolutionMode(1);
	if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_3) {
		xfade_AddTimedText("Lord Vader!  What a great privilege to have", GOV2_GREETING_FIRST_START,
						   GOV2_GREETING_FIRST_END, GOV2_CAPTION_FONT, GOV2_GREETING_FIRST_X,
						   GOV2_GREETING_FIRST_Y, GOV2_CAPTION_COLOR);
		xfade_AddTimedText("you unexpectedly grace my humble domain.", GOV2_GREETING_SECOND_START,
						   GOV2_GREETING_SECOND_END, GOV2_CAPTION_FONT, GOV2_GREETING_SECOND_X,
						   GOV2_GREETING_SECOND_Y, GOV2_CAPTION_COLOR);
		xfade_AddTimedText("How may I serve you?", GOV2_QUESTION_START, GOV2_QUESTION_END, GOV2_CAPTION_FONT,
						   GOV2_QUESTION_X, GOV2_QUESTION_Y, GOV2_CAPTION_COLOR);
	}
	if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_4) {
		xfade_AddTimedText("We received your grain shipments, Ghorin...", GOV2_SHIPMENT_START,
						   GOV2_SHIPMENT_END, GOV2_CAPTION_FONT, GOV2_SHIPMENT_X, GOV2_SHIPMENT_Y,
						   GOV2_CAPTION_COLOR);
		xfade_AddTimedText("and the Emperor has sent me here to", GOV2_REPLY_FIRST_START,
						   GOV2_REPLY_FIRST_END, GOV2_CAPTION_FONT, GOV2_REPLY_FIRST_X, GOV2_REPLY_FIRST_Y,
						   GOV2_CAPTION_COLOR);
		xfade_AddTimedText("personally repay you for your treachery.", GOV2_REPLY_SECOND_START,
						   GOV2_REPLY_SECOND_END, GOV2_CAPTION_FONT, GOV2_REPLY_SECOND_X, GOV2_REPLY_SECOND_Y,
						   GOV2_CAPTION_COLOR);
	}
	resourceFile = xres_Open_Resource(g_gov2ResourceNames[GOV2_RESOURCE_FILE]);
	xrect_Set_Rect(&frame, 0, 0, GOV2_BACKGROUND_WIDTH, GOV2_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xrect_Clear_Rect(&g_gov2RestoreRect);
	g_gov2PreviousScrollX = 0;
	g_gov2FullRefreshFrames = 0;
	g_gov2BackgroundHandle =
		xmemhdl_Alloc_Clear_Handle(GOV2_BACKGROUND_WIDTH * GOV2_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_GHORIN_2:
			slowFilmSlot = GOV2_SCENE_2_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_3:
			slowFilmSlot = GOV2_SCENE_3_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_4:
			slowFilmSlot = GOV2_SCENE_4_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_5:
			slowFilmSlot = GOV2_SCENE_5_SLOW_FILM;
			break;
		case XW_SCENE_GHORIN_6:
			slowFilmSlot = GOV2_SCENE_6_SLOW_FILM;
			break;
		default:
			/* The original derives an invalid slot from shell pointer bits. */
			slowFilmSlot = GOV2_SCENE_2_SLOW_FILM;
			break;
	}
	if ((uint16_t)xio_Is_System_Slower_Than(GOV2_SPEED_THRESHOLD) != 0)
		g_gov2Film = xfilm_Res_Callback_Film(resourceFile, g_gov2ResourceNames[slowFilmSlot], &frame, 0, 0, 0,
											 Gov2_film_Callback);
	else
		g_gov2Film =
			xfilm_Res_Callback_Film(resourceFile, g_gov2ResourceNames[slowFilmSlot + GOV2_FAST_FILM_OFFSET],
									&frame, 0, 0, 0, Gov2_film_Callback);
	xfilm_Set_Film_Def_Palette(g_gov2Film, shell->standardPalette);
	g_gov2BackgroundActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, GOV2_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_gov2BackgroundActor, Gov2_draw_Background);
	g_gov2CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, GOV2_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_gov2CloseActor, Gov2_user_Close);
	xactor_Set_Actor_Draw_Function(g_gov2CloseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_gov2CurrentDirtyRect);
	xview_Set_View_Update_Function(Gov2_end_View);
	Gov2_OpenMusic(resourceFile, g_gov2Film);
	Gov2_LoadSoundEffects(resourceFile, g_gov2Film);
#ifdef XW_MODERN
	XwGov2_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_gov2BackgroundHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x44BA70
void Gov2_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_GHORIN_1:
			nextScene = XW_SCENE_GHORIN_2;
			break;
		case XW_SCENE_GHORIN_2:
			nextScene = XW_SCENE_GHORIN_3;
			break;
		case XW_SCENE_GHORIN_3:
			nextScene = XW_SCENE_GHORIN_4;
			break;
		case XW_SCENE_GHORIN_4:
			nextScene = XW_SCENE_GHORIN_5;
			break;
		case XW_SCENE_GHORIN_5:
			nextScene = XW_SCENE_GHORIN_6;
			break;
		case XW_SCENE_GHORIN_6:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_gov2Film->cur_cel == g_gov2Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x44BB10
int16_t Gov2_film_Callback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Gov2_user_DirtyBounds);
				break;
			case GOV2_ACTOR_ROLE_SCROLL:
				g_gov2ScrollActor = actor;
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Gov2_StampForegroundAndBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Gov2_user_Sound);
				break;
		}
	} else if (object->id == FTC_PALETTE) {
		int16_t scene = shellext_Get_Cur_Scene();
		if (scene == XW_SCENE_GHORIN_2 || scene == XW_SCENE_GHORIN_5) {
			xpal_Start_Cycle(object->object);
		}
	}
	return handled;
}

// FUNCTION: XW 0x44BC10
int16_t Gov2_StampForegroundAndBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	int16_t drawResult = 0;
	int16_t backgroundXOffset = 0;
	int16_t canvasIndex;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (canvasIndex = 0; canvasIndex < GOV2_STAMP_CANVAS_COUNT; ++canvasIndex) {
		if (canvasIndex != 0) {
			uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_gov2BackgroundHandle);
			xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth,
								&previousHeight, GOV2_BACKGROUND_WIDTH, GOV2_BACKGROUND_HEIGHT, 0);
		}
		if (actor->draw != NULL) {
			drawResult =
				actor->draw(actor, &canvasBounds, &canvasBounds, backgroundXOffset + actor->x, actor->y, 1);
		}
		if (canvasIndex != 0) {
			xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
			xmemhdl_Unlock_Handle(g_gov2BackgroundHandle);
		}
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_GHORIN_2:
			case XW_SCENE_GHORIN_5:
			case XW_SCENE_GHORIN_6:
				break;
			default:
				backgroundXOffset -= GOV2_BACKGROUND_WIDTH;
				break;
		}
	}
	return drawResult;
}

// FUNCTION: XW 0x44BD00
void Gov2_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0)
		Gov2_PlaySoundCue(cue);
}

// FUNCTION: XW 0x44BD20
int16_t Gov2_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	if (g_gov2PreviousScrollX != g_gov2ScrollActor->x || g_gov2ScrollActor->var2 != 0) {
		Rect canvasBounds;
		uint8_t* screenPixels = xcanvas_Get_Screen_Buffer();
		const uint8_t* backgroundPixels;
		xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
		canvasBounds.left = g_gov2PreviousScrollX;
		xcanvas_Scroll_Clipped_Buffer(screenPixels, &canvasBounds,
									  g_gov2ScrollActor->x - g_gov2PreviousScrollX, 0, GOV2_BACKGROUND_WIDTH,
									  GOV2_BACKGROUND_HEIGHT);
		xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
		backgroundPixels = xmemhdl_Lock_Handle(g_gov2BackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &canvasBounds,
									  g_gov2ScrollActor->x + GOV2_BACKGROUND_WIDTH, 0, GOV2_BACKGROUND_WIDTH,
									  GOV2_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_gov2BackgroundHandle);
		g_gov2PreviousScrollX = g_gov2ScrollActor->x;
		xcanvas_Invalid_Screen_Diff();
	}
	if (xrect_Empty_Rect(&g_gov2RestoreRect) == 0) {
		const uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_gov2BackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_gov2RestoreRect, g_gov2RestoreRect.left,
									  g_gov2RestoreRect.top, GOV2_BACKGROUND_WIDTH, GOV2_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_gov2BackgroundHandle);
	}
	return 1;
}

// FUNCTION: XW 0x44BE70
void Gov2_user_DirtyBounds(Actor* actor, int time) {
	Rect actorBounds;

	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xcanvas_Clip_Rect_To_Canvas(&actorBounds);
		xrect_Enclose_Rect(&g_gov2CurrentDirtyRect, &actorBounds);
	}
	if (actor->var2 == GOV2_ACTOR_MOVING_REGION) {
		if (time == 0) {
			xrect_Clear_Rect(&g_gov2RestoreRect);
		} else {
			xrect_Copy_Rect(&g_gov2RestoreRect, &g_gov2PreviousMovingBounds);
			if (xrect_Empty_Rect(&g_gov2RestoreRect) == 0)
				xrect_Enclose_Rect(&g_gov2CurrentDirtyRect, &g_gov2RestoreRect);
		}
		xactor_Get_Actor_Rect(actor, &actorBounds);
		xrect_Copy_Rect(&g_gov2PreviousMovingBounds, &actorBounds);
		xcanvas_Clip_Rect_To_Canvas(&g_gov2PreviousMovingBounds);
	}
}

// FUNCTION: XW 0x44BF40
void Gov2_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_gov2Film->cur_cel == g_gov2Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		if (g_gov2ScrollActor->xv != 0 || g_gov2ScrollActor->yv != 0 || g_gov2ScrollActor->xvf != 0 ||
			g_gov2ScrollActor->yvf != 0)
			g_gov2FullRefreshFrames = GOV2_FULL_REFRESH_FRAMES;
		if (g_gov2FullRefreshFrames != 0 || g_gov2ScrollActor->var2 != 0) {
			xcanvas_Get_Drawing_Canvas_Bounds(&frame);
			if (g_gov2FullRefreshFrames != 0)
				--g_gov2FullRefreshFrames;
		} else {
			xrect_Copy_Rect(&frame, &g_gov2CurrentDirtyRect);
			xcanvas_Clip_Rect_To_Canvas(&frame);
		}
		actor->var1 = 0;
	}
	xrect_Clear_Rect(&g_gov2CurrentDirtyRect);
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(GOV2_MAIN_VIEW, &frame);
		xview_Set_View_Pos(GOV2_MAIN_VIEW, frame.left, frame.top);
	}
}

// FUNCTION: XW 0x44C160
void Gov2_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	int startGroup = 0;
	unsigned int startBeat = 0;
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		switch ((int16_t)shellext_Get_Cur_Scene()) {
			case XW_SCENE_GHORIN_2:
				startBeat = GOV2_MUSIC_SCENE_2_BEAT;
				break;
			case XW_SCENE_GHORIN_3:
				startBeat = GOV2_MUSIC_SCENE_3_BEAT;
				break;
			case XW_SCENE_GHORIN_4:
				startBeat = GOV2_MUSIC_SCENE_4_BEAT;
				break;
			case XW_SCENE_GHORIN_5:
				startGroup = GOV2_MUSIC_LATE_GROUP;
				startBeat = GOV2_MUSIC_SCENE_5_BEAT;
				break;
			case XW_SCENE_GHORIN_6:
				startGroup = GOV2_MUSIC_LATE_GROUP;
				startBeat = GOV2_MUSIC_SCENE_6_BEAT;
				break;
		}
		g_gov2MusicState.film = sceneFilm;
		g_gov2MusicState.sound = xsound_Find_Gmid("ghorin");
		if (g_gov2MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("gvmusic.lfd");
			g_gov2MusicState.sound = xsound_Res_Music(musicResource, "ghorin");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_gov2MusicState.sound);
			if (startBeat != 0) {
				soundext_ScanMidi(g_gov2MusicState.sound, startGroup, startBeat, GOV2_MUSIC_START_TICK);
			}
		}
		xsound_Set_Sound_Keep(g_gov2MusicState.sound);
		xsound_Set_Sound_User_Function(g_gov2MusicState.sound, Gov2_user_Music);
	}
}

// FUNCTION: XW 0x44C270
void Gov2_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_gov2MusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_GHORIN_4:
			if (filmCel == GOV2_MUSIC_SEEK_CEL) {
				int legacyTick;
				j_lolevel_ImPause();
				legacyTick = soundext_GetMusicParam(g_gov2MusicState.sound, XW_SOUND_QUERY_TICK, 0);
				j_lolevel_ImResume();
				soundext_JumpMidi(g_gov2MusicState.sound, GOV2_MUSIC_LATE_GROUP, GOV2_MUSIC_SEEK_BEAT,
								  legacyTick);
			}
			break;
		case XW_SCENE_GHORIN_6:
			if (filmCel == GOV2_MUSIC_FADE_CEL) {
				soundext_FadeVolume(g_gov2MusicState.sound, 0, GOV2_MUSIC_FADE_DURATION);
			}
			break;
	}
}

// FUNCTION: XW 0x44C2F0
void Gov2_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_LoadSfx(XW_SHELL_SFX_RAMP, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_BEEP_4_SECONDARY, 0, NULL, 0, 0);
	}
	if (ShellPreferences_GetSfxEnabled()) {
		ResFile* speechResource = xres_Open_Resource("gv2spch.lfd");
		if (speechResource) {
			if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_3) {
				g_gov2SpeechSounds[GOV2_SPEECH_GHORIN_0] =
					xsound_Res_Digital_Sound(speechResource, "ghorin0");
				g_gov2SpeechSounds[GOV2_SPEECH_GHORIN_1] =
					xsound_Res_Digital_Sound(speechResource, "ghorin1");
				g_gov2SpeechSounds[GOV2_SPEECH_GHORIN_2] =
					xsound_Res_Digital_Sound(speechResource, "ghorin2");
			}
			if (shellext_Get_Cur_Scene() == XW_SCENE_GHORIN_4) {
				g_gov2SpeechSounds[GOV2_SPEECH_DARTH_1] = xsound_Res_Digital_Sound(speechResource, "darth1");
				g_gov2SpeechSounds[GOV2_SPEECH_DARTH_2] = xsound_Res_Digital_Sound(speechResource, "darth2");
			}
			xres_Close_Resource(speechResource);
		}
	}
}

// FUNCTION: XW 0x44C3C0
void Gov2_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case GOV2_CUE_GHORIN_0:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_gov2SpeechSounds[GOV2_SPEECH_GHORIN_0]);
			break;
		case GOV2_CUE_GHORIN_1:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_gov2SpeechSounds[GOV2_SPEECH_GHORIN_1]);
			break;
		case GOV2_CUE_GHORIN_2:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_gov2SpeechSounds[GOV2_SPEECH_GHORIN_2]);
			break;
		case GOV2_CUE_DARTH_1:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_gov2SpeechSounds[GOV2_SPEECH_DARTH_1]);
			break;
		case GOV2_CUE_DARTH_2:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_Start_Resource_SFX(g_gov2SpeechSounds[GOV2_SPEECH_DARTH_2]);
			break;
	}
}
