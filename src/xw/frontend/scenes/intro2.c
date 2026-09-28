#include "xw/frontend/scenes/intro2.h"

#include "xw/audio/lolevel.h"
#include "xw/audio/soundext.h"
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include "xw/util/landru_display.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/intro2_task.h"
#endif

#include <landru/actcust.h>
#include <landru/canvas.h>
#include <landru/error.h>
#include <landru/io.h>
#include <landru/view.h>
#include <landru/viewadd.h>

// GLOBAL: XW 0x4D5198
const char* g_intro2ResourceFilename = "intro2.lfd";

// GLOBAL: XW 0x4D519C
const char* g_intro2SlowFilmName = "int2640a";

// GLOBAL: XW 0x4D51A0
const char* g_intro2FilmName = "int2640a";

// GLOBAL: XW 0x4F76F0
XwSceneMusicHandles g_intro2MusicState = { NULL, NULL };

// GLOBAL: XW 0x4F7910
Rect g_intro2DirtyRect = { 0 };

// GLOBAL: XW 0x4F7918
Film* g_intro2Film = NULL;

// GLOBAL: XW 0x4F791C
Actor* g_intro2BackgroundActor = NULL;

// GLOBAL: XW 0x4F7920
Actor* g_intro2BackgroundSource = NULL;

// GLOBAL: XW 0x4F7928
Rect g_intro2BackgroundRedrawRect = { 0 };

// GLOBAL: XW 0x4F7930
int16_t g_intro2PreviousXStorage = 0;

// GLOBAL: XW 0x4F7934
Actor* g_intro2EraseActor = NULL;

// GLOBAL: XW 0x4F7938
LandruHandle g_intro2BackgroundBuffers[INTRO2_BACKGROUND_BUFFER_COUNT] = { 0 };

// FUNCTION: XW 0x44FEC0
void Intro2_OpenMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_intro2MusicState.film = film;
		g_intro2MusicState.sound = xsound_Find_Gmid("inattack");
		if (g_intro2MusicState.sound == NULL) {
			ResFile* musicResource = xres_Open_Resource("inmusic.lfd");
			g_intro2MusicState.sound = xsound_Res_Music(musicResource, "inattack");
			xres_Close_Resource(musicResource);
			soundext_Start_Resource_Sound(g_intro2MusicState.sound);
			soundext_ScanMidi(g_intro2MusicState.sound, 0, INTRO2_MUSIC_START_BEAT, 0);
		}
		xsound_Set_Sound_Keep(g_intro2MusicState.sound);
		xsound_Set_Sound_User_Function(g_intro2MusicState.sound, Intro2_user_Music);
	}
}

// FUNCTION: XW 0x44FF70
void Intro2_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0) {
		lolevel_ImStopAllSounds();
	}
}

// FUNCTION: XW 0x44FF80
void Intro2_user_Music(Sound* sound, int time) {
	(void)sound;
	(void)time;
	switch (g_intro2MusicState.film->cur_cel) {
		case INTRO2_MUSIC_FIRST_CONTROL_CEL:
			soundext_SetHook(g_intro2MusicState.sound, XW_SOUND_CONTROL_DIRECT, INTRO2_MUSIC_FIRST_CONTROL,
							 0);
			break;
		case INTRO2_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_FadeVolume(g_intro2MusicState.sound, INTRO2_MUSIC_QUIET_VOLUME,
									INTRO2_MUSIC_QUIET_DURATION);
			break;
		case INTRO2_MUSIC_SECOND_CONTROL_CEL:
			soundext_SetHook(g_intro2MusicState.sound, XW_SOUND_CONTROL_DIRECT, INTRO2_MUSIC_SECOND_CONTROL,
							 0);
			break;
		case INTRO2_MUSIC_THIRD_CONTROL_CEL:
			soundext_SetHook(g_intro2MusicState.sound, XW_SOUND_CONTROL_DIRECT, INTRO2_MUSIC_THIRD_CONTROL,
							 0);
			break;
		case INTRO2_MUSIC_LOUD_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0)
				soundext_FadeVolume(g_intro2MusicState.sound, INTRO2_MUSIC_LOUD_VOLUME,
									INTRO2_MUSIC_LOUD_DURATION);
			break;
	}
}

// FUNCTION: XW 0x450100
void Intro2_LoadSoundEffects(void) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		if (ShellPreferences_GetSfxEnabled() != 0) {
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_2, 0, NULL, 0, 1);
		} else {
			soundext_LoadSfx(XW_SHELL_SFX_TIE_APPROACH_1, 0, NULL, 0, 0);
		}
		soundext_LoadSfx(XW_SHELL_SFX_FLY_SHOTS, 0, NULL, 0, 1);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_1, 0, NULL, 0, 0);
		soundext_LoadSfx(XW_SHELL_SFX_EXPLOSION_BIG, 0, NULL, 1, 0);
	}
	ShellPreferences_GetSfxEnabled();
}

