#include "xw/frontend/scenes/probe.h"
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
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/probe_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D69C8
char g_probeResourceNames[PROBE_RESOURCE_NAME_COUNT][PROBE_RESOURCE_NAME_CAPACITY] = {
	"battle4.lfd", "probe.lfd", "probe1_f", "probe2_f", "probe3_f"
};

// GLOBAL: XW 0x4F8884
XwProbeMusicState g_probeMusicState = { 0, NULL, NULL };

// GLOBAL: XW 0x4F929C
Film* g_probeFilm = NULL;

// GLOBAL: XW 0x4F92A0
Actor* g_probeCloseActor = NULL;

// GLOBAL: XW 0x4F92A4
LandruHandle g_probeBackgroundHandle = 0;

// FUNCTION: XW 0x45ABC0
void Probe_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		unsigned int startBeat;
		g_probeMusicState.param4Adjusted = 0;
		g_probeMusicState.film = sceneFilm;
		g_probeMusicState.sound = xsound_Find_Gmid("plans");
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_IMPERIAL_PROBES_2:
				startBeat = PROBE_MUSIC_SCENE_2_BEAT;
				break;
			case XW_SCENE_IMPERIAL_PROBES_3:
				startBeat = PROBE_MUSIC_SCENE_3_BEAT;
				break;
			default:
				startBeat = 0;
				break;
		}
		if (g_probeMusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("pnmusic.lfd");
			g_probeMusicState.sound = xsound_Res_Music(musicResource, "plans");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_probeMusicState.sound);
#ifdef XW_MODERN
			Dos94_soundext_SetGroup(g_probeMusicState.sound, PROBE_MUSIC_INITIAL_PARAM4);
#else
			soundext_SetGroup(0, PROBE_MUSIC_INITIAL_PARAM4);
#endif
			if (startBeat != 0) {
				soundext_ScanMidi(g_probeMusicState.sound, 0, startBeat, 0);
			}
		}
		xsound_Set_Sound_Keep(g_probeMusicState.sound);
		xsound_Set_Sound_User_Function(g_probeMusicState.sound, Probe_user_Music);
	}
}

// FUNCTION: XW 0x45ACB0
void Probe_user_Music(Sound* unusedSound, int unusedTime) {
	int filmCel = g_probeMusicState.film->cur_cel;
	(void)unusedSound;
	(void)unusedTime;
	if (shellext_Get_Cur_Scene() == XW_SCENE_IMPERIAL_PROBES_3) {
		if (g_probeMusicState.param4Adjusted == 0) {
			int beat = soundext_GetMusicParam(g_probeMusicState.sound, XW_SOUND_QUERY_BEAT, 0);
			if (beat > PROBE_MUSIC_ADJUST_AFTER_BEAT) {
#ifdef XW_MODERN
				Dos94_soundext_SetGroup(g_probeMusicState.sound, PROBE_MUSIC_ADJUSTED_PARAM4);
#else
				soundext_SetGroup(0, PROBE_MUSIC_ADJUSTED_PARAM4);
#endif
				++g_probeMusicState.param4Adjusted;
			}
		}
		if (filmCel == PROBE_MUSIC_FADE_CEL) {
			Sound* music = g_probeMusicState.sound;
			soundext_FadeVolume(music, 0, PROBE_MUSIC_FADE_DURATION);
		}
	}
}

