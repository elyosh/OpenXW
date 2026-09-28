#include "xw/flight/object/static.h"

#ifdef XW_MODERN
#include "xw_runtime/timing/flight_integration.h"
#include "xw_runtime/timing/reference_motion.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/snapshot/render_objects.h"
#endif

#ifdef XW_MODERN
#include "xw_dos94/flight/object/collision.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/flight/fview.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/collide.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/laser.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_scene.h"

#include <stdlib.h>

// FUNCTION: XW 0x425B10
void static_drawstaticobject(int missionObjectIndex) {
	int staticSlot = (uint16_t)missionObjectIndex;
	const uint16_t* animationFrames =
		g_modelTypeTable[g_missionObjects[staticSlot].objectType].animationFrames;
	if (animationFrames == NULL) {
		uint16_t state = g_missionObjects[staticSlot].stateByte;
		g_billboardObjectOrTypeIndex = missionObjectIndex + XW_MISSION_OBJECT_REF_BASE;
		if (state == 0) {
			RenderScene_QueueCraftDamageBillboards(missionObjectIndex + XW_MISSION_OBJECT_REF_BASE);
			RenderScene_DrawObjectModel(&g_missionObjects[staticSlot]);
		}
	} else {
		int16_t frameCode;
		uint16_t state = g_missionObjects[staticSlot].stateByte;
		g_billboardObjectOrTypeIndex = missionObjectIndex + XW_MISSION_OBJECT_REF_BASE;
		frameCode = animationFrames[state];
		if ((uint16_t)frameCode < ANIM_FRAME_JUMP_BASE) {
			if ((uint16_t)frameCode < ANIM_BITMAP_FIRST_FRAME) {
				RenderScene_DrawNoAssetSourceModel((const ObjectRecord*)&g_missionObjects[staticSlot], 0);
			} else if (g_objectViewZ >= 0) {
				int absRow0Z = g_objViewMat_R0_Z;
				int absRow1Z = g_objViewMat_R1_Z;
				int axisX, axisY;
				int angle;
				int16_t rotationAngle, screenX;
				int projectedX, projectedY, screenHigh;
				if (absRow0Z < 0)
					absRow0Z = (int32_t)(0u - (uint32_t)absRow0Z);
				if (absRow1Z < 0)
					absRow1Z = (int32_t)(0u - (uint32_t)absRow1Z);
				if (absRow0Z < absRow1Z) {
					axisX = g_objViewMat_R0_X;
					axisY = g_objViewMat_R0_Y;
				} else {
					axisX = g_objViewMat_R1_X;
					axisY = g_objViewMat_R1_Y;
				}
				if (axisX < 0)
					angle = trig2_arctan(axisY, (int32_t)(0u - (uint32_t)axisX));
				else
					angle = -trig2_arctan(axisY, axisX);
				rotationAngle = angle;
				projectedX = transfm2_getscreencoordx(g_objectViewX, g_objectViewZ);
				screenX = projectedX;
				screenHigh = projectedX & RENDER_DAMAGE_SCREEN_HIGH_MASK;
				if (screenHigh <= 0 && screenHigh >= RENDER_DAMAGE_SCREEN_HIGH_MASK) {
					projectedY = transfm2_getscreencoordy(g_objectViewY, g_objectViewZ);
					screenHigh = projectedY & RENDER_DAMAGE_SCREEN_HIGH_MASK;
					if (screenHigh <= 0 && screenHigh >= RENDER_DAMAGE_SCREEN_HIGH_MASK) {
						unsigned int halfHeight = g_flightVpHeight >> 1;
						anim_add_bitmap_draw(g_billboardObjectOrTypeIndex, frameCode, ANIM_BITMAP_UNIT_SCALE,
											 screenX, 2 * halfHeight - projectedY, g_objectViewZ,
											 rotationAngle);
					}
				}
			}
		}
	}
}

