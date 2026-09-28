#include "xw/frontend/scenes/outpost.h"
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
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/outpost_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>
#include <stdlib.h>

// GLOBAL: XW 0x4F883C
XwSceneMusicHandles g_outpostMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F8848
Sound* g_outpostSpeech[OUTPOST_SPEECH_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F8858
Actor* g_outpostBackgroundActor = NULL;

// GLOBAL: XW 0x4F8860
Rect g_outpostPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F8868
int16_t g_outpostRepeatBackground = 0;

// GLOBAL: XW 0x4F886C
Actor* g_outpostCloseActor = NULL;

// GLOBAL: XW 0x4F8870
Film* g_outpostFilm = NULL;

// GLOBAL: XW 0x4F8878
Rect g_outpostCurrentDirtyRect = { 0 };

// GLOBAL: XW 0x4F8880
LandruHandle g_outpostBackgroundHandle = 0;

// FUNCTION: XW 0x459D60
void Outpost_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_outpostMusicState.film = sceneFilm;
		g_outpostMusicState.sound = xsound_Find_Gmid("sendplan");
		if (g_outpostMusicState.sound == NULL) {
			unsigned int startBeat;
			ResFile* musicResource;
			switch (shellext_Get_Cur_Scene()) {
				case XW_SCENE_SEND_PLANS_2:
					startBeat = OUTPOST_MUSIC_SCENE_2_BEAT;
					break;
				case XW_SCENE_SEND_PLANS_3:
					startBeat = OUTPOST_MUSIC_SCENE_3_BEAT;
					break;
				case XW_SCENE_SEND_PLANS_4:
					startBeat = OUTPOST_MUSIC_SCENE_4_BEAT;
					break;
				case XW_SCENE_SEND_PLANS_5:
					startBeat = OUTPOST_MUSIC_SCENE_5_BEAT;
					break;
				case XW_SCENE_SEND_PLANS_6:
					startBeat = OUTPOST_MUSIC_SCENE_6_BEAT;
					break;
				default:
					startBeat = 0;
					break;
			}
			musicResource = xres_Open_Resource("opmusic.lfd");
			g_outpostMusicState.sound = xsound_Res_Music(musicResource, "sendplan");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_outpostMusicState.sound);
			if (startBeat != 0) {
				soundext_ScanMidi(g_outpostMusicState.sound, 0, startBeat, 0);
			}
		}
		xsound_Set_Sound_Keep(g_outpostMusicState.sound);
		xsound_Set_Sound_User_Function(g_outpostMusicState.sound, Outpost_user_Music);
	}
}

// FUNCTION: XW 0x459E60
void Outpost_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && shellext_Get_Cur_Scene() == XW_SCENE_SEND_PLANS_6) {
		Sound* music = xsound_Find_Gmid("sendplan");
		g_outpostMusicState.sound = music;
		if (music != NULL) {
#ifdef XW_MODERN
			Dos94_soundext_SetPriority(g_outpostMusicState.sound, 0);
#else
			soundext_SetPriority(0, 0);
#endif
			soundext_FadeVolume(g_outpostMusicState.sound, 0, OUTPOST_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x459EB0
void Outpost_user_Music(Sound* sound, int time) {
	if (shellext_Get_Cur_Scene() == XW_SCENE_SEND_PLANS_5) {
		if (time == OUTPOST_MUSIC_SCENE_5_QUIET_TIME) {
			soundext_FadeVolume(sound, OUTPOST_MUSIC_SCENE_5_QUIET_VOLUME, OUTPOST_MUSIC_CUE_FADE_DURATION);
		}
		if (time == OUTPOST_MUSIC_SCENE_5_LOUD_TIME) {
			soundext_FadeVolume(sound, OUTPOST_MUSIC_LOUD_VOLUME, OUTPOST_MUSIC_CUE_FADE_DURATION);
		}
	}
	if (shellext_Get_Cur_Scene() == XW_SCENE_SEND_PLANS_6) {
		if (time == OUTPOST_MUSIC_SCENE_6_QUIET_TIME) {
			soundext_FadeVolume(sound, OUTPOST_MUSIC_SCENE_6_QUIET_VOLUME, OUTPOST_MUSIC_CUE_FADE_DURATION);
		}
		if (time == OUTPOST_MUSIC_SCENE_6_LOUD_TIME) {
			soundext_FadeVolume(sound, OUTPOST_MUSIC_LOUD_VOLUME, OUTPOST_MUSIC_CUE_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x459F30
void Outpost_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_SEND_PLANS_1:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_1, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_3, 0, NULL, 0, 0);
			}
			break;
		case XW_SCENE_SEND_PLANS_2:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_SHUTTLE_3, 0, NULL, 0, 0);
			}
			break;
		case XW_SCENE_SEND_PLANS_3:
		case XW_SCENE_SEND_PLANS_4:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_MESSAGE, 0, NULL, 0, 0);
			}
			break;
		case XW_SCENE_SEND_PLANS_5:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_MESSAGE, 0, NULL, 0, 0);
			}
			if (ShellPreferences_GetSfxEnabled() != 0) {
				g_outpostSpeech[0] = soundext_LoadSpeech(OUTPOST_SCENE_5_SPEECH_1, 0, NULL, 0);
				g_outpostSpeech[1] = soundext_LoadSpeech(OUTPOST_SCENE_5_SPEECH_2, 0, NULL, 0);
				g_outpostSpeech[2] = soundext_LoadSpeech(OUTPOST_SCENE_5_SPEECH_3, 0, NULL, 0);
			}
			break;
		case XW_SCENE_SEND_PLANS_6:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_LoadSfx(XW_SHELL_SFX_MESSAGE, 0, NULL, 0, 0);
			}
			if (ShellPreferences_GetSfxEnabled() != 0) {
				g_outpostSpeech[0] = soundext_LoadSpeech(OUTPOST_SCENE_6_SPEECH_1, 0, NULL, 0);
				g_outpostSpeech[1] = soundext_LoadSpeech(OUTPOST_SCENE_6_SPEECH_2, 0, NULL, 0);
			}
			break;
	}
}