// FUNCTION: XW 0x450170
void Intro2_HandleSoundAction(int16_t action) {
	if (ShellPreferences_GetSfxEnabled() != 0) {
		switch (action) {
			case INTRO2_ACTION_TIE_APPROACH:
				if (ShellPreferences_GetSfxEnabled() != 0) {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_2);
				} else {
					soundext_Play_SFX(XW_SHELL_SFX_TIE_APPROACH_1);
				}
				break;
			case INTRO2_ACTION_FLY_SHOTS:
				soundext_Play_SFX(XW_SHELL_SFX_FLY_SHOTS);
				break;
			case INTRO2_ACTION_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_1);
				break;
			case INTRO2_ACTION_BIG_EXPLOSION:
				soundext_Play_SFX(XW_SHELL_SFX_EXPLOSION_BIG);
				break;
		}
	}
}

// FUNCTION: XW 0x454620
XwShellSceneResult Intro2_Attack(struct XwShellContext* shell) {
	ResFile* sceneResource = xres_Open_Resource(g_intro2ResourceFilename);
	Rect frame;
	xrect_Set_Rect(&frame, 0, 0, INTRO2_FRAME_WIDTH, INTRO2_FRAME_HEIGHT);
	if ((uint16_t)xio_Is_System_Slower_Than(INTRO2_BACKGROUND_CACHE_SPEED) != 0) {
		g_intro2BackgroundBuffers[0] =
			xmemhdl_Alloc_Clear_Handle(INTRO2_LOWER_BUFFER_BYTES, LANDRU_MEMORY_RESOURCE);
		g_intro2BackgroundBuffers[1] =
			xmemhdl_Alloc_Clear_Handle(INTRO2_BACKGROUND_BYTES, LANDRU_MEMORY_RESOURCE);
		xrect_Copy_Rect(&g_intro2BackgroundRedrawRect, &frame);
		xrect_Set_Rect(&g_intro2DirtyRect, 0, 0, 0, 0);
		xviewadd_Clear_View();
		xview_Disable_All_View_Erase();
		g_intro2Film = xfilm_Res_Callback_Film(sceneResource, g_intro2SlowFilmName, &frame, 0, 0, 0,
											   Intro2_film_Callback);
		xfilm_Set_Film_Def_Palette(g_intro2Film, shell->standardPalette);
		g_intro2BackgroundActor =
			xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, INTRO2_BACKGROUND_Z);
		xactor_Set_Actor_User_Function(g_intro2BackgroundActor, Intro2_user_Background);
		xactor_Set_Actor_Draw_Function(g_intro2BackgroundActor, Intro2_draw_Background);
		g_intro2EraseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, INTRO2_ERASE_Z);
		xactor_Set_Actor_User_Function(g_intro2EraseActor, Intro2_user_Erase);
		xactor_Set_Actor_Draw_Function(g_intro2EraseActor, Cutscene_DrawConditionalErase);
	} else {
		g_intro2Film =
			xfilm_Res_Callback_Film(sceneResource, g_intro2FilmName, &frame, 0, 0, 0, Intro2_film_Callback);
		xfilm_Set_Film_Def_Palette(g_intro2Film, shell->standardPalette);
	}
	xview_Set_View_Update_Function(Intro2_end_View);
	Intro2_OpenMusic(sceneResource, g_intro2Film);
	Intro2_LoadSoundEffects();
#ifdef XW_MODERN
	XwIntro2_RunView(sceneResource);
#else
	j_xviewadd_Handle_View();
	Intro2_CloseMusic();
	soundext_ResetEnabledSfxCache();
	xview_Clear_View_Update_Function();
	if ((uint16_t)xio_Is_System_Slower_Than(INTRO2_BACKGROUND_CACHE_SPEED) != 0) {
		xview_Enable_All_View_Erase();
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		xview_Set_View_Frame(0, &frame);
		xview_Set_View_Pos(0, frame.left, frame.top);
		xmemhdl_Free_Handle(g_intro2BackgroundBuffers[0]);
		xmemhdl_Free_Handle(g_intro2BackgroundBuffers[1]);
	}
	xres_Close_Resource(sceneResource);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x454870