// FUNCTION: XW 0x425CA0
int16_t static_laserstaticcollide(uint16_t sourceObjIdx, uint16_t missionObjectIndex) {
	int staticSlot;
	int16_t genusId;
	int objectType;
	int staticWorldX;
	int staticWorldY;
	int staticWorldZ;
	int endRelativeX;
	int endRelativeY;
	int endRelativeZ;
	int startRelativeX;
	int startRelativeY;
	int startRelativeZ;
	uint16_t maxBoundsExtent;
#ifdef XW_MODERN
	if (XwFlightTypes_Dos())
		return Dos94_static_laserstaticcollide(sourceObjIdx, missionObjectIndex);
#endif
	if (g_objectTable[sourceObjIdx].sourceObjectRef == missionObjectIndex + XW_MISSION_OBJECT_REF_BASE)
		return 0;
	staticSlot = missionObjectIndex;
	genusId = g_missionObjects[staticSlot].genusId;
	objectType = g_missionObjects[staticSlot].objectType;
	if (genusId == XW_GENUS_SCENERY)
		return 0;
	if (genusId == XW_GENUS_DEBRIS)
		return 0;
	if ((genusId == XW_GENUS_MINE || genusId == XW_GENUS_BUOY_SATELLITE_PROBE) &&
		g_objectTable[sourceObjIdx].iff == STATIC_COLLISION_IGNORED_IFF)
		return 0;
	create_getworldposition((uint16_t)(missionObjectIndex + XW_MISSION_OBJECT_REF_BASE), 0);
	staticWorldY = g_resolvedWorldY;
	staticWorldZ = g_resolvedWorldZ;
	staticWorldX = g_resolvedWorldX;
#ifdef XW_MODERN
	endRelativeX = (int32_t)((uint32_t)g_collisionProbeWorldX - (uint32_t)staticWorldX);
	endRelativeY = (int32_t)((uint32_t)g_collisionProbeWorldY - (uint32_t)staticWorldY);
	endRelativeZ = (int32_t)((uint32_t)g_collisionProbeWorldZ - (uint32_t)staticWorldZ);
#else
	endRelativeY = g_collisionProbeWorldY - staticWorldY;
	endRelativeZ = g_collisionProbeWorldZ - staticWorldZ;
	endRelativeX = g_collisionProbeWorldX - staticWorldX;
#endif
	if ((unsigned int)collide_roughdistance3d(endRelativeX, endRelativeY, endRelativeZ) >
		STATIC_COLLISION_MAX_DISTANCE)
		return 0;
#ifdef XW_MODERN
	startRelativeX = (int32_t)((uint32_t)g_collisionSegmentStartWorldX - (uint32_t)staticWorldX);
	startRelativeY = (int32_t)((uint32_t)g_collisionSegmentStartWorldY - (uint32_t)staticWorldY);
	startRelativeZ = (int32_t)((uint32_t)g_collisionSegmentStartWorldZ - (uint32_t)staticWorldZ);
#else
	startRelativeX = g_collisionSegmentStartWorldX - staticWorldX;
	startRelativeY = g_collisionSegmentStartWorldY - staticWorldY;
	startRelativeZ = g_collisionSegmentStartWorldZ - staticWorldZ;
#endif
	if ((unsigned int)collide_roughdistance3d(startRelativeX, startRelativeY, startRelativeZ) >
		STATIC_COLLISION_MAX_DISTANCE)
		return 0;
	maxBoundsExtent = g_modelTypeTable[objectType].maxBoundsExtent;
	if (maxBoundsExtent > STATIC_COLLISION_LARGE_EXTENT) {
		int16_t staticYaw;
		int16_t staticPitch;
		int16_t staticRoll;
		int rotatedSide;
		int rotatedUp;
		int rotatedForward;
		int dot;
		int16_t boundsMinX;
		int16_t boundsMinY;
		int16_t boundsMinZ;
		int16_t boundsMaxX;
		int16_t boundsMaxY;
		int16_t boundsMaxZ;
		g_collisionScratchPoint1X = endRelativeX;
		g_collisionScratchPoint1Y = endRelativeY;
		g_collisionScratchPoint1Z = endRelativeZ;
		g_collisionScratchPoint2X = startRelativeX;
		g_collisionScratchPoint2Y = startRelativeY;
		g_collisionScratchPoint2Z = startRelativeZ;
		staticYaw = (int16_t)(g_missionObjects[staticSlot].yawAngle8 << STATIC_COLLISION_ANGLE_SHIFT);
		staticPitch = (int16_t)(g_missionObjects[staticSlot].pitchAngle8 << STATIC_COLLISION_ANGLE_SHIFT);
		staticRoll = (int16_t)(g_missionObjects[staticSlot].rollAngle8 << STATIC_COLLISION_ANGLE_SHIFT);
		fview_calcrotatemove(staticPitch, staticYaw, NULL);
		fview_calcrotateorient(staticRoll, 0, NULL);
		g_fviewForwardX_Q15 = -g_fviewForwardX_Q15;
		g_fviewForwardY_Q15 = -g_fviewForwardY_Q15;
		g_fviewForwardZ_Q15 = -g_fviewForwardZ_Q15;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_fviewSideX_Q15 * (uint32_t)g_collisionScratchPoint1X +
						(uint32_t)g_fviewSideY_Q15 * (uint32_t)g_collisionScratchPoint1Y +
						(uint32_t)g_fviewSideZ_Q15 * (uint32_t)g_collisionScratchPoint1Z);
#else
		dot = g_fviewSideX_Q15 * g_collisionScratchPoint1X + g_fviewSideY_Q15 * g_collisionScratchPoint1Y +
			  g_fviewSideZ_Q15 * g_collisionScratchPoint1Z;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		rotatedSide = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_fviewUpX_Q15 * (uint32_t)g_collisionScratchPoint1X +
						(uint32_t)g_fviewUpY_Q15 * (uint32_t)g_collisionScratchPoint1Y +
						(uint32_t)g_fviewUpZ_Q15 * (uint32_t)g_collisionScratchPoint1Z);
