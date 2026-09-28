#include "xw/render/render_clip.h"

#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"

#include <stdlib.h>

// GLOBAL: XW 0x53C9A0
int g_clipIdxB[RENDER_CLIP_INDEX_CAPACITY] = { 0 };

// GLOBAL: XW 0x54CA20
int g_clipIdxA[RENDER_CLIP_INDEX_CAPACITY] = { 0 };

// GLOBAL: XW 0x55CAA8
int g_clipCountA = 0;

// GLOBAL: XW 0x55CAC8
int g_clipCountB = 0;

// GLOBAL: XW 0x55CACC
int32_t g_clipVertCursor = 0;

// FUNCTION: XW 0x4803A0
void RenderClip_ClipPolyTop(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices) {
	const ProjVertex* startVertex = &vertices[startVertexIndex];
	float startY = startVertex->screenY;
	const ProjVertex* endVertex = &vertices[endVertexIndex];
	float endY = endVertex->screenY;
	float deltaY, endX, endLight, endU, endV, endDepth, startX, startLight, startU, startV, startDepth,
		deltaX, deltaLight, deltaU, deltaV, deltaDepth;
	if (startY < 0.0f) {
		if (endY >= 0.0f) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaY = endY - startY;
			intersectionIndex = g_clipVertCursor++;
			endX = endVertex->screenX;
			endLight = endVertex->lightIntensity;
			endU = endVertex->u;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startX = startVertex->screenX;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			startDepth = startVertex->depth;
			deltaU = endU - startU;
			deltaLight = endLight - startLight;
			deltaV = endV - startV;
			deltaX = endX - startX;
			deltaDepth = endDepth - startDepth;
			if (-startY < endY) {
				float factor = -startY / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = deltaX * factor + startX;
				intersection->lightIntensity = deltaLight * factor + startLight;
				intersection->depth = deltaDepth * factor + startDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = deltaU * factor + startU;
					intersection->v = deltaV * factor + startV;
				}
			} else {
				float factor = endY / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = endX - deltaX * factor;
				intersection->lightIntensity = endLight - deltaLight * factor;
				intersection->depth = endDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = endU - deltaU * factor;
					intersection->v = endV - deltaV * factor;
				}
			}
			intersection->screenY = 0.0f;
			g_clipIdxB[g_clipCountB++] = intersectionIndex;
			g_clipIdxB[g_clipCountB++] = endVertexIndex;
		}
	} else {
		if (endY < 0.0f) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaY = endY - startY;
			intersectionIndex = g_clipVertCursor++;
			endX = endVertex->screenX;
			endLight = endVertex->lightIntensity;
			endU = endVertex->u;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startX = startVertex->screenX;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			startDepth = startVertex->depth;
			deltaU = endU - startU;
			deltaLight = endLight - startLight;
			deltaV = endV - startV;
			deltaX = endX - startX;
			deltaDepth = endDepth - startDepth;
			if (startY < -endY) {
				float factor = startY / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = startX - deltaX * factor;
				intersection->lightIntensity = startLight - deltaLight * factor;
				intersection->depth = startDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = startU - deltaU * factor;
					intersection->v = startV - deltaV * factor;
				}
			} else {
				float factor = -endY / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = deltaX * factor + endX;
				intersection->lightIntensity = deltaLight * factor + endLight;
				intersection->depth = deltaDepth * factor + endDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = deltaU * factor + endU;
					intersection->v = deltaV * factor + endV;
				}
			}
			intersection->screenY = 0.0f;
			g_clipIdxB[g_clipCountB++] = intersectionIndex;
		} else {
			g_clipIdxB[g_clipCountB++] = endVertexIndex;
		}
	}
}