// FUNCTION: XW 0x45AD30
void Probe_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (shellext_Get_Cur_Scene()) {
			case XW_SCENE_IMPERIAL_PROBES_1:
				soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 1);
				break;
			case XW_SCENE_IMPERIAL_PROBES_2:
				soundext_LoadSfx(XW_SHELL_SFX_HOVER_HUM, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_DROID, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_DROID_4, 0, NULL, 0, 0);
				break;
			case XW_SCENE_IMPERIAL_PROBES_3:
				soundext_LoadSfx(XW_SHELL_SFX_FLYBY_1, 0, NULL, 0, 1);
				soundext_LoadSfx(XW_SHELL_SFX_FLYBY_5, 0, NULL, 0, 1);
				soundext_LoadSfx(XW_SHELL_SFX_TORPEDO_2, 0, NULL, 0, 1);
				soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_FAR, 0, NULL, 0, 0);
				soundext_LoadSfx(XW_SHELL_SFX_BEEP_4_SECONDARY, 0, NULL, 0, 0);
				break;
		}
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x45AE00
void Probe_PlaySoundCue(int16_t cue) {
	switch (cue) {
		case PROBE_CUE_FLYBY_1:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_1);
			}
			break;
		case PROBE_CUE_FLYBY_5:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_FLYBY_5);
			}
			break;
		case PROBE_CUE_TORPEDO:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_TORPEDO_2);
			}
			break;
		case PROBE_CUE_EXPLOSION:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_FAR);
			}
			break;
		case PROBE_CUE_BEEP:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_BEEP_4_SECONDARY);
				soundext_Fade_SFX(XW_SHELL_SFX_BEEP_4_SECONDARY, PROBE_BEEP_VOLUME, 0);
			}
			break;
		case PROBE_CUE_START_HOVER:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_HOVER_HUM);
			}
			break;
		case PROBE_CUE_STOP_HOVER:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Stop_SFX(XW_SHELL_SFX_HOVER_HUM);
			}
			break;
		case PROBE_CUE_DROID:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_DROID);
			}
			break;
		case PROBE_CUE_DROID_4:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_Play_SFX(XW_SHELL_SFX_DROID_4);
			}
			break;
	}
}

