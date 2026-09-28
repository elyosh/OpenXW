#include "xw/render/render_quad.h"
#ifdef XW_MODERN
#include "aeron/compat/host.h"
#endif

#include "xw/assets/bitmap.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/object.h"
#include "xw/flight/xw.h"
#include "xw/math/math.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_clip.h"
#include "xw/render/render_scene.h"
#include "xw/render/render_texture.h"
#include "xw/render/renderer.h"
#include "xw/util/shared.h"

// GLOBAL: XW 0x4C32D0
const float g_hardwareDistantBillboardDepth = 0.00012205541133880615f;
// GLOBAL: XW 0x4C32D4
const float g_hardwareDistantBillboardReversedDepth = 0.9998779296875f;
// GLOBAL: XW 0x4D9DE0
unsigned int g_billboardGenus13FrameColors[RENDER_QUAD_COLOR_COUNT] = {
	0xFFFFFFFF, 0xE0FFFFFF, 0xF0FFFFFF, 0xF0FFFFFF, 0xE0FFFFFF, 0xD0FFFFFF, 0xB0FFFFFF, 0x90FFFFFF,
	0x70FFFFFF, 0x50FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF,
	0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF,
	0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF, 0x30FFFFFF
};
// GLOBAL: XW 0x4DECE8
int g_bilinearEnabled = 1;

