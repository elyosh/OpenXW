#include "xw/flight/object/starship.h"

#ifdef XW_MODERN
#include "xw_runtime/timing/reference_motion.h"
#endif

#ifdef XW_MODERN
#include "xw_dos94/assets/models.h"
#include "xw_dos94/flight/object/collision.h"
#include "xw_dos94/flight/object/effects.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/ai/pai.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/craft.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/player/user.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/math3d.h"
#include "xw/math/trig2.h"
#include "xw/util/memory.h"
#include "xw/util/shared.h"

#include <stdlib.h>

// GLOBAL: XW 0x4C321C
const float g_collisionNoHitFraction = 2.0f;

// GLOBAL: XW 0x4C3224
const float g_collisionRadiansPerAngleByte = 0.024543672800064087f;

// GLOBAL: XW 0x4C3228
const float g_collisionHitFractionBackoff = 0.1f;

// GLOBAL: XW 0x4C322C
const float g_collisionQ15ToFloat = 0.000030517578125f;

// GLOBAL: XW 0x4C3230
const float g_collisionPlaneSnapPositiveLimit = 10.0f;

// GLOBAL: XW 0x4C3234
const float g_collisionPlaneSnapNegativeLimit = -10.0f;

// GLOBAL: XW 0x4F49D0
int g_collideSweepFaceGroupOrdinal = 0;

// GLOBAL: XW 0x4F49D8
OptVector g_collideSweepWalkerStart = { 0 };

// GLOBAL: XW 0x4F49E8
OptVector g_collideSweepWalkerEnd = { 0 };

// GLOBAL: XW 0x4F49F4
int g_collideSweepHitMeshOrdinal = 0;

// GLOBAL: XW 0x4F49F8
OptVector g_collideSweepModelStart = { 0 };

// GLOBAL: XW 0x4F4A08
OptVector g_collideSweepModelEnd = { 0 };

// GLOBAL: XW 0x4F4A14
int g_collideSweepCurrentMeshOrdinal = 0;

// GLOBAL: XW 0x4F4A18
float g_collideCurrentMeshRotationAngle = 0.0f;

// GLOBAL: XW 0x4F4A1C
float g_collideSweepHitFraction = 0.0f;

// GLOBAL: XW 0x4F4A20
OptNode* g_collideCurrentMeshVertsNode = NULL;

// FUNCTION: XW 0x423A50
int starship_checkstarshiphit(uint16_t sourceObjIdx, uint16_t targetObjIdx) {
	int targetObjectIndex;
	ObjectRecord* targetObject;
	int endRelativeX, endRelativeY, endRelativeZ;
	int startRelativeX, startRelativeY, startRelativeZ;
	int endModelX, endModelY, endModelZ;
	int startModelX, startModelY, startModelZ;
	OptimizedPolyObject* model;
	unsigned int rootNodeIndex;
	int hitMeshOrdinal;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		return Dos94_starship_checkstarshiphit(sourceObjIdx, targetObjIdx);
	}
#endif
	targetObjectIndex = targetObjIdx;
	targetObject = &g_objectTable[targetObjectIndex];
	g_curCraft = (CraftData*)targetObject->instanceData;
#ifdef XW_MODERN
	endRelativeX = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)targetObject->worldX);
	startRelativeX = (int32_t)((uint32_t)g_collisionSegmentStartWorldX - (uint32_t)targetObject->worldX);
#else
	endRelativeX = g_collisionProbeWorldX - targetObject->worldX;
	startRelativeX = g_collisionSegmentStartWorldX - targetObject->worldX;
#endif
#ifdef XW_MODERN
	endRelativeY = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)targetObject->worldY);
	startRelativeY = (int32_t)((uint32_t)g_collisionSegmentStartWorldY - (uint32_t)targetObject->worldY);
#else
	endRelativeY = g_collisionProbeWorldY - targetObject->worldY;
	startRelativeY = g_collisionSegmentStartWorldY - targetObject->worldY;
#endif
#ifdef XW_MODERN
	endRelativeZ = (int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)targetObject->worldZ);
	startRelativeZ = (int32_t)((uint32_t)g_collisionSegmentStartWorldZ - (uint32_t)targetObject->worldZ);
#else
	endRelativeZ = g_collisionProbeWorldZ - targetObject->worldZ;
	startRelativeZ = g_collisionSegmentStartWorldZ - targetObject->worldZ;