#else
		dot = g_fviewUpX_Q15 * g_collisionScratchPoint1X + g_fviewUpY_Q15 * g_collisionScratchPoint1Y +
			  g_fviewUpZ_Q15 * g_collisionScratchPoint1Z;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		rotatedUp = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_fviewForwardX_Q15 * (uint32_t)g_collisionScratchPoint1X +
						(uint32_t)g_fviewForwardY_Q15 * (uint32_t)g_collisionScratchPoint1Y +
						(uint32_t)g_fviewForwardZ_Q15 * (uint32_t)g_collisionScratchPoint1Z);
#else
		dot = g_fviewForwardX_Q15 * g_collisionScratchPoint1X +
			  g_fviewForwardY_Q15 * g_collisionScratchPoint1Y +
			  g_fviewForwardZ_Q15 * g_collisionScratchPoint1Z;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		rotatedForward = dot >> FVIEW_MATRIX_FRACTION_BITS;
		g_collisionScratchPoint1X = (int16_t)rotatedSide;
		g_collisionScratchPoint1Y = (int16_t)rotatedUp;
		g_collisionScratchPoint1Z = (int16_t)rotatedForward;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_fviewSideX_Q15 * (uint32_t)g_collisionScratchPoint2X +
						(uint32_t)g_fviewSideY_Q15 * (uint32_t)g_collisionScratchPoint2Y +
						(uint32_t)g_fviewSideZ_Q15 * (uint32_t)g_collisionScratchPoint2Z);
#else
		dot = g_fviewSideX_Q15 * g_collisionScratchPoint2X + g_fviewSideY_Q15 * g_collisionScratchPoint2Y +
			  g_fviewSideZ_Q15 * g_collisionScratchPoint2Z;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		rotatedSide = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_fviewUpX_Q15 * (uint32_t)g_collisionScratchPoint2X +
						(uint32_t)g_fviewUpY_Q15 * (uint32_t)g_collisionScratchPoint2Y +
						(uint32_t)g_fviewUpZ_Q15 * (uint32_t)g_collisionScratchPoint2Z);
#else
		dot = g_fviewUpX_Q15 * g_collisionScratchPoint2X + g_fviewUpY_Q15 * g_collisionScratchPoint2Y +
			  g_fviewUpZ_Q15 * g_collisionScratchPoint2Z;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		rotatedUp = dot >> FVIEW_MATRIX_FRACTION_BITS;
#ifdef XW_MODERN
		dot = (int32_t)((uint32_t)g_fviewForwardX_Q15 * (uint32_t)g_collisionScratchPoint2X +
						(uint32_t)g_fviewForwardY_Q15 * (uint32_t)g_collisionScratchPoint2Y +
						(uint32_t)g_fviewForwardZ_Q15 * (uint32_t)g_collisionScratchPoint2Z);
