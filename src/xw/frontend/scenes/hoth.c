#include "xw/frontend/scenes/hoth.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/hoth_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D3FC8
char g_hothResourceNames[HOTH_RESOURCE_NAME_COUNT][HOTH_RESOURCE_NAME_SIZE] = { "hoth.lfd", "hoth1_f",
																				"hoth2_f", "hoth2_s",
																				"hoth3_f" };

// GLOBAL: XW 0x4F7620
XwSceneMusicHandles g_hothMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7628
Sound* g_hothSpeechSounds[HOTH_SPEECH_SLOT_COUNT] = { NULL };

// GLOBAL: XW 0x4F7638
int16_t g_hothLastBackgroundX = 0;

// GLOBAL: XW 0x4F7640
Rect g_hothDirtyRect = { 0, 0, 0, 0 };

// GLOBAL: XW 0x4F7648
Actor* g_hothBackgroundActor = NULL;

// GLOBAL: XW 0x4F7650
Rect g_hothRestoreRect = { 0, 0, 0, 0 };

// GLOBAL: XW 0x4F7658
LandruHandle g_hothExtensionHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F765C
Actor* g_hothScrollActor = NULL;

// GLOBAL: XW 0x4F7660
Film* g_hothFilm = NULL;

// GLOBAL: XW 0x4F7664
Actor* g_hothEraseActor = NULL;

// GLOBAL: XW 0x4F7668
Rect g_hothTrackedActorRect = { 0, 0, 0, 0 };

// GLOBAL: XW 0x4F7670
LandruHandle g_hothBackgroundHandle = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F7674
uint16_t g_hothFullRedrawCountdown = 0;

// FUNCTION: XW 0x44CD00
void Hoth_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	unsigned int startBeat = 0;
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_HOTH_1:
				startBeat = 0;
				break;
			case XW_SCENE_HOTH_2:
				startBeat = HOTH_MUSIC_SCENE_2_BEAT;
				break;
			case XW_SCENE_HOTH_3:
				startBeat = HOTH_MUSIC_SCENE_3_BEAT;
				break;
		}
		g_hothMusicState.film = film;
		g_hothMusicState.sound = xsound_Find_Gmid("hoth");
		if (g_hothMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("hhmusic.lfd");
			g_hothMusicState.sound = xsound_Res_Music(musicResource, "hoth");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_hothMusicState.sound);
			if (startBeat != 0) {
				soundext_ScanMidi(g_hothMusicState.sound, 0, HOTH_MUSIC_PREPARE_BEAT, 0);
				soundext_ScanMidi(g_hothMusicState.sound, HOTH_MUSIC_START_GROUP, startBeat,
								  HOTH_MUSIC_START_TICK);
			}
		}
		xsound_Set_Sound_Keep(g_hothMusicState.sound);
		xsound_Set_Sound_User_Function(g_hothMusicState.sound, Hoth_user_Music);
	}
}

// FUNCTION: XW 0x44CDF0
void Hoth_SetMusicLevel(int value, int duration) {
	if (g_hothMusicState.sound) {
		soundext_FadeVolume(g_hothMusicState.sound, value, duration);
	}
}

// FUNCTION: XW 0x44CE10
void Hoth_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && shellext_Get_Cur_Scene() == XW_SCENE_HOTH_1) {
		soundext_SetHook(g_hothMusicState.sound, XW_SOUND_CONTROL_DIRECT, 1, 0);
	}
}

// FUNCTION: XW 0x44CE40
void Hoth_user_Music(Sound* unusedSound, int unusedTime) {
	int currentCel = g_hothMusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_3 && currentCel == HOTH_MUSIC_FADE_CEL)
		soundext_FadeVolume(g_hothMusicState.sound, 0, HOTH_MUSIC_FADE_DURATION);
}

// FUNCTION: XW 0x44CE80
void Hoth_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		ResFile* speechResource;
		if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_3) {
			Hoth_SetMusicLevel(HOTH_MUSIC_SPEECH_LEVEL, HOTH_MUSIC_SPEECH_FADE_TICKS);
		}
		speechResource = xres_Open_Resource("hhspch.lfd");
		if (speechResource != NULL) {
			if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_3) {
				g_hothSpeechSounds[0] = xsound_Res_Digital_Sound(speechResource, "leia1");
				g_hothSpeechSounds[1] = xsound_Res_Digital_Sound(speechResource, "general1");
				g_hothSpeechSounds[2] = xsound_Res_Digital_Sound(speechResource, "leia2");
				g_hothSpeechSounds[3] = xsound_Res_Digital_Sound(speechResource, "general2");
			}
			if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_2) {
				g_hothSpeechSounds[0] = xsound_Res_Digital_Sound(speechResource, "tonton1");
				g_hothSpeechSounds[1] = xsound_Res_Digital_Sound(speechResource, "tonton2");
			}
			xres_Close_Resource(speechResource);
		}
	}
}

