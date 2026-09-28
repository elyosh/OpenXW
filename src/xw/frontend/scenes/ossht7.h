#ifndef XW_FRONTEND_SCENES_OSSHT7_H
#define XW_FRONTEND_SCENES_OSSHT7_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xw/landru_config.h"

#include <landru/actor.h>
#include <landru/film.h>
#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

struct XwShellContext;

enum {
	OSSHT7_SCENE_WIDTH = 640,
	OSSHT7_SCENE_HEIGHT = 480,
	OSSHT7_BUFFER_BYTES = OSSHT7_SCENE_WIDTH * OSSHT7_SCENE_HEIGHT,
	OSSHT7_CLOSE_Z = -200
};

extern const char* g_ossht7ResourceFilename;
extern const char* g_ossht7FilmName;
extern Actor* g_ossht7CloseActor;
extern LandruHandle g_ossht7BufferHandle;
extern Film* g_ossht7Film;
/* Declarations follow ascending original IDB address. */

/* 0x432860 */
XwShellSceneResult Ossht7_Play(struct XwShellContext* context);

/* 0x4329D0 */
void Ossht7_end_View(int time);

/* 0x432A30 */
void Ossht7_user_Close(Actor* actor, int time);

#ifdef __cplusplus
}
#endif

#endif