#else
		dot = g_fviewForwardX_Q15 * g_collisionScratchPoint2X +
			  g_fviewForwardY_Q15 * g_collisionScratchPoint2Y +
			  g_fviewForwardZ_Q15 * g_collisionScratchPoint2Z;
#endif
		if (dot >= FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MAX;
		if (dot <= -FVIEW_DOT_CLAMP_LIMIT)
			dot = FVIEW_DOT_CLAMP_MIN;
		rotatedForward = dot >> FVIEW_MATRIX_FRACTION_BITS;
		g_collisionScratchPoint2X = (int16_t)rotatedSide;
		g_collisionScratchPoint2Y = (int16_t)rotatedUp;
		g_collisionScratchPoint2Z = (int16_t)rotatedForward;
		boundsMinX = (int16_t)ModelMesh_GetBoundsMinX(objectType, STATIC_COLLISION_MESH_INDEX);
		boundsMinY = (int16_t)ModelMesh_GetBoundsMinY(objectType, STATIC_COLLISION_MESH_INDEX);
		boundsMinZ = (int16_t)ModelMesh_GetBoundsMinZ(objectType, STATIC_COLLISION_MESH_INDEX);
		boundsMaxX = (int16_t)ModelMesh_GetBoundsMaxX(objectType, STATIC_COLLISION_MESH_INDEX);
		boundsMaxY = (int16_t)ModelMesh_GetBoundsMaxY(objectType, STATIC_COLLISION_MESH_INDEX);
		boundsMaxZ = (int16_t)ModelMesh_GetBoundsMaxZ(objectType, STATIC_COLLISION_MESH_INDEX);
		if (g_collisionScratchPoint1X < boundsMinX && g_collisionScratchPoint2X < boundsMinX)
			return 0;
		if (g_collisionScratchPoint1Y < boundsMinZ && g_collisionScratchPoint2Y < boundsMinZ)
			return 0;
		if (g_collisionScratchPoint1Z < boundsMinY && g_collisionScratchPoint2Z < boundsMinY)
			return 0;
		if (g_collisionScratchPoint1X > boundsMaxX && g_collisionScratchPoint2X > boundsMaxX)
			return 0;
		if (g_collisionScratchPoint1Y > boundsMaxZ && g_collisionScratchPoint2Y > boundsMaxZ)
			return 0;
		if (g_collisionScratchPoint1Z > boundsMaxY && g_collisionScratchPoint2Z > boundsMaxY)
			return 0;
		return starship_CheckSweptMeshCollision(objectType, STATIC_COLLISION_MESH_INDEX,
												g_collisionScratchPoint1X, g_collisionScratchPoint1Y,
												g_collisionScratchPoint1Z, g_collisionScratchPoint2X,
												g_collisionScratchPoint2Y, g_collisionScratchPoint2Z) != 0;
	} else {
		uint16_t halfExtent = (uint16_t)((maxBoundsExtent >> STATIC_COLLISION_QUARTER_SHIFT) +
										 (maxBoundsExtent >> STATIC_COLLISION_EIGHTH_SHIFT));
		g_collisionSweepEndX = g_resolvedWorldX;
		g_collisionSweepStartX = g_resolvedWorldX;
		g_collisionSweepEndY = g_resolvedWorldY;
		g_collisionSweepStartY = g_resolvedWorldY;
		g_collisionSweepEndZ = g_resolvedWorldZ;
		g_collisionSweepStartZ = g_resolvedWorldZ;
		return collide_checkboxcollision(halfExtent);
	}
}