// FUNCTION: XW 0x4809B0
void RenderClip_ClipPolyBottom(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices) {
	const ProjVertex* startVertex = &vertices[startVertexIndex];
	float startY = startVertex->screenY;
	const ProjVertex* endVertex = &vertices[endVertexIndex];
	float endY = endVertex->screenY;
	float boundary = g_flightVpMaxY;
	float startDistance, endDistance;
	float deltaY, endX, endLight, endU, endV, endDepth, startX, startLight, startU, startV, startDepth,
		deltaX, deltaLight, deltaU, deltaV, deltaDepth;
	if (startY > boundary) {
		if (endY < boundary) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaY = endY - startY;
			startDistance = startY - boundary;
			endDistance = boundary - endY;
			intersectionIndex = g_clipVertCursor++;
			endX = endVertex->screenX;
			endLight = endVertex->lightIntensity;
			startX = startVertex->screenX;
			endU = endVertex->u;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startDepth = startVertex->depth;
			deltaX = endX - startX;
			deltaLight = endLight - startLight;
			deltaU = endU - startU;
			deltaV = endV - startV;
			deltaDepth = endDepth - startDepth;
			if (startDistance < endDistance) {
				float factor = startDistance / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = startX - deltaX * factor;
				intersection->lightIntensity = startLight - deltaLight * factor;
				intersection->depth = startDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = startU - deltaU * factor;
					intersection->v = startV - deltaV * factor;
				}
			} else {
				float factor = endDistance / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = deltaX * factor + endX;
				intersection->lightIntensity = deltaLight * factor + endLight;
				intersection->depth = deltaDepth * factor + endDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = deltaU * factor + endU;
					intersection->v = deltaV * factor + endV;
				}
			}
			intersection->screenY = g_flightVpMaxY;
			g_clipIdxA[g_clipCountA++] = intersectionIndex;
			g_clipIdxA[g_clipCountA++] = endVertexIndex;
		}
	} else {
		if (endY > boundary) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaY = endY - startY;
			startDistance = boundary - startY;
			endDistance = endY - boundary;
			intersectionIndex = g_clipVertCursor++;
			endX = endVertex->screenX;
			endLight = endVertex->lightIntensity;
			startX = startVertex->screenX;
			endU = endVertex->u;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startDepth = startVertex->depth;
			deltaX = endX - startX;
			deltaLight = endLight - startLight;
			deltaU = endU - startU;
			deltaV = endV - startV;
			deltaDepth = endDepth - startDepth;
			if (startDistance < endDistance) {
				float factor = startDistance / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = deltaX * factor + startX;
				intersection->lightIntensity = deltaLight * factor + startLight;
				intersection->depth = deltaDepth * factor + startDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = deltaU * factor + startU;
					intersection->v = deltaV * factor + startV;
				}
			} else {
				float factor = endDistance / deltaY;
				intersection = &vertices[intersectionIndex];
				intersection->screenX = endX - deltaX * factor;
				intersection->lightIntensity = endLight - deltaLight * factor;
				intersection->depth = endDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = endU - deltaU * factor;
					intersection->v = endV - deltaV * factor;
				}
			}
			intersection->screenY = g_flightVpMaxY;
			g_clipIdxA[g_clipCountA++] = intersectionIndex;
		} else {
			g_clipIdxA[g_clipCountA++] = endVertexIndex;
		}
	}
}

// FUNCTION: XW 0x481010
void RenderClip_ClipPolyLeft(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices) {
	const ProjVertex* startVertex = &vertices[startVertexIndex];
	float startX = startVertex->screenX;
	const ProjVertex* endVertex = &vertices[endVertexIndex];
	float endX = endVertex->screenX;
	float deltaX, endY, endLight, endU, endV, endDepth, startY, startLight, startU, startV, startDepth,
		deltaY, deltaLight, deltaU, deltaV, deltaDepth;
	if (startX < 0.0f) {
		if (endX >= 0.0f) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaX = endX - startX;
			intersectionIndex = g_clipVertCursor++;
			endY = endVertex->screenY;
			endLight = endVertex->lightIntensity;
			endU = endVertex->u;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startY = startVertex->screenY;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			startDepth = startVertex->depth;
			deltaU = endU - startU;
			deltaLight = endLight - startLight;
			deltaV = endV - startV;
			deltaY = endY - startY;
			deltaDepth = endDepth - startDepth;
			if (-startX < endX) {
				float factor = -startX / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = deltaY * factor + startY;
				intersection->lightIntensity = deltaLight * factor + startLight;
				intersection->depth = deltaDepth * factor + startDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = deltaU * factor + startU;
					intersection->v = deltaV * factor + startV;
				}
			} else {
				float factor = endX / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = endY - deltaY * factor;
				intersection->lightIntensity = endLight - deltaLight * factor;
				intersection->depth = endDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = endU - deltaU * factor;
					intersection->v = endV - deltaV * factor;
				}
			}
			intersection->screenX = 0.0f;
			g_clipIdxB[g_clipCountB++] = intersectionIndex;
			g_clipIdxB[g_clipCountB++] = endVertexIndex;
		}
	} else {
		if (endX < 0.0f) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaX = endX - startX;
			intersectionIndex = g_clipVertCursor++;
			endY = endVertex->screenY;
			endLight = endVertex->lightIntensity;
			endU = endVertex->u;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startY = startVertex->screenY;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			startDepth = startVertex->depth;
			deltaU = endU - startU;
			deltaLight = endLight - startLight;
			deltaV = endV - startV;
			deltaY = endY - startY;
			deltaDepth = endDepth - startDepth;
			if (startX < -endX) {
				float factor = startX / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = startY - deltaY * factor;
				intersection->lightIntensity = startLight - deltaLight * factor;
				intersection->depth = startDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = startU - deltaU * factor;
					intersection->v = startV - deltaV * factor;
				}
			} else {
				float factor = -endX / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = deltaY * factor + endY;
				intersection->lightIntensity = deltaLight * factor + endLight;
				intersection->depth = deltaDepth * factor + endDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = deltaU * factor + endU;
					intersection->v = deltaV * factor + endV;
				}
			}
			intersection->screenX = 0.0f;
			g_clipIdxB[g_clipCountB++] = intersectionIndex;
		} else {
			g_clipIdxB[g_clipCountB++] = endVertexIndex;
		}
	}
}

