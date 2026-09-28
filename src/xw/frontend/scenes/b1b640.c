#include "xw/frontend/scenes/b1b640.h"

#include "xw/audio/frontend_audio.h"
#include "xw/audio/soundext.h"
#include "xw/flight/mission/mission.h"
#include "xw/frontend/shipext.h"
#include "xw/util/landru_display.h"
#include "xw_runtime/integration/cutscene_callbacks.h"
#ifdef XW_MODERN
#include "xw_runtime/runtime/b1b640_task.h"
#endif
#include "xw/frontend/scenes/cutscene.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/frontend/shellext.h"

#include <landru/actanim.h>
#include <landru/actcust.h>
#include <landru/actdelt.h>
#include <landru/canvas.h>
#include <landru/cursor.h>
#include <landru/error.h>
#include <landru/fade.h>
#include <landru/fourcc.h>
#include <landru/memhdl.h>
#include <landru/view.h>
#include <string.h>

#include <stdlib.h>

// GLOBAL: XW 0x4CFB88
const char* g_b1b640MusicFilename = "bl1music.lfd";

// GLOBAL: XW 0x4CFB8C
const char* g_b1b640MusicName = "trofight";

// GLOBAL: XW 0x4CFB90
const char* g_b1b640HangarMusicName = "hangar";

// GLOBAL: XW 0x4CFCD8
int16_t g_b1b640JitterX[XW_B1B640_JITTER_COUNT] = { 0, 1, -1, 0, 1, -1, 0, 1 };

// GLOBAL: XW 0x4CFCE8
int16_t g_b1b640JitterY[XW_B1B640_JITTER_COUNT] = { 0, 0, 1, -1, 1, -1, 1, -1 };

// GLOBAL: XW 0x4CFCF8
B1B640Caption g_b1b640Captions[B1B640_CAPTION_COUNT] = {
	{ "This is Red Leader.  Stay close", "and watch for enemy fighters." },
	{ "This is Red Leader.  Don't worry", "about it.  Try again." },
	{ "This is Red Leader.  Not bad.", "But you still need training." },
	{ "This is Red Leader.  Excellent!", "You're ready for combat!" }
};

// GLOBAL: XW 0x4CFE38
char g_b1b640DigitalMusicPath[B1B640_MUSIC_PATH_CAPACITY] = "XwingCD\\music\\trofight.wav";

// GLOBAL: XW 0x4CFE54
char g_b1b640CaptionSeparator[B1B640_CAPTION_SEPARATOR_CAPACITY] = " ";

// GLOBAL: XW 0x4F4ACC
Sound* g_b1b640HangarMusic = NULL;

// GLOBAL: XW 0x4F4AD0
Sound* g_b1b640Music = NULL;

// GLOBAL: XW 0x4F4AD4
Film* g_b1b640MusicFilm = NULL;

// GLOBAL: XW 0x4F4AD8
Sound* g_b1b640Speech[XW_B1B640_SPEECH_COUNT] = { NULL, NULL, NULL };

// GLOBAL: XW 0x4F4AF4
int16_t g_b1b640CaptionVariant = 0;

// GLOBAL: XW 0x4F4AFC
Film* g_b1b640Film = NULL;

// GLOBAL: XW 0x4F4B00
Actor* g_b1b640CloseActor = NULL;

// GLOBAL: XW 0x4F4B04
LandruHandle g_b1b640BackgroundHandle = 0;

// FUNCTION: XW 0x4325C0
void B1b640_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_b1b640MusicFilm = sceneFilm;
		g_b1b640HangarMusic = xsound_Find_Gmid(g_b1b640HangarMusicName);
		if (g_b1b640HangarMusic != NULL) {
			xsound_Set_Sound_Keep(g_b1b640HangarMusic);
		}
		g_b1b640Music = xsound_Find_Gmid(g_b1b640MusicName);
		if (g_b1b640Music == NULL) {
			ResFile* resourceFile = xres_Open_Resource(g_b1b640MusicFilename);
			g_b1b640Music = xsound_Res_Music(resourceFile, g_b1b640MusicName);
			xres_Close_Resource(resourceFile);
			soundext_Start_Resource_Sound(g_b1b640Music);
		}
		xsound_Set_Sound_Keep(g_b1b640Music);
		xsound_Set_Sound_User_Function(g_b1b640Music, B1b640_MusicCallback);
	}
}