// FUNCTION: XW 0x426230
void static_laserhitstatic(uint16_t projectileObjIdx, uint16_t missionObjectIndex) {
	int16_t impactEffectType;
	uint8_t objectType;
	if (g_missionObjects[missionObjectIndex].genusId == XW_GENUS_ASTEROID) {
		impactEffectType = XW_OBJ_ASTEROID_IMPACT;
	} else {
		g_missionObjects[missionObjectIndex].genusId = g_missionObjects[missionObjectIndex].objectType;
		g_missionObjects[missionObjectIndex].objectType = XW_OBJ_NONE;
		impactEffectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
		collide_updatekills(g_objectTable[projectileObjIdx].sourceObjectRef, XW_OBJECT_SLOT_UNAVAILABLE, 1);
	}
	if (impactEffectType == XW_OBJ_ASTEROID_IMPACT) {
		g_objectTable[projectileObjIdx].worldX = g_resolvedWorldX;
		g_objectTable[projectileObjIdx].worldY = g_resolvedWorldY;
		g_objectTable[projectileObjIdx].worldZ = g_resolvedWorldZ;
	} else {
		create_getworldposition(missionObjectIndex + XW_MISSION_OBJECT_REF_BASE, 0);
		g_objectTable[projectileObjIdx].worldX = g_resolvedWorldX;
		g_objectTable[projectileObjIdx].worldY = g_resolvedWorldY;
		g_objectTable[projectileObjIdx].worldZ = g_resolvedWorldZ;
	}
#ifdef XW_MODERN
	objectType = XwFlightTypes_CanonicalType(g_objectTable[projectileObjIdx].objectType);
#else
	objectType = g_objectTable[projectileObjIdx].objectType;
#endif
	if (objectType == XW_OBJ_WARHEAD_149 || objectType == XW_OBJ_TRACKED_WARHEAD)
		impactEffectType = (math2_getrandom() & 1) + XW_OBJ_EXPLOSION_133;
#ifdef XW_MODERN
	g_objectTable[projectileObjIdx].objectType = XwFlightTypes_ObjectType(impactEffectType);
	if (XwFlightTypes_Dos())
		g_objectTable[projectileObjIdx].instanceData = NULL;
#else
	g_objectTable[projectileObjIdx].objectType = impactEffectType;
#endif
#ifdef XW_MODERN
	XwFlightIntegration_Reset(projectileObjIdx);
	XwRenderObjects_ReplaceMobile(projectileObjIdx);
#endif
	g_objectTable[projectileObjIdx].genusId = XW_GENUS_EXPLOSION_EFFECT;
	g_objectTable[projectileObjIdx].familyId = XW_OBJECT_FAMILY_5;
	g_objectTable[projectileObjIdx].animationState = XW_OBJECT_ANIMATION_BREAKUP;
	g_objectTable[projectileObjIdx].speed = 0;
	g_objectTable[projectileObjIdx].billboardScaleCode = 0;
	g_objectTable[projectileObjIdx].ageSeconds = 0;
	g_objectTable[projectileObjIdx].lifetimeTicks = 0;
	g_objectTable[projectileObjIdx].pitch = 0;
	g_objectTable[projectileObjIdx].yaw = 0;
	g_objectTable[projectileObjIdx].roll = 0;
	g_objectTable[projectileObjIdx].orientMatrixDirty = 1;
	g_objectTable[projectileObjIdx].moveVectorDirty = 1;
	if (impactEffectType == XW_OBJ_ASTEROID_IMPACT)
		fsfx_triggersfx(FSFX_ASTEROID_IMPACT_SLOT, projectileObjIdx);
	else
		fsfx_triggersfx(FSFX_OBJECT_EXPLOSION_SLOT, projectileObjIdx);
}

