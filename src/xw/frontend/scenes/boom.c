#include "xw/frontend/scenes/boom.h"
#ifdef XW_MODERN
#include "xw_dos94/audio/soundext.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/audio/imuse_session.h"
#endif

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"
#include "xw/frontend/shipext.h"

#include "xw/util/landru_display.h"
#include "xw/util/shared.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/boom_task.h"
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

// GLOBAL: XW 0x4D07E8
const char* g_boomFirstMusicFilename = "bl1music.lfd";

// GLOBAL: XW 0x4D07EC
const char* g_boomFirstMusicName = "trofight";

// GLOBAL: XW 0x4D07F0
const char* g_boomSecondMusicFilename = "bl4music.lfd";

// GLOBAL: XW 0x4D07F4
const char* g_boomSecondMusicName = "runs2";

// GLOBAL: XW 0x4D0818
char g_boomResourceFilename[BOOM_RESOURCE_NAME_CAPACITY] = "boom.lfd";

// GLOBAL: XW 0x4D0826
char g_boomFilmNames[BOOM_FILM_COUNT][BOOM_RESOURCE_NAME_CAPACITY] = { "boom1_f", "boom2_f", "boom2_s",
																	   "boom3_f", "boom3_l" };

// GLOBAL: XW 0x4F4C50
XwCutsceneMusicTransitionState g_boomMusicState = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F4C60
Sound* g_boomSpeech[BOOM_SPEECH_COUNT] = { NULL, NULL };

// GLOBAL: XW 0x4F4C6C
int16_t g_boomFullHeightBackground = 0;

// GLOBAL: XW 0x4F4C70
Actor* g_boomCloseActor = NULL;

// GLOBAL: XW 0x4F4C74
Film* g_boomFilm = NULL;

// GLOBAL: XW 0x4F4C78
LandruHandle g_boomBackgroundHandle = LANDRU_NULL_HANDLE;

// FUNCTION: XW 0x4369B0
void Boom_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		unsigned int startBeat;
		const char* musicName;
		const char* musicFilename;
		switch ((int16_t)shellext_Get_Cur_Scene()) {
			case XW_SCENE_RAMMING_ATTACK_2:
				startBeat = BOOM_MUSIC_SECOND_BEAT;
				musicName = g_boomFirstMusicName;
				musicFilename = g_boomFirstMusicFilename;
				break;
			case XW_SCENE_RAMMING_ATTACK_3:
				musicName = g_boomSecondMusicName;
				musicFilename = g_boomSecondMusicFilename;
				startBeat = BOOM_MUSIC_THIRD_BEAT;
				break;
			default:
				startBeat = BOOM_MUSIC_FIRST_BEAT;
				musicName = g_boomFirstMusicName;
				musicFilename = g_boomFirstMusicFilename;
				break;
		}
		g_boomMusicState.film = sceneFilm;
		g_boomMusicState.activeSound = xsound_Find_Gmid(musicName);
		if (g_boomMusicState.activeSound == NULL) {
			ResFile* resourceFile = xres_Open_Resource(musicFilename);
			g_boomMusicState.activeSound = xsound_Res_Music(resourceFile, musicName);
			g_boomMusicState.transitionSound = g_boomMusicState.activeSound;
			xres_Close_Resource(resourceFile);
			soundext_Start_Resource_Sound(g_boomMusicState.activeSound);
			soundext_ScanMidi(g_boomMusicState.activeSound, 0, startBeat, BOOM_MUSIC_START_TICK);
		}
		if ((int16_t)shellext_Get_Cur_Scene() == XW_SCENE_RAMMING_ATTACK_2) {
			ResFile* transitionResourceFile = xres_Open_Resource(g_boomSecondMusicFilename);
			g_boomMusicState.transitionSound =
				xsound_Res_Music(transitionResourceFile, g_boomSecondMusicName);
			xres_Close_Resource(transitionResourceFile);
		}
		xsound_Set_Sound_Keep(g_boomMusicState.activeSound);
		xsound_Set_Sound_User_Function(g_boomMusicState.activeSound, Boom_MusicCallback);
	}
}