// FUNCTION: XW 0x482AB0
void RenderQuad_DrawRotatedSprite(unsigned int angle, int screenCenterX, int screenCenterY, uint16_t scaleQ8,
								  struct XwBitmapFramePrefix* frame) {
	ProjVertex clipVertices[RENDER_QUAD_VERTEX_CAPACITY];
	unsigned int billboardColor;
	double normalizedDepth, widthAsDouble;
	float depth, maximumU, maximumV, heightAsFloat;
	int width, height, textureWidth, textureHeight, i;
	int halfWidth, halfHeight, rotatedX, cosineY, sineY;
#ifdef XW_MODERN
	/* Suppress before texture lookup so hidden classic draws do not refill the cache. */
	if (AeronDx5_IsClassicFlightRenderingSuppressed())
		return;
#endif
	screenCenterY = g_flightVpHeight - screenCenterY;
#ifdef XW_MODERN
	billboardColor = 0xFFFFFFFFu;
#endif
	if (g_billboardObjectOrTypeIndex < g_mainObjectSlotEnd) {
		if (g_objectTable[g_billboardObjectOrTypeIndex].genusId == XW_GENUS_EXPLOSION_EFFECT)
			billboardColor =
				g_billboardGenus13FrameColors[g_objectTable[g_billboardObjectOrTypeIndex].animationState &
											  (RENDER_QUAD_COLOR_COUNT - 1)];
		else
			billboardColor = 0xFFFFFFFFu;
	}
	if ((unsigned int)g_objectViewZ > RENDER_QUAD_DISTANT_DEPTH_THRESHOLD) {
		billboardColor = 0xFFFFFFFFu;
		normalizedDepth = g_hardwareDistantBillboardDepth;
		if (g_std3DZCmpMask == STD3D_ZCMP_LESS)
			normalizedDepth = g_hardwareDistantBillboardReversedDepth;
	} else {
		normalizedDepth = 1.0f / (g_objectViewZ * g_hardwareDepthReciprocalScale + 1.0f);
		if (g_std3DZCmpMask == STD3D_ZCMP_LESS)
			normalizedDepth = 1.0f - normalizedDepth;
	}
	depth = (float)normalizedDepth;
	width = frame->width;
	height = frame->height;
	if (width > STD3D_DEVICE_MAX_TEXTURE_SIZE) {
		nullsub_SharedNoOp();
		width = STD3D_DEVICE_MAX_TEXTURE_SIZE;
	}
	if (height > STD3D_DEVICE_MAX_TEXTURE_SIZE) {
		nullsub_SharedNoOp();
		height = STD3D_DEVICE_MAX_TEXTURE_SIZE;
	}
	widthAsDouble = (double)width;
	heightAsFloat = (float)height;
	textureWidth = 1;
	for (i = 0; i < RENDER_QUAD_TEXTURE_SIZE_BITS; ++i) {
		textureWidth *= 2;
		if (width <= textureWidth)
			break;
	}
	textureHeight = 1;
	for (i = 0; i < RENDER_QUAD_TEXTURE_SIZE_BITS; ++i) {
		textureHeight *= 2;
		if (height <= textureHeight)
			break;
	}
	if (g_pStd3DCurDevice->caps.bSquareOnlyTexture) {
		if (textureWidth > textureHeight)
			textureHeight = textureWidth;
		if (textureWidth < textureHeight)
			textureWidth = textureHeight;
	}
	maximumU = (float)(widthAsDouble / (double)textureWidth);
	maximumV = (float)(heightAsFloat / (double)textureHeight);
	halfWidth = (scaleQ8 * frame->width) >> RENDER_QUAD_HALF_EXTENT_SHIFT;
	halfHeight = (scaleQ8 * frame->height) >> RENDER_QUAD_HALF_EXTENT_SHIFT;
	rotatedX = trig2_cosinedwordmult(halfWidth, angle);
	rotatedX += trig2_sinedwordmult(halfHeight, angle);
	cosineY = trig2_cosinedwordmult(halfHeight, angle);
	sineY = trig2_sinedwordmult(halfWidth, angle);
	g_clipCountA = RENDER_QUAD_VERTEX_COUNT;
	g_clipVertCursor = RENDER_QUAD_VERTEX_COUNT;
	halfWidth = -halfWidth;
	clipVertices[0].screenX = (float)(screenCenterX + rotatedX);
	g_clipIdxA[0] = 0;
	g_clipIdxA[1] = 1;
	g_clipIdxA[2] = 2;
	clipVertices[0].screenY = (float)(screenCenterY + cosineY - sineY);
	g_clipIdxA[3] = 3;
	clipVertices[0].lightIntensity = 0.0f;
	clipVertices[0].u = 0.0f;
	clipVertices[0].v = 0.0f;
	clipVertices[0].depth = depth;
	rotatedX = trig2_cosinedwordmult(halfWidth, angle);
	rotatedX += trig2_sinedwordmult(halfHeight, angle);
	cosineY = trig2_cosinedwordmult(halfHeight, angle);
	clipVertices[1].screenX = (float)(screenCenterX + rotatedX);
	clipVertices[1].lightIntensity = 0.0f;
	clipVertices[1].u = maximumU;
	clipVertices[1].screenY = (float)(screenCenterY + cosineY - trig2_sinedwordmult(halfWidth, angle));
	clipVertices[1].v = 0.0f;
	clipVertices[1].depth = depth;
	halfHeight = -halfHeight;
	rotatedX = trig2_cosinedwordmult(halfWidth, angle);
	rotatedX += trig2_sinedwordmult(halfHeight, angle);
	cosineY = trig2_cosinedwordmult(halfHeight, angle);
	clipVertices[2].screenX = (float)(screenCenterX + rotatedX);
	clipVertices[2].screenY = (float)(screenCenterY + cosineY - trig2_sinedwordmult(halfWidth, angle));
	halfWidth = -halfWidth;
	clipVertices[2].lightIntensity = 0.0f;
	clipVertices[2].u = maximumU;
	clipVertices[2].v = maximumV;
	clipVertices[2].depth = depth;
	rotatedX = trig2_cosinedwordmult(halfWidth, angle);
	rotatedX += trig2_sinedwordmult(halfHeight, angle);
	cosineY = trig2_cosinedwordmult(halfHeight, angle);
	sineY = trig2_sinedwordmult(halfWidth, angle);
	clipVertices[3].v = maximumV;
	clipVertices[3].screenX = (float)(screenCenterX + rotatedX);
	clipVertices[3].depth = depth;
	clipVertices[3].screenY = (float)(screenCenterY + cosineY - sineY);
	clipVertices[3].lightIntensity = 0.0f;
	clipVertices[3].u = 0.0f;
	g_clipCountB = 0;
	if (g_clipCountA > 0) {
		int previous = g_clipIdxA[g_clipCountA - 1];
		for (i = 0; i < g_clipCountA; ++i) {
			int current = g_clipIdxA[i];
			RenderClip_ClipPolyTop(previous, current, clipVertices);
			previous = current;
		}
	}
	g_clipCountA = 0;
	if (g_clipCountB > 0) {
		int previous = g_clipIdxB[g_clipCountB - 1];
		for (i = 0; i < g_clipCountB; ++i) {
			int current = g_clipIdxB[i];
			RenderClip_ClipPolyBottom(previous, current, clipVertices);
			previous = current;
		}
	}
	g_clipCountB = 0;
	if (g_clipCountA > 0) {
		int previous = g_clipIdxA[g_clipCountA - 1];
		for (i = 0; i < g_clipCountA; ++i) {
			int current = g_clipIdxA[i];
			RenderClip_ClipPolyLeft(previous, current, clipVertices);
			previous = current;
		}
	}
	g_clipCountA = 0;
	if (g_clipCountB > 0) {
		int previous = g_clipIdxB[g_clipCountB - 1];
		for (i = 0; i < g_clipCountB; ++i) {
			int current = g_clipIdxB[i];
			RenderClip_ClipPolyRight(previous, current, clipVertices);
			previous = current;
		}
	}
	if (g_clipCountA >= 3) {
		unsigned int vertexColor;
		Std3DTexCacheNode* texture;
		if (g_clipCountA + g_d3dVertexCount > g_maxBatchVerts ||
			(int)(g_clipCountA + g_d3dIndexCount) > g_maxBatchTris) {
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
		if (g_capVertexAlpha) {
			vertexColor =
				((unsigned int)RENDER_HARDWARE_VERTEX_ALPHA << RENDER_HARDWARE_ALPHA_SHIFT) | 0xFFFFFFu;
			g_capVertexAlpha = 0;
		} else
			vertexColor = billboardColor;
		for (i = 0; i < g_clipCountA; ++i) {
			int index = g_clipIdxA[i];
			float y = clipVertices[index].screenY;
			float u = clipVertices[index].u;
			float v = clipVertices[index].v;
			float z = clipVertices[index].depth;
			g_flightVertexBuffer[g_d3dVertexCount].sx = clipVertices[index].screenX + g_flightVpOriginX;
			g_flightVertexBuffer[g_d3dVertexCount].sy = y + g_flightVpOriginY;
			g_flightVertexBuffer[g_d3dVertexCount].sz = z;
			g_flightVertexBuffer[g_d3dVertexCount].rhw = z;
			g_flightVertexBuffer[g_d3dVertexCount].tu = u;
			g_flightVertexBuffer[g_d3dVertexCount].tv = v;
			g_flightVertexBuffer[g_d3dVertexCount].color = vertexColor;
			g_flightVertexBuffer[g_d3dVertexCount].specular = 0;
			g_clipIdxA[i] = g_d3dVertexCount;
			++g_d3dVertexCount;
		}
		texture = RenderTexture_GetOrCreateBitmap(
			textureWidth, textureHeight, (uint16_t*)((uint8_t*)frame + frame->paletteOffset),
			(const uint8_t*)&frame->width + frame->spriteOffset, frame->rleFormatIndex);
		for (i = 2; i < g_clipCountA; ++i) {
			g_triBuffer[g_d3dIndexCount].v0 = g_clipIdxA[0];
			g_triBuffer[g_d3dIndexCount].v1 = g_clipIdxA[i - 1];
			g_triBuffer[g_d3dIndexCount].v2 = g_clipIdxA[i];
			g_triBuffer[g_d3dIndexCount].texture = texture;
			g_triBuffer[g_d3dIndexCount].flags = STD3D_RS_Z_TEST | STD3D_RS_SUBPIXEL | STD3D_RS_DITHER;
			if (g_bilinearEnabled)
				g_triBuffer[g_d3dIndexCount].flags += STD3D_RS_MIN_LINEAR | STD3D_RS_MAG_LINEAR;
			g_triBuffer[g_d3dIndexCount].flags += STD3D_RS_ALPHA_BLEND;
			++g_d3dIndexCount;
		}
	}
}