// FUNCTION: XW 0x4263D0
void static_updatemineguns(uint16_t missionObjectIndex) {
	uint8_t cooldown = g_missionObjects[missionObjectIndex].typeSpecificByte;
	int muzzleWorldX, muzzleWorldY, muzzleWorldZ;
	unsigned int nearestDistance;
	uint16_t targetObjectIndex;
	uint16_t scanObjectIndex;
	if (cooldown > (uint16_t)(g_elapsedTicks >> 1)) {
		g_missionObjects[missionObjectIndex].typeSpecificByte = cooldown - (g_elapsedTicks >> 1);
		return;
	}
	g_missionObjects[missionObjectIndex].typeSpecificByte = STATIC_MINE_COOLDOWN;
	nearestDistance = (unsigned int)-1;
	muzzleWorldX = g_missionObjects[missionObjectIndex].worldX * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	muzzleWorldY = g_missionObjects[missionObjectIndex].worldY * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	muzzleWorldZ = g_missionObjects[missionObjectIndex].worldZ * MISSION_OBJECT_WORLD_COORDINATE_SCALE;
	targetObjectIndex = 0;
	for (scanObjectIndex = 0; scanObjectIndex < XW_CRAFT_OBJECT_COUNT; ++scanObjectIndex) {
		if (g_objectTable[scanObjectIndex].objectType != XW_OBJ_NONE &&
			g_objectTable[scanObjectIndex].iff == 0) {
#ifdef XW_MODERN
			unsigned int candidateDistance = collide_roughdistance3d(
				(int32_t)((uint32_t)g_objectTable[scanObjectIndex].worldX - (uint32_t)muzzleWorldX),
				(int32_t)((uint32_t)g_objectTable[scanObjectIndex].worldY - (uint32_t)muzzleWorldY),
				(int32_t)((uint32_t)g_objectTable[scanObjectIndex].worldZ - (uint32_t)muzzleWorldZ));
#else
			unsigned int candidateDistance =
				collide_roughdistance3d(g_objectTable[scanObjectIndex].worldX - muzzleWorldX,
										g_objectTable[scanObjectIndex].worldY - muzzleWorldY,
										g_objectTable[scanObjectIndex].worldZ - muzzleWorldZ);
#endif
			if (candidateDistance < nearestDistance) {
				nearestDistance = candidateDistance;
				targetObjectIndex = scanObjectIndex;
			}
		}
	}
	if (nearestDistance < STATIC_MINE_TARGET_RANGE) {
		int targetIndex = targetObjectIndex;
		int16_t baseLeadSteps;
		int leadSteps;
		int16_t shotYaw, shotPitch;
		uint8_t objectType;
		uint16_t muzzleOffset;
		uint16_t clampedRange;
		uint16_t targetSpeed, rangeFactor, speedFactor, accuracyThreshold;
		uint16_t projectileObjectIndex;
#ifdef XW_MODERN
		int largeMine;
#endif
#ifdef XW_MODERN
		trig2_ctop((int32_t)((uint32_t)g_objectTable[targetIndex].worldX - (uint32_t)muzzleWorldX),
				   (int32_t)((uint32_t)g_objectTable[targetIndex].worldY - (uint32_t)muzzleWorldY),
				   (int32_t)((uint32_t)g_objectTable[targetIndex].worldZ - (uint32_t)muzzleWorldZ));
#else
		trig2_ctop(g_objectTable[targetIndex].worldX - muzzleWorldX,
				   g_objectTable[targetIndex].worldY - muzzleWorldY,
				   g_objectTable[targetIndex].worldZ - muzzleWorldZ);
#endif
#ifdef XW_MODERN
		g_trig2PolarDistance =
			(int32_t)((uint32_t)g_simStepScale * (uint32_t)g_trig2PolarDistance) >> STATIC_MINE_LEAD_SHIFT;
#else
		g_trig2PolarDistance = (g_simStepScale * g_trig2PolarDistance) >> STATIC_MINE_LEAD_SHIFT;
#endif
		baseLeadSteps = g_trig2PolarDistance;
		leadSteps = (uint16_t)((math2_getrandom() & STATIC_MINE_LEAD_RANDOM_MASK) + baseLeadSteps - 1);
#ifdef XW_MODERN
		trig2_ctop((int32_t)((uint32_t)g_objectTable[targetIndex].worldX +
							 (uint32_t)leadSteps * (uint32_t)XwReferenceMotion_Axis(targetIndex, 0) -
							 (uint32_t)muzzleWorldX),
				   (int32_t)((uint32_t)g_objectTable[targetIndex].worldY +
							 (uint32_t)leadSteps * (uint32_t)XwReferenceMotion_Axis(targetIndex, 1) -
							 (uint32_t)muzzleWorldY),
				   (int32_t)((uint32_t)g_objectTable[targetIndex].worldZ +
							 (uint32_t)leadSteps * (uint32_t)XwReferenceMotion_Axis(targetIndex, 2) -
							 (uint32_t)muzzleWorldZ));
#else
		trig2_ctop(
			g_objectTable[targetIndex].worldX +
				leadSteps * (g_objectTable[targetIndex].worldX - g_objectTable[targetIndex].prevWorldX) -
				muzzleWorldX,
			g_objectTable[targetIndex].worldY +
				leadSteps * (g_objectTable[targetIndex].worldY - g_objectTable[targetIndex].prevWorldY) -
				muzzleWorldY,
			g_objectTable[targetIndex].worldZ +
				leadSteps * (g_objectTable[targetIndex].worldZ - g_objectTable[targetIndex].prevWorldZ) -
				muzzleWorldZ);
#endif
		shotYaw = g_trig2Yaw;
		objectType = g_missionObjects[missionObjectIndex].objectType;
		shotPitch = g_trig2Pitch;
#ifdef XW_MODERN
		largeMine = objectType > (XwFlightTypes_Dos() ? 27 : STATIC_MINE_SMALL_TYPE_LIMIT);
		muzzleOffset = largeMine ? STATIC_MINE_LARGE_MUZZLE : STATIC_MINE_SMALL_MUZZLE;
#else
		muzzleOffset =
			objectType > STATIC_MINE_SMALL_TYPE_LIMIT ? STATIC_MINE_LARGE_MUZZLE : STATIC_MINE_SMALL_MUZZLE;
#endif
		if ((uint16_t)shotPitch < STATIC_MINE_ARC_EIGHTH) {
			muzzleWorldZ += muzzleOffset;
		} else if ((uint16_t)shotPitch > STATIC_MINE_ARC_THREE_EIGHTHS) {
#ifdef XW_MODERN
			if (largeMine)
#else
			if (objectType > STATIC_MINE_SMALL_TYPE_LIMIT)
#endif
				return;
			muzzleWorldZ -= muzzleOffset;
		} else {
			if ((uint16_t)shotYaw < STATIC_MINE_ARC_EIGHTH ||
				(uint16_t)shotYaw > STATIC_MINE_ARC_SEVEN_EIGHTHS) {
				muzzleWorldY += muzzleOffset;
			} else if ((uint16_t)shotYaw < STATIC_MINE_ARC_THREE_EIGHTHS) {
				muzzleWorldX += muzzleOffset;
			} else if ((uint16_t)shotYaw < STATIC_MINE_ARC_FIVE_EIGHTHS) {
				muzzleWorldY -= muzzleOffset;
			} else
				muzzleWorldX -= muzzleOffset;
		}
		clampedRange = g_trig2PolarDistance;
		if (g_trig2PolarDistance >= STATIC_MINE_TARGET_RANGE)
			clampedRange = STATIC_MINE_FULL_ACCURACY;
		targetSpeed = g_objectTable[targetObjectIndex].speed;
		rangeFactor = ~clampedRange;
		if (targetSpeed < STATIC_MINE_SPEED_THRESHOLD)
			speedFactor = STATIC_MINE_FULL_ACCURACY;
		else
			speedFactor = STATIC_MINE_ACCURACY_BASE - (targetSpeed << STATIC_MINE_SPEED_SHIFT);
		accuracyThreshold = math2_fraction(rangeFactor, speedFactor);
		if ((uint16_t)math2_getrandom() > accuracyThreshold) {
			int yawError =
				((uint16_t)math2_getrandom() - STATIC_MINE_AIM_ERROR_BIAS) & STATIC_MINE_AIM_ERROR_MASK;
			int16_t pitchError;
			if ((uint16_t)math2_getrandom() >= TRIG2_ANGLE_SIGN_BIT)
				yawError = -yawError;
			shotYaw += yawError;
			pitchError = (math2_getrandom() - STATIC_MINE_AIM_ERROR_BIAS) & STATIC_MINE_AIM_ERROR_MASK;
			if ((uint16_t)math2_getrandom() >= TRIG2_ANGLE_SIGN_BIT)
				shotPitch -= pitchError;
			else
				shotPitch += pitchError;
		}
		projectileObjectIndex = create_findslot(XW_GENUS_OTHER_PROJECTILE);
		if (projectileObjectIndex != XW_OBJECT_SLOT_UNAVAILABLE) {
			int projectileIndex = projectileObjectIndex;
			int launchOffsetX, launchOffsetY, launchOffsetZ;
			int guidanceIndex;
			g_objectTable[projectileIndex].familyId = STATIC_MINE_PROJECTILE_FAMILY;
			g_objectTable[projectileIndex].genusId = XW_GENUS_OTHER_PROJECTILE;
#ifdef XW_MODERN
			g_objectTable[projectileIndex].objectType = XwFlightTypes_ObjectType(XW_OBJ_LASER_146);
#else
			g_objectTable[projectileIndex].objectType = XW_OBJ_LASER_146;
#endif
			g_objectTable[projectileIndex].ageSeconds = 1;
			g_objectTable[projectileIndex].sourceObjectRef = targetObjectIndex + XW_MISSION_OBJECT_REF_BASE;
			g_objectTable[projectileIndex].sourceObjectType = XW_OBJ_NONE;
			g_objectTable[projectileIndex].iff = STATIC_MINE_PROJECTILE_IFF;
			g_objectTable[projectileIndex].pitch = shotPitch;
			g_objectTable[projectileIndex].roll = 0;
			g_objectTable[projectileIndex].yaw = shotYaw;
			g_objectTable[projectileIndex].orientMatrixDirty = 1;
			g_objectTable[projectileIndex].moveVectorDirty = 1;
			g_objectTable[projectileIndex].speed = g_projectileSpeedByType[STATIC_MINE_PROJECTILE_TYPE];
			g_objectTable[projectileIndex].damageAmount =
				g_projectileBaseDamageByType[STATIC_MINE_PROJECTILE_TYPE];
			g_objectTable[projectileIndex].lifetimeTicks =
				XW_SIMULATION_TICKS_PER_SECOND *
				g_projectileLifetimeSecondsByType[STATIC_MINE_PROJECTILE_TYPE];
			fview_calcrotatemove(shotPitch, shotYaw, &g_objectTable[projectileIndex]);
			g_objectTable[projectileIndex].prevWorldX = muzzleWorldX;
			g_objectTable[projectileIndex].prevWorldY = muzzleWorldY;
			g_objectTable[projectileIndex].prevWorldZ = muzzleWorldZ;
			launchOffsetZ = (int32_t)((uint64_t)(g_projectileLaunchOffsetByType[STATIC_MINE_PROJECTILE_TYPE] *
												 (int64_t)g_craftMoveY) >>
									  FVIEW_MATRIX_FRACTION_BITS);
			launchOffsetX = (int32_t)((uint64_t)(g_projectileLaunchOffsetByType[STATIC_MINE_PROJECTILE_TYPE] *
												 (int64_t)g_craftMoveX) >>
									  FVIEW_MATRIX_FRACTION_BITS);
			launchOffsetY = (int32_t)((uint64_t)(g_projectileLaunchOffsetByType[STATIC_MINE_PROJECTILE_TYPE] *
												 (int64_t)g_craftMoveZ) >>
									  FVIEW_MATRIX_FRACTION_BITS);
#ifdef XW_MODERN
			g_objectTable[projectileIndex].worldX =
				(int32_t)((uint32_t)muzzleWorldX + (uint32_t)launchOffsetX);
#else
			g_objectTable[projectileIndex].worldX = muzzleWorldX + launchOffsetX;
#endif
#ifdef XW_MODERN
			g_objectTable[projectileIndex].worldY =
				(int32_t)((uint32_t)muzzleWorldY + (uint32_t)launchOffsetY);
#else
			g_objectTable[projectileIndex].worldY = muzzleWorldY + launchOffsetY;
#endif
#ifdef XW_MODERN
			g_objectTable[projectileIndex].worldZ =
				(int32_t)((uint32_t)muzzleWorldZ + (uint32_t)launchOffsetZ);
#else
			g_objectTable[projectileIndex].worldZ = muzzleWorldZ + launchOffsetZ;
#endif
			fsfx_triggerlasersfx(projectileObjectIndex);
			guidanceIndex = (uint16_t)(projectileObjectIndex - XW_CRAFT_OBJECT_COUNT);
			g_objectTable[projectileIndex].instanceData = &g_warheadGuidanceTable[guidanceIndex];
			g_warheadGuidanceTable[guidanceIndex].homingTier = 0;
			g_warheadGuidanceTable[guidanceIndex].targetObjIdx = 0;
		}
	}
}
