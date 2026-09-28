#include "xw/frontend/scenes/bintro.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/bintro_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D0140
char g_bintroResourceNames[BINTRO_RESOURCE_NAME_COUNT][BINTRO_RESOURCE_NAME_SIZE] = { "bintro.lfd",
																					  "fleets.lfd", "bint1_f",
																					  "bint1_s", "bint2_f" };

// GLOBAL: XW 0x4F4B60
XwSceneMusicHandles g_bintroMusicState = { NULL, NULL };

// GLOBAL: XW 0x4F4B68
Rect g_bintroDirtyRect = { 0 };

// GLOBAL: XW 0x4F4B70
Actor* g_bintroBackgroundActor = NULL;

// GLOBAL: XW 0x4F4B74
Film* g_bintroFilm = NULL;

// GLOBAL: XW 0x4F4B78
Actor* g_bintroEraseActor = NULL;

// GLOBAL: XW 0x4F4B80
Rect g_bintroPreviousDirtyRect = { 0 };

// GLOBAL: XW 0x4F4B88
LandruHandle g_bintroBackgroundBuffer = LANDRU_NULL_HANDLE;

// GLOBAL: XW 0x4F4B90
Sound* g_bintroSpeechSounds[BINTRO_SPEECH_COUNT] = { NULL, NULL, NULL };

// FUNCTION: XW 0x4343E0
void BIntro_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_bintroMusicState.film = film;
		g_bintroMusicState.sound = xsound_Find_Gmid("sendplan");
		if (g_bintroMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("opmusic.lfd");
			g_bintroMusicState.sound = xsound_Res_Music(musicResource, "sendplan");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_bintroMusicState.sound);
			if (shellext_Get_Cur_Scene() == XW_SCENE_BWING_ARRIVAL_2) {
				soundext_ScanMidi(g_bintroMusicState.sound, 0, BINTRO_MUSIC_ARRIVAL_BEAT, 0);
			}
		}
		xsound_Set_Sound_Keep(g_bintroMusicState.sound);
		xsound_Set_Sound_User_Function(g_bintroMusicState.sound, BIntro_user_Music);
	}
}

// FUNCTION: XW 0x434490
void BIntro_SetLegacyMusicLevel(int value, int duration) {
	if (g_bintroMusicState.sound)
		soundext_FadeVolume(g_bintroMusicState.sound, value, duration);
}

// FUNCTION: XW 0x4344B0
void BIntro_user_Music(Sound* sound, int time) {
	int filmCel = g_bintroMusicState.film->cur_cel;
	(void)sound;
	(void)time;
	if (shellext_Get_Cur_Scene() == XW_SCENE_BWING_ARRIVAL_2 && filmCel == BINTRO_MUSIC_FADE_CEL)
		soundext_FadeVolume(g_bintroMusicState.sound, 0, BINTRO_MUSIC_FADE_DURATION);
}

