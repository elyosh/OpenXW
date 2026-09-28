#include "xw/flight/object/anim.h"

#ifdef XW_MODERN
#include "xw_dos94/audio/fsfx.h"
#include "xw_runtime/audio/imuse_session.h"
#include "xw_runtime/audio/player_engine.h"
#endif

#ifdef XW_MODERN
#include "xw_runtime/timing/flight_integration.h"
#endif

#ifdef XW_MODERN
#include "xw_dos94/flight/hyperspace.h"
#include "xw_runtime/runtime/flight_math.h"
#include "xw_runtime/runtime/flight_types.h"
#endif

#include "xw/assets/bitmap.h"
#include "xw/assets/model_mesh.h"
#include "xw/audio/fsfx.h"
#include "xw/audio/sound.h"
#include "xw/flight/death_star.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/fview.h"
#include "xw/flight/hud/msg.h"
#include "xw/flight/hud/panelrts.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/create.h"
#include "xw/flight/object/starship.h"
#include "xw/flight/player/user.h"
#include "xw/flight/replay/replay.h"
#include "xw/flight/shell_flight.h"
#include "xw/flight/xw.h"
#include "xw/math/math2.h"
#include "xw/math/transfm2.h"
#include "xw/math/trig2.h"
#include "xw/render/flight_view.h"
#include "xw/render/render_quad.h"
#include "xw/render/renderer.h"
#include "xw/render/rotscale.h"
#include "xw/util/memory.h"

#include <stdlib.h>

// GLOBAL: XW 0x62B324
int16_t g_sceneBillboardQueueCount = 0;

// GLOBAL: XW 0x62B4FC
uint32_t g_hyperspaceStreakLength = 0;

// GLOBAL: XW 0x62B91E
uint16_t g_playerHyperspaceElapsedTicks = 0;

// GLOBAL: XW 0x62B944
uint8_t g_hyperspaceflag = 0;

// GLOBAL: XW 0x62BAC0
uint16_t g_hyperspaceSavedDebrisEnabled = 0;

// GLOBAL: XW 0x62BAC2
uint16_t g_hyperspaceSavedBackdropsEnabled = 0;

// GLOBAL: XW 0x62C720
SceneBillboardQueueEntry g_sceneBillboardQueue[ANIM_BITMAP_QUEUE_CAPACITY] = { 0 };

// GLOBAL: XW 0x62CC68
uint8_t g_hyperspaceAbortAndCollisionsAllowed = 0;

// GLOBAL: XW 0x63BF4E
XwObjectGenus g_currentModelObjectGenus = 0;

// GLOBAL: XW 0x63BF50
uint16_t g_animationFrameIndex = 0;

// GLOBAL: XW 0x63BF54
const uint16_t* g_animationFrames = NULL;

// FUNCTION: XW 0x401070
void anim_drawverysimpleobject(uint16_t objectIndex) {
	uint16_t objectType;
	uint16_t frameCode;
	unsigned int frameIndex;
	int absRow0Z, absRow1Z;
	int rotationX, rotationY;
	int rotationAngle;
	int projectedX, projectedY, overflowBits;
	int16_t screenX;
	int screenY;
	uint16_t screenScale;

	g_billboardObjectOrTypeIndex = objectIndex;
	objectType = g_objectTable[objectIndex].objectType;
	g_currentModelObjectGenus = g_objectTable[objectIndex].genusId;
	g_animationFrames = g_modelTypeTable[objectType].animationFrames;
	if (objectType == XW_OBJ_DETACHED_COMPONENT) {
		frameCode = g_objectTable[objectIndex].animationState >> 1;
	} else {
		if (g_animationFrames == NULL)
			return;
		frameIndex = g_objectTable[objectIndex].animationState;
		g_animationFrameIndex = frameIndex;
		frameCode = g_animationFrames[frameIndex];
	}
	if (frameCode >= ANIM_FRAME_JUMP_BASE)
		return;
	if (frameCode < ANIM_BITMAP_FIRST_FRAME) {
		int16_t savedRenderState;
		if (objectType == XW_OBJ_DETACHED_COMPONENT)
			objectType = g_objectTable[objectIndex].sourceObjectType;
		savedRenderState = g_drawMarkingsFlag;
		RenderScene_DrawNoAssetSourceModel(&g_objectTable[objectIndex], frameCode);
		g_drawMarkingsFlag = savedRenderState;
		if (objectType == XW_OBJ_DETACHED_COMPONENT) {
			frameIndex = g_objectTable[objectIndex].secondaryAnimationState;
			g_animationFrameIndex = frameIndex;
			frameCode = g_fragmentSecondaryAnimationFrames[frameIndex];
		}
	}
	if (frameCode >= ANIM_FRAME_JUMP_BASE || frameCode < ANIM_BITMAP_FIRST_FRAME || g_objectViewZ < 0)
		return;
	absRow0Z = g_objViewMat_R0_Z;
	absRow1Z = g_objViewMat_R1_Z;
	if (absRow0Z < 0)
		absRow0Z = (int32_t)(0u - (uint32_t)absRow0Z);
	if (absRow1Z < 0)
		absRow1Z = (int32_t)(0u - (uint32_t)absRow1Z);
	if (absRow0Z < absRow1Z) {
		rotationX = g_objViewMat_R0_X;
		rotationY = g_objViewMat_R0_Y;
	} else {
		rotationX = g_objViewMat_R1_X;
		rotationY = g_objViewMat_R1_Y;
	}
	if (rotationX < 0)
		rotationAngle = trig2_arctan(rotationY, (int32_t)(0u - (uint32_t)rotationX));
	else
		rotationAngle = -trig2_arctan(rotationY, rotationX);
	projectedX = transfm2_getscreencoordx(g_objectViewX, g_objectViewZ);
	screenX = (int16_t)projectedX;
	overflowBits = projectedX & RENDER_DAMAGE_SCREEN_HIGH_MASK;
	if (overflowBits > 0 || overflowBits < RENDER_DAMAGE_SCREEN_HIGH_MASK)
		return;
	projectedY = transfm2_getscreencoordy(g_objectViewY, g_objectViewZ);
	overflowBits = projectedY & RENDER_DAMAGE_SCREEN_HIGH_MASK;
	if (overflowBits > 0 || overflowBits < RENDER_DAMAGE_SCREEN_HIGH_MASK)
		return;
	screenY = g_flightVpHeight - projectedY;
	screenScale = g_objectTable[objectIndex].billboardScaleCode;
	if (screenScale != 0) {
		screenScale <<= ANIM_BITMAP_SCALE_SHIFT;
		if (screenScale >= ANIM_BITMAP_UNIT_SCALE)
			screenScale += ANIM_BITMAP_UNIT_SCALE;
	} else {
		screenScale = ANIM_BITMAP_UNIT_SCALE;
	}
	anim_add_bitmap_draw(g_billboardObjectOrTypeIndex, frameCode, screenScale, screenX, screenY,
						 g_objectViewZ, rotationAngle);
}