// FUNCTION: XW 0x44CF70
void Hoth_HandleSoundAction(int16_t action) {
	switch (action) {
		case HOTH_ACTION_FLYBY:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
			}
			break;
		case HOTH_ACTION_SPEECH_10:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_hothSpeechSounds[0]);
			}
			break;
		case HOTH_ACTION_SPEECH_11:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_hothSpeechSounds[1]);
			}
			break;
		case HOTH_ACTION_SPEECH_12:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_hothSpeechSounds[2]);
			}
			break;
		case HOTH_ACTION_SPEECH_13:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_hothSpeechSounds[3]);
				Hoth_SetMusicLevel(HOTH_MUSIC_FULL_LEVEL, HOTH_MUSIC_RESTORE_TICKS);
			}
			break;
		case HOTH_ACTION_SPEECH_20:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_hothSpeechSounds[0]);
			}
			break;
		case HOTH_ACTION_SPEECH_21:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_hothSpeechSounds[1]);
			}
			break;
	}
}

// FUNCTION: XW 0x44D090
XwShellSceneResult Hoth_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	int16_t filmIndex;
	LandruDisplay_SetLowResolutionMode(1);
	if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_1)
		xfade_AddTimedText("Rebel Fleet arrives at the planet Hoth.", HOTH_ARRIVAL_START, HOTH_ARRIVAL_END,
						   HOTH_CAPTION_FONT, HOTH_ARRIVAL_X, HOTH_ARRIVAL_Y, HOTH_ARRIVAL_COLOR);
	if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_3) {
		xfade_AddTimedText("General, what word is there of", HOTH_QUESTION_FIRST_START,
						   HOTH_QUESTION_FIRST_END, HOTH_CAPTION_FONT, HOTH_QUESTION_FIRST_X,
						   HOTH_QUESTION_FIRST_Y, HOTH_QUESTION_FIRST_COLOR);
		xfade_AddTimedText("our evacuation from Habassa?", HOTH_QUESTION_SECOND_START,
						   HOTH_QUESTION_SECOND_END, HOTH_CAPTION_FONT, HOTH_QUESTION_SECOND_X,
						   HOTH_QUESTION_SECOND_Y, HOTH_QUESTION_SECOND_COLOR);
		xfade_AddTimedText("The fleet has arrived safely, Princess.", HOTH_ANSWER_START, HOTH_ANSWER_END,
						   HOTH_CAPTION_FONT, HOTH_ANSWER_X, HOTH_ANSWER_Y, HOTH_ANSWER_COLOR);
		xfade_AddTimedText("Thank goodness.  Is our Hoth base operational?", HOTH_BASE_START, HOTH_BASE_END,
						   HOTH_CAPTION_FONT, HOTH_BASE_X, HOTH_BASE_Y, HOTH_BASE_COLOR);
		xfade_AddTimedText("Yes, the Rebel Alliance has", HOTH_HOME_FIRST_START, HOTH_HOME_FIRST_END,
						   HOTH_CAPTION_FONT, HOTH_HOME_FIRST_X, HOTH_HOME_FIRST_Y, HOTH_HOME_FIRST_COLOR);
		xfade_AddTimedText("found a new home.", HOTH_HOME_SECOND_START, HOTH_HOME_SECOND_END,
						   HOTH_CAPTION_FONT, HOTH_HOME_SECOND_X, HOTH_HOME_SECOND_Y, HOTH_HOME_SECOND_COLOR);
	}
	resourceFile = xres_Open_Resource(g_hothResourceNames[HOTH_RESOURCE_FILE]);
	xrect_Set_Rect(&frame, 0, 0, HOTH_BACKGROUND_WIDTH, HOTH_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xrect_Clear_Rect(&g_hothRestoreRect);
	g_hothLastBackgroundX = 0;
	g_hothFullRedrawCountdown = 0;
	g_hothBackgroundHandle =
		xmemhdl_Alloc_Clear_Handle(HOTH_BACKGROUND_WIDTH * HOTH_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	g_hothExtensionHandle =
		xmemhdl_Alloc_Clear_Handle(HOTH_EXTENSION_WIDTH * HOTH_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_HOTH_1:
			filmIndex = HOTH_FIRST_FILM;
			break;
		case XW_SCENE_HOTH_2:
			if ((uint16_t)xio_Is_System_Slower_Than(HOTH_SPEED_THRESHOLD) != 0 || Shared_ReturnZero() != 0)
				filmIndex = HOTH_SECOND_SLOW_FILM;
			else
				filmIndex = HOTH_SECOND_FAST_FILM;
			break;
		case XW_SCENE_HOTH_3:
			filmIndex = HOTH_THIRD_FILM;
			break;
		default:
			/* The original derives an invalid film index from shell pointer bits. */
			filmIndex = HOTH_FIRST_FILM;
			break;
	}
	xio_Is_System_Slower_Than(HOTH_SPEED_THRESHOLD);
	g_hothFilm = xfilm_Res_Callback_Film(resourceFile, g_hothResourceNames[filmIndex], &frame, 0, 0, 0,
										 Hoth_film_Callback);
	xfilm_Set_Film_Def_Palette(g_hothFilm, shell->standardPalette);
	g_hothBackgroundActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, HOTH_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_hothBackgroundActor, Hoth_draw_Background);
	g_hothEraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, HOTH_ERASE_Z);
	xactor_Set_Actor_User_Function(g_hothEraseActor, Hoth_user_Erase);
	xactor_Set_Actor_Draw_Function(g_hothEraseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_hothDirtyRect);
	xview_Set_View_Update_Function(Hoth_end_View);
	Hoth_OpenMusic(resourceFile, g_hothFilm);
	Hoth_LoadSoundEffects(resourceFile, g_hothFilm);