// FUNCTION: XW 0x4344F0
XwShellSceneResult BIntro_BWingArrival(struct XwShellContext* shell) {
	ResFile* fleetResource;
	ResFile* introResource;
	Rect frame;
	int16_t filmNameIndex;
	LandruDisplay_SetLowResolutionMode(1);
	if (shellext_Get_Cur_Scene() == XW_SCENE_BWING_ARRIVAL_2) {
		xfade_AddTimedText("Lt. Farlander, do you have", BINTRO_QUESTION_FIRST_START,
						   BINTRO_QUESTION_FIRST_END, BINTRO_CAPTION_FONT, BINTRO_QUESTION_FIRST_X,
						   BINTRO_QUESTION_FIRST_Y, BINTRO_QUESTION_FIRST_COLOR);
		xfade_AddTimedText("the B-wings combat-ready?", BINTRO_QUESTION_SECOND_START,
						   BINTRO_QUESTION_SECOND_END, BINTRO_CAPTION_FONT, BINTRO_QUESTION_SECOND_X,
						   BINTRO_QUESTION_SECOND_Y, BINTRO_QUESTION_SECOND_COLOR);
		xfade_AddTimedText("Yes Cmdr. Skywalker, they will be a", BINTRO_ANSWER_FIRST_START,
						   BINTRO_ANSWER_FIRST_END, BINTRO_CAPTION_FONT, BINTRO_ANSWER_FIRST_X,
						   BINTRO_ANSWER_FIRST_Y, BINTRO_ANSWER_FIRST_COLOR);
		xfade_AddTimedText("powerful new weapon against the Empire.", BINTRO_ANSWER_SECOND_START,
						   BINTRO_ANSWER_SECOND_END, BINTRO_CAPTION_FONT, BINTRO_ANSWER_SECOND_X,
						   BINTRO_ANSWER_SECOND_Y, BINTRO_ANSWER_SECOND_COLOR);
		xfade_AddTimedText("Good work, Lieutenant.", BINTRO_PRAISE_START, BINTRO_PRAISE_END,
						   BINTRO_CAPTION_FONT, BINTRO_PRAISE_X, BINTRO_PRAISE_Y, BINTRO_PRAISE_COLOR);
	}
	if (shellext_Get_Cur_Scene() == XW_SCENE_BWING_ARRIVAL_1)
		xfade_AddTimedText("New B-wings arrive at Rebel Fleet.", BINTRO_ARRIVAL_START, BINTRO_ARRIVAL_END,
						   BINTRO_CAPTION_FONT, BINTRO_ARRIVAL_X, BINTRO_ARRIVAL_Y, BINTRO_ARRIVAL_COLOR);
	fleetResource = xres_Open_Resource(g_bintroResourceNames[BINTRO_FLEET_RESOURCE]);
	introResource = xres_Open_Resource(g_bintroResourceNames[BINTRO_INTRO_RESOURCE]);
	xrect_Set_Rect(&frame, 0, 0, BINTRO_BACKGROUND_WIDTH, BINTRO_BACKGROUND_HEIGHT);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	xrect_Clear_Rect(&g_bintroPreviousDirtyRect);
	xrect_Clear_Rect(&g_bintroDirtyRect);
	g_bintroBackgroundBuffer = xmemhdl_Alloc_Clear_Handle(BINTRO_BACKGROUND_WIDTH * BINTRO_BACKGROUND_HEIGHT,
														  LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_BWING_ARRIVAL_1:
			filmNameIndex =
				((uint16_t)xio_Is_System_Slower_Than(BINTRO_SPEED_THRESHOLD) != 0) + BINTRO_ARRIVAL_FAST_FILM;
			break;
		case XW_SCENE_BWING_ARRIVAL_2:
			filmNameIndex = BINTRO_DIALOGUE_FILM;
			break;
		default:
			/* The original derives an invalid index from shell pointer bits. */
			filmNameIndex = BINTRO_ARRIVAL_FAST_FILM;
			break;
	}
	g_bintroFilm = xfilm_Res_Callback_Film(introResource, g_bintroResourceNames[filmNameIndex], &frame, 0, 0,
										   0, BIntro_film_Callback);
	xfilm_Set_Film_Def_Palette(g_bintroFilm, shell->standardPalette);
	g_bintroBackgroundActor =
		xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BINTRO_BACKGROUND_Z);
	xactor_Set_Actor_Draw_Function(g_bintroBackgroundActor, BIntro_draw_Background);
	g_bintroEraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BINTRO_ERASE_Z);
	xactor_Set_Actor_User_Function(g_bintroEraseActor, BIntro_user_Erase);
	xactor_Set_Actor_Draw_Function(g_bintroEraseActor, Cutscene_DrawConditionalErase);
	xrect_Clear_Rect(&g_bintroDirtyRect);
	xview_Set_View_Update_Function(BIntro_end_View);
	BIntro_OpenMusic(introResource, g_bintroFilm);
	BIntro_LoadSoundEffects();
#ifdef XW_MODERN
	XwBIntro_RunView(fleetResource, introResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_bintroBackgroundBuffer);
	xres_Close_Resource(introResource);
	xres_Close_Resource(fleetResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x4347E0
void BIntro_end_View(int time) {
	int16_t exitScene;
	int16_t nextScene;

	(void)time;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_BWING_ARRIVAL_1:
			nextScene = XW_SCENE_BWING_ARRIVAL_2;
			break;
		case XW_SCENE_BWING_ARRIVAL_2:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_bintroFilm->cur_cel == g_bintroFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x434850
int16_t BIntro_film_Callback(Film* film, FilmObject* filmObject) {
	int16_t handled = 0;
	if (filmObject->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, filmObject, filmObject + 1);
		actor = filmObject->object;
		switch (actor->var1) {
			case CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS:
				xactor_Set_Actor_User_Function(actor, BIntro_user_DirtyBounds);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				BIntro_StampBackground(actor);
				handled = 1;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, BIntro_user_SoundAction);
				break;
		}
	}
	return handled;
}