// FUNCTION: XW 0x401270
void anim_add_bitmap_draw(uint16_t objectRef, int16_t frameCode, int16_t screenScale, int16_t screenX,
						  int16_t screenY, int depth, int16_t rotationAngle) {
	if (g_sceneBillboardQueueCount < ANIM_BITMAP_QUEUE_CAPACITY) {
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].objectOrTypeIndex = objectRef;
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].frame = frameCode;
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].screenSize = screenScale;
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].screenX = screenX;
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].screenY = screenY;
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].depthZ = depth;
		g_sceneBillboardQueue[g_sceneBillboardQueueCount].rotationAngle = rotationAngle;
		++g_sceneBillboardQueueCount;
	}
}

// FUNCTION: XW 0x4012E0
void anim_sort_and_draw_bitmaps(int drawTargetMarkersUnused) {
	int16_t lastIndex;
	int16_t swapped = 1;
	(void)drawTargetMarkersUnused;
	for (; g_sceneBillboardQueueCount-- != 0;) {
		lastIndex = g_sceneBillboardQueueCount;
		if (swapped) {
			int compareIndex;
			swapped = 0;
			for (compareIndex = 0; (uint16_t)compareIndex < lastIndex; ++compareIndex) {
				int index = (uint16_t)compareIndex;
				if (g_sceneBillboardQueue[index].depthZ > g_sceneBillboardQueue[index + 1].depthZ) {
					SceneBillboardQueueEntry temporary = g_sceneBillboardQueue[index];
					g_sceneBillboardQueue[index] = g_sceneBillboardQueue[index + 1];
					g_sceneBillboardQueue[index + 1] = temporary;
					lastIndex = g_sceneBillboardQueueCount;
					swapped = 1;
				}
			}
		}
		anim_draw_bitmap(&g_sceneBillboardQueue[lastIndex]);
	}
}

// FUNCTION: XW 0x4013E0
void anim_draw_bitmap(const struct SceneBillboardQueueEntry* quadRecord) {
	uint16_t selection = quadRecord->frame & ANIM_BITMAP_SELECTION_MASK;
	uint16_t objectRef = quadRecord->objectOrTypeIndex;
	uint16_t frameIndex = selection & ANIM_BITMAP_FRAME_MASK;
	uint16_t modelType = selection >> ANIM_BITMAP_MODEL_SHIFT;
	uint16_t scaleQ8;
	uint8_t* modelBytes;
	XwBitmapModelPrefix* model;
	const uint32_t* frameOffsets;
	XwBitmapFramePrefix* frame;
	g_flightSwRotSpriteSpanRunsEnabled = 1;
	g_billboardObjectOrTypeIndex = objectRef;
	if ((objectRef & XW_OBJECT_REF_KIND_MASK) == XW_MISSION_OBJECT_REF_BASE) {
		uint16_t missionIndex = objectRef - XW_MISSION_OBJECT_REF_BASE;
		uint16_t worldY = g_missionObjects[missionIndex].worldY;
		uint16_t worldZ = g_missionObjects[missionIndex].worldZ;
		g_billboardObjectOrTypeIndex = objectRef;
		g_camRelWorldX += ((uint16_t)g_missionObjects[missionIndex].worldX << ANIM_MISSION_POSITION_SHIFT) -
						  (uint32_t)g_flightCamera.worldPosition.x;
		g_camRelWorldY += (worldY << ANIM_MISSION_POSITION_SHIFT) - (uint32_t)g_flightCamera.worldPosition.y;
		g_camRelWorldZ += (worldZ << ANIM_MISSION_POSITION_SHIFT) - (uint32_t)g_flightCamera.worldPosition.z;
	} else {
		int x = g_objectTable[objectRef].worldX - (uint32_t)g_flightCamera.worldPosition.x;
		int y = g_objectTable[objectRef].worldY;
		int z = g_objectTable[objectRef].worldZ;
		g_camRelWorldX = x;
		g_camRelWorldY = y - (uint32_t)g_flightCamera.worldPosition.y;
		g_camRelWorldZ = z - (uint32_t)g_flightCamera.worldPosition.z;
	}
	g_objectViewZ = quadRecord->depthZ;
	scaleQ8 = rotscale_calcscale(quadRecord->depthZ, g_modelTypeTable[modelType].maxBoundsExtent,
								 quadRecord->screenSize);
	modelBytes = (uint8_t*)Memory_LockHandle(g_modelTypeTable[modelType].memoryHandle);
	model = (XwBitmapModelPrefix*)modelBytes;
	frameOffsets = (const uint32_t*)(modelBytes + model->frameOffsetsOffset);
	frame = (XwBitmapFramePrefix*)(modelBytes + frameOffsets[frameIndex]);
	if (g_useHardware3D) {
		RenderQuad_DrawRotatedSprite((uint16_t)quadRecord->rotationAngle, quadRecord->screenX,
									 quadRecord->screenY, scaleQ8, frame);
	} else {
		rotscale_preparefastdraw(quadRecord->rotationAngle);
		rotscale_preparecolor(frame);
		rotscale_rotatescaleimage(quadRecord->screenX, quadRecord->screenY, scaleQ8, frame);
	}
}