// FUNCTION: XW 0x45A090
void Outpost_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case OUTPOST_CUE_MESSAGE:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_MESSAGE);
			}
			break;
		case OUTPOST_CUE_SHUTTLE_3:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_3);
			}
			break;
		case OUTPOST_CUE_SHUTTLE_1:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_SHUTTLE_1);
			}
			break;
		case OUTPOST_CUE_SPEECH_1:
		case OUTPOST_CUE_SPEECH_1_REPEAT:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_outpostSpeech[0]);
			}
			break;
		case OUTPOST_CUE_SPEECH_2:
		case OUTPOST_CUE_SPEECH_2_REPEAT:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_outpostSpeech[1]);
			}
			break;
		case OUTPOST_CUE_SPEECH_3:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_outpostSpeech[2]);
			}
			break;
		case OUTPOST_CUE_FADE_MESSAGE:
			soundext_Fade_SFX(XW_SHELL_SFX_MESSAGE, 0, OUTPOST_MESSAGE_FADE_DURATION);
			break;
	}
}

// FUNCTION: XW 0x45A170
XwShellSceneResult Outpost_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	int16_t useNormalErase = 1;
	LandruDisplay_SetLowResolutionMode(1);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_SEND_PLANS_1:
			xfade_AddTimedText("Rebel Corvette deploying captured", OUTPOST_DEPLOY_FIRST_START,
							   OUTPOST_DEPLOY_FIRST_END, OUTPOST_CAPTION_FONT, OUTPOST_DEPLOY_FIRST_X,
							   OUTPOST_DEPLOY_FIRST_Y, OUTPOST_DEPLOY_FIRST_COLOR);
			xfade_AddTimedText("Imperial Communication Satellites.", OUTPOST_DEPLOY_SECOND_START,
							   OUTPOST_DEPLOY_SECOND_END, OUTPOST_CAPTION_FONT, OUTPOST_DEPLOY_SECOND_X,
							   OUTPOST_DEPLOY_SECOND_Y, OUTPOST_DEPLOY_SECOND_COLOR);
			break;
		case XW_SCENE_SEND_PLANS_2:
			xfade_AddTimedText("The modified CommSats scatter into deep space.", OUTPOST_SCATTER_START,
							   OUTPOST_SCATTER_END, OUTPOST_CAPTION_FONT, OUTPOST_SCATTER_X,
							   OUTPOST_SCATTER_Y, OUTPOST_SCATTER_COLOR);
			break;
		case XW_SCENE_SEND_PLANS_3:
			xfade_AddTimedText("A lone CommSat transmits near the Cron Drift,", OUTPOST_TRANSMIT_FIRST_START,
							   OUTPOST_TRANSMIT_FIRST_END, OUTPOST_CAPTION_FONT, OUTPOST_TRANSMIT_FIRST_X,
							   OUTPOST_TRANSMIT_FIRST_Y, OUTPOST_TRANSMIT_FIRST_COLOR);
			xfade_AddTimedText("sending Imperial data to a nearby Rebel outpost.",
							   OUTPOST_TRANSMIT_SECOND_START, OUTPOST_TRANSMIT_SECOND_END,
							   OUTPOST_CAPTION_FONT, OUTPOST_TRANSMIT_SECOND_X, OUTPOST_TRANSMIT_SECOND_Y,
							   OUTPOST_TRANSMIT_SECOND_COLOR);
			break;
		case XW_SCENE_SEND_PLANS_4:
			xfade_AddTimedText("Rebel listening post Ax-235 in an", OUTPOST_OUTPOST_FIRST_START,
							   OUTPOST_OUTPOST_FIRST_END, OUTPOST_CAPTION_FONT, OUTPOST_OUTPOST_FIRST_X,
							   OUTPOST_OUTPOST_FIRST_Y, OUTPOST_OUTPOST_FIRST_COLOR);
			xfade_AddTimedText("asteroid field within the Cron Drift.", OUTPOST_OUTPOST_SECOND_START,
							   OUTPOST_OUTPOST_SECOND_END, OUTPOST_CAPTION_FONT, OUTPOST_OUTPOST_SECOND_X,
							   OUTPOST_OUTPOST_SECOND_Y, OUTPOST_OUTPOST_SECOND_COLOR);
			break;
		case XW_SCENE_SEND_PLANS_5:
			xfade_AddTimedText("We just intercepted a transmission.", OUTPOST_INTERCEPT_START,
							   OUTPOST_INTERCEPT_END, OUTPOST_CAPTION_FONT, OUTPOST_INTERCEPT_X,
							   OUTPOST_INTERCEPT_Y, OUTPOST_INTERCEPT_COLOR);
			xfade_AddTimedText("Sounds like an Imperial code.", OUTPOST_CODE_START, OUTPOST_CODE_END,
							   OUTPOST_CAPTION_FONT, OUTPOST_CODE_X, OUTPOST_CODE_Y, OUTPOST_CODE_COLOR);
			xfade_AddTimedText("That's right!  I'm decoding it now.", OUTPOST_DECODE_START,
							   OUTPOST_DECODE_END, OUTPOST_CAPTION_FONT, OUTPOST_DECODE_X, OUTPOST_DECODE_Y,
							   OUTPOST_DECODE_COLOR);
			break;
		case XW_SCENE_SEND_PLANS_6:
			xfade_AddTimedText("Looks like plans for an Imperial Battle Station!", OUTPOST_PLANS_START,
							   OUTPOST_PLANS_END, OUTPOST_CAPTION_FONT, OUTPOST_PLANS_X, OUTPOST_PLANS_Y,
							   OUTPOST_PLANS_COLOR);
			xfade_AddTimedText("Relay it with the next High Security transmissions.", OUTPOST_RELAY_START,
							   OUTPOST_RELAY_END, OUTPOST_CAPTION_FONT, OUTPOST_RELAY_X, OUTPOST_RELAY_Y,
							   OUTPOST_RELAY_COLOR);
			break;
	}
	resourceFile = xres_Open_Resource("outpost.lfd");
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	g_outpostRepeatBackground = 0;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_SEND_PLANS_1:
			g_outpostBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				OUTPOST_BACKGROUND_WIDTH * OUTPOST_HALF_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
			g_outpostRepeatBackground = 1;
			g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out1_f", &frame, 0, 0, 0,
													Outpost_film_SavedBackgroundCallback);
			useNormalErase = 0;
			break;
		case XW_SCENE_SEND_PLANS_2:
			if ((uint16_t)xio_Is_System_Slower_Than(OUTPOST_SPEED_THRESHOLD) != 0) {
				g_outpostBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
					OUTPOST_BACKGROUND_WIDTH * OUTPOST_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out2a_s", &frame, 0, 0, 0,
														Outpost_film_SavedBackgroundCallback);
				useNormalErase = 0;
			} else {
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out2a_f", &frame, 0, 0, 0,
														Outpost_film_NormalCallback);
			}
			break;
		case XW_SCENE_SEND_PLANS_3:
			if ((uint16_t)xio_Is_System_Slower_Than(OUTPOST_SPEED_THRESHOLD) != 0) {
				g_outpostBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
					OUTPOST_BACKGROUND_WIDTH * OUTPOST_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out2b_s", &frame, 0, 0, 0,
														Outpost_film_SavedBackgroundCallback);
				useNormalErase = 0;
			} else {
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out2b_f", &frame, 0, 0, 0,
														Outpost_film_NormalCallback);
			}
			break;
		case XW_SCENE_SEND_PLANS_4:
			if ((uint16_t)xio_Is_System_Slower_Than(OUTPOST_SPEED_THRESHOLD) != 0) {
				g_outpostBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
					OUTPOST_BACKGROUND_WIDTH * OUTPOST_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out3_s", &frame, 0, 0, 0,
														Outpost_film_SavedBackgroundCallback);
				useNormalErase = 0;
			} else {
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out3_f", &frame, 0, 0, 0,
														Outpost_film_NormalCallback);
			}
			break;
		case XW_SCENE_SEND_PLANS_5:
			g_outpostBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				OUTPOST_BACKGROUND_WIDTH * OUTPOST_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
			if ((uint16_t)xio_Is_System_Slower_Than(OUTPOST_SPEED_THRESHOLD) != 0) {
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out4_s", &frame, 0, 0, 0,
														Outpost_film_SavedBackgroundCallback);
				useNormalErase = 0;
			} else {
				g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out4_f", &frame, 0, 0, 0,
														Outpost_film_SavedBackgroundCallback);
				useNormalErase = 0;
			}
			break;
		case XW_SCENE_SEND_PLANS_6:
			g_outpostBackgroundHandle = xmemhdl_Alloc_Clear_Handle(
				OUTPOST_BACKGROUND_WIDTH * OUTPOST_BACKGROUND_HEIGHT, LANDRU_MEMORY_RESOURCE);
			g_outpostFilm = xfilm_Res_Callback_Film(resourceFile, "out5_f", &frame, 0, 0, 0,
													Outpost_film_SavedBackgroundCallback);
			useNormalErase = 0;
			break;
	}
	xfilm_Set_Film_Def_Palette(g_outpostFilm, shell->standardPalette);
	if (useNormalErase == 0) {
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		xcanvas_Invalid_Screen_Diff();
		g_outpostBackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, OUTPOST_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_outpostBackgroundActor, Outpost_user_Background);
		xactor_Set_Actor_Draw_Function(g_outpostBackgroundActor, Outpost_draw_Background);
		g_outpostCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, OUTPOST_CLOSE_Z);
		xactor_Set_Actor_User_Function(g_outpostCloseActor, Outpost_user_Close);
		xactor_Set_Actor_Draw_Function(g_outpostCloseActor, XwCutscene_DrawCloseOnRefresh);
	}
	xview_Set_View_Update_Function(Outpost_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Outpost_OpenMusic(resourceFile, g_outpostFilm);
	Outpost_LoadSoundEffects(resourceFile, g_outpostFilm);
#ifdef XW_MODERN
	XwOutpost_RunView(resourceFile, useNormalErase);
#else
	j_xviewadd_Handle_View();
	Outpost_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if (useNormalErase == 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_outpostBackgroundHandle);
	}
	xres_Close_Resource(resourceFile);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x45A6B0