void Intro2_end_View(int time) {
	int16_t exitScene;

	(void)time;
	if (g_savedShellPreferences.introPlaybackMode == 0) {
		if (shellext_Check_Scene_Exit(&exitScene, XW_SCENE_INTRO_REBEL_BRIDGE, XW_SCENE_REGISTER_INITIAL,
									  g_intro2Film->cur_cel == g_intro2Film->cels) != 0) {
			xerror_Set_Landru_Exit(exitScene);
		}
	} else if (g_intro2Film->cur_cel == g_intro2Film->cels) {
		xerror_Set_Landru_Exit(XW_SCENE_INTRO_REBEL_BRIDGE);
	}
	xrect_Set_Rect(&g_intro2DirtyRect, 0, 0, 0, 0);
}

// FUNCTION: XW 0x4548E0
int16_t Intro2_film_Callback(Film* film, FilmObject* filmObject) {
	int16_t consumeObject = 0;
	if (filmObject->id == FTC_ACTOR) {
		Actor* actor;
		xfilm_Rewind_Actor_Film(film, filmObject, filmObject + 1);
		actor = filmObject->object;
		if (xio_Is_System_Slower_Than(INTRO2_BACKGROUND_CACHE_SPEED) != 0) {
			if (actor->var1 == CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND) {
				Intro2_StampBackgroundBuffers(actor);
				if (actor->var2 == INTRO2_BACKGROUND_MOVEMENT_SOURCE) {
					actor->draw = NULL;
					g_intro2BackgroundSource = actor;
				} else {
					consumeObject = 1;
				}
			} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS) {
				xactor_Set_Actor_User_Function(actor, Intro2_user_DirtyBounds);
			} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
				xactor_Set_Actor_User_Function(actor, Intro2_user_SoundAction);
			}
		} else if (actor->var1 == CUTSCENE_ACTOR_ROLE_SOUND) {
			xactor_Set_Actor_User_Function(actor, Intro2_user_SoundAction);
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x4549A0
int16_t Intro2_StampBackgroundBuffers(Actor* actor) {
	int16_t drawResult = 0;
	int16_t previousHeight;
	int16_t previousWidth;
	uint8_t* previousPixels;
	Rect actorClip;
	Rect canvasBounds;
	Rect previousClip;
	int16_t bufferIndex;
	int16_t bufferXOffset = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	for (bufferIndex = 0; bufferIndex < INTRO2_BACKGROUND_BUFFER_COUNT; ++bufferIndex) {
		int16_t bufferYOffset;
		uint8_t* bufferPixels;
		if (bufferIndex == 0) {
			if (actor->draw != NULL)
				drawResult =
					actor->draw(actor, &canvasBounds, &canvasBounds, bufferXOffset + actor->x, actor->y, 1);
			bufferYOffset = INTRO2_SCROLL_BAND_HEIGHT;
		} else {
			bufferYOffset = 0;
		}
		bufferPixels = xmemhdl_Lock_Handle(g_intro2BackgroundBuffers[bufferIndex]);
		if (bufferIndex != 0)
			xcanvas_Push_Canvas(&previousPixels, bufferPixels, &previousClip, &previousWidth, &previousHeight,
								INTRO2_BACKGROUND_WIDTH, INTRO2_BACKGROUND_HEIGHT, 0);
		else
			xcanvas_Push_Canvas(&previousPixels, bufferPixels, &previousClip, &previousWidth, &previousHeight,
								INTRO2_BACKGROUND_WIDTH, INTRO2_LOWER_BUFFER_HEIGHT, 0);
		if (actor->draw != NULL) {
			xrect_Copy_Rect(&actorClip, &actor->frame);
			xcanvas_Clip_Rect_To_Canvas(&actorClip);
			xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
			drawResult = actor->draw(actor, &canvasBounds, &actorClip, bufferXOffset + actor->x,
									 actor->y - bufferYOffset, 1);
			xcanvas_Max_Drawing_Canvas_Clip();
		}
		xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
		xmemhdl_Unlock_Handle(g_intro2BackgroundBuffers[bufferIndex]);
		bufferXOffset += INTRO2_BACKGROUND_WIDTH;
	}
	return drawResult;
}

// FUNCTION: XW 0x454B00
void Intro2_user_DirtyBounds(Actor* actor, int time) {
	Rect visibleRect;
	(void)time;
	if (xactor_Is_Actor_Visible(actor) != 0) {
		xactor_Get_Actor_Rect(actor, &visibleRect);
		xcanvas_Clip_Rect_To_Canvas(&visibleRect);
		xrect_Enclose_Rect(&g_intro2DirtyRect, &visibleRect);
	}
}

// FUNCTION: XW 0x454B50
void Intro2_user_SoundAction(Actor* actor, int time) {
	int16_t action = actor->var2;
	(void)time;
	if (action != 0) {
		Intro2_HandleSoundAction(action);
	}
}

// FUNCTION: XW 0x454B70
void Intro2_user_Erase(Actor* actor, int time) {
	Rect frame;
	(void)time;
	if (g_intro2Film->cur_cel == g_intro2Film->cels) {
		xcanvas_Get_Drawing_Canvas_Bounds(&frame);
		actor->var1 = 1;
	} else {
		xrect_Copy_Rect(&frame, &g_intro2DirtyRect);
		xrect_Enclose_Rect(&frame, &g_intro2BackgroundRedrawRect);
		actor->var1 = 0;
	}
	xview_Set_View_Frame(CUTSCENE_MAIN_VIEW, &frame);
	xview_Set_View_Pos(CUTSCENE_MAIN_VIEW, frame.left, frame.top);
}

// FUNCTION: XW 0x454BF0
void Intro2_user_Background(Actor* actor, int time) {
	if (time == 0) {
		g_intro2PreviousXStorage = g_intro2BackgroundSource->x;
	}
	if (g_intro2BackgroundSource->xv != 0 || g_intro2BackgroundSource->yv != 0 ||
		g_intro2BackgroundSource->xvf != 0 || g_intro2BackgroundSource->yvf != 0) {
		actor->var1 = INTRO2_BACKGROUND_REDRAW_FRAMES;
	}
	if (actor->var1 != 0) {
		xcanvas_Get_Drawing_Canvas_Bounds(&g_intro2BackgroundRedrawRect);
		--actor->var1;
	}
}

// FUNCTION: XW 0x454C50
int16_t Intro2_draw_Background(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	Rect bufferRect;
	Rect previousClip;
	int16_t bufferScreenX;
	int16_t scrollDeltaX;
	int16_t bufferIndex;
	(void)actor;
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0)
		return 0;
	bufferScreenX = g_intro2BackgroundSource->x;
	scrollDeltaX = bufferScreenX - g_intro2PreviousXStorage;
	g_intro2PreviousXStorage = bufferScreenX;
	if (scrollDeltaX != 0) {
		uint8_t* screenPixels = xcanvas_Get_Screen_Buffer();
		xrect_Set_Rect(&bufferRect, 0, 0, INTRO2_BACKGROUND_WIDTH, INTRO2_SCROLL_BAND_HEIGHT);
#ifdef XW_MODERN
		xcanvas_Scroll_Clipped_Buffer(screenPixels, &bufferRect, scrollDeltaX, 0,
									  xcanvas_Get_Current_Canvas_Bitmap()->w, INTRO2_SCROLL_BAND_HEIGHT);
#else
		xcanvas_Scroll_Clipped_Buffer(screenPixels, &bufferRect, scrollDeltaX, 0, INTRO2_BACKGROUND_WIDTH,
									  INTRO2_SCROLL_BAND_HEIGHT);
#endif
	}
	xcanvas_Get_Raster_Clip(&previousClip);
	xcanvas_Set_Drawing_Canvas_Clip(&g_intro2BackgroundRedrawRect);
	for (bufferIndex = 0; bufferIndex < INTRO2_BACKGROUND_BUFFER_COUNT; ++bufferIndex) {
		const uint8_t* bufferPixels = xmemhdl_Lock_Handle(g_intro2BackgroundBuffers[bufferIndex]);
		if (bufferIndex == 0) {
			xrect_Set_Rect(&bufferRect, 0, 0, INTRO2_BACKGROUND_WIDTH, INTRO2_LOWER_BUFFER_HEIGHT);
			stub_Copy_From_Clipped_Buffer(bufferPixels, &bufferRect, bufferScreenX, INTRO2_SCROLL_BAND_HEIGHT,
										  INTRO2_BACKGROUND_WIDTH, INTRO2_LOWER_BUFFER_HEIGHT);
		} else {
			xrect_Set_Rect(&bufferRect, 0, 0, INTRO2_BACKGROUND_WIDTH, INTRO2_BACKGROUND_HEIGHT);
			stub_Copy_From_Clipped_Buffer(bufferPixels, &bufferRect, bufferScreenX, 0,
										  INTRO2_BACKGROUND_WIDTH, INTRO2_BACKGROUND_HEIGHT);
		}
		xmemhdl_Unlock_Handle(g_intro2BackgroundBuffers[bufferIndex]);
		bufferScreenX -= INTRO2_BACKGROUND_WIDTH;
	}
	xcanvas_Set_Drawing_Canvas_Clip(&previousClip);
	xrect_Copy_Rect(&g_intro2BackgroundRedrawRect, &g_intro2DirtyRect);
	return 1;
}