// FUNCTION: XW 0x4015A0
void anim_updateanimation(void) {
	uint16_t objectIndex;
	uint16_t missionIndex;
	if (g_flightGlobalCountdownTimers.ticks[XW_TIMER_ANIMATION] != 0)
		return;
	g_flightGlobalCountdownTimers.ticks[XW_TIMER_ANIMATION] = ANIM_UPDATE_INTERVAL;
	if (g_deathStarSurfaceModeActive != 0)
		anim_AdvanceSurfaceObjectStateCounters();
	for (objectIndex = 0; objectIndex < XW_OBJECT_COUNT; ++objectIndex) {
		uint8_t objectTypeByte = g_objectTable[objectIndex].objectType;
		uint16_t objectType = objectTypeByte;
		CraftData* originalCraft;
		XwObjectGenus genus;
		if (objectType == XW_OBJ_NONE)
			continue;
		originalCraft = (CraftData*)g_objectTable[objectIndex].instanceData;
		genus = g_objectTable[objectIndex].genusId;
		g_currentModelObjectGenus = genus;
		g_animationFrames = g_modelTypeTable[objectType].animationFrames;
		switch (genus) {
			case XW_GENUS_STARFIGHTER:
			case XW_GENUS_TRANSPORT:
			case XW_GENUS_UTILITY:
			case XW_GENUS_FREIGHTER:
			case XW_GENUS_STARSHIP: {
				uint16_t meshCount = (uint16_t)ModelMesh_GetCachedObjectTypeMeshCount(objectType);
				int16_t foilChanged = 0;
				int meshIndex;
				g_curCraft = (CraftData*)g_objectTable[objectIndex].instanceData;
				for (meshIndex = 0; (uint16_t)meshIndex < meshCount; ++meshIndex) {
					int meshType = ModelMesh_GetCachedObjectTypeMeshType(objectType, (uint16_t)meshIndex);
					if (meshType == MODEL_MESH_TYPE_DAMAGE_ANIMATION) {
						g_animationFrames = g_componentDamageAnimationFrames;
						/* The original indexes this state by mesh count, not the current mesh. */
						g_animationFrameIndex = originalCraft->componentState[meshCount];
						anim_updateanimstate(objectIndex);
						originalCraft->componentState[meshCount] = (uint8_t)g_animationFrameIndex;
					}
					if (g_curCraft->objectKind == XW_CRAFT_OBJECT_KIND_3) {
						if (g_deathStarSurfaceModeActive == 0 &&
							g_modelTypeTable[objectType].maxBoundsExtent > ANIM_LARGE_CRAFT_EXTENT) {
							starship_createstarshipexplo__partial(objectIndex);
							meshIndex += ANIM_LARGE_CRAFT_MESH_SKIP;
						} else {
							if ((uint16_t)math2_getrandom() < ANIM_COMPONENT_DETACH_THRESHOLD)
								create_blowoffcomponent(objectIndex, 0);
							if ((uint16_t)math2_getrandom() < ANIM_CRAFT_EMBER_THRESHOLD)
								create_createember(objectIndex);
						}
					}
					if (meshType == MODEL_MESH_TYPE_BRIDGE && objectType == XW_OBJ_B_WING) {
						if (g_curCraft->sFoilState & XW_SFOIL_MOVING) {
							if (g_curCraft->sFoilState & XW_SFOIL_CLOSED) {
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] <
									ANIM_BWING_CLOSED_ROTATION)
									g_curCraft->meshRotation[(uint16_t)meshIndex] += ANIM_BWING_ROTATION_STEP;
							} else if (g_curCraft->meshRotation[(uint16_t)meshIndex] > 0) {
								g_curCraft->meshRotation[(uint16_t)meshIndex] -= ANIM_BWING_ROTATION_STEP;
							}
						}
					}
					if (meshType == MODEL_MESH_TYPE_SFOIL && (g_curCraft->sFoilState & XW_SFOIL_MOVING)) {
						if (g_curCraft->sFoilState & XW_SFOIL_CLOSED) {
							if (objectType == XW_OBJ_X_WING) {
								int rotationLimit;
								if (ModelMesh_GetCenterZ(XW_OBJ_X_WING, (uint16_t)meshIndex) < 0)
									rotationLimit = ANIM_XWING_LOWER_CLOSED_ROTATION;
								else
									rotationLimit = ANIM_XWING_UPPER_CLOSED_ROTATION;
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] == 0 &&
									g_flightAudioMode != 0 &&
#ifdef XW_MODERN
									imuse_get_param(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT,
													IMUSE_PARAM_SOUND_PLAY_COUNT)
#else
									Sound_GetParam(FSFX_SFOIL_MOVEMENT_SLOT, SOUND_PARAM_INSTANCE_COUNT)
#endif
										<= 0)
									fsfx_triggersfx(FSFX_SFOIL_MOVEMENT_SLOT, FSFX_UNPOSITIONED_OBJECT);
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] < (uint16_t)rotationLimit) {
									++g_curCraft->meshRotation[(uint16_t)meshIndex];
									foilChanged = 1;
								}
							} else if (objectType == XW_OBJ_B_WING) {
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] == 0 &&
									g_flightAudioMode != 0 &&
#ifdef XW_MODERN
									imuse_get_param(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT,
													IMUSE_PARAM_SOUND_PLAY_COUNT)
#else
									Sound_GetParam(FSFX_SFOIL_MOVEMENT_SLOT, SOUND_PARAM_INSTANCE_COUNT)
#endif
										<= 0)
									fsfx_triggersfx(FSFX_SFOIL_MOVEMENT_SLOT, FSFX_UNPOSITIONED_OBJECT);
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] <
									ANIM_BWING_CLOSED_ROTATION) {
									g_curCraft->meshRotation[(uint16_t)meshIndex] += ANIM_BWING_ROTATION_STEP;
									foilChanged = 1;
								}
							}
						} else {
							if (objectType == XW_OBJ_X_WING) {
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] ==
										ANIM_XWING_UPPER_CLOSED_ROTATION &&
									g_flightAudioMode != 0 &&
#ifdef XW_MODERN
									imuse_get_param(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT,
													IMUSE_PARAM_SOUND_PLAY_COUNT)
#else
									Sound_GetParam(FSFX_SFOIL_MOVEMENT_SLOT, SOUND_PARAM_INSTANCE_COUNT)
#endif
										<= 0)
									fsfx_triggersfx(FSFX_SFOIL_MOVEMENT_SLOT, FSFX_UNPOSITIONED_OBJECT);
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] > 0) {
									--g_curCraft->meshRotation[(uint16_t)meshIndex];
									foilChanged = 1;
								}
							} else if (objectType == XW_OBJ_B_WING) {
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] ==
										ANIM_BWING_CLOSED_ROTATION &&
									g_flightAudioMode != 0 &&