#ifdef XW_MODERN
	XwHoth_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	Hoth_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_hothExtensionHandle);
	xmemhdl_Free_Handle(g_hothBackgroundHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x44D3D0
void Hoth_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_HOTH_1:
			nextScene = XW_SCENE_HOTH_2;
			break;
		case XW_SCENE_HOTH_2:
			nextScene = XW_SCENE_HOTH_3;
			break;
		case XW_SCENE_HOTH_3:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_hothFilm->cur_cel == g_hothFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x44D440
int16_t Hoth_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Hoth_user_DirtyBounds);
				break;
			case HOTH_ACTOR_ROLE_SCROLL:
				g_hothScrollActor = actor;
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Hoth_StampBackground(actor);
				consumeObject = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Hoth_user_SoundAction);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x44D510
int16_t Hoth_StampBackground(Actor* actor) {
	int16_t previousHeight;
	int16_t previousWidth;
	uint8_t* previousPixels;
	Rect previousClip;
	Rect canvasBounds;
	int16_t drawResult = 0;
	int16_t drawXOffset = 0;
	int16_t partIndex;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (partIndex = 0; partIndex < HOTH_BACKGROUND_PARTS; ++partIndex) {
		if (partIndex != 0) {
			uint8_t* extensionPixels = xmemhdl_Lock_Handle(g_hothExtensionHandle);
			xcanvas_Push_Canvas(&previousPixels, extensionPixels, &previousClip, &previousWidth,
								&previousHeight, HOTH_EXTENSION_WIDTH, HOTH_BACKGROUND_HEIGHT, 0);
		} else {
			uint8_t* backgroundPixels = xmemhdl_Lock_Handle(g_hothBackgroundHandle);
			xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth,
								&previousHeight, HOTH_BACKGROUND_WIDTH, HOTH_BACKGROUND_HEIGHT, 0);
		}
		if (actor->draw != NULL)
			drawResult =
				actor->draw(actor, &canvasBounds, &canvasBounds, actor->x + drawXOffset, actor->y, 1);
		if (partIndex != 0) {
			xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
			xmemhdl_Unlock_Handle(g_hothExtensionHandle);
		} else {
			xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
			xmemhdl_Unlock_Handle(g_hothBackgroundHandle);
		}
		switch ((int)(int16_t)shellext_Get_Cur_Scene()) {
			case XW_SCENE_HOTH_1:
			case XW_SCENE_HOTH_3:
				break;
			default:
				drawXOffset -= HOTH_BACKGROUND_WIDTH;
				break;
		}
	}
	return drawResult;
}

