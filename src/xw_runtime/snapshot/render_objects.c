#include "xw_runtime/snapshot/render_objects.h"
#include "xw/assets/model_mesh.h"
#include "xw/flight/death_star.h"
#include "xw/flight/mission/mission.h"
#include "xw/flight/object/anim.h"
#include "xw/flight/object/create.h"
#include "xw/render/render_scene.h"
#include "xw_runtime/runtime/flight_types.h"
#include "xw_runtime/runtime/profile.h"
#include "xw_runtime/snapshot/render_capture.h"
#include <aeron/aeron.h>
#include <stdio.h>
#include <string.h>

static uint32_t mobile_generation[XW_OBJECT_COUNT], mission_generation[MISSION_OBJECT_COUNT];
static bool reported_error;

void XwRenderObjects_Reset(void) {
	reported_error = false;
	for (unsigned i = 0; i < XW_OBJECT_COUNT; ++i)
		mobile_generation[i] = 1;
	for (unsigned i = 0; i < MISSION_OBJECT_COUNT; ++i)
		mission_generation[i] = 1;
}

static bool CaptureError(XwRenderSnapshot* out, unsigned reference, const char* reason) {
	if (!reported_error) {
		char message[512];
		snprintf(message, sizeof message, "Cannot capture flight object 0x%04x:\n\n%s", reference, reason);
		Aeron_RequestFatalError("Renderer Error", message);
		reported_error = true;
	}
	out->object_count = out->craft_count = 0;
	return false;
}

static void Replace(uint32_t* generation) {
	if (*generation == UINT32_MAX)
		XwRenderCapture_WorldChanged();
	else
		++*generation;
}

void XwRenderObjects_ReplaceMobile(uint16_t slot) {
	if (slot < XW_OBJECT_COUNT)
		Replace(&mobile_generation[slot]);
}

void XwRenderObjects_ReplaceMission(uint16_t slot) {
	if (slot < MISSION_OBJECT_COUNT)
		Replace(&mission_generation[slot]);
}

XwSnapObjectId XwRenderObjects_Id(uint16_t reference) {
	const XwFlightProfile* profile = XwProfile_ActiveFlight();
	if (reference < profile->object_count && reference < XW_OBJECT_COUNT &&
		g_objectTable[reference].objectType)
		return (XwSnapObjectId) { .kind = XW_SNAP_OBJECT_MOBILE,
								  .slot = reference,
								  .generation = mobile_generation[reference] };
	unsigned slot = (unsigned)reference - XW_MISSION_OBJECT_REF_BASE;
	if (slot < profile->static_object_count && slot < MISSION_OBJECT_COUNT &&
		g_missionObjects[slot].objectType)
		return (XwSnapObjectId) { .kind = XW_SNAP_OBJECT_MISSION,
								  .slot = slot,
								  .generation = mission_generation[slot] };
	return (XwSnapObjectId) { 0 };
}

static void CaptureCraft(XwSnapCraft* out, const CraftData* craft, uint8_t type,
						 const XwFlightProfile* profile) {
	unsigned count = XwFlightTypes_ComponentCount(type);
	*out = (XwSnapCraft) { .component_count =
							   count < profile->component_count ? count : profile->component_count,
						   .object_kind = craft->objectKind,
						   .sfoil_state = craft->sFoilState,
						   .working_subsystems = craft->workingSubsystems,
						   .laser_redirect = craft->laserRedirect,
						   .shield_redirect = craft->shieldRedirect };
	memcpy(out->component_state, craft->componentState, profile->component_count);
	memcpy(out->component_hp, craft->componentHp, profile->component_count);
	memcpy(out->mesh_rotation, craft->meshRotation, profile->component_count);
	if (!XwFlightTypes_Dos() && count < XW_SNAP_COMPONENTS) {
		unsigned index = craft->componentState[count];
		if (index < sizeof g_componentDamageAnimationFrames / sizeof g_componentDamageAnimationFrames[0]) {
			uint16_t frame = g_componentDamageAnimationFrames[index];
			if (frame >= RENDER_DAMAGE_FIRST_FRAME && frame < ANIM_FRAME_JUMP_BASE) {
				out->damage_frame = frame;
				out->damage_frame_valid = 1;
			}
		}
	}
	unsigned definition = XwFlightTypes_Definition(type);
	if (definition < profile->craft_definition_count) {
		unsigned engines = g_craftTypeDefs[definition].engineCount;
		out->engine_count = engines < 4 ? engines : 4;
		if (craft->workingSubsystems & XW_CRAFT_SUBSYSTEM_ENGINE)
			memcpy(out->engine_output_q16, craft->engineOutputQ16,
				   out->engine_count * sizeof out->engine_output_q16[0]);
	}
}