// FUNCTION: XW 0x432680
void B1b640_CloseMusic(void) {
	if (ShellPreferences_GetMusicEnabled() != 0 && shellext_Get_Cur_Scene() != XW_SCENE_INTRO_FORMATION) {
		Sound* music = xsound_Find_Gmid(g_b1b640MusicName);
		g_b1b640Music = music;
		if (music != NULL)
			soundext_FadeVolume(music, 0, B1B640_MUSIC_FADE_DURATION);
	}
}

// FUNCTION: XW 0x4326C0
void B1b640_MusicCallback(Sound* unusedSound, int unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
	switch (g_b1b640MusicFilm->cur_cel) {
		case B1B640_MUSIC_QUIET_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_b1b640HangarMusic, B1B640_MUSIC_QUIET_VOLUME,
									B1B640_MUSIC_FADE_DURATION);
			}
			break;
		case B1B640_MUSIC_CONTROL_CEL: {
			Sound* hangarMusic = xsound_Find_Gmid(g_b1b640HangarMusicName);
			g_b1b640HangarMusic = hangarMusic;
			if (hangarMusic != NULL) {
				soundext_SetHook(hangarMusic, XW_SOUND_CONTROL_DIRECT, B1B640_MUSIC_CONTROL_VALUE, 0);
			}
			break;
		}
		case B1B640_MUSIC_LOUD_CEL:
			if (ShellPreferences_GetSfxEnabled() != 0) {
				soundext_FadeVolume(g_b1b640HangarMusic, B1B640_MUSIC_LOUD_VOLUME,
									B1B640_MUSIC_FADE_DURATION);
			}
			break;
	}
}

// FUNCTION: XW 0x432750
void B1b640_OpenSounds(ResFile* unusedResourceFile, Film* unusedFilm, int16_t variant) {
	(void)unusedResourceFile;
	(void)unusedFilm;
	ShellPreferences_GetSfxEnabled();
	if (ShellPreferences_GetSfxEnabled() != 0) {
		g_b1b640Speech[0] = soundext_LoadSpeech(XW_SHELL_SPEECH_RED_LEADER, 0, NULL, 0);
		switch (variant) {
			case B1B640_CAPTION_FORMATION:
				g_b1b640Speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_STAY_CLOSE, 1, NULL, 0);
				break;
			case B1B640_CAPTION_TRY_AGAIN:
				g_b1b640Speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_DONT_WORRY, 1, NULL, 0);
				g_b1b640Speech[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_TRY_AGAIN, 2, NULL, 0);
				break;
			case B1B640_CAPTION_KEEP_TRAINING:
				g_b1b640Speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_NOT_BAD, 1, NULL, 0);
				g_b1b640Speech[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_STILL_NEED, 2, NULL, 0);
				break;
			case B1B640_CAPTION_READY_COMBAT:
				g_b1b640Speech[1] = soundext_LoadSpeech(XW_SHELL_SPEECH_TRAINING_EXCELLENT, 1, NULL, 0);
				g_b1b640Speech[2] = soundext_LoadSpeech(XW_SHELL_SPEECH_READY_COMBAT, 2, NULL, 0);
				break;
		}
	}
}

// FUNCTION: XW 0x432840
void B1b640_PlaySoundCue(int16_t cue) {
	if (ShellPreferences_GetSfxEnabled()) {
		soundext_Start_Resource_SFX(g_b1b640Speech[cue - 1]);
	}
}