#ifdef XW_MODERN
									imuse_get_param(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT,
													IMUSE_PARAM_SOUND_PLAY_COUNT)
#else
									Sound_GetParam(FSFX_SFOIL_MOVEMENT_SLOT, SOUND_PARAM_INSTANCE_COUNT)
#endif
										<= 0)
									fsfx_triggersfx(FSFX_SFOIL_MOVEMENT_SLOT, FSFX_UNPOSITIONED_OBJECT);
								if (g_curCraft->meshRotation[(uint16_t)meshIndex] > 0) {
									g_curCraft->meshRotation[(uint16_t)meshIndex] -= ANIM_BWING_ROTATION_STEP;
									foilChanged = 1;
								}
							}
						}
					}
				}
				if (g_curCraft->sFoilState & XW_SFOIL_MOVING) {
					if (g_curCraft->sFoilState & XW_SFOIL_CLOSED) {
						if (!foilChanged) {
							g_curCraft->sFoilState = XW_SFOIL_CLOSED;
							g_msgArgTable[0] = XW_MSG_SFOILS_CLOSED_POSITION;
							msg_messageprintf(XW_MSG_SFOILS_POSITION_REACHED);
							if (g_flightAudioMode != 0) {

#ifdef XW_MODERN
								imuse_stop_sound(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT);
#else
								Sound_StopOldestInstanceById(FSFX_SFOIL_MOVEMENT_SLOT);
#endif
								fsfx_triggersfx(FSFX_SFOIL_FINISHED_SLOT, g_playerFlightState.objectIndex);
							}
						}
					} else if (!foilChanged) {
						g_curCraft->sFoilState = 0;
						g_msgArgTable[0] = XW_MSG_SFOILS_OPEN_POSITION;
						msg_messageprintf(XW_MSG_SFOILS_POSITION_REACHED);
						if (g_flightAudioMode != 0) {

#ifdef XW_MODERN
							imuse_stop_sound(g_dos94Imuse, FSFX_SFOIL_MOVEMENT_SLOT);
#else
							Sound_StopOldestInstanceById(FSFX_SFOIL_MOVEMENT_SLOT);
#endif
							fsfx_triggersfx(FSFX_SFOIL_FINISHED_SLOT, g_playerFlightState.objectIndex);
						}
					}
				}
				break;
			}
			case XW_GENUS_DEBRIS:
			case XW_GENUS_EXPLOSION_EFFECT:
				if (objectType == XW_OBJ_DETACHED_COMPONENT) {
					g_objectTable[objectIndex].secondaryAnimationState = 0;
					if ((uint16_t)math2_getrandom() < ANIM_COMPONENT_EMBER_THRESHOLD)
						create_createember(objectIndex);
				} else {
					g_animationFrameIndex = g_objectTable[objectIndex].animationState;
					if (g_animationFrames != NULL)
						anim_updateanimstate(objectIndex);
					g_objectTable[objectIndex].animationState = (uint8_t)g_animationFrameIndex;
				}
				break;
			default:
				break;
		}
	}
	for (missionIndex = 0; missionIndex < MISSION_OBJECT_COUNT; ++missionIndex) {
		uint16_t objectType = g_missionObjects[missionIndex].objectType;
		if (objectType != XW_OBJ_NONE) {
			g_animationFrames = g_modelTypeTable[objectType].animationFrames;
			if (g_animationFrames != NULL) {
				g_animationFrameIndex = g_missionObjects[missionIndex].stateByte;
				anim_updateanimstate((uint16_t)(missionIndex + XW_MISSION_OBJECT_REF_BASE));
				g_missionObjects[missionIndex].stateByte = (uint8_t)g_animationFrameIndex;
			}
		}
	}
}