// FUNCTION: XW 0x44D640
void Hoth_user_SoundAction(Actor* actor, int unusedTime) {
	int16_t action = actor->var2;
	(void)unusedTime;
	if (action != 0) {
		Hoth_HandleSoundAction(action);
	}
}

// FUNCTION: XW 0x44D660
int16_t Hoth_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
							 int16_t unusedY, int16_t refresh) {
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0)
		return 0;
	if (g_hothLastBackgroundX != g_hothScrollActor->x || g_hothScrollActor->var2 != 0) {
		Rect canvasBounds;
		const uint8_t* pixels;
		xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
		pixels = xmemhdl_Lock_Handle(g_hothBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(pixels, &canvasBounds, g_hothScrollActor->x, 0, HOTH_BACKGROUND_WIDTH,
									  HOTH_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_hothBackgroundHandle);
		pixels = xmemhdl_Lock_Handle(g_hothExtensionHandle);
		stub_Copy_From_Clipped_Buffer(pixels, &canvasBounds, g_hothScrollActor->x + HOTH_BACKGROUND_WIDTH, 0,
									  HOTH_EXTENSION_WIDTH, HOTH_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_hothExtensionHandle);
		g_hothLastBackgroundX = g_hothScrollActor->x;
		xcanvas_Invalid_Screen_Diff();
	}
	if (xrect_Empty_Rect(&g_hothRestoreRect) == 0) {
		const uint8_t* pixels = xmemhdl_Lock_Handle(g_hothBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(pixels, &g_hothRestoreRect, g_hothRestoreRect.left,
									  g_hothRestoreRect.top, HOTH_BACKGROUND_WIDTH, HOTH_BACKGROUND_HEIGHT);
		xmemhdl_Unlock_Handle(g_hothBackgroundHandle);
	}
	return 1;
}

// FUNCTION: XW 0x44D7A0
void Hoth_user_DirtyBounds(Actor* actor, int time) {
	Rect actorRect;

	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &actorRect);
		xcanvas_Clip_Rect_To_Canvas(&actorRect);
		xrect_Enclose_Rect(&g_hothDirtyRect, &actorRect);
	}
	if (actor->var2 == HOTH_ACTOR_MOVING_REGION) {
		if (time == 0) {
			xrect_Clear_Rect(&g_hothRestoreRect);
		} else {
			xrect_Copy_Rect(&g_hothRestoreRect, &g_hothTrackedActorRect);
			if (xrect_Empty_Rect(&g_hothRestoreRect) == 0)
				xrect_Enclose_Rect(&g_hothDirtyRect, &g_hothRestoreRect);
		}
		xactor_Get_Actor_Rect(actor, &actorRect);
		xrect_Copy_Rect(&g_hothTrackedActorRect, &actorRect);
		xcanvas_Clip_Rect_To_Canvas(&g_hothTrackedActorRect);
	}
}

// FUNCTION: XW 0x44D870
void Hoth_user_Erase(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_hothFilm->cur_cel == g_hothFilm->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = HOTH_FILM_COMPLETE;
	} else {
		if (g_hothScrollActor->xv != 0 || g_hothScrollActor->yv != 0 || g_hothScrollActor->xvf != 0 ||
			g_hothScrollActor->yvf != 0)
			g_hothFullRedrawCountdown = HOTH_FULL_REDRAW_FRAMES;
		if (g_hothFullRedrawCountdown != 0 || g_hothScrollActor->var2 != 0) {
			xcanvas_Get_Drawing_Canvas_Bounds(&frame);
			if (g_hothFullRedrawCountdown != 0)
				--g_hothFullRedrawCountdown;
		} else {
			xrect_Copy_Rect(&frame, &g_hothDirtyRect);
			xcanvas_Clip_Rect_To_Canvas(&frame);
		}
		actor->var1 = 0;
	}
	if (shellext_Get_Cur_Scene() == XW_SCENE_HOTH_1) {
		xrect_Copy_Rect(&g_hothRestoreRect, &g_hothDirtyRect);
		frame.left -= HOTH_SCENE_1_LEFT_MARGIN;
		g_hothRestoreRect.left -= HOTH_SCENE_1_LEFT_MARGIN;
	}
	xrect_Clear_Rect(&g_hothDirtyRect);
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
