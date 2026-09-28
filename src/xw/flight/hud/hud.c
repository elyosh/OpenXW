#include "xw/flight/hud/hud.h"

#include "xw/flight/feinput.h"
#include "xw/flight/flight_display.h"
#include "xw/flight/hud/festring.h"
#include "xw/math/math.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"
#include "xw/render/rtsrgb.h"
#include "xw/render/rtsvga2.h"
#include "xw/render/std3d.h"
#include "xw/render/sw3d.h"

#include <string.h>

// GLOBAL: XW 0x561378
uint16_t g_panelBoxSpanScratch[HUD_BOX_SCRATCH_PIXEL_COUNT] = { 0 };

// FUNCTION: XW 0x483280
void Hud_DrawBoxOverlayHW(int x, int y, int width, int height, int colorIdx, int depth) {
	if (depth == HUD_BOX_NEAR_DEPTH && width == HUD_BOX_DOT_SIZE && height == HUD_BOX_DOT_SIZE) {
		uint8_t savedBackground;
		uint16_t screenX, screenY;
		FlightDisplay_LockSurface();
		savedBackground = g_flightTextBgColor;
		g_flightTextBgColor = colorIdx;
		screenX = g_flightVpX + x;
		screenY = g_flightVpY + y;
		festring_setbound(g_flightVpX, g_flightVpY, g_flightVpX + g_flightVpWidth,
						  g_flightVpY + g_flightVpHeight);
		g_flightFillRectClippedFn(screenX, screenY, screenX + HUD_BOX_DOT_SIZE, screenY + HUD_BOX_DOT_SIZE);
		g_flightTextBgColor = savedBackground;
		FlightDisplay_UnlockSurface();
	} else {
		int right, bottom, cornerWidth, cornerHeight;
		uint32_t color;
		float normalizedDepth;
		if (g_d3dVertexCount + HUD_BOX_VERTEX_BUDGET > g_maxBatchVerts ||
			(int)(g_d3dIndexCount + HUD_BOX_TRIANGLE_BUDGET) > g_maxBatchTris) {
			Math_SetFpuExtendedPrecisionMode();
			if (!g_isPowerVr)
				std3D_StartScene();
			std3D_LockExecuteBuffer();
			std3D_AddVertices(g_flightVertexBuffer, g_d3dVertexCount);
			std3D_BeginInstructions();
			std3D_AddTriangles(g_triBuffer, g_d3dIndexCount);
			std3D_ExecuteBuffer();
			if (!g_isPowerVr)
				std3D_EndScene();
			Math_SetFpuSinglePrecisionMode();
			g_d3dIndexCount = 0;
			g_d3dVertexCount = 0;
		}
		right = x + width;
		bottom = y + height;
		cornerWidth = width >> HUD_BOX_CORNER_SHIFT;
		color = (int)g_swPalette[colorIdx].r - HUD_BOX_PALETTE_ALPHA_BIAS;
		color = (color << HUD_BOX_COLOR_CHANNEL_SHIFT) + g_swPalette[colorIdx].g;
		color = ((color << HUD_BOX_COLOR_CHANNEL_SHIFT) + g_swPalette[colorIdx].b)
				<< HUD_BOX_RGB6_EXPAND_SHIFT;
		cornerHeight = height >> HUD_BOX_CORNER_SHIFT;
		if (cornerWidth < HUD_BOX_MIN_CORNER_LENGTH)
			cornerWidth = HUD_BOX_MIN_CORNER_LENGTH;
		if (cornerHeight < HUD_BOX_MIN_CORNER_LENGTH)
			cornerHeight = HUD_BOX_MIN_CORNER_LENGTH;
		if (cornerWidth > width)
			cornerWidth = width;
		if (cornerHeight > height)
			cornerHeight = height;
		if (depth < HUD_BOX_NEAR_DEPTH)
			depth = HUD_BOX_NEAR_DEPTH;
		normalizedDepth = 1.0f / (depth * g_hardwareDepthReciprocalScale + 1.0f);
		if (g_std3DZCmpMask == STD3D_ZCMP_LESS)
			normalizedDepth = 1.0f - normalizedDepth;
		if (y >= 0 && y < g_flightVpHeight) {
			int left = x;
			int end = x + cornerWidth;
			if (left < 0)
				left = 0;
			if (end >= g_flightVpWidth)
				end = g_flightVpWidth - 1;
			if (left < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(y);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(y);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(y + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(y + 1);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
			left = right - cornerWidth;
			end = right;
			if (left < 0)
				left = 0;
			if (end >= g_flightVpWidth)
				end = g_flightVpWidth - 1;
			if (left < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(y);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(y);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(y + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(y + 1);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
		}
		if (bottom >= 0 && bottom < g_flightVpHeight) {
			int left = x;
			int end = x + cornerWidth;
			if (left < 0)
				left = 0;
			if (end >= g_flightVpWidth)
				end = g_flightVpWidth - 1;
			if (left < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(bottom);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(bottom);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(bottom + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(bottom + 1);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
			left = right - cornerWidth;
			end = right + 1;
			if (left < 0)
				left = 0;
			if (end >= g_flightVpWidth)
				end = g_flightVpWidth - 1;
			if (left < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(bottom);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(bottom);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(bottom + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(left);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(bottom + 1);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
		}
		if (x >= 0 && x < g_flightVpWidth) {
			int top = y;
			int end = y + cornerHeight;
			if (top < 0)
				top = 0;
			if (end >= g_flightVpHeight)
				end = g_flightVpHeight - 1;
			if (top < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(x);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(top);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(x);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(x + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(x + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(top);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
			top = bottom - cornerHeight;
			end = bottom;
			if (top < 0)
				top = 0;
			if (end >= g_flightVpHeight)
				end = g_flightVpHeight - 1;
			if (top < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(x);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(top);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(x);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(x + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(x + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(top);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
		}
		if (right >= 0 && right < g_flightVpWidth) {
			int top = y;
			int end = y + cornerHeight;
			if (top < 0)
				top = 0;
			if (end >= g_flightVpHeight)
				end = g_flightVpHeight - 1;
			if (top < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(right);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(top);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(right);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(right + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(right + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(top);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
			top = bottom - cornerHeight;
			end = bottom;
			if (top < 0)
				top = 0;
			if (end >= g_flightVpHeight)
				end = g_flightVpHeight - 1;
			if (top < end) {
				int vertexIndex;
				g_flightVertexBuffer[g_d3dVertexCount].sx = g_flightVpOriginX + (double)(right);
				g_flightVertexBuffer[g_d3dVertexCount].sy = g_flightVpOriginY + (double)(top);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sx = g_flightVpOriginX + (double)(right);
				g_flightVertexBuffer[g_d3dVertexCount + 1].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sx = g_flightVpOriginX + (double)(right + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 2].sy = g_flightVpOriginY + (double)(end);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sx = g_flightVpOriginX + (double)(right + 1);
				g_flightVertexBuffer[g_d3dVertexCount + 3].sy = g_flightVpOriginY + (double)(top);
				for (vertexIndex = 0; vertexIndex < HUD_BOX_SEGMENT_VERTEX_COUNT; ++vertexIndex) {
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].sz = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].rhw = normalizedDepth;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tu = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].tv = 0.0f;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].color = color;
					g_flightVertexBuffer[vertexIndex + g_d3dVertexCount].specular = 0;
				}
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 1;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_triBuffer[g_d3dIndexCount].v0 = g_d3dVertexCount;
				g_triBuffer[g_d3dIndexCount].v1 = g_d3dVertexCount + 2;
				g_triBuffer[g_d3dIndexCount].v2 = g_d3dVertexCount + 3;
				g_triBuffer[g_d3dIndexCount].texture = NULL;
				g_triBuffer[g_d3dIndexCount].flags =
					STD3D_RS_Z_TEST | STD3D_RS_Z_WRITE | STD3D_RS_DISABLE_MONO;
				++g_d3dIndexCount;
				g_d3dVertexCount += HUD_BOX_SEGMENT_VERTEX_COUNT;
			}
		}
	}
}

// FUNCTION: XW 0x49DB90
void Hud_DrawBoxInXTrans(int x, int y, int width, int height, int colorIndex, int viewDepth) {
	int bottom = height + y;
	int right;
	int cornerWidth, cornerHeight;
#ifdef XW_MODERN
	int fillWidth;
#endif
	int row;
	uint8_t* sourceSpan;
	float inverseDepth;
	if (bottom <= 0)
		return;
	right = width + x;
	if (right <= 0 || x >= g_flightVpWidth || y >= g_flightVpHeight || height <= 0 || width <= 0)
		return;
	if (g_useHardware3D != 0) {
		Hud_DrawBoxOverlayHW(x, y, width, height, colorIndex, viewDepth);
		return;
	}
	cornerWidth = width >> HUD_BOX_CORNER_SHIFT;
	cornerHeight = height >> HUD_BOX_CORNER_SHIFT;
	if (cornerWidth < HUD_BOX_MIN_CORNER_LENGTH)
		cornerWidth = HUD_BOX_MIN_CORNER_LENGTH;
	if (cornerHeight < HUD_BOX_MIN_CORNER_LENGTH)
		cornerHeight = HUD_BOX_MIN_CORNER_LENGTH;
	if (cornerWidth > width)
		cornerWidth = width;
	if (cornerHeight > height)
		cornerHeight = height;
#ifdef XW_MODERN
	fillWidth = cornerWidth;
	/* Clipped spans consume at most the visible width; keep the corner geometry intact. */
	if (fillWidth > g_flightVpWidth - (x > 0 ? x : 0))
		fillWidth = g_flightVpWidth - (x > 0 ? x : 0);
#endif
	sourceSpan = (uint8_t*)g_panelBoxSpanScratch;
	if (g_flightBytesPerPixel == FLIGHT_DISPLAY_DIRECT_BYTES_PER_PIXEL) {
		uint16_t color;
		if (x > 0)
			sourceSpan = (uint8_t*)&g_panelBoxSpanScratch[x];
		color = g_flightTextPalette[(uint8_t)colorIndex];
		if (cornerWidth > 0) {
			uint16_t* pixels = (uint16_t*)sourceSpan;
			int pixelIndex;
#ifdef XW_MODERN
			for (pixelIndex = 0; pixelIndex < fillWidth; ++pixelIndex)
#else
			for (pixelIndex = 0; pixelIndex < cornerWidth; ++pixelIndex)
#endif
				pixels[pixelIndex] = color;
		}
	} else {
		if (x > 0)
			sourceSpan = (uint8_t*)g_panelBoxSpanScratch + x;
		if (cornerWidth > 0)
#ifdef XW_MODERN
			memset(sourceSpan, colorIndex, fillWidth);
#else
			memset(sourceSpan, colorIndex, cornerWidth);
#endif
	}
	if (viewDepth < HUD_BOX_NEAR_DEPTH)
		viewDepth = HUD_BOX_NEAR_DEPTH;
	inverseDepth = (double)(unsigned int)g_projScaleInt / viewDepth;
	if (g_flightSurfaceAlreadyLocked == 0)
		FlightDisplay_LockSurface();
	if (y >= 0) {
		int end = x + cornerWidth;
		if (end > 0 && x < g_flightVpWidth) {
			int start = x;
			if (start < 0)
				start = 0;
			if (end > g_flightVpWidth)
				end = g_flightVpWidth;
			sw3d_BlitOccludedSpan(sourceSpan, start, end, y, inverseDepth);
		}
		end = right;
		if (right > 0 && right - cornerWidth < g_flightVpWidth) {
			int start = right - cornerWidth;
			if (start < 0)
				start = 0;
			if (end > g_flightVpWidth)
				end = g_flightVpWidth;
			sw3d_BlitOccludedSpan(sourceSpan, start, end, y, inverseDepth);
		}
	}
	if (bottom <= g_flightVpHeight) {
		int end = x + cornerWidth;
		if (end > 0 && x < g_flightVpWidth) {
			int start = x;
			if (start < 0)
				start = 0;
			if (end > g_flightVpWidth)
				end = g_flightVpWidth;
			sw3d_BlitOccludedSpan(sourceSpan, start, end, bottom - 1, inverseDepth);
		}
		end = right;
		if (right > 0 && right - cornerWidth < g_flightVpWidth) {
			int start = right - cornerWidth;
			if (start < 0)
				start = 0;
			if (end > g_flightVpWidth)
				end = g_flightVpWidth;
			sw3d_BlitOccludedSpan(sourceSpan, start, end, bottom - 1, inverseDepth);
		}
	}
	for (row = 1; row < cornerHeight; ++row) {
		int scanY = y + row;
		if (scanY >= 0 && scanY < g_flightVpHeight) {
			if (x >= 0)
				sw3d_BlitOccludedSpan(sourceSpan, x, x + 1, scanY, inverseDepth);
			if (right <= g_flightVpWidth)
				sw3d_BlitOccludedSpan(sourceSpan, right - 1, right, scanY, inverseDepth);
		}
	}
	for (row = height - cornerHeight; row < height - 1; ++row) {
		int scanY = y + row;
		if (row >= cornerHeight && scanY >= 0 && scanY < g_flightVpHeight) {
			if (x >= 0)
				sw3d_BlitOccludedSpan(sourceSpan, x, x + 1, scanY, inverseDepth);
			if (right <= g_flightVpWidth)
				sw3d_BlitOccludedSpan(sourceSpan, right - 1, right, scanY, inverseDepth);
		}
	}
	if (g_flightSurfaceAlreadyLocked == 0)
		FlightDisplay_UnlockSurface();
}