// FUNCTION: XW 0x432A60
XwShellSceneResult B1b640_Play(struct XwShellContext* shell) {
	ResFile* resourceFile;
	Rect frame;
	char captionText[B1B640_CAPTION_CAPACITY];
	int16_t captionColor;
	int captionIndex;
	g_b1b640CaptionVariant = B1B640_CAPTION_FORMATION;
	if (shellext_Get_Cur_Scene() != XW_SCENE_INTRO_FORMATION) {
		if (g_missionRuntimeState.provingGroundsLevel > shipext_Get_Train_Level() + 1)
			g_b1b640CaptionVariant = B1B640_CAPTION_KEEP_TRAINING +
									 (g_missionRuntimeState.provingGroundsLevel > B1B640_COMBAT_READY_LEVEL);
		else
			g_b1b640CaptionVariant = B1B640_CAPTION_TRY_AGAIN;
	}
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_INTRO_FORMATION:
			captionColor = B1B640_FORMATION_COLOR;
			break;
		case XW_SCENE_TRAINING_RESULTS_AWING:
			captionColor = B1B640_AWING_COLOR;
			break;
		case XW_SCENE_TRAINING_RESULTS_XWING:
			captionColor = B1B640_XWING_COLOR;
			break;
		case XW_SCENE_TRAINING_RESULTS_YWING:
			captionColor = B1B640_YWING_COLOR;
			break;
		case XW_SCENE_TRAINING_RESULTS_BWING:
			captionColor = B1B640_BWING_COLOR;
			break;
		default:
			/* The original derives a caption color from shell pointer bits. */
			captionColor = B1B640_FORMATION_COLOR;
			break;
	}
	captionIndex = g_b1b640CaptionVariant;
	strcpy(captionText, g_b1b640Captions[captionIndex].firstLine);
	strcat(captionText, g_b1b640CaptionSeparator);
	strcat(captionText, g_b1b640Captions[captionIndex].secondLine);
	xfade_AddTimedText(captionText, B1B640_CAPTION_START, B1B640_CAPTION_END, B1B640_CAPTION_FONT,
					   B1B640_CAPTION_X, B1B640_CAPTION_Y, captionColor);
	resourceFile = xres_Open_Resource("b1b_640.lfd");
	xrect_Set_Rect(&frame, 0, 0, B1B640_BACKGROUND_WIDTH, B1B640_BACKGROUND_HEIGHT);
	g_b1b640BackgroundHandle = xmemhdl_Alloc_Clear_Handle(B1B640_BACKGROUND_WIDTH * B1B640_BACKGROUND_HEIGHT,
														  LANDRU_MEMORY_RESOURCE);
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_INTRO_FORMATION:
			g_b1b640Film =
				xfilm_Res_Callback_Film(resourceFile, "formup", &frame, 0, 0, 0, B1b640_film_Callback);
			break;
		case XW_SCENE_TRAINING_RESULTS_AWING:
			g_b1b640Film =
				xfilm_Res_Callback_Film(resourceFile, "tenda_f", &frame, 0, 0, 0, B1b640_film_Callback);
			break;
		case XW_SCENE_TRAINING_RESULTS_XWING:
			g_b1b640Film =
				xfilm_Res_Callback_Film(resourceFile, "tendx_f", &frame, 0, 0, 0, B1b640_film_Callback);
			break;
		case XW_SCENE_TRAINING_RESULTS_YWING:
			g_b1b640Film =
				xfilm_Res_Callback_Film(resourceFile, "tendy_f", &frame, 0, 0, 0, B1b640_film_Callback);
			break;
		case XW_SCENE_TRAINING_RESULTS_BWING:
			g_b1b640Film =
				xfilm_Res_Callback_Film(resourceFile, "tendb_f", &frame, 0, 0, 0, B1b640_film_Callback);
			break;
	}
	g_b1b640CloseActor = xactcust_Alloc_Custom_Actor(LANDRU_NULL_HANDLE, &frame, 0, 0, B1B640_CLOSE_Z);
	xactor_Set_Actor_User_Function(g_b1b640CloseActor, B1b640_user_Close);
	xactor_Set_Actor_Draw_Function(g_b1b640CloseActor, XwCutscene_DrawClose);
	xfilm_Set_Film_Def_Palette(g_b1b640Film, shell->standardPalette);
	xview_Set_View_Update_Function(B1b640_end_View);
	if ((uint16_t)xcursor_Is_Cursor_Visible() != 0)
		xcursor_Hide_Cursor();
	B1b640_OpenMusic(resourceFile, g_b1b640Film);
	B1b640_OpenSounds(resourceFile, g_b1b640Film, g_b1b640CaptionVariant);
	if (shellext_Get_Cur_Scene() != XW_SCENE_INTRO_FORMATION)
		FrontendAudio_PlayFile(g_b1b640DigitalMusicPath, 0);
