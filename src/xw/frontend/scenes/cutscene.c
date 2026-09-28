#include "xw/frontend/scenes/cutscene.h"

#include "xw/audio/soundext.h"
#include "xw/frontend/shell_preferences.h"
#include "xw/landru_config.h"

#include <landru/canvas.h>
#include <landru/view.h>

#include <stdlib.h>

// GLOBAL: XW 0x4D02C0
const char* g_troFightMusicResourceFilename = "bl1music.lfd";

// GLOBAL: XW 0x4D02C4
const char* g_troFightMusicName = "trofight";

// GLOBAL: XW 0x4D02C8
const char* g_hangarMusicName = "hangar";

// GLOBAL: XW 0x4F4B9C
Sound* g_troFightMusic = NULL;

// GLOBAL: XW 0x4F4BA0
Film* g_troFightMusicFilm = NULL;

// FUNCTION: XW 0x434B20
int16_t Cutscene_DrawConditionalErase(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	if (actor->var1 != 0) {
		xcanvas_Erase_Canvas();
	}
	/* Shared Landru ignores the draw callback result. */
	return 0;
}

// FUNCTION: XW 0x434CD0
void Cutscene_EnsureTroFightMusic(ResFile* unusedResourceFile, Film* film) {
	(void)unusedResourceFile;
	if (ShellPreferences_GetMusicEnabled() != 0) {
		g_troFightMusicFilm = film;
		g_troFightMusic = xsound_Find_Gmid(g_hangarMusicName);
		if (g_troFightMusic != NULL) {
			xsound_Set_Sound_Keep(g_troFightMusic);
		}
		g_troFightMusic = xsound_Find_Gmid(g_troFightMusicName);
		if (g_troFightMusic == NULL) {
			ResFile* musicResourceFile = xres_Open_Resource(g_troFightMusicResourceFilename);
			g_troFightMusic = xsound_Res_Music(musicResourceFile, g_troFightMusicName);
			xres_Close_Resource(musicResourceFile);
			soundext_Start_Resource_Sound(g_troFightMusic);
		}
		xsound_Set_Sound_Keep(g_troFightMusic);
		xsound_Set_Sound_User_Function(g_troFightMusic, Cutscene_IgnoreSoundEvent);
	}
}

// FUNCTION: XW 0x437390
void Cutscene_DrawClose(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	(void)refresh;
	if (actor->var1 != 0) {
		xcanvas_Erase_Canvas();
	}
}

// FUNCTION: XW 0x455050
int16_t Cutscene_DrawEraseViewClip(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								   int16_t unusedY, int16_t refresh) {
	Rect viewClip;
	(void)unusedActor;
	(void)unusedFrame;
	(void)unusedClip;
	(void)unusedX;
	(void)unusedY;
	if (refresh == 0) {
		return 0;
	}
	xview_Get_View_Clip_Frame(CUTSCENE_MAIN_VIEW, &viewClip);
	xcanvas_Erase_Canvas_Rect(&viewClip);
	return 1;
}

// FUNCTION: XW 0x458D00
void Cutscene_IgnoreSoundEvent(Sound* unusedSound, int32_t unusedTime) {
	(void)unusedSound;
	(void)unusedTime;
}

// FUNCTION: XW 0x46A710
int Cutscene_DrawCloseOnRefresh(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
								int16_t refresh) {
	(void)frame;
	(void)clip;
	(void)x;
	(void)y;
	if (refresh == 0) {
		return 0;
	}
	if (actor->var1 != 0) {
		xcanvas_Erase_Canvas();
	}
	return 1;
}