// FUNCTION: XW 0x481610
void RenderClip_ClipPolyRight(int startVertexIndex, int endVertexIndex, struct ProjVertex* vertices) {
	const ProjVertex* startVertex = &vertices[startVertexIndex];
	float startX = startVertex->screenX;
	const ProjVertex* endVertex = &vertices[endVertexIndex];
	float endX = endVertex->screenX;
	float boundary = g_flightVpWidth;
	float startDistance, endDistance;
	float deltaX, endY, endLight, endU, endV, endDepth, startY, startLight, startU, startV, startDepth,
		deltaY, deltaLight, deltaU, deltaV, deltaDepth;
	if (startX > boundary) {
		if (endX <= boundary) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaX = endX - startX;
			startDistance = startX - boundary;
			endDistance = boundary - endX;
			intersectionIndex = g_clipVertCursor++;
			endY = endVertex->screenY;
			endLight = endVertex->lightIntensity;
			startY = startVertex->screenY;
			endU = endVertex->u;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startDepth = startVertex->depth;
			deltaY = endY - startY;
			deltaLight = endLight - startLight;
			deltaU = endU - startU;
			deltaV = endV - startV;
			deltaDepth = endDepth - startDepth;
			if (startDistance < endDistance) {
				float factor = startDistance / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = startY - deltaY * factor;
				intersection->lightIntensity = startLight - deltaLight * factor;
				intersection->depth = startDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = startU - deltaU * factor;
					intersection->v = startV - deltaV * factor;
				}
			} else {
				float factor = endDistance / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = deltaY * factor + endY;
				intersection->lightIntensity = deltaLight * factor + endLight;
				intersection->depth = deltaDepth * factor + endDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = deltaU * factor + endU;
					intersection->v = deltaV * factor + endV;
				}
			}
			intersection->screenX = g_flightVpWidth;
			g_clipIdxA[g_clipCountA++] = intersectionIndex;
			g_clipIdxA[g_clipCountA++] = endVertexIndex;
		}
	} else {
		if (endX > boundary) {
			int intersectionIndex;
			ProjVertex* intersection;
			deltaX = endX - startX;
			startDistance = boundary - startX;
			endDistance = endX - boundary;
			intersectionIndex = g_clipVertCursor++;
			endY = endVertex->screenY;
			endLight = endVertex->lightIntensity;
			startY = startVertex->screenY;
			endU = endVertex->u;
			startLight = startVertex->lightIntensity;
			startU = startVertex->u;
			startV = startVertex->v;
			endV = endVertex->v;
			endDepth = endVertex->depth;
			startDepth = startVertex->depth;
			deltaY = endY - startY;
			deltaLight = endLight - startLight;
			deltaU = endU - startU;
			deltaV = endV - startV;
			deltaDepth = endDepth - startDepth;
			if (startDistance < endDistance) {
				float factor = startDistance / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = deltaY * factor + startY;
				intersection->lightIntensity = deltaLight * factor + startLight;
				intersection->depth = deltaDepth * factor + startDepth;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / startDepth;
					float deltaViewDepth = projectionScale / endDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = deltaU * textureFactor + startU;
					intersection->v = deltaV * textureFactor + startV;
				} else {
					intersection->u = deltaU * factor + startU;
					intersection->v = deltaV * factor + startV;
				}
			} else {
				float factor = endDistance / deltaX;
				intersection = &vertices[intersectionIndex];
				intersection->screenY = endY - deltaY * factor;
				intersection->lightIntensity = endLight - deltaLight * factor;
				intersection->depth = endDepth - deltaDepth * factor;
				if (deltaDepth != 0.0f) {
					double projectionScale = (double)(uint32_t)g_projScaleInt;
					float baseViewDepth = projectionScale / endDepth;
					float deltaViewDepth = projectionScale / startDepth - baseViewDepth;
					float textureFactor =
						(projectionScale / intersection->depth - baseViewDepth) / deltaViewDepth;
					intersection->u = endU - deltaU * textureFactor;
					intersection->v = endV - deltaV * textureFactor;
				} else {
					intersection->u = endU - deltaU * factor;
					intersection->v = endV - deltaV * factor;
				}
			}
			intersection->screenX = g_flightVpWidth;
			g_clipIdxA[g_clipCountA++] = intersectionIndex;
		} else {
			g_clipIdxA[g_clipCountA++] = endVertexIndex;
		}
	}
}