#ifdef XW_MODERN
	XwB1b640_RunView(resourceFile);
#else
	j_xviewadd_Handle_View();
	B1b640_CloseMusic();
	soundext_RecheckSfxPreference();
	xview_Clear_View_Update_Function();
	xview_Enable_All_View_Erase();
	xcanvas_Get_Drawing_Canvas_Bounds(&frame);
	xview_Set_View_Frame(0, &frame);
	xview_Set_View_Pos(0, frame.left, frame.top);
	xmemhdl_Free_Handle(g_b1b640BackgroundHandle);
	xres_Close_Resource(resourceFile);
	LandruDisplay_ForwardLegacyNoOp(0);
	return xerror_Get_Landru_Exit();
#endif
}

// FUNCTION: XW 0x432DC0
void B1b640_end_View(int unusedTime) {
	int16_t exitScene;
	int16_t nextScene;
	int16_t nextSection;
	(void)unusedTime;
	switch (shellext_Get_Cur_Scene()) {
		case XW_SCENE_INTRO_FORMATION:
			nextScene = XW_SCENE_INTRO_OSSHT7;
			nextSection = XW_SCENE_REGISTER_INITIAL;
			break;
		case XW_SCENE_TRAINING_RESULTS_AWING:
			nextScene = XW_SCENE_TRAINING_RETURN_AWING;
			nextSection = XW_SCENE_DEBRIEF_PROVING_GROUNDS;
			break;
		case XW_SCENE_TRAINING_RESULTS_XWING:
			nextScene = XW_SCENE_TRAINING_RETURN_XWING;
			nextSection = XW_SCENE_DEBRIEF_PROVING_GROUNDS;
			break;
		case XW_SCENE_TRAINING_RESULTS_YWING:
			nextScene = XW_SCENE_TRAINING_RETURN_YWING;
			nextSection = XW_SCENE_DEBRIEF_PROVING_GROUNDS;
			break;
		case XW_SCENE_TRAINING_RESULTS_BWING:
			nextScene = XW_SCENE_TRAINING_RETURN_BWING;
			nextSection = XW_SCENE_DEBRIEF_PROVING_GROUNDS;
			break;
#ifdef XW_MODERN
		default:
			return;
#endif
	}
	if (shellext_Check_Scene_Exit(&exitScene, nextScene, nextSection,
								  g_b1b640Film->cur_cel == g_b1b640Film->cels) != 0) {
		xerror_Set_Landru_Exit(exitScene);
	}
}

// FUNCTION: XW 0x432E70
int16_t B1b640_film_Callback(Film* film, FilmObject* object) {
	int16_t consumeObject = 0;
	Actor* actor;
	if (object->id == FTC_ACTOR) {
		xfilm_Rewind_Actor_Film(film, object, object + 1);
		actor = object->object;
		switch (actor->var1) {
			case B1B640_ACTOR_ROLE_JITTER:
				xactor_Set_Actor_User_Function(actor, B1b640_user_Jitter);
				xactor_Set_Actor_Draw_Function(actor, B1b640_draw_Jitter);
				break;
			case CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND:
				B1b640_film_Actor_To_Background(actor);
				if (actor->var2 == B1B640_BACKGROUND_TILED) {
					xactor_Set_Actor_Draw_Function(actor, B1b640_draw_Background);
				}
				consumeObject = actor->var2 == B1B640_BACKGROUND_CONSUME;
				break;
			case CUTSCENE_ACTOR_ROLE_SOUND:
				xactor_Set_Actor_User_Function(actor, B1b640_user_Sound);
				break;
		}
	}
	return consumeObject;
}