// FUNCTION: XW 0x45B9F0
XwShellSceneResult Probe_Play(struct XwShellContext* shell) {
	ResFile* battleResource;
	ResFile* sceneResource;
	int16_t filmNameIndex;
	Rect frame;
	LandruDisplay_SetLowResolutionMode(1);
	battleResource = xres_Open_Resource(g_probeResourceNames[PROBE_BATTLE_RESOURCE]);
	sceneResource = xres_Open_Resource(g_probeResourceNames[PROBE_SCENE_RESOURCE]);
	xrect_Set_Rect(&frame, 0, 0, PROBE_BACKGROUND_WIDTH, PROBE_BACKGROUND_HEIGHT);
	g_probeBackgroundHandle = xmemhdl_Alloc_Clear_Handle(PROBE_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
	xviewadd_Clear_View();
	xview_Disable_All_View_Erase();
	xcanvas_Invalid_Screen_Diff();
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_IMPERIAL_PROBES_1:
			xfade_AddTimedText("Two Rebel ships work in conjunction to replace", PROBE_CAPTION_START,
							   PROBE_CAPTION_END, PROBE_CAPTION_FONT, PROBE_CAPTION_X, PROBE_FIRST_CAPTION_Y,
							   PROBE_CAPTION_COLOR);
			xfade_AddTimedText("Imperial probes with modified Rebel probes.", PROBE_CAPTION_START,
							   PROBE_CAPTION_END, PROBE_CAPTION_FONT, PROBE_CAPTION_X,
							   PROBE_FIRST_CAPTION_SECOND_Y, PROBE_CAPTION_COLOR);
			filmNameIndex = PROBE_FIRST_FILM;
			break;
		case XW_SCENE_IMPERIAL_PROBES_2:
			xfade_AddTimedText("Inside the cargo ship, modified probes are", PROBE_CAPTION_START,
							   PROBE_CAPTION_END, PROBE_CAPTION_FONT, PROBE_CAPTION_X, PROBE_SECOND_CAPTION_Y,
							   PROBE_CAPTION_COLOR);
			xfade_AddTimedText("readied for release.", PROBE_CAPTION_START, PROBE_CAPTION_END,
							   PROBE_CAPTION_FONT, PROBE_CAPTION_X, PROBE_SECOND_CAPTION_SECOND_Y,
							   PROBE_CAPTION_COLOR);
			filmNameIndex = PROBE_SECOND_FILM;
			break;
		case XW_SCENE_IMPERIAL_PROBES_3:
			xfade_AddTimedText("Each Imperial probe is replaced in a quick swap.", PROBE_CAPTION_START,
							   PROBE_CAPTION_END, PROBE_CAPTION_FONT, PROBE_CAPTION_X, PROBE_THIRD_CAPTION_Y,
							   PROBE_CAPTION_COLOR);
			filmNameIndex = PROBE_THIRD_FILM;
			break;
		default:
			/* The original uses incidental shell-pointer bits as the index. */
			filmNameIndex = PROBE_FIRST_FILM;
			break;
	}
	g_probeFilm = xfilm_Res_Callback_Film(sceneResource, g_probeResourceNames[filmNameIndex], &frame, 0, 0, 0,
										  Probe_film_Callback);
	g_probeCloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, PROBE_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_probeCloseActor, Probe_user_Close);
	xactor_Set_Actor_Draw_Function(g_probeCloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_probeFilm, shell->standardPalette);
	xview_Set_View_Update_Function(Probe_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	Probe_OpenMusic(sceneResource, g_probeFilm);
	Probe_LoadSoundEffects(sceneResource, g_probeFilm);
#ifdef XW_MODERN
	XwProbe_RunView(battleResource, sceneResource);
#else
	j_xviewadd_Handle_View();
	j_ShellPreferences_GetMusicEnabled();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_probeBackgroundHandle);
	xres_Close_Resource(sceneResource);
	xres_Close_Resource(battleResource);
	LandruDisplay_SetLowResolutionMode(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x45BC60
void Probe_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;

	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_IMPERIAL_PROBES_1:
			nextScene = XW_SCENE_IMPERIAL_PROBES_2;
			break;
		case XW_SCENE_IMPERIAL_PROBES_2:
			nextScene = XW_SCENE_IMPERIAL_PROBES_3;
			break;
		case XW_SCENE_IMPERIAL_PROBES_3:
			nextScene = shipext_Get_Pending_Medal_Scene();
			break;
		default:
			nextScene = XW_SCENE_EXIT_SHELL;
			break;
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, shipext_Get_Pending_Medal_Scene(),
								  g_probeFilm->cur_cel == g_probeFilm->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x45BCD0
int16_t Probe_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
			Probe_film_Actor_To_Background(actor);
			if (actor->var2 == PROBE_BACKGROUND_TILED) {
				xactor_Set_Actor_Draw_Function(actor, Probe_draw_TiledBackground);
			}
			consumeObject = actor->var2 == PROBE_BACKGROUND_CONSUME;
		}
		if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Probe_user_SoundCue);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x45BD40
int16_t Probe_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_probeBackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						PROBE_BACKGROUND_WIDTH, PROBE_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_probeBackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x45BE20
void Probe_user_SoundCue(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		Probe_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x45BE40
int16_t Probe_draw_TiledBackground(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								   int16_t refresh) {
	Rect sourceRect;
	int16_t baseX;
	int16_t baseY;
	int16_t tileX;
	int16_t tileY;
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
		baseX -= PROBE_BACKGROUND_WIDTH;
	}
	if (y >= 0) {
		baseY -= PROBE_BACKGROUND_HEIGHT;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, PROBE_BACKGROUND_WIDTH, PROBE_BACKGROUND_HEIGHT);
	backgroundPixels = (const uint8_t*)xmemhdl_Lock_Handle(g_probeBackgroundHandle);
	for (tileY = 0; tileY < PROBE_TILE_ROWS * PROBE_BACKGROUND_HEIGHT; tileY += PROBE_BACKGROUND_HEIGHT) {
		for (tileX = 0; tileX < PROBE_TILE_COLUMNS * PROBE_BACKGROUND_WIDTH;
			 tileX += PROBE_BACKGROUND_WIDTH) {
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, tileX + baseX, tileY + baseY,
										  PROBE_BACKGROUND_WIDTH, PROBE_BACKGROUND_HEIGHT);
		}
	}
	xmemhdl_Unlock_Handle(g_probeBackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x45BF20
void Probe_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_probeFilm->cur_cel == g_probeFilm->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}