// FUNCTION: XW 0x401AC0
void anim_updateanimstate(uint16_t objectRef) {
	const uint16_t* frames = g_animationFrames;
	uint16_t nextFrame = (uint16_t)(g_animationFrameIndex + 1);
	uint16_t frameCommand;
	g_animationFrameIndex = nextFrame;
	frameCommand = frames[nextFrame];
	if (frameCommand == ANIM_FRAME_END_OBJECT) {
		if ((objectRef & XW_OBJECT_REF_KIND_MASK) == XW_MISSION_OBJECT_REF_BASE) {
#ifdef XW_MODERN
			g_missionObjects[objectRef - XW_MISSION_OBJECT_REF_BASE].objectType = XW_OBJ_NONE;
#else
			g_missionObjects[objectRef].objectType = XW_OBJ_NONE;
#endif
		} else {
			g_objectTable[objectRef].objectType = XW_OBJ_NONE;
		}
	} else if (frameCommand == ANIM_FRAME_RESTART) {
		g_animationFrameIndex = 0;
	} else if (frameCommand >= ANIM_FRAME_JUMP_BASE && frameCommand != ANIM_FRAME_ADVANCE) {
		g_animationFrameIndex = (uint16_t)(frameCommand - ANIM_FRAME_JUMP_BASE);
	}
}

// FUNCTION: XW 0x401B50
void anim_AdvanceSurfaceObjectStateCounters(void) {
	int cellIndex;
	for (cellIndex = 0; cellIndex != DEATH_STAR_SURFACE_CELL_COUNT; ++cellIndex) {
		XwSurfaceCellDamageState* cell = &g_surfaceCellDamageStates[cellIndex];
		if (cell->cellKey != 0) {
			int objectIndex;
			for (objectIndex = 0; objectIndex != DEATH_STAR_SURFACE_OBJECT_COUNT; ++objectIndex) {
				if (cell->objectHealthOrEffectState[objectIndex] >= DEATH_STAR_DESTRUCTION_STATE_FIRST) {
					++cell->objectHealthOrEffectState[objectIndex];
				}
			}
		}
	}
}