// FUNCTION: XW 0x432F10
int16_t B1b640_film_Actor_To_Background(Actor* actor) {
	Rect previousClip;
	Rect canvasBounds;
	Rect actorClip;
	uint8_t* previousPixels;
	int16_t previousWidth;
	int16_t previousHeight;
	uint8_t* backgroundPixels;
	int16_t drawResult = 0;
	xcanvas_Get_Drawing_Canvas_Bounds(&canvasBounds);
	backgroundPixels = xmemhdl_Lock_Handle(g_b1b640BackgroundHandle);
	xcanvas_Push_Canvas(&previousPixels, backgroundPixels, &previousClip, &previousWidth, &previousHeight,
						B1B640_BACKGROUND_WIDTH, B1B640_BACKGROUND_HEIGHT, 0);
	if (actor->draw != NULL) {
		xrect_Copy_Rect(&actorClip, &actor->frame);
		xcanvas_Clip_Rect_To_Canvas(&actorClip);
		xcanvas_Set_Drawing_Canvas_Clip(&actorClip);
		drawResult = actor->draw(actor, &canvasBounds, &actorClip, actor->x, actor->y, 1);
		xcanvas_Max_Drawing_Canvas_Clip();
	}
	xcanvas_Pop_Canvas(previousPixels, &previousClip, previousWidth, previousHeight);
	xmemhdl_Unlock_Handle(g_b1b640BackgroundHandle);
	return drawResult;
}

// FUNCTION: XW 0x432FF0
void B1b640_user_Sound(Actor* actor, int unusedTime) {
	int16_t cue = actor->var2;
	(void)unusedTime;
	if (cue != 0) {
		B1b640_PlaySoundCue(cue);
	}
}

// FUNCTION: XW 0x433010
int16_t B1b640_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t x, int16_t y,
							   int16_t refresh) {
	Rect sourceRect;
	int16_t baseX;
	int16_t baseY;
	int16_t tileX;
	int16_t tileY;
	const uint8_t* backgroundPixels;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	if (refresh == 0) {
		return 0;
	}
	baseX = x;
	baseY = y;
	if (x >= 0) {
		baseX -= B1B640_TILE_STEP_X;
	}
	if (y >= 0) {
		baseY -= B1B640_TILE_STEP_Y;
	}
	xrect_Set_Rect(&sourceRect, 0, 0, B1B640_BACKGROUND_WIDTH, B1B640_BACKGROUND_HEIGHT);
	backgroundPixels = (const uint8_t*)xmemhdl_Lock_Handle(g_b1b640BackgroundHandle);
	for (tileY = 0; tileY < B1B640_TILE_ROWS * B1B640_TILE_STEP_Y; tileY += B1B640_TILE_STEP_Y) {
		for (tileX = 0; tileX < B1B640_BACKGROUND_WIDTH; tileX += B1B640_TILE_STEP_X) {
			stub_Copy_From_Clipped_Buffer(backgroundPixels, &sourceRect, tileX + baseX, tileY + baseY,
										  B1B640_BACKGROUND_WIDTH, B1B640_BACKGROUND_HEIGHT);
		}
	}
	xmemhdl_Unlock_Handle(g_b1b640BackgroundHandle);
	return 1;
}

// FUNCTION: XW 0x4330F0
void B1b640_user_Close(Actor* actor, int unusedTime) {
	(void)unusedTime;
	if (g_b1b640Film->cur_cel == g_b1b640Film->cels) {
		actor->var1 = 1;
	} else {
		actor->var1 = 0;
	}
}

// FUNCTION: XW 0x433120
void B1b640_user_Jitter(Actor* actor, int time) {
	if (actor->var2 != 0 && ((actor->id + (unsigned int)time) & 1) != 0) {
		actor->id = rand() & (XW_B1B640_JITTER_COUNT - 1);
	} else {
		actor->id = 0;
	}
}

// FUNCTION: XW 0x433150
int16_t B1b640_draw_Jitter(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	int16_t jitteredX = g_b1b640JitterX[actor->id] + x;
	int16_t jitteredY = g_b1b640JitterY[actor->id] + y;

	if (actor->res_type == FOURCC_DELT) {
		xactdelt_Draw_Delta_Actor(actor, frame, clip, jitteredX, jitteredY, refresh);
	} else {
		xactanim_Draw_Anim_Actor(actor, frame, clip, jitteredX, jitteredY, refresh);
	}
	return 1;
}
