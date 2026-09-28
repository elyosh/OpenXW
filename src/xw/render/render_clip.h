#ifndef XW_RENDER_RENDER_CLIP_H
#define XW_RENDER_RENDER_CLIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct ProjVertex;

enum { RENDER_CLIP_INDEX_CAPACITY = 32, RENDER_CLIP_NEAR_DEPTH = 1 };

extern int g_clipIdxB[RENDER_CLIP_INDEX_CAPACITY];
extern int g_clipCountB;
extern int g_clipIdxA[RENDER_CLIP_INDEX_CAPACITY];
extern int g_clipCountA;
extern int32_t g_clipVertCursor;

/* Declarations follow ascending original IDB address. */

/* 0x4803A0 */
void RenderClip_ClipPolyTop(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices);

/* 0x4809B0 */
void RenderClip_ClipPolyBottom(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices);

/* 0x481010 */
void RenderClip_ClipPolyLeft(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices);

/* 0x481610 */
void RenderClip_ClipPolyRight(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices);

/* 0x481C70 */
void RenderClip_ClipPolyNear(int prevVertIndex, int curVertIndex, struct ProjVertex* vertices);

#ifdef __cplusplus
}
#endif

#endif
