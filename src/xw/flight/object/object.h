#ifndef XW_FLIGHT_OBJECT_OBJECT_H
#define XW_FLIGHT_OBJECT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct ObjectRecord ObjectRecord;
typedef struct XwObjectSlotRange XwObjectSlotRange;

enum { XW_OBJECT_REF_KIND_MASK = 0xFF00, XW_MISSION_OBJECT_REF_BASE = 0x3800 };

enum { XW_OBJECT_ANIMATION_BREAKUP = 2 };

enum { XW_OBJECT_FAMILY_CRAFT = 0, XW_OBJECT_FAMILY_5 = 5 };

enum { XW_OBJECT_OPPOSING_IFF_MASK = 1 };

typedef uint8_t XwObjectGenus;

enum XwObjectGenusValues {
	XW_GENUS_STARFIGHTER = 0x0,
	XW_GENUS_TRANSPORT = 0x1,
	XW_GENUS_UTILITY = 0x2,
	XW_GENUS_FREIGHTER = 0x3,
	XW_GENUS_STARSHIP = 0x4,
	XW_GENUS_PLAYER_PROJECTILE = 0x5,
	XW_GENUS_OTHER_PROJECTILE = 0x6,
	XW_GENUS_MINE = 0x7,
	XW_GENUS_BUOY_SATELLITE_PROBE = 0x8,
	XW_GENUS_ASTEROID = 0x9,
	XW_GENUS_DEBRIS = 0xA,
	XW_GENUS_PLANET_BACKDROP = 0xB,
	XW_GENUS_STARFIELD_BACKDROP = 0xC,
	XW_GENUS_EXPLOSION_EFFECT = 0xD,
	XW_GENUS_SCENERY = 0xE
};

/* Original IDB size: 4 bytes. */
struct XwObjectSlotRange {
	/* IDB +0x0 */
	uint16_t start;
	/* IDB +0x2 */
	uint16_t end;
};

typedef uint8_t XwObjectTypeId;

enum XwObjectTypeIdValues {
	XW_OBJ_NONE = 0x0,
	XW_OBJ_X_WING = 0x1,
	XW_OBJ_Y_WING = 0x2,
	XW_OBJ_A_WING = 0x3,
	XW_OBJ_B_WING = 0x4,
	XW_OBJ_TIE_FIGHTER = 0x5,
	XW_OBJ_TIE_INTERCEPTOR = 0x6,
	XW_OBJ_TIE_BOMBER = 0x7,
	XW_OBJ_TIE_ADVANCED = 0x8,
	XW_OBJ_MODEL_TIEDEL = 0x9,
	XW_OBJ_MODEL_MISLBT = 0xC,
	XW_OBJ_MODEL_FGHTA = 0xD,
	XW_OBJ_MODEL_Z_95 = 0xE,
	XW_OBJ_MODEL_FGHTB = 0xF,
	XW_OBJ_ASSAULT_GUNBOAT = 0x10,
	XW_OBJ_SHUTTLE = 0x11,
	XW_OBJ_MODEL_SHUTB = 0x12,
	XW_OBJ_MODEL_PATRL = 0x13,
	XW_OBJ_MODEL_PATRLB = 0x14,
	XW_OBJ_TRANSPORT = 0x15,
	XW_OBJ_MODEL_TRANSB = 0x16,
	XW_OBJ_MODEL_ESCTRP = 0x17,
	XW_OBJ_TUG = 0x18,
	XW_OBJ_MODEL_UTILA = 0x19,
	XW_OBJ_CONTAINER = 0x1A,
	XW_OBJ_MODEL_CONA = 0x1B,
	XW_OBJ_MODEL_CONB = 0x1C,
	XW_OBJ_MODEL_CONC = 0x1D,
	XW_OBJ_MODEL_BARGE = 0x1E,
	XW_OBJ_FREIGHTER = 0x20,
	XW_OBJ_MODEL_FRTB = 0x21,
	XW_OBJ_MODEL_FRTA = 0x22,
	XW_OBJ_MODEL_FRTC = 0x23,
	XW_OBJ_MODEL_LTFRTA = 0x25,
	XW_OBJ_MODEL_CORTN = 0x26,
	XW_OBJ_CORELLIAN_CORVETTE = 0x28,
	XW_OBJ_MODEL_CORVTA = 0x29,
	XW_OBJ_NEBULON_B_FRIGATE = 0x2A,
	XW_OBJ_MODEL_FRIGA = 0x2B,
	XW_OBJ_MODEL_PASNGR = 0x2C,
	XW_OBJ_MODEL_CRUSA = 0x2D,
	XW_OBJ_MODEL_CRUSB = 0x2E,
	XW_OBJ_MODEL_ESCRT = 0x2F,
	XW_OBJ_MODEL_DREAD = 0x30,
	XW_OBJ_CALAMARI_CRUISER = 0x31,
	XW_OBJ_MODEL_LTCAL = 0x32,
	XW_OBJ_INTERDICTOR = 0x33,
	XW_OBJ_MODEL_VSD = 0x34,
	XW_OBJ_IMPERIAL_STAR_DESTROYER = 0x35,
	XW_OBJ_MODEL_COND = 0x37,
	XW_OBJ_MODEL_CONE = 0x38,
	XW_OBJ_MODEL_CONF = 0x39,
	XW_OBJ_MODEL_CONG = 0x3A,
	XW_OBJ_MODEL_CONH = 0x3B,
	XW_OBJ_MODEL_PLATA = 0x3C,
	XW_OBJ_MODEL_PLATB = 0x3D,
	XW_OBJ_MODEL_PLATC = 0x3E,
	XW_OBJ_MODEL_PLATAB = 0x3F,
	XW_OBJ_MODEL_PLATBA = 0x40,
	XW_OBJ_MODEL_PLATCA = 0x41,
	XW_OBJ_MODEL_ASTH44 = 0x42,
	XW_OBJ_MODEL_ASTG33 = 0x43,
	XW_OBJ_MODEL_ASTW21 = 0x44,
	XW_OBJ_MODEL_FACTRY = 0x45,
	XW_OBJ_DETACHED_COMPONENT = 0x59,
	XW_OBJ_EXPLOSION_133 = 0x85,
	XW_OBJ_EXPLOSION_134 = 0x86,
	XW_OBJ_EXPLOSION_135 = 0x87,
	XW_OBJ_EXPLOSION_136 = 0x88,
	XW_OBJ_ASTEROID_IMPACT = 0x89,
	XW_OBJ_EXPLOSION_138 = 0x8A,
	XW_OBJ_LASER_143 = 0x8F,
	XW_OBJ_LASER_144 = 0x90,
	XW_OBJ_LASER_145 = 0x91,
	XW_OBJ_LASER_146 = 0x92,
	XW_OBJ_ION_147 = 0x93,
	XW_OBJ_ION_148 = 0x94,
	XW_OBJ_WARHEAD_149 = 0x95,
	XW_OBJ_TRACKED_WARHEAD = 0x96
};