// FUNCTION: XW 0x401B90
void anim_dohyperspace(void) {
	uint16_t elapsedTicks;
	elapsedTicks = g_playerHyperspaceElapsedTicks;
	elapsedTicks += g_elapsedTicks;
	g_playerHyperspaceElapsedTicks = elapsedTicks;
	switch (g_hyperspaceflag) {
		case ANIM_HYPERSPACE_ALIGN: {
			ObjectRecord* playerObject = g_playerFlightState.object;
			int16_t roll;
			int16_t yaw;
			int16_t pitch;
			if ((playerObject->objectType == XW_OBJ_X_WING || playerObject->objectType ==
#ifdef XW_MODERN
																  XwFlightTypes_ObjectType(XW_OBJ_B_WING)
#else
																  XW_OBJ_B_WING
#endif
					 ) &&
				g_playerFlightState.craft->sFoilState != XW_SFOIL_CLOSED) {
				g_playerFlightState.craft->sFoilState = XW_SFOIL_CLOSED;
				g_playerFlightState.craft->sFoilState |= XW_SFOIL_MOVING;
#ifdef XW_MODERN
				if (XwFlightTypes_Dos())
					Dos94Hyperspace_SfoilSound();
#endif
				elapsedTicks = g_playerHyperspaceElapsedTicks;
				playerObject = g_playerFlightState.object;
			}
			roll = playerObject->roll;
			yaw = playerObject->yaw;
			pitch = playerObject->pitch;
			if (roll == 0 && pitch == ANIM_QUARTER_TURN && yaw == 0) {
				if (elapsedTicks >= ANIM_HYPERSPACE_START_TICK) {
					user_checkreplaycamera();
					g_playerFlightState.object->speed = ANIM_HYPERSPACE_JUMP_SPEED;
					if (g_replayviewmode != 0) {
						g_hyperspaceflag = ANIM_HYPERSPACE_IDLE;
					} else {
						g_hyperspaceflag = ANIM_HYPERSPACE_SETUP;
						msg_messageprintf(XW_MSG_ENTERING_HYPERSPACE);
					}
					playerObject = g_playerFlightState.object;
				} else {
					if (elapsedTicks >= ANIM_HYPERSPACE_PATH_CHECK_TICK) {
						int16_t blocked = 0;
						int16_t slot;
						g_hyperspaceAbortAndCollisionsAllowed = 0;
						for (slot = 0; slot < XW_CRAFT_OBJECT_COUNT; ++slot) {
							int objectType = g_objectTable[slot].objectType;
							if (objectType != XW_OBJ_NONE) {
								int dx = g_objectTable[slot].worldX - playerObject->worldX;
								int dy = g_objectTable[slot].worldY - playerObject->worldY;
								int dz = g_objectTable[slot].worldZ - playerObject->worldZ;
								int extent;
								int playerExtent;
								if (dx < 0)
									dx = -dx;
								if (dz < 0)
									dz = -dz;
								extent = g_modelTypeTable[objectType].maxBoundsExtent;
								dx -= extent;
								dz -= extent;
								playerExtent = g_modelTypeTable[playerObject->objectType].maxBoundsExtent;
								if (dx < playerExtent && dz < playerExtent && dy > 0 &&
									dy < ANIM_HYPERSPACE_PATH_LENGTH) {
									blocked = 1;
									break;
								}
							}
						}
						if (blocked == 0) {
							int16_t missionSlot;
							for (missionSlot = 0; missionSlot < MISSION_OBJECT_COUNT; ++missionSlot) {
								if (g_missionObjects[missionSlot].objectType != XW_OBJ_NONE) {
									int dx;
									int dy;
									int dz;
									int extent;
									int playerExtent;
									create_getworldposition(missionSlot + XW_MISSION_OBJECT_REF_BASE, 0);
									playerObject = g_playerFlightState.object;
									dx = g_resolvedWorldX - playerObject->worldX;
									dy = g_resolvedWorldY - playerObject->worldY;
									dz = g_resolvedWorldZ - playerObject->worldZ;
									if (dx < 0)
										dx = -dx;
									if (dz < 0)
										dz = -dz;
									extent = g_modelTypeTable[g_missionObjects[missionSlot].objectType]
												 .maxBoundsExtent;
									dx -= extent;
									dz -= extent;
									playerExtent = g_modelTypeTable[playerObject->objectType].maxBoundsExtent;
									if (dx < playerExtent && dz < playerExtent && dy > 0 &&
										dy < ANIM_HYPERSPACE_PATH_LENGTH) {
										blocked = 1;
										break;
									}
								}
							}
						}
						if (g_deathStarSurfaceModeActive != 0 &&
							playerObject->worldZ < ANIM_HYPERSPACE_SURFACE_CLEARANCE)
							blocked = 1;
						if (blocked != 0) {
							msg_messageprintf(XW_MSG_HYPERSPACE_PATH_BLOCKED);
							fsfx_triggersfx(FSFX_HYPERSPACE_BLOCKED_SLOT, FSFX_UNPOSITIONED_OBJECT);
							g_hyperspaceflag = ANIM_HYPERSPACE_IDLE;
							g_playerFlightState.object->speed = ANIM_HYPERSPACE_ABORT_SPEED;
							playerObject = g_playerFlightState.object;
						}
#ifdef XW_MODERN
						XwFlightMath_MoveHyperspace(ANIM_HYPERSPACE_LAUNCH_STEP);
#else
						playerObject->worldY += ANIM_HYPERSPACE_LAUNCH_STEP * g_elapsedTicks;
#endif
						playerObject = g_playerFlightState.object;
					}
				}
			} else {
				int16_t angleStep = ANIM_HYPERSPACE_ALIGN_RATE * g_elapsedTicks;
				if ((uint16_t)roll < ANIM_HALF_TURN) {
					roll -= angleStep;
					if ((uint16_t)roll >= ANIM_HALF_TURN)
						roll = 0;
				} else {
					roll += angleStep;
					if ((uint16_t)roll < ANIM_HALF_TURN)
						roll = 0;
				}
				if ((uint16_t)yaw < ANIM_HALF_TURN) {
					yaw -= angleStep;
					if ((uint16_t)yaw >= ANIM_HALF_TURN)
						yaw = 0;
				} else {
					yaw += angleStep;
					if ((uint16_t)yaw < ANIM_HALF_TURN)
						yaw = 0;
				}
				if ((uint16_t)pitch < ANIM_THREE_QUARTER_TURN && (uint16_t)pitch > ANIM_QUARTER_TURN) {
					pitch -= angleStep;
					if ((uint16_t)pitch < ANIM_QUARTER_TURN)
						pitch = ANIM_QUARTER_TURN;
				} else if (pitch != ANIM_QUARTER_TURN) {
					pitch += angleStep;
					if ((uint16_t)pitch > ANIM_QUARTER_TURN && (uint16_t)pitch < ANIM_THREE_QUARTER_TURN)
						pitch = ANIM_QUARTER_TURN;
				}
			}
			playerObject->roll = roll;
			g_playerFlightState.object->yaw = yaw;
			g_playerFlightState.object->pitch = pitch;
			g_playerFlightState.object->orientMatrixDirty = 1;
			g_playerFlightState.object->moveVectorDirty = 1;
			g_playerFlightState.craft->pitch = pitch;
			return;
		}
		case ANIM_HYPERSPACE_SETUP: {
			uint16_t savedTicks = g_flightAccumulatedTicks;
			int index;
			int playerIndex;
			for (index = 0; index != XW_FLIGHT_PILOT_SLOT_COUNT; ++index) {
				uint8_t objectRef = g_PilotObjectRefs[index];
				if (objectRef != UINT8_MAX) {
					if (g_objectTable[objectRef].objectType != XW_OBJ_NONE)
						fediskio_updatepilotrecord(objectRef, 0, 0);
					else
						fediskio_updatepilotrecord(objectRef, ANIM_HYPERSPACE_LOST_PILOT, 1);
				}
			}
			playerIndex = g_playerFlightState.objectIndex;
			g_flightAccumulatedTicks = savedTicks;
			g_playerHyperspaceElapsedTicks = ANIM_HYPERSPACE_START_TICK;
			for (index = 0; index != XW_OBJECT_COUNT; ++index) {
				if (index != playerIndex) {
					g_objectTable[index].objectType = XW_OBJ_NONE;
					g_objectTable[index].ageSeconds = 0;
					g_objectTable[index].lifetimeTicks = 0;
				}
			}
			for (index = 0; index != MISSION_OBJECT_COUNT; ++index)
				g_missionObjects[index].objectType = XW_OBJ_NONE;
			g_missionHeader.flightGroupCount = 0;
			for (index = 0; index != MISSION_OBJECT_COUNT; ++index) {
				int16_t x = math2_fraction(math2_getrandom(), g_flightVpWidth) - (g_flightVpWidth >> 1);
				uint32_t depth = (math2_getrandom() >> ANIM_HYPERSPACE_RANDOM_DEPTH_SHIFT) &
								 ANIM_HYPERSPACE_RANDOM_DEPTH_MASK;
				int16_t z = math2_fraction(math2_getrandom(), g_flightVpHeight);
				depth += ANIM_HYPERSPACE_RANDOM_DEPTH_BASE;
				g_missionObjects[index].worldX = x;
				g_missionObjects[index].worldY = depth;
				g_missionObjects[index].worldZ = z - (g_flightVpHeight >> 1);
				g_missionObjects[index].objectType = XW_OBJ_NONE;
			}
			g_hyperspaceSavedBackdropsEnabled = g_backdropsEnabled;
			g_hyperspaceSavedDebrisEnabled = g_debrisEnabled;
			g_backdropsEnabled = 0;
			g_debrisEnabled = 0;
			g_playerFlightState.object->worldX = 0;
			g_playerFlightState.object->worldY = 0;
			g_playerFlightState.object->worldZ = 0;
#ifdef XW_MODERN
			XwFlightIntegration_Reposition(g_playerFlightState.objectIndex);
#endif
			g_deathStarSurfaceModeActive = 0;
			g_hyperspaceStreakLength = ANIM_HYPERSPACE_STREAK_INITIAL;
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				Dos94Hyperspace_Init();
#endif
			g_textureCacheFlushPending = 1;
			g_hyperspaceflag = ANIM_HYPERSPACE_DEPART;
			fsfx_triggersfx(FSFX_HYPERSPACE_DEPARTURE_SLOT, FSFX_UNPOSITIONED_OBJECT);
			return;
		}
		case ANIM_HYPERSPACE_DEPART:
#ifdef XW_MODERN
			if (XwFlightTypes_Dos())
				Dos94Hyperspace_Streak(false);
#endif
			if (elapsedTicks >= ANIM_HYPERSPACE_DEPART_MOVE_TICK) {
#ifdef XW_MODERN
				XwFlightMath_MoveHyperspace(ANIM_HYPERSPACE_STEP);
#else
				g_playerFlightState.object->worldY += ANIM_HYPERSPACE_STEP * g_elapsedTicks;
#endif
				elapsedTicks = g_playerHyperspaceElapsedTicks;
			} else {
#ifdef XW_MODERN
				if (!XwFlightTypes_Dos())
#endif
					g_hyperspaceStreakLength += ANIM_HYPERSPACE_STEP * g_elapsedTicks;
			}
			if (elapsedTicks >= ANIM_HYPERSPACE_EXTERNAL_TICK) {
				g_playerHyperspaceElapsedTicks = ANIM_HYPERSPACE_EXTERNAL_TICK;
				g_playerFlightState.object->worldY = 0;
#ifdef XW_MODERN
				XwFlightIntegration_Reposition(g_playerFlightState.objectIndex);
#endif
				g_flightCamera.focusObjectRef = -1;
				g_flightCamera.worldPosition.x = ANIM_HYPERSPACE_CAMERA_SIDE;
				g_flightCamera.worldPosition.y = -ANIM_HYPERSPACE_CAMERA_FORWARD;
				g_flightCamera.worldPosition.z = ANIM_HYPERSPACE_CAMERA_SIDE;
				g_flightCamera.externalViewActive = 1;
				panelrts_setnewpilotview(ANIM_HYPERSPACE_EXTERNAL_VIEW);
				g_flightCamera.hudAimX = ANIM_HYPERSPACE_CAMERA_PITCH;
				g_flightCamera.hudAimY = ANIM_HYPERSPACE_CAMERA_PITCH;
				g_hyperspaceflag = ANIM_HYPERSPACE_EXTERNAL;
				if (g_flightAudioMode != 0)

#ifdef XW_MODERN
					imuse_stop_sound(g_dos94Imuse, FSFX_HYPERSPACE_DEPARTURE_SLOT);
#else
					Sound_StopOldestInstanceById(FSFX_HYPERSPACE_DEPARTURE_SLOT);
#endif
				fsfx_triggersfx(FSFX_HYPERSPACE_EXTERNAL_SLOT, FSFX_UNPOSITIONED_OBJECT);
			}
			return;
		case ANIM_HYPERSPACE_EXTERNAL:
#ifdef XW_MODERN
			XwFlightMath_MoveHyperspace(ANIM_HYPERSPACE_STEP);
#else
			g_playerFlightState.object->worldY += ANIM_HYPERSPACE_STEP * g_elapsedTicks;
#endif
			if (g_playerHyperspaceElapsedTicks >= ANIM_HYPERSPACE_RETURN_TICK) {
				g_playerHyperspaceElapsedTicks = ANIM_HYPERSPACE_RETURN_TICK;
				g_flightCamera.focusObjectRef = g_playerFlightState.objectIndex;
				g_flightCamera.externalViewActive = 0;
				panelrts_setnewpilotview(0);
				g_flightCamera.hudAimX = 0;
				g_flightCamera.hudAimY = 0;
				g_hyperspaceflag = ANIM_HYPERSPACE_RETURN;
				fsfx_triggersfx(FSFX_HYPERSPACE_RETURN_SLOT, FSFX_UNPOSITIONED_OBJECT);
			}
			return;
		case ANIM_HYPERSPACE_RETURN:
			if (elapsedTicks >= ANIM_HYPERSPACE_RETURN_FADE_TICK) {
#ifdef XW_MODERN
				if (XwFlightTypes_Dos())
					Dos94Hyperspace_Streak(true);
				else
#endif
				{
					g_hyperspaceStreakLength += -ANIM_HYPERSPACE_STEP * g_elapsedTicks;
					if (g_hyperspaceStreakLength < ANIM_HYPERSPACE_STREAK_MINIMUM)
						g_hyperspaceStreakLength = ANIM_HYPERSPACE_STREAK_MINIMUM;
				}
			} else {
#ifdef XW_MODERN
				XwFlightMath_MoveHyperspace(-ANIM_HYPERSPACE_STEP);
#else
				g_playerFlightState.object->worldY += -ANIM_HYPERSPACE_STEP * g_elapsedTicks;
#endif
				elapsedTicks = g_playerHyperspaceElapsedTicks;
			}
			if (elapsedTicks >= ANIM_HYPERSPACE_ARRIVE_TICK) {
				g_playerHyperspaceElapsedTicks = ANIM_HYPERSPACE_ARRIVE_TICK;
				g_flightCamera.focusObjectRef = -1;
				g_flightCamera.worldPosition.x = -ANIM_HYPERSPACE_CAMERA_SIDE;
				g_flightCamera.worldPosition.y = ANIM_HYPERSPACE_CAMERA_FORWARD;
				g_flightCamera.worldPosition.z = ANIM_HYPERSPACE_CAMERA_SIDE;
				g_flightCamera.externalViewActive = 1;
				panelrts_setnewpilotview(ANIM_HYPERSPACE_EXTERNAL_VIEW);
				g_flightCamera.hudAimX = ANIM_HYPERSPACE_CAMERA_PITCH;
				g_flightCamera.hudAimY = ANIM_HYPERSPACE_CAMERA_RETURN_YAW;
				g_playerFlightState.object->worldY -= ANIM_HYPERSPACE_RETURN_OFFSET;
#ifdef XW_MODERN
				XwFlightIntegration_Reposition(g_playerFlightState.objectIndex);
#endif
				g_backdropsEnabled = g_hyperspaceSavedBackdropsEnabled;
				g_debrisEnabled = g_hyperspaceSavedDebrisEnabled;
				g_hyperspaceflag = ANIM_HYPERSPACE_ARRIVE;
			}
			return;
		case ANIM_HYPERSPACE_ARRIVE: {
			int worldY;
			int step;
			g_playerFlightState.object->speed = ANIM_HYPERSPACE_ARRIVE_SPEED;
			worldY = g_playerFlightState.object->worldY;
			step = ANIM_HYPERSPACE_SLOWDOWN_POSITION - worldY;
			if (step <= 0) {
				uint8_t objectType = g_playerFlightState.object->objectType;
				if ((objectType == XW_OBJ_X_WING && g_playerFlightState.craft->sFoilState != 0) ||
					(objectType ==
#ifdef XW_MODERN
						 XwFlightTypes_ObjectType(XW_OBJ_B_WING)
#else
						 XW_OBJ_B_WING
#endif
					 && g_playerFlightState.craft->sFoilState != 0)) {
					g_playerFlightState.craft->sFoilState = 0;
					g_playerFlightState.craft->sFoilState |= XW_SFOIL_MOVING;
#ifdef XW_MODERN
					if (XwFlightTypes_Dos())
						Dos94Hyperspace_SfoilSound();
#endif
				} else if (worldY > 0) {
					g_hyperspaceflag = ANIM_HYPERSPACE_IDLE;
					g_missionRuntimeState.flightExitRequested = ANIM_HYPERSPACE_FLIGHT_EXIT;
					g_flightCamera.externalDistance -= (uint16_t)g_playerFlightState.object->worldY;
					g_playerFlightState.object->speed = 0;
					msg_messageprintf(XW_MSG_HYPERSPACE_JUMP_COMPLETED);
				}
			} else {
				step >>= ANIM_HYPERSPACE_SLOWDOWN_SHIFT;
				if (step > ANIM_HYPERSPACE_STEP)
					step = ANIM_HYPERSPACE_STEP;
				g_playerFlightState.object->worldY = worldY + g_elapsedTicks * step;
			}
			return;
		}
		default:
			return;
	}
}