static void CaptureMobile(XwSnapObject* out, unsigned slot) {
	const ObjectRecord* object = &g_objectTable[slot];
	*out = (XwSnapObject) {
		.id = XwRenderObjects_Id(slot),
		.craft_index = UINT16_MAX,
		.world_pos = { object->worldX, object->worldY, object->worldZ },
		.previous_world_pos = { object->prevWorldX, object->prevWorldY, object->prevWorldZ },
		.yaw = object->yaw,
		.pitch = object->pitch,
		.roll = object->roll,
		.cached_rows_q15 = { object->cachedSideX, object->cachedSideY, object->cachedSideZ,
							 object->cachedForwardX, object->cachedForwardY, object->cachedForwardZ,
							 object->cachedUpX, object->cachedUpY, object->cachedUpZ },
		.source_ref = object->sourceObjectRef,
		.source_type = object->sourceObjectType,
		.speed = object->speed,
		.type = object->objectType,
		.genus = object->genusId,
		.family = object->familyId,
		.iff = object->iff,
		.markings = object->markings,
		.billboard_scale = object->billboardScaleCode,
		.animation_state = object->animationState,
		.secondary_animation_state = object->secondaryAnimationState,
		.orientation_dirty = object->orientMatrixDirty,
		.slot_class = slot < XW_CRAFT_OBJECT_COUNT ? XW_SNAP_SLOT_CRAFT
												   : (slot >= (unsigned)g_debrisObjectSlotStart &&
															  slot < (unsigned)g_debrisObjectSlotEnd
														  ? XW_SNAP_SLOT_LOCAL_DEBRIS
														  : XW_SNAP_SLOT_MAIN_OTHER)
	};
}

static void MissionAngles(XwSnapObject* out, const XwMissionObjectRecord* object) {
	out->yaw = (uint16_t)object->yawAngle8 << 8;
	out->pitch = (uint16_t)object->pitchAngle8 << 8;
	out->roll = (uint16_t)object->rollAngle8 << 8;
}

static void CaptureMission(XwSnapObject* out, unsigned slot) {
	const XwMissionObjectRecord* object = &g_missionObjects[slot];
	*out = (XwSnapObject) { .id = XwRenderObjects_Id(XW_MISSION_OBJECT_REF_BASE + slot),
							.craft_index = UINT16_MAX,
							.world_pos = { object->worldX * 256, object->worldY * 256, object->worldZ * 256 },
							.previous_world_pos = { object->worldX * 256, object->worldY * 256,
													object->worldZ * 256 },
							.type = object->objectType,
							.genus = object->genusId,
							.family = g_modelTypeTable[object->objectType].familyId,
							.state = object->stateByte,
							.type_specific = object->typeSpecificByte,
							.orientation_dirty = 1,
							.slot_class = XW_SNAP_SLOT_MISSION };
	MissionAngles(out, object);
}

bool XwRenderObjects_Capture(XwRenderSnapshot* out) {
	const XwFlightProfile* profile = XwProfile_ActiveFlight();
	/* Model submission gates are latched with the world, including skipped classic draws. */
	out->hyperspace.phase = g_hyperspaceflag;
	out->special.proving_grounds_active = g_missionRuntimeState.provingGroundsActive != 0;
	out->special.surface_active = g_deathStarSurfaceModeActive != 0;
	if (profile->object_count > XW_OBJECT_COUNT || profile->static_object_count > MISSION_OBJECT_COUNT ||
		profile->component_count > XW_SNAP_COMPONENTS)
		return CaptureError(out, UINT16_MAX, "profile exceeds snapshot bounds");
	out->object_count = out->craft_count = 0;
	for (unsigned slot = 0; slot < profile->object_count; ++slot) {
		const ObjectRecord* object = &g_objectTable[slot];
		if (!object->objectType)
			continue;
		if (object->objectType >= profile->model_count)
			return CaptureError(out, slot, "type is outside the active profile");
		XwSnapObject* captured = &out->objects[out->object_count++];
		CaptureMobile(captured, slot);
		if (slot >= XW_CRAFT_OBJECT_COUNT || object->familyId != XW_OBJECT_FAMILY_CRAFT ||
			object->genusId > XW_GENUS_STARSHIP)
			continue;
		/* Detached fragments can borrow this pool too, but are not craft owners. */
		for (unsigned craft = 0; craft < XW_CRAFT_OBJECT_COUNT; ++craft) {
			if (object->instanceData != &g_craftTable[craft])
				continue;
			captured->craft_index = out->craft_count;
			CaptureCraft(&out->crafts[out->craft_count++], &g_craftTable[craft], object->objectType, profile);
			break;
		}
		if (captured->craft_index == UINT16_MAX)
			return CaptureError(out, slot, "craft has no owner in the craft pool");
	}
	for (unsigned slot = 0; slot < profile->static_object_count; ++slot) {
		if (!g_missionObjects[slot].objectType)
			continue;
		if (g_missionObjects[slot].objectType >= profile->model_count)
			return CaptureError(out, XW_MISSION_OBJECT_REF_BASE + slot, "type is outside the active profile");
		CaptureMission(&out->objects[out->object_count++], slot);
	}
	return true;
}

void XwRenderObjects_MissionPose(uint16_t slot) {
	XwRenderSnapshot* pending = XwRenderCapture_Pending();
	if (!pending || slot >= MISSION_OBJECT_COUNT)
		return;
	for (unsigned i = 0; i < pending->object_count; ++i) {
		XwSnapObject* object = &pending->objects[i];
		if (object->id.kind == XW_SNAP_OBJECT_MISSION && object->id.slot == slot &&
			object->id.generation == mission_generation[slot]) {
			MissionAngles(object, &g_missionObjects[slot]);
			return;
		}
	}
}