void Outpost_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_SEND_PLANS_1:
			nextScene = XW_SCENE_SEND_PLANS_2;
			break;
		case XW_SCENE_SEND_PLANS_2:
			nextScene = XW_SCENE_SEND_PLANS_3;
			break;
		case XW_SCENE_SEND_PLANS_3:
			nextScene = XW_SCENE_SEND_PLANS_4;
			break;
		case XW_SCENE_SEND_PLANS_4:
			nextScene = XW_SCENE_SEND_PLANS_5;
			break;
		case XW_SCENE_SEND_PLANS_5:
			nextScene = XW_SCENE_SEND_PLANS_6;
			break;
		case XW_SCENE_SEND_PLANS_6:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
#ifdef XW_MODERN
		default:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_outpostFilm->cur_cel == g_outpostFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x45A750
int16_t Outpost_film_SavedBackgroundCallback(Film* film, FilmObject* object) {
	int16_t handled = 0;
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, Outpost_user_DirtyActor);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				Outpost_film_Actor_To_Background(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, Outpost_user_SoundCue);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x45A7D0
int16_t Outpost_film_NormalCallback(Film* film, FilmObject* object) {
	if (object->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND)
			xactor_Set_Actor_User_Function(actor, Outpost_user_SoundCue);
	}
	return 0;
}