// FUNCTION: XW 0x436AD0
void Boom_MusicCallback(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_RAMMING_ATTACK_2) {
		if (g_boomMusicState.film->cur_cel == BOOM_MUSIC_TRANSITION_CEL) {
			soundext_ClearTriggers();
			soundext_SetTriggerContext(g_boomMusicState.activeSound, BOOM_MUSIC_TRANSITION_MARKER);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_START, g_boomMusicState.transitionSound->id, 0, 0,
										 0, 0, 0);
			soundext_QueueTriggerCommand(XW_MUSIC_TRIGGER_END, 0, 0, 0, 0, 0, 0);
			xsound_Set_Sound_Keep(g_boomMusicState.transitionSound);
		}
	} else if (shellext_Get_Cur_Scene() == XW_SCENE_RAMMING_ATTACK_3 &&
			   g_boomMusicState.film->cur_cel == BOOM_MUSIC_REPOSITION_CEL) {
		int legacyBeat;
		int legacyTick;
		j_lolevel_ImPause();
		legacyBeat = soundext_GetMusicParam(g_boomMusicState.transitionSound, XW_SOUND_QUERY_BEAT, 0);
		legacyTick = soundext_GetMusicParam(g_boomMusicState.transitionSound, XW_SOUND_QUERY_TICK, 0);
		j_lolevel_ImResume();
		soundext_JumpMidi(g_boomMusicState.transitionSound, BOOM_MUSIC_REPOSITION_GROUP,
						  BOOM_MUSIC_REPOSITION_BEAT - legacyBeat % BOOM_MUSIC_BEAT_PERIOD, legacyTick);
#ifdef XW_MODERN
		Dos94_soundext_SetTranspose(g_boomMusicState.transitionSound, 0, BOOM_MUSIC_PARAM9_VALUE);
#else
		soundext_SetTranspose(0, 0, BOOM_MUSIC_PARAM9_VALUE);
#endif
	}
}

// FUNCTION: XW 0x436BE0
void Boom_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_BIG, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_TORCH, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 0);
	}
	if (ShellPreferences_GetSfxEnabled() != 0 && shellext_Get_Cur_Scene() == XW_SCENE_RAMMING_ATTACK_1) {
		ResFile* speechResource = xres_Open_Resource("bmspch.lfd");
		if (speechResource != NULL) {
			g_boomSpeech[0] = xsound_Res_Digital_Sound(speechResource, "rebel1");
			g_boomSpeech[1] = xsound_Res_Digital_Sound(speechResource, "rebel2");
			xres_Close_Resource(speechResource);
		}
	}
}

// FUNCTION: XW 0x436C90
void Boom_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (cue) {
			case BOOM_CUE_FLYBY_1:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
				break;
			case BOOM_CUE_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_BIG);
				break;
			case BOOM_CUE_TORCH:
				soundext_Play_SFX(XW_SHELL_SFX_TORCH);
				break;
			case BOOM_CUE_FLYBY_5:
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_5);
				break;
			case BOOM_CUE_REBEL_1:
				if (ShellPreferences_GetSfxEnabled() != 0) {
					soundext_Start_Resource_SFX(g_boomSpeech[0]);
				}
				break;
			case BOOM_CUE_REBEL_2:
				if (ShellPreferences_GetSfxEnabled() != 0) {
					soundext_Start_Resource_SFX(g_boomSpeech[1]);
				}
				break;
		}
	}
}