/* Original IDB size: 83 bytes. */
struct ObjectRecord {
	/* IDB +0x0: Object family/category, copied from model definition at craft spawn: 0 craft, 1 projectile, 3
	 * debris, 5 explosion/effect in verified constructors. Nonzero excludes it from craft AI; NOT an inactive
	 * flag (objectType==0 marks a free slot). */
	uint8_t familyId;
	/* IDB +0x1: Runtime XwObjectGenus; copied from model metadata at craft spawn or assigned by
	 * projectile/debris/effect constructors. */
	XwObjectGenus genusId;
	/* IDB +0x2: Runtime species/model-table index. Zero means inactive. Ship IDs are named in XwObjectTypeId;
	 * unlisted non-ship IDs remain valid. Distinct from mission craftType and CraftData.craftTypeIndex. */
	XwObjectTypeId objectType;
	/* IDB +0x3: Encoded billboard/effect scale. SceneBillboard_QueueObjectTextured computes code<<6, adds 256
	 * when result>=256, or uses 256 when code==0. Explosion spawners derive this byte from effect scale;
	 * shared object data, not an AI state. */
	uint8_t billboardScaleCode;
	/* IDB +0x4 */
	int worldX;
	/* IDB +0x8 */
	int worldY;
	/* IDB +0xC */
	int worldZ;
	/* IDB +0x10 */
	int prevWorldX;
	/* IDB +0x14 */
	int prevWorldY;
	/* IDB +0x18 */
	int prevWorldZ;
	/* IDB +0x1C: Verified in FVIEW_SetObjectTransform, FVIEW_calcrotateorient and
	 * RenderBillboard_DrawRollAlignedObjectModel. Cached vectors use signed Q15. */
	int16_t yaw;
	/* IDB +0x1E: Verified in FVIEW_SetObjectTransform, FVIEW_calcrotateorient and
	 * RenderBillboard_DrawRollAlignedObjectModel. Cached vectors use signed Q15. */
	int16_t pitch;
	/* IDB +0x20: Verified in FVIEW_SetObjectTransform, FVIEW_calcrotateorient and
	 * RenderBillboard_DrawRollAlignedObjectModel. Cached vectors use signed Q15. */
	int16_t roll;
	/* IDB +0x22 */
	int16_t rollImpulseRate;
	/* IDB +0x24: Unsigned internal speed used by HUD conversion with Q16 factor 0x71C7 and flight movement.
	 */
	uint16_t speed;
	/* IDB +0x26: Fractional low word of speed integration. DYNAMIX_addvelocity/subvelocity accumulate
	 * g_mathDivideFractionQ16 and carry/borrow into speed; initialized to zero at craft spawn. */
	uint16_t speedFractionQ16;
	/* IDB +0x28: Impact damage source value read by collide_damagecraft (before target-genus/component
	 * scaling). Craft spawn uses min(4*model bounds extent,0x7FFF); projectile spawn uses per-type base plus
	 * source speed. Despite that unusual constructor arithmetic, collision readers do consume it as damage.
	 */
	uint16_t damageAmount;
	/* IDB +0x2A */
	int16_t lifetimeTicks;
	/* IDB +0x2C: Initialized to 1 at spawn; XW_updatetime increments this counter once per mission clock
	 * second. */
	uint16_t ageSeconds;
	/* IDB +0x2E: Projectile source/attribution reference. Normal shots store firing object index; mine firing
	 * stores selected target index + 0x3800. */
	uint16_t sourceObjectRef;
	/* IDB +0x30: Original runtime species of a craft/projectile/effect source. Detached-component type89 uses
	 * this byte to select the original model in RenderScene_DrawNoAssetSourceModel. Surface/mine/gate guns
	 * set zero. */
	XwObjectTypeId sourceObjectType;
	/* IDB +0x31: Allegiance/IFF. Initialized from the craft-type default or mission override; AI uses iff ^ 1
	 * for the opposing side and messages compare it with the player. */
	uint8_t iff;
	/* IDB +0x32: Copied from mission flight-group markings; RenderScene_DrawNoAssetSourceModel supplies it as
	 * g_nodeSwitchIndex. */
	uint8_t markings;
	/* IDB +0x33: Animation-stream index read by SceneBillboard_QueueObjectTextured and advanced by
	 * ANIM_updateanimation. For object type 89 (detached component), holds 2*mesh index instead. */
	uint8_t animationState;
	/* IDB +0x34: Secondary effect-animation index used for detached-component type 89 with word_4C5060.
	 * Initialized to 0, set to 2 for breakup effects, reset by ANIM_updateanimation. */
	uint8_t secondaryAnimationState;
	/* IDB +0x35: Movement direction cache invalid flag; FVIEW_calcrotatemove writes +54/+56/+58 then clears
	 * this byte. */
	uint8_t moveVectorDirty;
	/* IDB +0x36: Signed Q15 unrolled movement component: negative current row2.X, copied from g_craftMoveX by
	 * FVIEW_calcrotatemove. */
	int16_t moveX;
	/* IDB +0x38: Signed Q15 unrolled movement component: negative current row2.Y, copied from g_craftMoveZ by
	 * FVIEW_calcrotatemove. */
	int16_t moveY;
	/* IDB +0x3A: Signed Q15 unrolled movement component: negative current row2.Z, copied from g_craftMoveY by
	 * FVIEW_calcrotatemove. */
	int16_t moveZ;
	/* IDB +0x3C: Nonzero forces FVIEW_SetObjectTransform to rebuild from supplied angles; zero uses cached
	 * signed Q15 basis at +61..77. FVIEW_calcrotateorient fills cache then clears this byte. */
	uint8_t orientMatrixDirty;
	/* IDB +0x3D: Signed Q15 forward X: negative of current row2.X before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedForwardX;
	/* IDB +0x3F: Signed Q15 forward Y: negative of current row2.Y before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedForwardY;
	/* IDB +0x41: Signed Q15 forward Z: negative of current row2.Z before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedForwardZ;
	/* IDB +0x43: Signed Q15 side X: equal to current row0.X before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedSideX;
	/* IDB +0x45: Signed Q15 side Y: equal to current row0.Y before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedSideY;
	/* IDB +0x47: Signed Q15 side Z: equal to current row0.Z before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedSideZ;
	/* IDB +0x49: Signed Q15 up X: equal to current row1.X before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedUpX;
	/* IDB +0x4B: Signed Q15 up Y: equal to current row1.Y before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedUpY;
	/* IDB +0x4D: Signed Q15 up Z: equal to current row1.Z before camera-only flips. Written by
	 * FVIEW_calcrotateorient, sign-extended by FVIEW_SetObjectTransform when orientation cache is valid. */
	int16_t cachedUpZ;
	/* IDB +0x4F: Polymorphic instance-data pointer. Live craft slots bind CraftData; projectile constructors
	 * bind WarheadGuidanceState (3 bytes); some debris and the synthetic Death Star render object use
	 * other/copied storage. Keep void* globally; cast only after establishing object kind/context. */
	void* instanceData;
};

/* Declarations follow ascending original IDB address. */

#ifdef __cplusplus
}
#endif

#endif