// FUNCTION: XW 0x45A810
int16_t Outpost_film_Actor_To_Background(Actor* actor) {
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
	backgroundPixels = xmemhdl_Lock_Handle(g_outpostBackgroundHandle);
	backgroundHeight =
		g_outpostRepeatBackground != 0 ? OUTPOST_HALF_BACKGROUND_HEIGHT : OUTPOST_BACKGROUND_HEIGHT;
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						OUTPOST_BACKGROUND_WIDTH, backgroundHeight, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_outpostBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x45A900
void Outpost_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Outpost_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x45A920
void Outpost_user_Background(Actor* unusedActor, int time) {
	(void)unusedActor;
	if (time == 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_outpostPreviousDirtyRect);
	} else {
		xrect_Copy_Rect(&g_outpostPreviousDirtyRect, &g_outpostCurrentDirtyRect);
	}
	xrect_Clear_Rect(&g_outpostCurrentDirtyRect);
}

// FUNCTION: XW 0x45A970
int16_t Outpost_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								int16_t refresh) {
	const uint8_t* backgroundPixels;
	Rect sourceRect;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0)
		return 0;
	backgroundPixels = xmemhdl_Lock_Handle(g_outpostBackgroundHandle);
	if (g_outpostRepeatBackground != 0) {
		if (g_outpostPreviousDirtyRect.top < OUTPOST_HALF_BACKGROUND_HEIGHT) {
			xrect_Copy_Rect(&sourceRect, &g_outpostPreviousDirtyRect);
			if (sourceRect.bottom > OUTPOST_HALF_BACKGROUND_HEIGHT)
				sourceRect.bottom = OUTPOST_HALF_BACKGROUND_HEIGHT;
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, sourceRect.left, sourceRect.top,
										  OUTPOST_BACKGROUND_WIDTH, OUTPOST_HALF_BACKGROUND_HEIGHT);
		}
		if (g_outpostPreviousDirtyRect.bottom > OUTPOST_HALF_BACKGROUND_HEIGHT) {
			xrect_Copy_Rect(&sourceRect, &g_outpostPreviousDirtyRect);
			xrect_Offset_Rect(&sourceRect, 0, -OUTPOST_HALF_BACKGROUND_HEIGHT);
			if (sourceRect.top < 0)
				sourceRect.top = 0;
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, sourceRect.left,
										  sourceRect.top + OUTPOST_HALF_BACKGROUND_HEIGHT,
										  OUTPOST_BACKGROUND_WIDTH, OUTPOST_HALF_BACKGROUND_HEIGHT);
		}
	} else {
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &g_outpostPreviousDirtyRect,
									  g_outpostPreviousDirtyRect.left, g_outpostPreviousDirtyRect.top,
									  OUTPOST_BACKGROUND_WIDTH, OUTPOST_BACKGROUND_HEIGHT);
	}
	xmemhdl_Unlock_Handle(g_outpostBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x45AA90
void Outpost_user_DirtyActor(Actor* actor, int unusedTime) {
	Rect visibleBounds;
	Rect actorFrame;
	(void)unusedTime;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleBounds);
		xactor_Get_Actor_Frame(actor, &actorFrame);
		xcanvas_Clip_Rect_To_Canvas(&visibleBounds);
		xrect_Clip_Rect(&visibleBounds, &actorFrame);
		xrect_Enclose_Rect(&g_outpostCurrentDirtyRect, &visibleBounds);
	}
	if (actor->var2 != 0) {
		g_outpostFilm->var1 = OUTPOST_REFRESH_FULL_CANVAS;
	}
}

// FUNCTION: XW 0x45AB10
void Outpost_user_Close(Actor* actor, int unusedTime) {
	Rect frame;
	(void)unusedTime;
	if (g_outpostFilm->cur_cel == g_outpostFilm->cels || g_outpostFilm->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		g_outpostFilm->var1 = 0;
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_outpostPreviousDirtyRect);
		xrect_Enclose_Rect(&frame, &g_outpostCurrentDirtyRect);
		actor->var1 = 0;
	}
	if (!xrect_Empty_Rect(&frame)) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
}