// FUNCTION: XW 0x481C70
void RenderClip_ClipPolyNear(int prevVertIndex, int curVertIndex, struct ProjVertex* vertices) {
	int outputIndex = curVertIndex;
	ProjVertex* previousVertex = &vertices[prevVertIndex];
	ProjVertex* currentVertex = &vertices[curVertIndex];
	float previousDepth = previousVertex->depth;
	float currentDepth = currentVertex->depth;
	if (previousDepth < 0.0f) {
		if (currentDepth >= 0.0f) {
			float viewDepth = (double)(uint32_t)g_projScaleInt / currentDepth;
			int intersectionIndex = g_clipVertCursor++;
			ProjVertex* intersection = &vertices[intersectionIndex];
			float previousX = previousVertex->screenX;
			float previousY = previousVertex->screenY;
			float previousU = previousVertex->u;
			float previousV = previousVertex->v;
			float previousLight = previousVertex->lightIntensity;
			float currentLight = currentVertex->lightIntensity;
			float centeredX = currentVertex->screenX - (g_flightVpWidth >> 1);
			float centeredY =
				currentVertex->screenY - (double)((uint32_t)g_projOffsetY + (g_flightVpHeight >> 1));
			float factor = previousDepth / (viewDepth - previousDepth - RENDER_CLIP_NEAR_DEPTH);
			float currentX = viewDepth * centeredX * g_invProjScale;
			float currentY = viewDepth * centeredY * g_invProjScale;
			float deltaU = currentVertex->u - previousU;
			float deltaV = currentVertex->v - previousV;
			intersection->lightIntensity = previousLight - (currentLight - previousLight) * factor;
			intersection->screenX = previousX - (currentX - previousX) * factor;
			intersection->screenY = previousY - (currentY - previousY) * factor;
			intersection->u = previousU - deltaU * factor;
			intersection->v = previousV - deltaV * factor;
			{
				double projectionScale = (double)(uint32_t)g_projScaleInt;
				intersection->depth = projectionScale;
				intersection->screenX = projectionScale * intersection->screenX;
				intersection->screenY = projectionScale * intersection->screenY;
			}
			intersection->screenX = (g_flightVpWidth >> 1) + intersection->screenX;
			intersection->screenY =
				(double)((uint32_t)g_projOffsetY + (g_flightVpHeight >> 1)) + intersection->screenY;
			g_clipIdxA[g_clipCountA++] = intersectionIndex;
			g_clipIdxA[g_clipCountA++] = curVertIndex;
		}
	} else {
		if (currentDepth < 0.0f) {
			float viewDepth = (double)(uint32_t)g_projScaleInt / previousDepth;
			ProjVertex* intersection;
			float currentX = currentVertex->screenX;
			float currentY = currentVertex->screenY;
			float currentU = currentVertex->u;
			float currentV = currentVertex->v;
			float currentLight = currentVertex->lightIntensity;
			float previousLight = previousVertex->lightIntensity;
			float centeredX = previousVertex->screenX - (g_flightVpWidth >> 1);
			float centeredY =
				previousVertex->screenY - (double)((uint32_t)g_projOffsetY + (g_flightVpHeight >> 1));
			float previousX = viewDepth * centeredX * g_invProjScale;
			float previousY = viewDepth * centeredY * g_invProjScale;
			float deltaU = currentU - previousVertex->u;
			float deltaV = currentV - previousVertex->v;
			float factor = currentDepth / (currentDepth - viewDepth - -RENDER_CLIP_NEAR_DEPTH);
			outputIndex = g_clipVertCursor++;
			intersection = &vertices[outputIndex];
			intersection->lightIntensity = currentLight - (currentLight - previousLight) * factor;
			intersection->screenX = currentX - (currentX - previousX) * factor;
			intersection->screenY = currentY - (currentY - previousY) * factor;
			intersection->u = currentU - deltaU * factor;
			intersection->v = currentV - deltaV * factor;
			{
				double projectionScale = (double)(uint32_t)g_projScaleInt;
				intersection->depth = projectionScale;
				intersection->screenX = projectionScale * intersection->screenX;
				intersection->screenY = projectionScale * intersection->screenY;
			}
			intersection->screenX = (g_flightVpWidth >> 1) + intersection->screenX;
			intersection->screenY =
				(double)((uint32_t)g_projOffsetY + (g_flightVpHeight >> 1)) + intersection->screenY;
		}
		g_clipIdxA[g_clipCountA++] = outputIndex;
	}
}