#endif
	if (targetObject->orientMatrixDirty != 0) {
		fview_calcrotatemove(targetObject->pitch, targetObject->yaw, targetObject);
		fview_calcrotateorient(targetObject->roll, 0, targetObject);
	}
	endModelX = (int32_t)((uint32_t)((uint64_t)((int64_t)endRelativeX * targetObject->cachedSideX) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelX = (int32_t)((uint32_t)endModelX +
						  (uint32_t)((uint64_t)((int64_t)endRelativeY * targetObject->cachedSideY) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelX = (int32_t)((uint32_t)endModelX +
						  (uint32_t)((uint64_t)((int64_t)endRelativeZ * targetObject->cachedSideZ) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelY = (int32_t)((uint32_t)((uint64_t)((int64_t)endRelativeX * targetObject->cachedForwardX) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelY = (int32_t)((uint32_t)endModelY +
						  (uint32_t)((uint64_t)((int64_t)endRelativeY * targetObject->cachedForwardY) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelY = (int32_t)((uint32_t)endModelY +
						  (uint32_t)((uint64_t)((int64_t)endRelativeZ * targetObject->cachedForwardZ) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelY = (int32_t)(0u - (uint32_t)endModelY);
	endModelZ = (int32_t)((uint32_t)((uint64_t)((int64_t)endRelativeX * targetObject->cachedUpX) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelZ = (int32_t)((uint32_t)endModelZ +
						  (uint32_t)((uint64_t)((int64_t)endRelativeY * targetObject->cachedUpY) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	endModelZ = (int32_t)((uint32_t)endModelZ +
						  (uint32_t)((uint64_t)((int64_t)endRelativeZ * targetObject->cachedUpZ) >>
									 FVIEW_MATRIX_FRACTION_BITS));
	startModelX = (int32_t)((uint32_t)((uint64_t)((int64_t)startRelativeX * targetObject->cachedSideX) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelX = (int32_t)((uint32_t)startModelX +
							(uint32_t)((uint64_t)((int64_t)startRelativeY * targetObject->cachedSideY) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelX = (int32_t)((uint32_t)startModelX +
							(uint32_t)((uint64_t)((int64_t)startRelativeZ * targetObject->cachedSideZ) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelY = (int32_t)((uint32_t)((uint64_t)((int64_t)startRelativeX * targetObject->cachedForwardX) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelY = (int32_t)((uint32_t)startModelY +
							(uint32_t)((uint64_t)((int64_t)startRelativeY * targetObject->cachedForwardY) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelY = (int32_t)((uint32_t)startModelY +
							(uint32_t)((uint64_t)((int64_t)startRelativeZ * targetObject->cachedForwardZ) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelY = (int32_t)(0u - (uint32_t)startModelY);
	startModelZ = (int32_t)((uint32_t)((uint64_t)((int64_t)startRelativeX * targetObject->cachedUpX) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelZ = (int32_t)((uint32_t)startModelZ +
							(uint32_t)((uint64_t)((int64_t)startRelativeY * targetObject->cachedUpY) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	startModelZ = (int32_t)((uint32_t)startModelZ +
							(uint32_t)((uint64_t)((int64_t)startRelativeZ * targetObject->cachedUpZ) >>
									   FVIEW_MATRIX_FRACTION_BITS));
	g_collideSweepHitMeshOrdinal = 0;
	g_collideSweepHitFraction = g_collisionNoHitFraction;
	g_collideSweepModelStart.x = g_collideSweepWalkerStart.x = (float)startModelX;
	g_collideSweepModelStart.y = g_collideSweepWalkerStart.y = (float)startModelY;
	g_collideSweepModelStart.z = g_collideSweepWalkerStart.z = (float)startModelZ;
	g_collideSweepModelEnd.x = g_collideSweepWalkerEnd.x = (float)endModelX;
	g_collideSweepModelEnd.y = g_collideSweepWalkerEnd.y = (float)endModelY;
	g_collideSweepModelEnd.z = g_collideSweepWalkerEnd.z = (float)endModelZ;
	model = (OptimizedPolyObject*)Memory_LockHandle(g_loadedModels[targetObject->objectType]);
	if (model == NULL)
		return 0;
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	g_collideSweepCurrentMeshOrdinal = 0;
	g_collideCurrentMeshVertsNode = NULL;
	for (rootNodeIndex = 0; rootNodeIndex < (unsigned int)model->rootNodeCount; ++rootNodeIndex) {
		OptNode* rootNode;
		unsigned int rotationByte;
		g_collideCurrentMeshRotationAngle = 0.0f;
		rootNode = model->rootNodes[rootNodeIndex];
		if (rootNode->nodeType == OPT_TEXTURE)
			continue;
		++g_collideSweepCurrentMeshOrdinal;
		if (sourceObjIdx == targetObjIdx) {
			int meshType = ModelMesh_GetCachedObjectTypeMeshType(g_objectTable[targetObjectIndex].objectType,
																 g_collideSweepCurrentMeshOrdinal - 1);
			if (meshType == MODEL_MESH_TYPE_4 || meshType == MODEL_MESH_TYPE_5 ||
				meshType == MODEL_MESH_TYPE_21)
				continue;
		}
		if (g_curCraft->componentHp[g_collideSweepCurrentMeshOrdinal - 1] == 0)
			continue;
		rotationByte = g_curCraft->meshRotation[g_collideSweepCurrentMeshOrdinal - 1];
		if (rotationByte != 0 && g_missionRuntimeState.provingGroundsActive == 0 &&
			sourceObjIdx == g_playerFlightState.objectIndex)
			continue;
		g_collideCurrentMeshRotationAngle = (double)rotationByte * g_collisionRadiansPerAngleByte;
		if (rotationByte == 0) {
			const MeshDescriptor* bounds =
				ModelMesh_GetCachedDescriptor(targetObject->objectType, g_collideSweepCurrentMeshOrdinal - 1);
			int bound;
			if (bounds == NULL)
				continue;
			bound = (int)(int64_t)bounds->boxMin.x;
			if (endModelX < bound && startModelX < bound) {
				nullsub_SharedNoOp();
				continue;
			}
			bound = (int)(int64_t)bounds->boxMin.y;
			if (endModelY < bound && startModelY < bound) {
				nullsub_SharedNoOp();
				continue;
			}
			bound = (int)(int64_t)bounds->boxMin.z;
			if (endModelZ < bound && startModelZ < bound) {
				nullsub_SharedNoOp();
				continue;
			}
			bound = (int)(int64_t)bounds->boxMax.x;
			if (endModelX > bound && startModelX > bound) {
				nullsub_SharedNoOp();
				continue;
			}
			bound = (int)(int64_t)bounds->boxMax.y;
			if (endModelY > bound && startModelY > bound) {
				nullsub_SharedNoOp();
				continue;
			}
			bound = (int)(int64_t)bounds->boxMax.z;
			if (endModelZ > bound && startModelZ > bound) {
				nullsub_SharedNoOp();
				continue;
			}
			nullsub_SharedNoOp();
		}
		starship_TestSweepAgainstOptNode(model, rootNode);
		g_collideSweepWalkerStart = g_collideSweepModelStart;
		g_collideSweepWalkerEnd = g_collideSweepModelEnd;
	}
	nullsub_SharedNoOp();
	hitMeshOrdinal = g_collideSweepHitMeshOrdinal;
	if (hitMeshOrdinal != 0) {
		g_collideSweepHitFraction -= g_collisionHitFractionBackoff;
		if (g_collideSweepHitFraction < 0.0f)
			g_collideSweepHitFraction = 0.0f;
#ifdef XW_MODERN
		g_collisionHitOffsetX = (int)(int64_t)((int32_t)((uint32_t)g_collisionProbeWorldX -
														 (uint32_t)g_collisionSegmentStartWorldX) *
											   (double)g_collideSweepHitFraction);
#else
		g_collisionHitOffsetX = (int)(int64_t)((g_collisionProbeWorldX - g_collisionSegmentStartWorldX) *
											   (double)g_collideSweepHitFraction);
#endif
#ifdef XW_MODERN
		g_collisionHitOffsetY = (int)(int64_t)((int32_t)((uint32_t)g_collisionProbeWorldY -
														 (uint32_t)g_collisionSegmentStartWorldY) *
											   (double)g_collideSweepHitFraction);
#else
		g_collisionHitOffsetY = (int)(int64_t)((g_collisionProbeWorldY - g_collisionSegmentStartWorldY) *
											   (double)g_collideSweepHitFraction);
#endif
#ifdef XW_MODERN
		g_collisionHitOffsetZ = (int)(int64_t)((int32_t)((uint32_t)g_collisionProbeWorldZ -
														 (uint32_t)g_collisionSegmentStartWorldZ) *
											   (double)g_collideSweepHitFraction);
#else
		g_collisionHitOffsetZ = (int)(int64_t)((g_collisionProbeWorldZ - g_collisionSegmentStartWorldZ) *
											   (double)g_collideSweepHitFraction);
#endif
	}
	return hitMeshOrdinal;
}

// FUNCTION: XW 0x424000
int starship_CheckSweptMeshCollision(int objectType, int meshIndex, int point1X, int point1Y, int point1Z,
									 int point2X, int point2Y, int point2Z) {
	unsigned int modelHandle;
	OptimizedPolyObject* model;
	int rootIndex;
	g_collideSweepModelStart.x = g_collideSweepWalkerStart.x = (float)point1X;
	g_collideSweepModelStart.y = g_collideSweepWalkerStart.y = (float)point1Y;
	g_collideSweepModelStart.z = g_collideSweepWalkerStart.z = (float)point1Z;
	g_collideSweepModelEnd.x = g_collideSweepWalkerEnd.x = (float)point2X;
	g_collideSweepModelEnd.y = g_collideSweepWalkerEnd.y = (float)point2Y;
	g_collideSweepModelEnd.z = g_collideSweepWalkerEnd.z = (float)point2Z;
	modelHandle = g_loadedModels[objectType];
	g_collideSweepHitFraction = g_collisionNoHitFraction;
	g_collideSweepHitMeshOrdinal = 0;
	model = (OptimizedPolyObject*)Memory_LockHandle(modelHandle);
	if (model == NULL)
		return 0;
	if (model->selfMarker != model)
		OptModel_AdjustOptimizedPolyObjectPointers(model);
	g_collideSweepCurrentMeshOrdinal = 0;
	g_collideCurrentMeshVertsNode = NULL;
	for (rootIndex = 0; rootIndex < model->rootNodeCount; ++rootIndex) {
		OptNode* root;
		g_collideCurrentMeshRotationAngle = 0.0f;
		root = model->rootNodes[rootIndex];
		if (root->nodeType != OPT_TEXTURE && ++g_collideSweepCurrentMeshOrdinal - 1 == meshIndex) {
			g_collideCurrentMeshRotationAngle = 0.0f;
			if (g_surfaceSpecialTargetCollisionMode != 0)
				g_collideSweepFaceGroupOrdinal = 0;
			starship_TestSweepAgainstOptNode(model, root);
			g_collideSweepWalkerStart = g_collideSweepModelStart;
			g_collideSweepWalkerEnd = g_collideSweepModelEnd;
		}
	}
	nullsub_SharedNoOp();
	if (g_collideSweepHitMeshOrdinal != 0) {
		g_collideSweepHitFraction -= g_collisionHitFractionBackoff;
		if (g_collideSweepHitFraction < 0.0f)
			g_collideSweepHitFraction = 0.0f;
#ifdef XW_MODERN
		g_collisionHitOffsetX =
			(int)((int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)g_collisionSegmentStartWorldX) *
				  (double)g_collideSweepHitFraction);
#else
		g_collisionHitOffsetX = (int)((g_collisionProbeWorldX - g_collisionSegmentStartWorldX) *
									  (double)g_collideSweepHitFraction);
#endif
#ifdef XW_MODERN
		g_collisionHitOffsetY =
			(int)((int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)g_collisionSegmentStartWorldY) *
				  (double)g_collideSweepHitFraction);
#else
		g_collisionHitOffsetY = (int)((g_collisionProbeWorldY - g_collisionSegmentStartWorldY) *
									  (double)g_collideSweepHitFraction);
#endif
#ifdef XW_MODERN
		g_collisionHitOffsetZ =
			(int)((int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)g_collisionSegmentStartWorldZ) *
				  (double)g_collideSweepHitFraction);
#else
		g_collisionHitOffsetZ = (int)((g_collisionProbeWorldZ - g_collisionSegmentStartWorldZ) *
									  (double)g_collideSweepHitFraction);
#endif
	}
	return g_collideSweepHitMeshOrdinal;
}

// FUNCTION: XW 0x424240
int starship_TestSweepAgainstOptNode(struct OptimizedPolyObject* object, struct OptNode* node) {
	int selectedChild;
	int childIndex;
	if (node == NULL)
		return 0;
	selectedChild = 0;
	while (node->nodeType == OPT_NODEREF) {
		if (g_cacheResolvedOptNodeRefs != 0) {
			if (*(char*)node->param2 != 0) {
				node->pName = (char*)OptModel_ResolveNodeRef(object, (const char*)node->param2);
				*(char*)node->param2 = 0;
			}
			node = (OptNode*)node->pName;
		} else {
			node = OptModel_ResolveNodeRef(object, (const char*)node->param2);
		}
		if (node == NULL)
			return 0;
	}
	switch (node->nodeType) {
		case OPT_NODE_TYPE_23:
			if (g_collideCurrentMeshRotationAngle != 0.0f) {
				const float* rotation = (const float*)node->param2;
				float axisAngle[STARSHIP_AXIS_ANGLE_COUNT];
				float matrix[STARSHIP_MATRIX_CAPACITY];
				g_collideSweepWalkerStart.x -= rotation[STARSHIP_COORD_X];
				g_collideSweepWalkerStart.y -= rotation[STARSHIP_COORD_Y];
				g_collideSweepWalkerStart.z -= rotation[STARSHIP_COORD_Z];
				g_collideSweepWalkerEnd.x -= rotation[STARSHIP_COORD_X];
				g_collideSweepWalkerEnd.y -= rotation[STARSHIP_COORD_Y];
				g_collideSweepWalkerEnd.z -= rotation[STARSHIP_COORD_Z];
				axisAngle[STARSHIP_COORD_X] = rotation[STARSHIP_ROTATION_AXIS_X] * g_collisionQ15ToFloat;
				axisAngle[STARSHIP_COORD_Y] = rotation[STARSHIP_ROTATION_AXIS_Y] * g_collisionQ15ToFloat;
				axisAngle[STARSHIP_COORD_Z] = rotation[STARSHIP_ROTATION_AXIS_Z] * g_collisionQ15ToFloat;
				axisAngle[STARSHIP_AXIS_ANGLE_COUNT - 1] = g_collideCurrentMeshRotationAngle;
				Math3D_BuildAxisAngleMatrix(matrix, axisAngle);
				Math3D_RotateVec3(&g_collideSweepWalkerStart.x, matrix);
				Math3D_RotateVec3(&g_collideSweepWalkerEnd.x, matrix);
				g_collideSweepWalkerStart.x += rotation[STARSHIP_COORD_X];
				g_collideSweepWalkerStart.y += rotation[STARSHIP_COORD_Y];
				g_collideSweepWalkerStart.z += rotation[STARSHIP_COORD_Z];
				g_collideSweepWalkerEnd.x += rotation[STARSHIP_COORD_X];
				g_collideSweepWalkerEnd.y += rotation[STARSHIP_COORD_Y];
				g_collideSweepWalkerEnd.z += rotation[STARSHIP_COORD_Z];
				g_collideCurrentMeshRotationAngle = 0.0f;
			}
			break;
		case OPT_NODE_TYPE_21:
			selectedChild = 1;
			break;
		case OPT_MESHVERTS: {
			const OptVector* bounds;
			g_collideCurrentMeshVertsNode = node;
			bounds = &((const OptVector*)node->param2)[node->param1 - STARSHIP_BOUNDS_VECTOR_COUNT];
			if (g_collideCurrentMeshRotationAngle == 0.0f) {
				if (g_collideSweepWalkerStart.x < bounds[0].x && g_collideSweepWalkerEnd.x < bounds[0].x)
					return 1;
				if (g_collideSweepWalkerStart.y < bounds[0].y && g_collideSweepWalkerEnd.y < bounds[0].y)
					return 1;
				if (g_collideSweepWalkerStart.z < bounds[0].z && g_collideSweepWalkerEnd.z < bounds[0].z)
					return 1;
				if (g_collideSweepWalkerStart.x > bounds[1].x && g_collideSweepWalkerEnd.x > bounds[1].x)
					return 1;
				if (g_collideSweepWalkerStart.y > bounds[1].y && g_collideSweepWalkerEnd.y > bounds[1].y)
					return 1;
				if (g_collideSweepWalkerStart.z > bounds[1].z && g_collideSweepWalkerEnd.z > bounds[1].z)
					return 1;
			}
			break;
		}
		case OPT_FACEDATA:
		case OPT_FACEDATA_15:
		case OPT_FACEDATA_16:
		case OPT_FACEDATA_17: {
			const OptPackedFaceData* data;
			const OptPackedFaceRecord* faces;
			const OptVector* normals;
			const float* vertices;
			int faceIndex;
			++g_collideSweepFaceGroupOrdinal;
			data = (const OptPackedFaceData*)node->param2;
			faces = (const OptPackedFaceRecord*)((const uint8_t*)data + sizeof(data->edgeCount));
			normals = (const OptVector*)&faces[node->param1];
			vertices = (const float*)g_collideCurrentMeshVertsNode->param2;
			for (faceIndex = 0; faceIndex < node->param1; ++faceIndex) {
				float hitFraction;
				float projectedPoint[STARSHIP_COORD_COUNT];
				if (starship_IntersectSegmentWithFacePlane(
						&normals[faceIndex].x,
						&vertices[STARSHIP_COORD_COUNT * faces[faceIndex].vertexIndices[0]],
						&g_collideSweepWalkerStart.x, &g_collideSweepWalkerEnd.x, &hitFraction) != 0 &&
					hitFraction < g_collideSweepHitFraction) {
					projectedPoint[STARSHIP_COORD_X] =
						(g_collideSweepWalkerEnd.x - g_collideSweepWalkerStart.x) * hitFraction +
						g_collideSweepWalkerStart.x;
					projectedPoint[STARSHIP_COORD_Y] =
						(g_collideSweepWalkerEnd.y - g_collideSweepWalkerStart.y) * hitFraction +
						g_collideSweepWalkerStart.y;
					projectedPoint[STARSHIP_COORD_Z] =
						(g_collideSweepWalkerEnd.z - g_collideSweepWalkerStart.z) * hitFraction +
						g_collideSweepWalkerStart.z;
					if (starship_PointInFacePolygon(&normals[faceIndex].x, vertices,
													faces[faceIndex].vertexIndices, projectedPoint)) {
						if (g_surfaceSpecialTargetCollisionMode != 0 &&
							(g_collideSweepFaceGroupOrdinal == STARSHIP_SPECIAL_FACE_GROUP_2 ||
							 g_collideSweepFaceGroupOrdinal == STARSHIP_SPECIAL_FACE_GROUP_3 ||
							 g_collideSweepFaceGroupOrdinal == STARSHIP_SPECIAL_FACE_GROUP_7))
							g_surfaceSpecialTargetHit = 1;
						g_collideSweepHitFraction = hitFraction;
						g_collideSweepHitMeshOrdinal = g_collideSweepCurrentMeshOrdinal;
					}
				}
			}
			break;
		}
	}
	if (node->childCount == 0)
		return 0;
	if (selectedChild != 0)
		return starship_TestSweepAgainstOptNode(object, node->pChildren[selectedChild - 1]);
	for (childIndex = 0; childIndex < node->childCount; ++childIndex) {
		if (starship_TestSweepAgainstOptNode(object, node->pChildren[childIndex]) != 0)
			return 1;
	}
	return 0;
}

// FUNCTION: XW 0x424770
int starship_IntersectSegmentWithFacePlane(const float* faceNormal, const float* faceVertex,
										   const float* segmentStart, const float* segmentEnd, float* outT) {
	float startX = segmentStart[STARSHIP_COORD_X] - faceVertex[STARSHIP_COORD_X];
	float startY = segmentStart[STARSHIP_COORD_Y] - faceVertex[STARSHIP_COORD_Y];
	float startZ = segmentStart[STARSHIP_COORD_Z] - faceVertex[STARSHIP_COORD_Z];
	float endX = segmentEnd[STARSHIP_COORD_X] - faceVertex[STARSHIP_COORD_X];
	float endY = segmentEnd[STARSHIP_COORD_Y] - faceVertex[STARSHIP_COORD_Y];
	float endZ = segmentEnd[STARSHIP_COORD_Z] - faceVertex[STARSHIP_COORD_Z];
	float startDistance = startX * faceNormal[STARSHIP_COORD_X] + startY * faceNormal[STARSHIP_COORD_Y];
	float endDistance = endX * faceNormal[STARSHIP_COORD_X] + endY * faceNormal[STARSHIP_COORD_Y];
	startDistance += startZ * faceNormal[STARSHIP_COORD_Z];
	endDistance += endZ * faceNormal[STARSHIP_COORD_Z];
	if (startDistance < g_collisionPlaneSnapPositiveLimit &&
		startDistance > g_collisionPlaneSnapNegativeLimit)
		startDistance = 0.0f;
	if (endDistance < g_collisionPlaneSnapPositiveLimit && endDistance > g_collisionPlaneSnapNegativeLimit)
		endDistance = 0.0f;
	if (startDistance == 0.0f) {
		*outT = 0.0f;
		return 1;
	}
	if (endDistance == 0.0f) {
		*outT = 1.0f;
		return 1;
	}
	if (startDistance < 0.0f && endDistance > 0.0f) {
		*outT = startDistance / (endDistance - startDistance);
		if (*outT < 0.0f)
			*outT = -*outT;
		return 1;
	}
	if (endDistance < 0.0f && startDistance > 0.0f) {
		*outT = startDistance / (startDistance - endDistance);
		if (*outT < 0.0f)
			*outT = -*outT;
		return 1;
	}
	return 0;
}

// FUNCTION: XW 0x424920
int32_t starship_PointInFacePolygon(const float* faceNormal, const float* vertexCoords,
									const int* faceVertexIndices, float* projectedPoint) {
	float absX;
	float absY;
	float absZ;
	int axisU;
	int axisV;
	int vertexBase;
	const float* vertex0U;
	const float* vertex0V;
	float vertexU;
	float vertexV;
	float previousU;
	float previousV;
	float edgeCross;
	int firstEdgeNegative;

	absX = faceNormal[STARSHIP_COORD_X];
	absY = faceNormal[STARSHIP_COORD_Y];
	absZ = faceNormal[STARSHIP_COORD_Z];
	if (absX < 0.0f)
		absX = -absX;
	if (absY < 0.0f)
		absY = -absY;
	if (absZ < 0.0f)
		absZ = -absZ;

	/* The projected pair occupies the original point's Y and Z slots. */
	if (absZ >= absY && absZ >= absX) {
		axisU = STARSHIP_COORD_X;
		axisV = STARSHIP_COORD_Y;
		projectedPoint[2] = projectedPoint[1];
		projectedPoint[1] = projectedPoint[0];
	} else if (absY >= absX && absY >= absZ) {
		axisU = STARSHIP_COORD_X;
		axisV = STARSHIP_COORD_Z;
		projectedPoint[1] = projectedPoint[0];
	} else {
		axisU = STARSHIP_COORD_Y;
		axisV = STARSHIP_COORD_Z;
	}

	vertex0U = &vertexCoords[axisU + STARSHIP_COORD_COUNT * faceVertexIndices[0]];
	vertex0V = &vertexCoords[axisV + STARSHIP_COORD_COUNT * faceVertexIndices[0]];
	vertexBase = STARSHIP_COORD_COUNT * faceVertexIndices[1];
	vertexU = vertexCoords[axisU + vertexBase];
	vertexV = vertexCoords[axisV + vertexBase];
	previousU = *vertex0U;
	previousV = *vertex0V;
	if ((projectedPoint[1] - previousU) * (vertexV - previousV) -
			(projectedPoint[2] - previousV) * (vertexU - previousU) <
		0.0f) {
		firstEdgeNegative = 1;
	} else {
		firstEdgeNegative = 0;
	}

	previousU = vertexU;
	previousV = vertexV;
	vertexBase = STARSHIP_COORD_COUNT * faceVertexIndices[2];
	vertexU = vertexCoords[axisU + vertexBase];
	vertexV = vertexCoords[axisV + vertexBase];
	edgeCross = (projectedPoint[1] - previousU) * (vertexV - previousV) -
				(projectedPoint[2] - previousV) * (vertexU - previousU);
	if (edgeCross < 0.0f && !firstEdgeNegative)
		return 0;
	if (edgeCross >= 0.0f && firstEdgeNegative)
		return 0;

	if (faceVertexIndices[3] != STARSHIP_FACE_VERTEX_NONE) {
		previousU = vertexU;
		previousV = vertexV;
		vertexBase = STARSHIP_COORD_COUNT * faceVertexIndices[3];
		vertexU = vertexCoords[axisU + vertexBase];
		vertexV = vertexCoords[axisV + vertexBase];
		edgeCross = (projectedPoint[1] - previousU) * (vertexV - previousV) -
					(projectedPoint[2] - previousV) * (vertexU - previousU);
		if (edgeCross < 0.0f && !firstEdgeNegative)
			return 0;
		if (edgeCross >= 0.0f && firstEdgeNegative)
			return 0;
	}

	previousU = vertexU;
	previousV = vertexV;
	vertexU = *vertex0U;
	vertexV = *vertex0V;
	edgeCross = (projectedPoint[1] - previousU) * (vertexV - previousV) -
				(projectedPoint[2] - previousV) * (vertexU - previousU);
	if (edgeCross < 0.0f && !firstEdgeNegative)
		return 0;
	if (edgeCross >= 0.0f && firstEdgeNegative)
		return 0;
	return 1;
}

// FUNCTION: XW 0x424C00
int starship_damagecomponent(uint16_t victimObjIdx, int16_t hitMeshIndex, uint16_t damageAmount) {
	uint16_t meshIndex, effectIndex;
	uint8_t componentHp;
	int meshSlot, scale;
	uint16_t effectSlot, victimSlot;
	int16_t centerX, centerY, centerZ, rotationSin, rotationCos;
	int rotatedSide, rotatedUp;
	uint16_t rotation, meshRotation;
	uint8_t objectType;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		return Dos94_starship_damagecomponent(victimObjIdx, hitMeshIndex, damageAmount);
	}
#endif
	if (hitMeshIndex == 0)
		return damageAmount;
	meshIndex = hitMeshIndex - 1;
	meshSlot = meshIndex;
	componentHp = g_curCraft->componentHp[meshSlot];
	if (componentHp == 0 || componentHp == STARSHIP_COMPONENT_HP_INDESTRUCTIBLE)
		return damageAmount;
	damageAmount >>= STARSHIP_COMPONENT_DAMAGE_SHIFT;
	if (componentHp <= damageAmount) {
		g_curCraft->componentHp[meshSlot] = 0;
		damageAmount = (damageAmount - componentHp) << STARSHIP_COMPONENT_DAMAGE_SHIFT;
		victimSlot = victimObjIdx;
		g_curCraft->componentState[meshSlot] = STARSHIP_COMPONENT_DESTROYED;
		objectType = g_objectTable[victimSlot].objectType;
		if (objectType == XW_OBJ_CORELLIAN_CORVETTE) {
			if (meshIndex == STARSHIP_DESTRUCTION_MESH_2) {
				g_curCraft->weaponSlots[0].firingGate = 0;
				g_curCraft->weaponSlots[1].firingGate = 0;
				g_curCraft->weaponSlots[0].laserCharge = 0;
				g_curCraft->weaponSlots[1].laserCharge = 0;
			} else if (meshIndex == STARSHIP_DESTRUCTION_MESH_0) {
				g_curCraft->weaponSlots[2].firingGate = 0;
				g_curCraft->weaponSlots[3].firingGate = 0;
				g_curCraft->weaponSlots[2].laserCharge = 0;
				g_curCraft->weaponSlots[3].laserCharge = 0;
			}
		} else if (objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER &&
				   g_curCraft->componentHp[STARSHIP_SHIELD_GENERATOR_FIRST] == 0 &&
				   g_curCraft->componentHp[STARSHIP_SHIELD_GENERATOR_SECOND] == 0) {
			g_curCraft->shieldEnergy[XW_SHIELD_FRONT] = 0;
			g_curCraft->workingSubsystems &= ~XW_CRAFT_SUBSYSTEM_SHIELDS;
		}
		effectIndex = create_findslot(XW_GENUS_EXPLOSION_EFFECT);
		if (effectIndex != XW_OBJECT_SLOT_UNAVAILABLE) {
			effectSlot = effectIndex;
			g_objectTable[effectSlot].worldX = g_objectTable[victimSlot].worldX;
			g_objectTable[effectSlot].worldY = g_objectTable[victimSlot].worldY;
			g_objectTable[effectSlot].worldZ = g_objectTable[victimSlot].worldZ;
			centerX = ModelMesh_GetCenterX(g_objectTable[victimSlot].objectType, meshSlot);
			centerY = ModelMesh_GetCenterY(g_objectTable[victimSlot].objectType, meshSlot);
			centerZ = ModelMesh_GetCenterZ(g_objectTable[victimSlot].objectType, meshSlot);
			if (g_missionRuntimeState.provingGroundsActive != 0 &&
				(meshRotation = g_curCraft->meshRotation[meshSlot]) != 0) {
				rotation = meshRotation << STARSHIP_MESH_ROTATION_SHIFT;
				rotationSin = trig2_getsignedsin(rotation);
				rotationCos = trig2_getsignedcos(rotation);
				rotatedSide = rotationCos * centerX - rotationSin * centerZ;
				if (rotatedSide >= STARSHIP_ROTATION_CLAMP_LIMIT)
					rotatedSide = STARSHIP_ROTATION_CLAMP_MAX;
				if (rotatedSide <= -STARSHIP_ROTATION_CLAMP_LIMIT)
					rotatedSide = STARSHIP_ROTATION_CLAMP_MIN;
				rotatedUp = rotationSin * centerX + rotationCos * centerZ;
				if (rotatedUp >= STARSHIP_ROTATION_CLAMP_LIMIT)
					rotatedUp = STARSHIP_ROTATION_CLAMP_MAX;
				if (rotatedUp <= -STARSHIP_ROTATION_CLAMP_LIMIT)
					rotatedUp = STARSHIP_ROTATION_CLAMP_MIN;
				centerX = rotatedSide >> STARSHIP_ROTATION_FRACTION_BITS;
				centerZ = rotatedUp >> STARSHIP_ROTATION_FRACTION_BITS;
			}
			pai_calcrotatedpoint(&g_objectTable[victimSlot], centerX, centerZ, -centerY);
#ifdef XW_MODERN
			g_objectTable[effectSlot].worldX =
				(int32_t)((uint32_t)g_objectTable[effectSlot].worldX + (uint32_t)g_rotatedX);
#else
			g_objectTable[effectSlot].worldX += g_rotatedX;
#endif
#ifdef XW_MODERN
			g_objectTable[effectSlot].worldY =
				(int32_t)((uint32_t)g_objectTable[effectSlot].worldY + (uint32_t)g_rotatedY);
#else
			g_objectTable[effectSlot].worldY += g_rotatedY;
#endif
#ifdef XW_MODERN
			g_objectTable[effectSlot].worldZ =
				(int32_t)((uint32_t)g_objectTable[effectSlot].worldZ + (uint32_t)g_rotatedZ);
#else
			g_objectTable[effectSlot].worldZ += g_rotatedZ;
#endif
			g_objectTable[effectSlot].objectType =
				XW_OBJ_EXPLOSION_133 + (math2_getrandom() & STARSHIP_COMPONENT_EXPLOSION_VARIANT_MASK);
			g_objectTable[effectSlot].genusId = XW_GENUS_EXPLOSION_EFFECT;
			g_objectTable[effectSlot].familyId = XW_OBJECT_FAMILY_5;
			g_objectTable[effectSlot].animationState = STARSHIP_COMPONENT_EXPLOSION_STATE;
			g_objectTable[effectSlot].ageSeconds = 0;
			g_objectTable[effectSlot].lifetimeTicks = 0;
			g_objectTable[effectSlot].speed = g_objectTable[victimSlot].speed;
			g_objectTable[effectSlot].pitch = g_objectTable[victimSlot].pitch;
			g_objectTable[effectSlot].yaw = g_objectTable[victimSlot].yaw;
			g_objectTable[effectSlot].roll = 0;
			g_objectTable[effectSlot].orientMatrixDirty = 1;
			g_objectTable[effectSlot].moveVectorDirty = 1;
			fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, effectIndex);
			scale = ModelMesh_GetComponentMaxExtent(g_objectTable[victimSlot].objectType, meshSlot) >>
					STARSHIP_COMPONENT_EXTENT_SHIFT;
			if ((int16_t)scale > STARSHIP_COMPONENT_SCALE_MAX)
				scale = STARSHIP_COMPONENT_SCALE_MAX;
			g_objectTable[effectSlot].billboardScaleCode = scale;
		}
	} else {
		g_curCraft->componentHp[meshSlot] = componentHp - damageAmount;
		damageAmount = 0;
	}
	return damageAmount;
}

// FUNCTION: XW 0x424FD0
void starship_createstarshipexplo__partial(uint16_t objectIdx) {
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_starship_createstarshipexplo__partial(objectIdx);
		return;
	}
#endif
	if ((uint16_t)math2_getrandom() < g_craftExplosionSpawnThreshold) {
		ObjectRecord* objectRecord = &g_objectTable[objectIdx];
		uint16_t objectType;
		uint16_t meshSelectionRoll;
		uint16_t meshIndex;
		uint16_t effectObjectIdx;
		unsigned int effectSize;
		g_curCraft = (CraftData*)objectRecord->instanceData;
		objectType = objectRecord->objectType;
		if (objectRecord->orientMatrixDirty != 0)
			fview_newcalcrotate(objectRecord->roll, objectRecord->pitch, objectRecord->yaw, 0, objectRecord);
		meshSelectionRoll = math2_getrandom();
		if (objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER) {
			if (meshSelectionRoll < STARSHIP_RECURRING_EXPLOSION_FIRST_THRESHOLD)
				meshIndex = STARSHIP_RECURRING_EXPLOSION_FIRST_MESH;
			else
				meshIndex = meshSelectionRoll < STARSHIP_RECURRING_EXPLOSION_SECOND_THRESHOLD
								? STARSHIP_RECURRING_EXPLOSION_SECOND_MESH
								: STARSHIP_RECURRING_EXPLOSION_THIRD_MESH;
		} else {
			meshIndex = meshSelectionRoll % ModelMesh_GetCachedObjectTypeMeshCount(objectType);
		}
		if (g_curCraft->componentHp[meshIndex] != 0) {
			effectSize =
				g_modelTypeTable[objectType].maxBoundsExtent >> STARSHIP_COMPONENT_EXPLOSION_SCALE_SHIFT;
			effectObjectIdx = starship_makestarshipcompexplo(objectRecord, meshIndex, effectSize,
															 math2_getrandom() &
																 STARSHIP_RECURRING_EXPLOSION_HARDPOINT_MASK);
			if (effectObjectIdx != XW_OBJECT_SLOT_UNAVAILABLE)
				fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, effectObjectIdx);
		}
	}
}

// FUNCTION: XW 0x4250E0
int starship_makestarshipcompexplo(struct ObjectRecord* object, uint16_t meshIndex, unsigned int screenScale,
								   int chooseRandomHardpoint) {
	uint16_t objectType = object->objectType;
	int16_t localSide;
	int16_t localForward;
	int16_t localUp;
	uint16_t effectIndex;
	if ((uint16_t)chooseRandomHardpoint == 0) {
		localSide = ModelMesh_GetCenterX(objectType, meshIndex);
		localForward = ModelMesh_GetCenterY(objectType, meshIndex);
		localUp = ModelMesh_GetCenterZ(objectType, meshIndex);
	} else {
		int hardpointCount = ModelMesh_CountHardpoints(objectType, meshIndex);
		chooseRandomHardpoint = (uint16_t)math2_getrandom() % hardpointCount;
		localSide = ModelMesh_GetVertexX(objectType, meshIndex, (uint16_t)chooseRandomHardpoint);
		localForward = ModelMesh_GetVertexY(objectType, meshIndex, (uint16_t)chooseRandomHardpoint);
		localUp = ModelMesh_GetVertexZ(objectType, meshIndex, (uint16_t)chooseRandomHardpoint);
	}
	pai_calcrotatedpoint(object, localSide, localUp, -localForward);
	effectIndex = create_findslot(XW_GENUS_EXPLOSION_EFFECT);
	if (effectIndex == XW_OBJECT_SLOT_UNAVAILABLE)
		return XW_OBJECT_SLOT_UNAVAILABLE;
#ifdef XW_MODERN
	g_objectTable[effectIndex].worldX = (int32_t)((uint32_t)object->worldX + (uint32_t)g_rotatedX);
#else
	g_objectTable[effectIndex].worldX = object->worldX + g_rotatedX;
#endif
#ifdef XW_MODERN
	g_objectTable[effectIndex].worldY = (int32_t)((uint32_t)object->worldY + (uint32_t)g_rotatedY);
#else
	g_objectTable[effectIndex].worldY = object->worldY + g_rotatedY;
#endif
#ifdef XW_MODERN
	g_objectTable[effectIndex].worldZ = (int32_t)((uint32_t)object->worldZ + (uint32_t)g_rotatedZ);
#else
	g_objectTable[effectIndex].worldZ = object->worldZ + g_rotatedZ;
#endif
	if ((uint16_t)chooseRandomHardpoint == 0)
		g_objectTable[effectIndex].objectType = XW_OBJ_EXPLOSION_135;
	else
		g_objectTable[effectIndex].objectType =
			XW_OBJ_EXPLOSION_133 + (math2_getrandom() & STARSHIP_COMPONENT_EXPLOSION_VARIANT_MASK);
	g_objectTable[effectIndex].genusId = XW_GENUS_EXPLOSION_EFFECT;
	g_objectTable[effectIndex].familyId = XW_OBJECT_FAMILY_5;
	g_objectTable[effectIndex].animationState = STARSHIP_COMPONENT_EXPLOSION_STATE;
	g_objectTable[effectIndex].ageSeconds = 0;
	g_objectTable[effectIndex].lifetimeTicks = 0;
	g_objectTable[effectIndex].billboardScaleCode = screenScale >> STARSHIP_COMPONENT_EXPLOSION_SCALE_SHIFT;
	g_objectTable[effectIndex].speed = 0;
	g_objectTable[effectIndex].pitch = 0;
	g_objectTable[effectIndex].yaw = 0;
	g_objectTable[effectIndex].roll = 0;
	g_objectTable[effectIndex].orientMatrixDirty = 1;
	g_objectTable[effectIndex].moveVectorDirty = 1;
	return effectIndex;
}

// FUNCTION: XW 0x425280
void starship_createstarshipexplo(uint16_t objectIndex) {
	ObjectRecord* object;
	uint16_t objectType;
	uint16_t slot;
	int slotsRemaining;
	uint16_t meshIndex;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos()) {
		Dos94_starship_createstarshipexplo(objectIndex);
		return;
	}
#endif
	object = &g_objectTable[objectIndex];
	g_curCraft = (CraftData*)object->instanceData;
	objectType = object->objectType;
	if (object->orientMatrixDirty != 0)
		fview_newcalcrotate(object->roll, object->pitch, object->yaw, 0, object);
	slot = g_objectSlotRangeByGenus[XW_GENUS_EXPLOSION_EFFECT].start;
	for (slotsRemaining = STARSHIP_DESTRUCTION_CLEAR_COUNT; slotsRemaining != 0; --slotsRemaining, ++slot)
		g_objectTable[(uint16_t)slot].objectType = XW_OBJ_NONE;
	if (objectType == XW_OBJ_CORELLIAN_CORVETTE) {
		for (meshIndex = 0; meshIndex < STARSHIP_CORVETTE_EXPLOSION_MESH_COUNT; ++meshIndex)
			starship_makestarshipcompexplo(object, meshIndex, STARSHIP_CORVETTE_EXPLOSION_SCALE,
										   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
	} else if (objectType == XW_OBJ_NEBULON_B_FRIGATE) {
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_0, STARSHIP_CORVETTE_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_2,
									   STARSHIP_FRIGATE_MESH_2_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_3,
									   STARSHIP_FRIGATE_MESH_3_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
	} else if (objectType == XW_OBJ_IMPERIAL_STAR_DESTROYER) {
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_0,
									   STARSHIP_DESTROYER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_5,
									   STARSHIP_DESTROYER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_12,
									   STARSHIP_DESTROYER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_22,
									   STARSHIP_DESTROYER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_HARDPOINT_ROLL_1);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_22,
									   STARSHIP_DESTROYER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_HARDPOINT_ROLL_2);
	} else if (objectType == XW_OBJ_CALAMARI_CRUISER) {
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_0, STARSHIP_CRUISER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_1, STARSHIP_CRUISER_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
	} else {
		starship_makestarshipcompexplo(object, STARSHIP_DESTRUCTION_MESH_0, STARSHIP_DEFAULT_EXPLOSION_SCALE,
									   STARSHIP_DESTRUCTION_RANDOM_HARDPOINT);
	}
	fsfx_triggersfx(STARSHIP_DESTRUCTION_SOUND, objectIndex);
}

// FUNCTION: XW 0x425420
void starship_firelasergunner(uint16_t sourceObjIdx, uint16_t weaponSlotIdx, uint16_t targetRef) {
	int weaponSlotIndex;
	ObjectRecord* sourceObject;
	int8_t cooldownAndProjectileFlags;
	uint8_t cooldown;
	uint16_t aiSkillQ16;
	uint16_t cooldownStep;
	uint16_t craftType;
	int16_t hardpointX, hardpointY, hardpointZ;
	int muzzleWorldX, muzzleWorldY, muzzleWorldZ;
	int targetIndex;
	int targetDeltaX, targetDeltaY, targetDeltaZ;
	int targetLocalUp, targetLocalSide, targetLocalForward;
	int dot;
	uint8_t targetYawByte, firingArcFlags;
	uint16_t yawDelta, pitchDelta, maxYawDelta, maxPitchDelta;
	int16_t pitchArcCenter;
	uint16_t baseLeadSteps, leadSteps;
	uint16_t leadRandom;
	int16_t shotYaw;
	uint16_t shotPitch;
	uint16_t projectileObjectIndex, projectileType;
	int projectileIndex, projectileDataIndex;
	int launchX, launchY, launchZ;
	int guidanceIndex;
	if (g_curCraft->workingSubsystems == 0)
		return;
	weaponSlotIndex = weaponSlotIdx;
	sourceObject = &g_objectTable[sourceObjIdx];
	cooldownAndProjectileFlags = g_curCraft->weaponSlots[weaponSlotIndex].laserCharge;
	aiSkillQ16 = g_curCraft->aiSkillQ16;
	cooldown = cooldownAndProjectileFlags & STARSHIP_GUN_COOLDOWN_MASK;
	if (aiSkillQ16 >= STARSHIP_GUN_HIGH_SKILL)
		cooldownStep = g_elapsedTicks >> STARSHIP_GUN_HIGH_SKILL_SHIFT;
	else if (aiSkillQ16 >= STARSHIP_GUN_MEDIUM_SKILL)
		cooldownStep = g_elapsedTicks >> STARSHIP_GUN_MEDIUM_SKILL_SHIFT;
	else
		cooldownStep = g_elapsedTicks / STARSHIP_GUN_LOW_SKILL_DIVISOR;
	if (cooldown > cooldownStep) {
		g_curCraft->weaponSlots[weaponSlotIndex].laserCharge =
			(int8_t)(cooldownAndProjectileFlags - cooldownStep);
		return;
	}
	g_curCraft->weaponSlots[weaponSlotIndex].laserCharge =
		cooldownAndProjectileFlags & STARSHIP_GUN_ALTERNATE_PROJECTILE;
	g_curCraft->weaponSlots[weaponSlotIndex].laserCharge |= STARSHIP_GUN_COOLDOWN;
	muzzleWorldY = sourceObject->worldY;
	muzzleWorldZ = sourceObject->worldZ;
	muzzleWorldX = sourceObject->worldX;
	craftType = g_curCraft->craftTypeIndex;
	hardpointZ = g_craftTypeDefs[craftType].weaponHardpoints[weaponSlotIdx].z;
	hardpointX = g_craftTypeDefs[craftType].weaponHardpoints[weaponSlotIdx].x;
	hardpointY = g_craftTypeDefs[craftType].weaponHardpoints[weaponSlotIdx].y;
	pai_calcrotatedpoint(sourceObject, hardpointX, hardpointZ, hardpointY);
	muzzleWorldX = (int)((uint32_t)muzzleWorldX + g_rotatedX);
	muzzleWorldZ = (int)((uint32_t)muzzleWorldZ + g_rotatedZ);
	muzzleWorldY = (int)((uint32_t)muzzleWorldY + g_rotatedY);
	targetIndex = targetRef;
	targetDeltaX = (int)((uint32_t)g_objectTable[targetIndex].worldX - muzzleWorldX);
	targetDeltaY = (int)((uint32_t)g_objectTable[targetIndex].worldY - muzzleWorldY);
	targetDeltaZ = (int)((uint32_t)g_objectTable[targetIndex].worldZ - muzzleWorldZ);
	if ((unsigned int)collide_roughdistance3d(targetDeltaX, targetDeltaY, targetDeltaZ) >
		STARSHIP_GUN_MAX_RANGE)
		return;
#ifdef XW_MODERN
	dot = (int)((int16_t)targetDeltaX * (uint32_t)sourceObject->cachedUpX +
				(int16_t)targetDeltaY * (uint32_t)sourceObject->cachedUpY +
				(int16_t)targetDeltaZ * (uint32_t)sourceObject->cachedUpZ);
#else
	dot = (int16_t)targetDeltaX * sourceObject->cachedUpX + (int16_t)targetDeltaY * sourceObject->cachedUpY +
		  (int16_t)targetDeltaZ * sourceObject->cachedUpZ;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	targetLocalUp = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int)((int16_t)targetDeltaX * (uint32_t)sourceObject->cachedSideX +
				(int16_t)targetDeltaY * (uint32_t)sourceObject->cachedSideY +
				(int16_t)targetDeltaZ * (uint32_t)sourceObject->cachedSideZ);
#else
	dot = (int16_t)targetDeltaX * sourceObject->cachedSideX +
		  (int16_t)targetDeltaY * sourceObject->cachedSideY +
		  (int16_t)targetDeltaZ * sourceObject->cachedSideZ;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	targetLocalSide = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
	dot = (int)((int16_t)targetDeltaX * (uint32_t)sourceObject->cachedForwardX +
				(int16_t)targetDeltaY * (uint32_t)sourceObject->cachedForwardY +
				(int16_t)targetDeltaZ * (uint32_t)sourceObject->cachedForwardZ);
#else
	dot = (int16_t)targetDeltaX * sourceObject->cachedForwardX +
		  (int16_t)targetDeltaY * sourceObject->cachedForwardY +
		  (int16_t)targetDeltaZ * sourceObject->cachedForwardZ;
#endif
	if (dot >= FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MAX;
	if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
		dot = FVIEW_DOT_CLAMP_MIN;
	targetLocalForward = dot >> FVIEW_MATRIX_FRACTION_BITS;
	trig2_ctop((int16_t)targetLocalSide - hardpointX, (int16_t)targetLocalForward - hardpointY,
			   (int16_t)targetLocalUp - hardpointZ);
	targetYawByte = (uint8_t)((uint16_t)g_trig2Yaw >> STARSHIP_GUN_ANGLE_BYTE_SHIFT);
	firingArcFlags = g_craftTypeDefs[craftType].weaponHardpoints[weaponSlotIdx].firingArcOrAmmoCount;
	yawDelta = (uint16_t)(g_trig2Yaw -
						  ((firingArcFlags & STARSHIP_GUN_ARC_FIELD_MASK) << STARSHIP_GUN_ARC_CENTER_SHIFT));
	firingArcFlags >>= STARSHIP_GUN_ARC_FIELD_BITS;
	maxYawDelta = (uint16_t)(((firingArcFlags & STARSHIP_GUN_ARC_FIELD_MASK) + 1)
							 << STARSHIP_GUN_ARC_WIDTH_SHIFT);
	firingArcFlags >>= STARSHIP_GUN_ARC_FIELD_BITS;
	pitchArcCenter = (int16_t)((firingArcFlags & STARSHIP_GUN_ARC_FIELD_MASK)
							  << STARSHIP_GUN_ARC_CENTER_SHIFT);
	firingArcFlags >>= STARSHIP_GUN_ARC_FIELD_BITS;
	maxPitchDelta = (uint16_t)(((firingArcFlags & STARSHIP_GUN_ARC_FIELD_MASK) + 1)
							   << STARSHIP_GUN_ARC_WIDTH_SHIFT);
	if (yawDelta >= STARSHIP_GUN_HALF_TURN)
		yawDelta = (uint16_t)-yawDelta;
	pitchDelta = (uint16_t)(g_trig2Pitch - pitchArcCenter);
	if (pitchDelta >= STARSHIP_GUN_HALF_TURN)
		pitchDelta = (uint16_t)-pitchDelta;
	if (yawDelta > maxYawDelta || pitchDelta > maxPitchDelta)
		return;
	trig2_ctop((int)((uint32_t)g_objectTable[targetIndex].worldX - muzzleWorldX),
			   (int)((uint32_t)g_objectTable[targetIndex].worldY - muzzleWorldY),
			   (int)((uint32_t)g_objectTable[targetIndex].worldZ - muzzleWorldZ));
#ifdef XW_MODERN
	g_trig2PolarDistance = (int)((uint32_t)g_simStepScale * g_trig2PolarDistance) >> STARSHIP_GUN_LEAD_SHIFT;
#else
	g_trig2PolarDistance = (g_simStepScale * g_trig2PolarDistance) >> STARSHIP_GUN_LEAD_SHIFT;
#endif
	baseLeadSteps = (uint16_t)g_trig2PolarDistance;
	leadRandom = math2_getrandom();
	leadSteps = (uint16_t)math2_fraction(
		(uint16_t)(baseLeadSteps + (leadRandom & STARSHIP_GUN_LEAD_RANDOM_MASK) - 1), g_curCraft->aiSkillQ16);
#ifdef XW_MODERN
	trig2_ctop((int)((uint32_t)g_objectTable[targetIndex].worldX +
					 leadSteps * (uint32_t)XwReferenceMotion_Axis(targetIndex, 0) - muzzleWorldX),
			   (int)((uint32_t)g_objectTable[targetIndex].worldY +
					 leadSteps * (uint32_t)XwReferenceMotion_Axis(targetIndex, 1) - muzzleWorldY),
			   (int)((uint32_t)g_objectTable[targetIndex].worldZ +
					 leadSteps * (uint32_t)XwReferenceMotion_Axis(targetIndex, 2) - muzzleWorldZ));
#else
	trig2_ctop((int)((uint32_t)g_objectTable[targetIndex].worldX +
					 leadSteps * ((uint32_t)g_objectTable[targetIndex].worldX -
								  g_objectTable[targetIndex].prevWorldX) -
					 muzzleWorldX),
			   (int)((uint32_t)g_objectTable[targetIndex].worldY +
					 leadSteps * ((uint32_t)g_objectTable[targetIndex].worldY -
								  g_objectTable[targetIndex].prevWorldY) -
					 muzzleWorldY),
			   (int)((uint32_t)g_objectTable[targetIndex].worldZ +
					 leadSteps * ((uint32_t)g_objectTable[targetIndex].worldZ -
								  g_objectTable[targetIndex].prevWorldZ) -
					 muzzleWorldZ));
#endif
	shotYaw = g_trig2Yaw;
	shotPitch = (uint16_t)g_trig2Pitch;
#ifdef XW_MODERN
	if (sourceObject->objectType == XwFlightTypes_ObjectType(XW_OBJ_CORELLIAN_CORVETTE)) {
#else
	if (sourceObject->objectType == XW_OBJ_CORELLIAN_CORVETTE) {
#endif
		uint8_t meshYaw = (uint8_t)(STARSHIP_GUN_MESH_HALF_TURN - targetYawByte);
#ifdef XW_MODERN
		if (XwFlightTypes_Dos())
			g_curCraft->meshRotation[Dos94_corvetteguncomponent(pitchArcCenter != 0)] = meshYaw;
		else
#endif
			if (pitchArcCenter != 0)
			g_curCraft->meshRotation[STARSHIP_GUN_CORVETTE_SECOND_MESH] = meshYaw;
		else
			g_curCraft->meshRotation[STARSHIP_GUN_CORVETTE_FIRST_MESH] = meshYaw;
	}
	if ((uint16_t)math2_getrandom() > 0xFFFF) {
		int16_t scatter = ((uint16_t)math2_getrandom() - 0x100) & 0x3FF;
		if ((uint16_t)math2_getrandom() >= STARSHIP_GUN_HALF_TURN)
			scatter = -scatter;
		shotYaw += scatter;
		scatter = ((uint16_t)math2_getrandom() - 0x100) & 0x3FF;
		if ((uint16_t)math2_getrandom() >= STARSHIP_GUN_HALF_TURN)
			shotPitch -= scatter;
		else
			shotPitch += scatter;
	}
	projectileObjectIndex = create_findslot(XW_GENUS_OTHER_PROJECTILE);
	if (projectileObjectIndex == XW_OBJECT_SLOT_UNAVAILABLE)
		return;
	projectileIndex = projectileObjectIndex;
	g_objectTable[projectileIndex].familyId = LASER_PROJECTILE_FAMILY;
	g_objectTable[projectileIndex].genusId = XW_GENUS_OTHER_PROJECTILE;
	if (sourceObject->iff == 0) {
		g_objectTable[projectileIndex].iff = 0;
#ifdef XW_MODERN
		projectileType = XwFlightTypes_ObjectType(XW_OBJ_LASER_144);
#else
		projectileType = XW_OBJ_LASER_144;
#endif
	} else {
		g_objectTable[projectileIndex].iff = 1;
#ifdef XW_MODERN
		projectileType = XwFlightTypes_ObjectType(XW_OBJ_LASER_146);
#else
		projectileType = XW_OBJ_LASER_146;
#endif
	}
	g_objectTable[projectileIndex].objectType = (uint8_t)projectileType;
	if (g_curCraft->weaponSlots[weaponSlotIndex].laserCharge & STARSHIP_GUN_ALTERNATE_PROJECTILE) {
		++g_objectTable[projectileIndex].objectType;
		++projectileType;
	}
	g_objectTable[projectileIndex].ageSeconds = LASER_PROJECTILE_INITIAL_AGE;
	g_objectTable[projectileIndex].sourceObjectRef = sourceObjIdx;
	g_objectTable[projectileIndex].sourceObjectType = sourceObject->objectType;
	g_objectTable[projectileIndex].pitch = (int16_t)shotPitch;
#ifdef XW_MODERN
	projectileDataIndex = XwFlightTypes_CanonicalType(projectileType) - LASER_PROJECTILE_FIRST_TYPE;
#else
	projectileDataIndex = projectileType - LASER_PROJECTILE_FIRST_TYPE;
#endif
	g_objectTable[projectileIndex].roll = 0;
	g_objectTable[projectileIndex].yaw = shotYaw;
	g_objectTable[projectileIndex].orientMatrixDirty = 1;
	g_objectTable[projectileIndex].speed = g_projectileSpeedByType[projectileDataIndex];
	g_objectTable[projectileIndex].moveVectorDirty = 1;
	g_objectTable[projectileIndex].damageAmount = g_projectileBaseDamageByType[projectileDataIndex];
	g_objectTable[projectileIndex].lifetimeTicks =
		(int16_t)(STARSHIP_GUN_LIFETIME_MULTIPLIER * XW_SIMULATION_TICKS_PER_SECOND *
				  g_projectileLifetimeSecondsByType[projectileDataIndex]);
	fview_calcrotatemove((int16_t)shotPitch, shotYaw, &g_objectTable[projectileIndex]);
	g_objectTable[projectileIndex].prevWorldX = muzzleWorldX;
	g_objectTable[projectileIndex].prevWorldY = muzzleWorldY;
	g_objectTable[projectileIndex].prevWorldZ = muzzleWorldZ;
	launchX = (int)((uint64_t)((int64_t)g_projectileLaunchOffsetByType[projectileDataIndex] * g_craftMoveX) >>
					LASER_LAUNCH_BASIS_SHIFT);
	launchY = (int)((uint64_t)((int64_t)g_projectileLaunchOffsetByType[projectileDataIndex] * g_craftMoveZ) >>
					LASER_LAUNCH_BASIS_SHIFT);
	launchZ = (int)((uint64_t)((int64_t)g_projectileLaunchOffsetByType[projectileDataIndex] * g_craftMoveY) >>
					LASER_LAUNCH_BASIS_SHIFT);
	g_objectTable[projectileIndex].worldX = (int)((uint32_t)muzzleWorldX + launchX);
	g_objectTable[projectileIndex].worldY = (int)((uint32_t)muzzleWorldY + launchY);
	g_objectTable[projectileIndex].worldZ = (int)((uint32_t)muzzleWorldZ + launchZ);
	guidanceIndex = (uint16_t)(projectileObjectIndex - XW_CRAFT_OBJECT_COUNT);
	g_objectTable[projectileIndex].instanceData = &g_warheadGuidanceTable[guidanceIndex];
	g_warheadGuidanceTable[guidanceIndex].homingTier = 0;
	g_warheadGuidanceTable[guidanceIndex].targetObjIdx = 0;
	fsfx_triggerlasersfx(projectileObjectIndex);
}