// FUNCTION: XW 0x4348D0
int16_t BIntro_StampBackground(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_bintroBackgroundBuffer);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						BINTRO_BACKGROUND_WIDTH, BINTRO_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		drawResult = actor->draw(actor, &canvasBounds, &canvasBounds, actor->x, actor->y, 1);
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_bintroBackgroundBuffer);
	return drawResult;
}

// FUNCTION: XW 0x434980
void BIntro_user_SoundAction(Actor* actor, int time) {
	int16_t action = actor->var2;
	(void)time;
	if (action != 0) {
		BIntro_HandleSoundAction(action);
	}
}

// FUNCTION: XW 0x4349A0
int16_t BIntro_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Rect canvasBounds;
	const uint8_t* backgroundPixels;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_bintroBackgroundBuffer);
	stub_Copy_From_Clipped_Buffer(backgroundPixels, &canvasBounds, canvasBounds.left, canvasBounds.top,
								  BINTRO_BACKGROUND_WIDTH, BINTRO_BACKGROUND_HEIGHT);
	xmemhdl_Unlock_Handle(g_bintroBackgroundBuffer);
	return 1;
}

// FUNCTION: XW 0x434A10
void BIntro_user_DirtyBounds(Actor* actor, int time) {
	Rect visibleRect;
	(void)time;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleRect);
		xcanvas_Clip_Rect_To_Canvas(&visibleRect);
		xrect_Enclose_Rect(&g_bintroDirtyRect, &visibleRect);
	}
}

// FUNCTION: XW 0x434A60
void BIntro_user_Erase(Actor* actor, int time) {
	Rect frame;
	if (g_bintroFilm->cur_cel != g_bintroFilm->cels && time != 0) {
		xrect_Copy_Rect(&frame, &g_bintroDirtyRect);
		xrect_Enclose_Rect(&frame, &g_bintroPreviousDirtyRect);
		actor->var1 = 0;
	} else {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = BINTRO_ERASE_FULL_CANVAS;
	}
	if (xrect_Empty_Rect(&frame) == 0) {
		xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
		xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
	}
	xrect_Copy_Rect(&g_bintroPreviousDirtyRect, &g_bintroDirtyRect);
	xrect_Clear_Rect(&g_bintroDirtyRect);
}

// FUNCTION: XW 0x434B40
void BIntro_LoadSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 1);
	}
	if (ShellPreferences_GetSfxEnabled() != 0) {
		ResFile* speechResource;
		if (shellext_Get_Cur_Scene() == XW_SCENE_BWING_ARRIVAL_2) {
			BIntro_SetLegacyMusicLevel(BINTRO_MUSIC_SPEECH_LEVEL, BINTRO_MUSIC_RISE_DURATION);
		}
		speechResource = xres_Open_Resource("bispch.lfd");
		if (speechResource != NULL) {
			if (shellext_Get_Cur_Scene() == XW_SCENE_BWING_ARRIVAL_2) {
				g_bintroSpeechSounds[BINTRO_SPEECH_LUKE_1] =
					xsound_Res_Digital_Sound(speechResource, "luke1");
				g_bintroSpeechSounds[BINTRO_SPEECH_FARLAN] =
					xsound_Res_Digital_Sound(speechResource, "farlan");
				g_bintroSpeechSounds[BINTRO_SPEECH_LUKE_2] =
					xsound_Res_Digital_Sound(speechResource, "luke2");
			}
			xres_Close_Resource(speechResource);
		}
	}
}

// FUNCTION: XW 0x434C00
void BIntro_HandleSoundAction(int16_t action) {
	switch (action) {
		case BINTRO_ACTION_FLYBY_1:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
			}
			break;
		case BINTRO_ACTION_FLYBY_5:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_5);
			}
			break;
		case BINTRO_ACTION_LUKE_1:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_bintroSpeechSounds[BINTRO_SPEECH_LUKE_1]);
			}
			break;
		case BINTRO_ACTION_FARLAN:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_bintroSpeechSounds[BINTRO_SPEECH_FARLAN]);
			}
			break;
		case BINTRO_ACTION_LUKE_2:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Start_Resource_SFX(g_bintroSpeechSounds[BINTRO_SPEECH_LUKE_2]);
				BIntro_SetLegacyMusicLevel(BINTRO_MUSIC_FULL_LEVEL, BINTRO_MUSIC_RISE_DURATION);
			}
			break;
	}
}