// FUNCTION: XW 0x436D40
XwShellSceneResult Boom_Play(struct XwShellContext* shell) {
	ResFile* sceneResource;
	int16_t filmVariant;
	LandruHandle backgroundHandle;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	if (shellext_Get_Cur_Scene() == XW_SCENE_RAMMING_ATTACK_1) {
		xfade_AddTimedText("Ramming course locked in.", BOOM_FIRST_CAPTION_START, BOOM_FIRST_CAPTION_END,
						   BOOM_CAPTION_FONT, BOOM_FIRST_CAPTION_X, BOOM_CAPTION_Y, BOOM_CAPTION_COLOR);
		xfade_AddTimedText("Right!  Let's get out of here.", BOOM_SECOND_CAPTION_START,
						   BOOM_SECOND_CAPTION_END, BOOM_CAPTION_FONT, BOOM_SECOND_CAPTION_X, BOOM_CAPTION_Y,
						   BOOM_CAPTION_COLOR);
	}
	sceneResource = xres_Open_Resource(g_boomResourceFilename);
	xrect_Set_Rect(&frame, 0, 0, BOOM_BACKGROUND_WIDTH, BOOM_BACKGROUND_HEIGHT);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_RAMMING_ATTACK_1:
			filmVariant = BOOM_FIRST_FILM;
			g_boomFullHeightBackground = 1;
			break;
		case XW_SCENE_RAMMING_ATTACK_2:
			if ((uint16_t)xio_Is_System_Slower_Than(BOOM_FILM_SPEED_THRESHOLD) != 0) {
				filmVariant = BOOM_SECOND_SLOW_FILM;
				g_boomFullHeightBackground = 1;
			} else {
				filmVariant = BOOM_SECOND_FAST_FILM;
				g_boomFullHeightBackground = 1;
			}
			break;
		case XW_SCENE_RAMMING_ATTACK_3:
			if (Shared_ReturnZero() != 0) {
				filmVariant = BOOM_THIRD_CROPPED_FILM;
				g_boomFullHeightBackground = 0;
			} else {
				filmVariant = BOOM_THIRD_FULL_FILM;
				g_boomFullHeightBackground = 1;
			}
			break;
		default:
			/* The original derives an unchecked film index from shell-pointer bits. */
			filmVariant = BOOM_FIRST_FILM;
			break;
	}
	if (g_boomFullHeightBackground != 0)
		backgroundHandle = xmemhdl_Alloc_Clear_Handle(BOOM_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	else
		backgroundHandle = xmemhdl_Alloc_Clear_Handle(BOOM_CROPPED_BYTES, LANDRU_MEMORY_RESOURCE);
	g_boomBackgroundHandle = backgroundHandle;
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	g_boomFilm = xfilm_Res_Callback_Film(sceneResource, g_boomFilmNames[filmVariant - BOOM_FIRST_FILM],
										 &frame, 0, 0, 0, Boom_film_Callback);
	g_boomCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, BOOM_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_boomCloseActor, Boom_user_Close);
	xactor_Set_Actor_Draw_Function(g_boomCloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_boomFilm, shell->standardPalette);
	xview_Set_View_Update_Function(Boom_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Boom_OpenMusic(sceneResource, g_boomFilm);
	Boom_OpenSounds(sceneResource, g_boomFilm);
#ifdef XW_MODERN
	XwBoom_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_boomBackgroundHandle);
	xres_Close_Resource(sceneResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x436FB0
void Boom_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_RAMMING_ATTACK_1:
			nextScene = XW_SCENE_RAMMING_ATTACK_2;
			break;
		case XW_SCENE_RAMMING_ATTACK_2:
			nextScene = XW_SCENE_RAMMING_ATTACK_3;
			break;
		case XW_SCENE_RAMMING_ATTACK_3:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_boomFilm->cur_cel == g_boomFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x437020
int16_t Boom_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Boom_film_Actor_To_Background(actor);
			if (actor->var2 == BOOM_BACKGROUND_DRAW)
				xactor_Set_Actor_Draw_Function(actor, Boom_draw_Background);
			consumeObject = actor->var2 == BOOM_BACKGROUND_CONSUME;
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Boom_user_Sound);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4370A0
int16_t Boom_film_Actor_To_Background(Actor* actor) {
	int16_t previousHeight;
	int16_t previousWidth;
	uint8_t* previousPixels;
	Rect actorClip;
	Rect previousClip;
	Rect canvasBounds;
	int16_t drawResult = 0;
	int16_t cropOriginY;
	uint8_t* backgroundPixels;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_boomBackgroundHandle);
	if (g_boomFullHeightBackground != 0) {
		xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
							BOOM_BACKGROUND_WIDTH, BOOM_BACKGROUND_HEIGHT, 0);
		cropOriginY = 0;
	} else {
		xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
							BOOM_BACKGROUND_WIDTH, BOOM_CROPPED_HEIGHT, 0);
		cropOriginY = BOOM_CROP_ORIGIN_Y;
	}
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xrect_Offset_Rect(&actorClip, 0, -cropOriginY);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y - cropOriginY, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_boomBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x4371D0
void Boom_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Boom_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x4371F0
int16_t Boom_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x, int16_t y,
							 int16_t refresh) {
	Rect sourceRect;
	int16_t baseX;
	int16_t baseY;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	if (refresh == 0)
		return 0;
	baseX = x;
	baseY = y;
	if (g_boomFullHeightBackground != 0) {
		const uint8_t* backgroundPixels;
		int16_t tileX;
		int16_t tileY;
		while (baseX >= 0)
			baseX -= BOOM_BACKGROUND_WIDTH;
		while (baseY >= 0)
			baseY -= BOOM_BACKGROUND_HEIGHT;
		xrect_Set_Rect(&sourceRect, 0, 0, BOOM_BACKGROUND_WIDTH, BOOM_BACKGROUND_HEIGHT);
		backgroundPixels = xmemhdl_Lock_Handle(g_boomBackgroundHandle);
		for (tileY = 0; tileY < BOOM_TILE_ROWS * BOOM_BACKGROUND_HEIGHT; tileY += BOOM_BACKGROUND_HEIGHT) {
			for (tileX = 0; tileX < BOOM_TILE_COLUMNS * BOOM_BACKGROUND_WIDTH;
				 tileX += BOOM_BACKGROUND_WIDTH) {
				stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, tileX + baseX, tileY + baseY,
											  BOOM_BACKGROUND_WIDTH, BOOM_BACKGROUND_HEIGHT);
			}
		}
		xmemhdl_Unlock_Handle(g_boomBackgroundHandle);
	} else {
		const uint8_t* backgroundPixels;
		xrect_Set_Rect(&sourceRect, 0, 0, BOOM_BACKGROUND_WIDTH, BOOM_CROPPED_HEIGHT);
		backgroundPixels = xmemhdl_Lock_Handle(g_boomBackgroundHandle);
		stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, 0, BOOM_CROP_ORIGIN_Y,
									  BOOM_BACKGROUND_WIDTH, BOOM_CROPPED_HEIGHT);
		xmemhdl_Unlock_Handle(g_boomBackgroundHandle);
	}
	return 1;
}

// FUNCTION: XW 0x437360
void Boom_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_boomFilm->cur_cel == g_boomFilm->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}
