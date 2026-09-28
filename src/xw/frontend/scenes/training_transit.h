#ifndef XW_FRONTEND_SCENES_TRAINING_TRANSIT_H
#define XW_FRONTEND_SCENES_TRAINING_TRANSIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <landru/rect.h>
#include <landru/res.h>
#include <landru/sound.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	TRAINING_TRANSIT_CAPTION_FACILITY_END = 40,
	TRAINING_TRANSIT_BACKGROUND_Z = 200,
	TRAINING_TRANSIT_CLOSE_Z = -200,
	TRAINING_TRANSIT_SAVE_Z = 1,
	TRAINING_TRANSIT_BLACK_Z = 100,
	TRAINING_TRANSIT_SAVE_BUFFER_CAPACITY = 200000
};

extern Actor* g_trainingTransitBlackActor;
extern Actor* g_trainingTransitBackgroundActor;
extern Actor* g_trainingTransitCloseActor;

enum {
	TRAINING_TRANSIT_MUSIC_START_CEL = 10,
	TRAINING_TRANSIT_MUSIC_INDEPENDENCE_START_CEL = 20,
	TRAINING_TRANSIT_MUSIC_PREVIOUS_FADE_CEL = 12,
	TRAINING_TRANSIT_MUSIC_PARAM4_VALUE = 120,
	TRAINING_TRANSIT_MUSIC_PREVIOUS_FADE_DURATION = 480,
	TRAINING_TRANSIT_MARCH_CLOSE_DURATION = 180,
	TRAINING_TRANSIT_PLANS_CLOSE_DURATION = 300
};

enum {
	TRAINING_TRANSIT_MARCH_BEAT = 32,
	TRAINING_TRANSIT_PLANS_BEAT = 11,
	TRAINING_TRANSIT_LAUNCH_BEAT = 30
};

enum { TRAINING_TRANSIT_HYPERSPACE_RATE_CHANGE_TIME = 34 };

enum { TRAINING_TRANSIT_BACKGROUND_WIDTH = 640, TRAINING_TRANSIT_BACKGROUND_HEIGHT = 480 };

enum {
	TRAINING_TRANSIT_CAPTION_FONT = 3,
	TRAINING_TRANSIT_CAPTION_START = 4,
	TRAINING_TRANSIT_CAPTION_DEPARTURE_END = 30,
	TRAINING_TRANSIT_CAPTION_TRANSIT_END = 50,
	TRAINING_TRANSIT_CAPTION_Y = 450,
	TRAINING_TRANSIT_CAPTION_DEPARTURE_COLOR = 93,
	TRAINING_TRANSIT_CAPTION_HYPERSPACE_COLOR = 107,
	TRAINING_TRANSIT_CAPTION_TRAINING_COLOR = 26,
	TRAINING_TRANSIT_CAPTION_COMBAT_COLOR = 29
};

enum { TRAINING_TRANSIT_ACTOR_ROLE_NON_REFRESHABLE = 20 };

enum {
	TRAINING_TRANSIT_SOUND_CUE_1 = 1,
	TRAINING_TRANSIT_SOUND_CUE_2 = 2,
	TRAINING_TRANSIT_SOUND_CUE_3 = 3,
	TRAINING_TRANSIT_SOUND_CUE_4 = 4,
	TRAINING_TRANSIT_SOUND_CUE_5 = 5
};

typedef struct XwTrainingTransitMusicState XwTrainingTransitMusicState;

/* Original IDB size: 16 bytes. */
struct XwTrainingTransitMusicState {
	/* IDB +0x0: Plans/swirls delayed-start sound, or launch started by ensure helper; reset at enabled open.
	 */
	Sound* sceneSound;
	/* IDB +0x4: Previous named sound retained for optional cel-12 fade. */
	Sound* previousSound;
	/* IDB +0x8: Film used to time audio callback. */
	Film* film;
	/* IDB +0xC: Set for scenes 43/53/80; no clear writer found. */
	int fadePreviousAtCel12;
};

extern Actor* g_trainingTransitBackgroundSaveActor;
extern Rect g_trainingTransitPreviousDirtyRect;
extern Film* g_trainingTransitFilm;
extern Rect g_trainingTransitCurrentDirtyRect;
extern LandruHandle g_trainingTransitBackgroundHandle;
extern XwTrainingTransitMusicState g_trainingTransitMusicState;
extern int16_t g_trainingTransitUseTransitSfx;

/* Declarations follow ascending original IDB address. */

/* 0x463350 */
XwShellSceneResult TrainingTransit_PlayDeparture(struct XwShellContext* shell);

/* 0x463660 */
XwShellSceneResult TrainingTransit_PlayFacilityFlight(struct XwShellContext* shell);

/* 0x463C20 */
void TrainingTransit_end_View(int time);

/* 0x463EF0 */
int16_t TrainingTransit_film_FacilityCallback(Film* film, FilmObject* object);

/* 0x463F40 */
int16_t TrainingTransit_film_DepartureCallback(Film* film, FilmObject* object);

/* 0x463F80 */
void TrainingTransit_user_SoundCue(Actor* actor, int unusedTime);

/* 0x463FA0 */
void TrainingTransit_user_SaveRegion(Actor* actor, int unusedTime);

/* 0x464010 */
void TrainingTransit_user_Background(Actor* unusedActor, int time);

/* 0x464060 */
int16_t TrainingTransit_draw_Background(Actor* unusedActor, Rect* unusedFrame, Rect* unusedClip,
										int16_t unusedX, int16_t unusedY, int16_t refresh);

/* 0x4640C0 */
void TrainingTransit_user_Close(Actor* actor, int unusedTime);

/* 0x4680C0 */
void TrainingTransit_OpenMusic(ResFile* unusedResourceFile, Film* sceneFilm);

/* 0x4683F0 */
Sound* TrainingTransit_EnsureMusic(const char* name, const char* filename, unsigned int beatIndex);

/* 0x468480 */
void TrainingTransit_CloseMusic(void);

/* 0x468660 */
void TrainingTransit_user_Music(Sound* unusedSound, int unusedTime);

/* 0x4686D0 */
void TrainingTransit_LoadSoundEffects(ResFile* unusedResourceFile, Film* unusedFilm);

/* 0x468750 */
void TrainingTransit_PlaySoundCue(int16_t cue);

#ifdef __cplusplus
}
#endif

#endif
