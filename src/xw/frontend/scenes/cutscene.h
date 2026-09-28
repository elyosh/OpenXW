#ifndef XW_FRONTEND_SCENES_CUTSCENE_H
#define XW_FRONTEND_SCENES_CUTSCENE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/rect.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>

enum { CUTSCENE_MAIN_VIEW = 0 };

enum {
	CUTSCENE_ACTOR_ROLE_DIRTY_BOUNDS = 10,
	CUTSCENE_ACTOR_ROLE_STAMP_BACKGROUND = 20,
	CUTSCENE_ACTOR_ROLE_SOUND = 50
};

typedef struct XwCutsceneMusicTransitionState XwCutsceneMusicTransitionState;
typedef struct XwCutsceneSpeechState XwCutsceneSpeechState;
typedef struct XwSceneMusicHandles XwSceneMusicHandles;

/* Original IDB size: 12 bytes. */
struct XwCutsceneMusicTransitionState {
	/* IDB +0x0: Kept active resource with music callback. */
	Sound* activeSound;
	/* IDB +0x4: Resource used for transition/jump; may alias activeSound or retain prior value. */
	Sound* transitionSound;
	/* IDB +0x8: Current film clock for music cues. */
	Film* film;
};

/* Original IDB size: 16 bytes. */
struct XwCutsceneSpeechState {
	/* IDB +0x0: Three scene speech resources; loader assigns var1 slot IDs0..2. */
	Sound* speech[3];
	/* IDB +0xC: Film current cel supplies speech timing independently of callback time. */
	Film* film;
};

/* Original IDB size: 8 bytes. */
struct XwSceneMusicHandles {
	/* IDB +0x0: Scene music resource, stored from XSOUND_Find_Gmid/XSOUND_Res_Music and passed to sound
	 * control calls. */
	Sound* sound;
	/* IDB +0x4: Associated scene film, stored on open; read by timed callbacks such as BINTRO_user_Music.
	 * Award-box code stores it without a known reader. */
	Film* film;
};

extern const char* g_troFightMusicResourceFilename;
extern const char* g_troFightMusicName;
extern const char* g_hangarMusicName;
extern Sound* g_troFightMusic;
extern Film* g_troFightMusicFilm;

/* Declarations follow ascending original IDB address. */

/* 0x434B20 */
int16_t Cutscene_DrawConditionalErase(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y,
									  int16_t refresh);

/* 0x434CD0 */
void Cutscene_EnsureTroFightMusic(ResFile* unusedResourceFile, Film* film);

/* 0x437390 */
void Cutscene_DrawClose(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

/* 0x455050 */
int16_t Cutscene_DrawEraseViewClip(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip, int16_t unusedX,
								   int16_t unusedY, int16_t refresh);

/* 0x458D00 */
void Cutscene_IgnoreSoundEvent(Sound* unusedSound, int32_t unusedTime);

/* 0x46A710 */
int Cutscene_DrawCloseOnRefresh(Actor* actor, Rect* frame, Rect* clip, int16_t x, int16_t y, int16_t refresh);

#ifdef __cplusplus
}
#endif

#endif
