#ifndef XW_FLIGHT_OBJECT_CRAFT_H
#define XW_FLIGHT_OBJECT_CRAFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/flight/object/laser.h>
#include <xw/flight/object/move.h>

typedef struct CraftData CraftData;
typedef struct CraftLaserState CraftLaserState;
typedef struct CraftWeaponSlot CraftWeaponSlot;
typedef struct XwCraftKillStats XwCraftKillStats;
typedef struct XwCraftLocalPoint XwCraftLocalPoint;
typedef struct XwCraftModelBounds XwCraftModelBounds;
typedef struct XwCraftTypeDef XwCraftTypeDef;
typedef struct XwWeaponHitStats XwWeaponHitStats;

/* Preserve the observed kind value without inferring a more specific state. */
enum {
	XW_CRAFT_OBJECT_KIND_0 = 0,
	XW_CRAFT_OBJECT_KIND_1 = 1,
	XW_CRAFT_OBJECT_KIND_2 = 2,
	XW_CRAFT_OBJECT_KIND_3 = 3,
	XW_CRAFT_OBJECT_KIND_4 = 4,
	XW_CRAFT_OBJECT_KIND_5 = 5,
	XW_CRAFT_OBJECT_KIND_6 = 6
};

enum {
	XW_AI_PITCH_INACTIVE = 0,
	XW_AI_PITCH_DECREMENT = 1,
	XW_AI_PITCH_INCREMENT = 2,
	XW_AI_PITCH_TARGET_REACHED = 3,
	XW_AI_ROLL_STATE_1 = 1,
	XW_AI_ROLL_STATE_2 = 2,
	XW_AI_ROLL_STATE_3 = 3,
	XW_AI_ROLL_TARGET_REACHED = 4,
	XW_AI_YAW_INACTIVE = 0,
	XW_AI_YAW_STATE_1 = 1,
	XW_AI_YAW_STEER = 2,
	XW_AI_YAW_TARGET_REACHED = 3,
	XW_AI_COMMAND_FULL = 0xFFFF
};

enum { XW_SFOIL_MOVING = 1, XW_SFOIL_CLOSED = 2 };

enum { XW_CRAFT_NO_AI_LEADER = 0xFF };

enum { XW_CRAFT_ESCORT_NO_FUTURE_GROUP = 0x80, XW_CRAFT_ESCORT_WAITING_FOR_GROUP = 0xFF };

enum { XW_CRAFT_THROTTLE_HALF = 0x8000, XW_CRAFT_THROTTLE_FULL = 0xFFFF };

enum {
	XW_CRAFT_SUBSYSTEM_ALL = 0xFF,
	XW_CRAFT_BOARDING_COMPLETE = 2,
	XW_CRAFT_SUBSYSTEM_SHIELDS = 1,
	XW_CRAFT_SUBSYSTEM_2 = 2,
	XW_CRAFT_SUBSYSTEM_TARGETING = 4,
	XW_CRAFT_SUBSYSTEM_LAUNCHER = 8,
	XW_CRAFT_SUBSYSTEM_ATTITUDE = 0x20,
	XW_CRAFT_SUBSYSTEM_ENGINE = 0x40,
	XW_CRAFT_SUBSYSTEM_HYPERDRIVE = 0x80,
	XW_LAUNCHER_FIRE_MODE_MASK = 0x7F,
	XW_LAUNCHER_FIRE_LINKED = 3,
	XW_LAUNCHER_SELECTED_SLOT_SHIFT = 7
};

enum { XW_CRAFT_TYPE_COUNT = 69 };

enum {
	XW_SHIELD_FRONT = 0,
	XW_SHIELD_REAR = 1,
	XW_SHIELD_BANK_COUNT = 2,
	XW_SHIELD_MAX_CHARGE_MULTIPLIER = 2
};

/* Original IDB size: 24 bytes. */
struct CraftLaserState {
	/* IDB +0x0: Projectile class for each laser group. */
	uint8_t projectileTypeId[4];
	/* IDB +0x4: Firing link mode: 1=single cycling slot, 2=paired slots, 3=all group slots; zero disables
	 * firing. */
	uint8_t linkMode[4];
	/* IDB +0x8: Decremented after non-player automatic group firing; reaching zero clears linkMode. */
	uint8_t burstShotsRemaining[4];
	/* IDB +0xC: Next slot selected by single/pair fire; wraps within model group first/last slots. */
	uint8_t nextSlot[4];
	/* IDB +0x10: Per-group cooldown: firing writes 78*shots+2; weapon update subtracts elapsed ticks with a
	 * zero floor. */
	int16_t fireCooldownTicks[4];
};

/* Original IDB size: 4 bytes. */
struct CraftWeaponSlot {
	/* IDB +0x0: Dual-use signed word: 0 selects ordinary fixed/warhead slots; -1 means no turret target;
	 * positive values encode turret target object index + 1. AI target selection refreshes nonzero slots, and
	 * laser_weaponsfire subtracts one before automated fire. */
	int16_t firingGate;
	/* IDB +0x2: Ordinary lasers: signed charge byte replenished/clamped to 0..127, consumed on firing, >=64
	 * selects high-charge projectile. Automated turret slots: low 7 bits are cooldown (reset 59), high bit
	 * selects alternate projectile. */
	int8_t laserCharge;
	/* IDB +0x3: Slot quantity from the craft-type slot descriptor; warhead firing decrements this ammunition
	 * byte. */
	uint8_t count;
};

/* Original IDB size: 28 bytes. */
struct XwCraftKillStats {
	/* IDB +0x0: Kill counts by the 24-entry X-Wing statistics category; saturates at 255. */
	uint8_t spacecraftByType[24];
	/* IDB +0x18: Destroyed static space objects; incremented when victim index is 0xFFFF and spaceObject is
	 * nonzero. */
	uint16_t spaceObjects;
	/* IDB +0x1A: Destroyed Death Star buildings; incremented when victim index is 0xFFFF and spaceObject is
	 * zero. */
	uint16_t deathStarBuildings;
};

/* Original IDB size: 6 bytes. */
struct XwCraftLocalPoint {
	/* IDB +0x0 */
	int16_t side;
	/* IDB +0x2 */
	int16_t up;
	/* IDB +0x4 */
	int16_t forward;
};

/* Original IDB size: 8 bytes. */
struct XwCraftModelBounds {
	/* IDB +0x0: Shared right-shift count used while reducing unsigned dimensions to <=640; extent reader uses
	 * low byte as x86 shift count. */
	uint16_t boundSizeShift;
	/* IDB +0x2: Model X span reduced by shared shift; stored low word, sign-extended by target extent reader.
	 */
	int16_t boundSizeX;
	/* IDB +0x4: Model Y span reduced by shared shift; stored low word, sign-extended by target extent reader.
	 */
	int16_t boundSizeY;
	/* IDB +0x6: Model Z span reduced by shared shift; stored low word, sign-extended by target extent reader.
	 */
	int16_t boundSizeZ;
};

typedef uint16_t XwCraftSpecies;

enum XwCraftSpeciesValues {
	XW_CRAFT_SPECIES_NONE = 0x0,
	XW_CRAFT_SPECIES_X_WING = 0x1,
	XW_CRAFT_SPECIES_Y_WING = 0x2,
	XW_CRAFT_SPECIES_A_WING = 0x3,
	XW_CRAFT_SPECIES_TIE_FIGHTER = 0x4,
	XW_CRAFT_SPECIES_TIE_INTERCEPTOR = 0x5,
	XW_CRAFT_SPECIES_TIE_BOMBER = 0x6,
	XW_CRAFT_SPECIES_ASSAULT_GUNBOAT = 0x7,
	XW_CRAFT_SPECIES_TRANSPORT = 0x8,
	XW_CRAFT_SPECIES_SHUTTLE = 0x9,
	XW_CRAFT_SPECIES_TUG = 0xA,
	XW_CRAFT_SPECIES_CONTAINER = 0xB,
	XW_CRAFT_SPECIES_FREIGHTER = 0xC,
	XW_CRAFT_SPECIES_CALAMARI_CRUISER = 0xD,
	XW_CRAFT_SPECIES_NEBULON_B_FRIGATE = 0xE,
	XW_CRAFT_SPECIES_CORELLIAN_CORVETTE = 0xF,
	XW_CRAFT_SPECIES_IMPERIAL_STAR_DESTROYER = 0x10,
	XW_CRAFT_SPECIES_TIE_ADVANCED = 0x11
};

/* Original IDB size: 15 bytes. */
struct XwWeaponHitStats {
	/* IDB +0x0 */
	uint16_t laserShotsFired;
	/* IDB +0x2: Debrief spacecraft-hit category; includes hit-credit calls for static mission objects. */
	uint16_t laserSpacecraftHits;
	/* IDB +0x4: Surface/terrain-hit category, recorded separately from spacecraft hits. */
	uint16_t laserSurfaceHits;
	/* IDB +0x6 */
	uint16_t ionShotsFired;
	/* IDB +0x8: Debrief spacecraft-hit category; includes hit-credit calls for static mission objects. */
	uint16_t ionSpacecraftHits;
	/* IDB +0xA: Surface/terrain-hit category, recorded separately from spacecraft hits. */
	uint16_t ionSurfaceHits;
	/* IDB +0xC */
	uint8_t warheadsFired;
	/* IDB +0xD: Debrief spacecraft-hit category; includes hit-credit calls for static mission objects. */
	uint8_t warheadSpacecraftHits;
	/* IDB +0xE: Surface/terrain-hit category, recorded separately from spacecraft hits. */
	uint8_t warheadSurfaceHits;
};

/* Original IDB size: 462 bytes. */
struct CraftData {
	/* IDB +0x0: Index into the 69-entry g_craftTypeDefs performance/weapon table, obtained via
	 * GetModelIndexFromType(runtime species). Not a mission craftType or runtime species ID; e.g. X-wing
	 * species1 maps to definition0. */
	uint8_t craftTypeIndex;
	/* IDB +0x1: Initialized to 3 in Mission_InitFlightGroupObjectSlot. No semantic reader established in the
	 * 244-function audit; retain a neutral name. */
	uint8_t field01;
	/* IDB +0x2: Unsigned Q16 AI skill from g_aiSkillValueQ16ByLevel (or larger pilot override) at spawn;
	 * pai_ProcessPlan derives a tier and maneuvers scale steering steps from it. */
	uint16_t aiSkillQ16;
	/* IDB +0x4 */
	uint8_t field04;
	/* IDB +0x5 */
	uint8_t flightGroupIndex;
	/* IDB +0x6: Leader object slot used by AI formation/plan selection; 0xFF means no leader. Initialized
	 * from spawn context; dereferenced by pai_UpdateAllCraftAI and follow-leader maneuvers. */
	uint8_t aiLeaderObjectIndex;
	/* IDB +0x7 */
	uint8_t gap07[2];
	/* IDB +0x9: Current craft pitch, initialized from spawn pitch and integrated by DYNAMIX_planedynamics
	 * before copying to ObjectRecord.pitch. 0x4000 is level; distinct from aiTargetPitch. */
	uint16_t pitch;
	/* IDB +0xB: Current yaw copy: initialized from g_spawnYaw and refreshed from ObjectRecord.yaw by
	 * DYNAMIX_planedynamics. Not an AI target. */
	uint16_t yaw;
	/* IDB +0xD: Unresolved bytes; no semantic access established in this audit. Do not import descendant
	 * breakup-rate meanings without X-Wing evidence. */
	uint8_t field0D[4];
	/* IDB +0x11: Camera-space coordinate written by FlightView_ComputeObjectViewPosition for slots below 28
	 * and copied to the matching global. */
	int viewX;
	/* IDB +0x15: Camera-space coordinate written by FlightView_ComputeObjectViewPosition for slots below 28
	 * and copied to the matching global. */
	int viewY;
	/* IDB +0x19: Camera-space coordinate written by FlightView_ComputeObjectViewPosition for slots below 28
	 * and copied to the matching global. */
	int viewZ;
	/* IDB +0x1D: Initialized to zero at spawn; purpose unresolved. */
	uint8_t field1D;
	/* IDB +0x1E: S-foil animation state: bit 0 transition pending, bit 1 requested closed.
	 * ANIM_updateanimation settles to 0 (open) or 2 (closed); player command toggles bit 1 and sets bit 0.
	 * LASER_firelasersystem permits fire only at 0; hyperspace closes foils. */
	uint8_t sFoilState;
	/* IDB +0x1F: Current AI plan ID indexing g_planDataPtrs. pai_ProcessPlan installs transition next-plan
	 * IDs; pai_ApplyPendingPlanTargetAndManeuver consumes target/aiManeuverId bytes. */
	uint8_t aiCurrentPlanId;
	/* IDB +0x20: AI order/default leader-plan ID, initialized from g_orderLeaderPlanId[mission order]; NOT
	 * the raw mission order ordinal. Persists across current-plan transitions; boarding values 40..44 select
	 * actions. */
	uint8_t aiOrderPlanId;
	/* IDB +0x21: AI mission-waypoint cursor; initialized to 1 and advanced/wrapped by
	 * paiman_AdvanceOrderWaypoint. Encoded into aiTargetRef as 0x8000 + index. */
	uint8_t aiWaypointIndex;
	/* IDB +0x22: AI plan saved by the player hold/wait command (plan 66), restored before resuming or
	 * assigning a wingman target. */
	uint8_t aiSavedPlanId;
	/* IDB +0x23: AI plan evaluation interval in simulation ticks (236 per mission second). Reloads
	 * aiThinkTimerTicks. Temporarily 59 during hyperspace exit; original interval saved at +0x35. */
	uint16_t aiThinkIntervalTicks;
	/* IDB +0x25: AI plan countdown in simulation ticks; decremented/clamped to zero by XW_updatetime.
	 * pai_UpdateAllCraftAI runs when zero and reloads aiThinkIntervalTicks. */
	uint16_t aiThinkTimerTicks;
	/* IDB +0x27: AI target reference: runtime object or high-bit-set mission waypoint (0x8000 + waypoint),
	 * 255 means none. Resolved by pai_UpdateAimPointFromOrderTarget; not a player targeting-computer
	 * selection. */
	uint16_t aiTargetRef;
	/* IDB +0x29: AI world-space steering/aim coordinate, written by target resolution and maneuver
	 * lead/formation/boarding calculations. Distinct from actual ObjectRecord.worldX/Y/Z and camera-space
	 * viewX/Y/Z. */
	int aiAimPointX;
	/* IDB +0x2D: AI world-space steering/aim coordinate, written by target resolution and maneuver
	 * lead/formation/boarding calculations. Distinct from actual ObjectRecord.worldX/Y/Z and camera-space
	 * viewX/Y/Z. */
	int aiAimPointY;
	/* IDB +0x31: AI world-space steering/aim coordinate, written by target resolution and maneuver
	 * lead/formation/boarding calculations. Distinct from actual ObjectRecord.worldX/Y/Z and camera-space
	 * viewX/Y/Z. */
	int aiAimPointZ;
	/* IDB +0x35: Overloaded AI scratch word: candidate object target normally (255 none, 251 evasive
	 * completion, 254 damage/abort sentinel); PAIMAN_initoutofhyperspacemaneuver stores aiThinkIntervalTicks
	 * here and exit completion restores it. Never type this unconditionally as an object index. */
	uint16_t aiCandidateTargetOrSavedInterval;
	/* IDB +0x37: Escorted flight-group index, selected from primary/secondary mission targets. Writer uses
	 * 0x80 when no future escort target remains and 0xFF while waiting for a future target. */
	uint8_t aiEscortTargetFlightGroup;
	/* IDB +0x38: Copied from mission throttlePreset; cruise/rendezvous use it as throttle preset, escort as
	 * formation slot, boarding as duration in 14160-tick units (copied to an 8-bit counter). */
	uint16_t aiOrderParameter;
	/* IDB +0x3A: Object index of the last attacker; 255 means none in X-Wing (not BoP's 0xFFFF). */
	uint16_t lastAttackerObjIdx;
	/* IDB +0x3C: Overloaded progress: completed attack runs, or remaining boarding-duration units. Spawn
	 * resets it; attack increments; boarding initializes/decrements. */
	uint8_t aiOrderProgress;
	/* IDB +0x3D: Incremented per launch by fightershootorder; reset on attack break-off. */
	uint8_t aiWarheadsFiredThisRun;
	/* IDB +0x3E: Incremented by collide_laserhitcraft; cleared on aiManeuverId entry; compared with
	 * craft-type evade-hit threshold. */
	uint8_t aiHitsThisManeuver;
	/* IDB +0x3F: Initialized from g_spawnObjectKind. 0=active; AI sweep excludes kinds 3 and 4 (dying
	 * states). */
	uint8_t objectKind;
	/* IDB +0x40: AI aiManeuverId ID indexing the 29-entry initialization/update callback tables; installed by
	 * pai_ApplyPendingPlanTargetAndManeuver. */
	uint8_t aiManeuverId;
	/* IDB +0x41: AI aiManeuverId phase, zeroed by paiman_initmaneuver; interpretation is callback-specific
	 * (e.g. Immelmann 0..2, hyperspace exit 0..8). */
	uint8_t aiManeuverPhase;
	/* IDB +0x42: Primary AI aiManeuverId countdown, 236 simulation ticks per mission second. XW_updatetime
	 * subtracts elapsed ticks and clamps signed underflow to zero. Boarding uses 14160-tick units. */
	uint16_t aiManeuverTimerTicks;
	/* IDB +0x44: Auxiliary AI aiManeuverId countdown, not an enum/plan-state ID. XW_updatetime decrements it
	 * like aiManeuverTimerTicks; turn/jink/cruise/scissors/hyperspace callbacks reload and test for zero. */
	uint16_t aiManeuverAuxTimerTicks;
	/* IDB +0x46: Per-craft base maximum speed copied from XwCraftTypeDef.maxSpeed at spawn. Dynamics modifies
	 * the speed target for power allocation/throttle and Death Star conditions; player speed matching also
	 * reads it. */
	uint16_t maxSpeed;
	/* IDB +0x48: Initialized to 0xFFFF at spawn alongside angular rate factors. No consuming X-Wing read
	 * established; do not label as a speed scale from descendant layout alone. */
	uint16_t field48;
	/* IDB +0x4A: AI climb completion gate: DYNAMIX_planedynamics clears state 1 and levels pitch at 0x4000
	 * when worldZ reaches aiAimPointZ; maneuvers reset it when changing vertical steering. */
	uint8_t aiClimbState;
	/* IDB +0x4B: AI dive pullout gate: state 1 invokes DYNAMIX_pulloutdive. That helper sets 2 (finished),
	 * clears aiPitchState and levels pitch when worldZ-aiAimPointZ<=256. Other maneuvers reset to zero. */
	uint8_t aiDiveState;
	/* IDB +0x4C: Base pitch angular-rate limit copied from craft type at spawn; divided by g_simStepScale in
	 * dynamics. Pitch/roll limits also feed player input scaling; not AI-exclusive. */
	uint16_t pitchRateLimit;
	/* IDB +0x4E: Unsigned Q16 multiplier of the base pitch rate in DYNAMIX_planedynamics, before the AI
	 * command fraction. Initialized to 0xFFFF; no other writer established. Not angular acceleration. */
	uint16_t aiPitchRateScaleQ16;
	/* IDB +0x50: AI PITCH controller (historical aiPitchState): 0 inactive, 1 decrement pitch, 2 increment
	 * pitch, 3 target reached. DYNAMIX_planedynamics integrates CraftData.pitch and handles pole wrapping. */
	uint8_t aiPitchState;
	/* IDB +0x51: AI pitch force flag: nonzero bypasses snap-to-target completion, allowing looping maneuvers.
	 * Cleared at pitch pole wrap; set by Split-S/Immelmann. */
	uint8_t aiPitchForce;
	/* IDB +0x52: AI desired pitch in engine angle units; 0x4000 is level. Set from g_trig2Pitch or
	 * aiManeuverId constants; consumed against CraftData.pitch. */
	uint16_t aiTargetPitch;
	/* IDB +0x54: Unsigned Q16 AI pitch command fraction. Effective tick step =
	 * fraction(fraction(pitchRateLimit/g_simStepScale, aiPitchRateScaleQ16), aiPitchStepQ16); 0xFFFF is full
	 * command. */
	uint16_t aiPitchStepQ16;
	/* IDB +0x56: Base roll angular-rate limit copied from craft type at spawn; divided by g_simStepScale in
	 * dynamics. Pitch/roll limits also feed player input scaling; not AI-exclusive. */
	uint16_t rollRateLimit;
	/* IDB +0x58: Unsigned Q16 multiplier of the base roll rate in DYNAMIX_planedynamics, before the AI
	 * command fraction. Initialized to 0xFFFF; no other writer established. Not angular acceleration. */
	uint16_t aiRollRateScaleQ16;
	/* IDB +0x5A: AI roll controller: 0 inactive; 1..3 integrated by dynamics; 3 continuously rolls in
	 * direction selected by aiTargetRoll high bit; 1/2 approach target; 4 reached/settled. */
	uint8_t aiRollState;
	/* IDB +0x5B: AI roll target (modular angle); in roll state 3 its high bit selects continuous roll
	 * direction. Consumed against ObjectRecord.roll. */
	uint16_t aiTargetRoll;
	/* IDB +0x5D: Unsigned Q16 AI roll command fraction, applied after rollRateLimit/g_simStepScale and
	 * aiRollRateScaleQ16. */
	uint16_t aiRollStepQ16;
	/* IDB +0x5F: Base yaw angular-rate limit copied from craft type at spawn; divided by g_simStepScale in
	 * dynamics. Pitch/roll limits also feed player input scaling; not AI-exclusive. */
	uint16_t yawRateLimit;
	/* IDB +0x61: Unsigned Q16 multiplier of the base yaw rate in DYNAMIX_planedynamics, before the AI command
	 * fraction. Initialized to 0xFFFF; no other writer established. Not angular acceleration. */
	uint16_t aiYawRateScaleQ16;
	/* IDB +0x63: AI YAW controller (historical aiYawState): 0 inactive, 2 steer, 3 reached. paiman_setturn
	 * snaps within 0x300 angle units; dynamics uses shortest modular yaw delta. */
	uint8_t aiYawState;
	/* IDB +0x64: AI desired yaw, compared to ObjectRecord.yaw. paiman_setflighttotarget copies g_trig2Yaw
	 * plus bias; maneuvers override it. */
	uint16_t aiTargetYaw;
	/* IDB +0x66: Unsigned Q16 AI yaw command fraction. Effective tick step =
	 * fraction(fraction(yawRateLimit/g_simStepScale, aiYawRateScaleQ16), aiYawStepQ16). */
	uint16_t aiYawStepQ16;
	/* IDB +0x68: Formation-table index. */
	uint8_t aiFormationType;
	/* IDB +0x69: Spacing-table index. Spawn selects 4 for non-fighters or 9 for hangar launch; return-home
	 * and hangar orders also change it. */
	uint8_t aiFormationSpacing;
	/* IDB +0x6A: Zero-based craft number assigned from the spawn-loop index; display adds one for multi-craft
	 * flight groups. */
	uint8_t craftIndexInFlightGroup;
	/* IDB +0x6B: AI world-space displacement remaining: formation/boarding correction, collision avoidance or
	 * jink. MOVE_moveobjects consumes each component independently when workingSubsystems!=0: clamp to +/-
	 * aiDisplacementRateLimit (250 for maneuver 18), divide signed low word by g_simStepScale, subtract
	 * consumed displacement and add it to frame movement. If divided step is zero, consume the entire
	 * remaining component. Cleared at maneuver initialization. */
	int aiDisplacementX;
	/* IDB +0x6F: AI world-space displacement remaining: formation/boarding correction, collision avoidance or
	 * jink. MOVE_moveobjects consumes each component independently when workingSubsystems!=0: clamp to +/-
	 * aiDisplacementRateLimit (250 for maneuver 18), divide signed low word by g_simStepScale, subtract
	 * consumed displacement and add it to frame movement. If divided step is zero, consume the entire
	 * remaining component. Cleared at maneuver initialization. */
	int aiDisplacementY;
	/* IDB +0x73: AI world-space displacement remaining: formation/boarding correction, collision avoidance or
	 * jink. MOVE_moveobjects consumes each component independently when workingSubsystems!=0: clamp to +/-
	 * aiDisplacementRateLimit (250 for maneuver 18), divide signed low word by g_simStepScale, subtract
	 * consumed displacement and add it to frame movement. If divided step is zero, consume the entire
	 * remaining component. Cleared at maneuver initialization. */
	int aiDisplacementZ;
	/* IDB +0x77: Four unsigned Q16 commanded engine throttles; paiman_setpower writes and
	 * DYNAMIX_planedynamics consumes them. Active count is the craft-type engine count. */
	uint16_t engineThrottle[4];
	/* IDB +0x7F: Per-engine Q16 multiplier initialized to 0xFFFF for each engine at spawn. Dynamics
	 * multiplies engineThrottle by this. Damage/health semantics are not established. */
	uint16_t enginePowerScaleQ16[4];
	/* IDB +0x87: Dynamics stores fraction(engineThrottle[i], enginePowerScaleQ16[i]) when propulsion works;
	 * the same computed value is weighted into combined throttle. No separate consumer established; skipped
	 * engine slots are not refreshed. */
	uint16_t engineOutputQ16[4];
	/* IDB +0x8F: Accumulated hull damage: increased by non-ion collision/weapon damage, compared with hullMax
	 * and divided into thirds by the HUD. */
	uint16_t hullDamage;
	/* IDB +0x91 */
	uint16_t systemDamageHullThreshold;
	/* IDB +0x93: Hull destruction threshold; damage code compares hullDamage against this value. */
	uint16_t hullMax;
	/* IDB +0x95: HUD element availability: bit 0x02 gates power bars and bit 0x04 gates the speed display.
	 * Distinct from workingSubsystems at offset 150. */
	uint8_t activeHudFeatureMask;
	/* IDB +0x96: Operational subsystem mask. Initialized to 0xFF; disable order clears it. Dynamics tests
	 * 0x40 for propulsion and 0x20 for attitude controls. */
	uint8_t workingSubsystems;
	/* IDB +0x97: Bits select cockpit overlay sprites at HUD layout indices 58..73. Initialized to 0; player
	 * hull hits AND with two ORed random words. Individual bit semantics are not established. */
	uint16_t cockpitOverlayMask;
	/* IDB +0x99: Initialized to 0xFFFF at spawn; no semantic reader established. */
	uint16_t field99;
	/* IDB +0x9B: Initialized to zero at spawn; no semantic reader established. */
	uint16_t field9B;
	/* IDB +0x9D: Initialized to zero at spawn; no semantic reader established. */
	uint8_t field9D;
	/* IDB +0x9E: Capture/departure override: 0 absent; boarding order 43 writes captor flightGroupIndex |
	 * 0x80 and changes IFF. Low 7 bits choose departure mothership FG settings in flyhome/enterhangar;
	 * nonzero also marks capture for arrival/departure accounting. */
	uint8_t captorFlightGroupOverride;
	/* IDB +0x9F */
	uint8_t isInspected;
	/* IDB +0xA0: Initialized to 0. State 2 completes the await-boarding predicate and restores
	 * workingSubsystems. */
	uint8_t boardingState;
	/* IDB +0xA1: 16-byte cargo text copied from the flight group; selects specialCargo for specialCraftIndex.
	 */
	char cargoName[16];
	/* IDB +0xB1: Front and rear signed shield energy; transferable between banks. */
	int16_t shieldEnergy[XW_SHIELD_BANK_COUNT];
	/* IDB +0xB5: Shield power allocation 0..4. Player action cycles it; recharge loop adds 20*(value-2) to
	 * shield pools. */
	uint8_t shieldRedirect;
	/* IDB +0xB6: Player-selected shield bank distribution; HUD element 19 displays this state. */
	uint8_t shieldDistribMode;
	/* IDB +0xB7: Number of laser groups processed by the weapon update; group arrays have capacity four. */
	uint8_t cannonClassCount;
	/* IDB +0xB8: Laser power allocation 0..4. Player action cycles it; recharge loop adds 2*value-4 to laser
	 * charge. */
	uint8_t laserRedirect;
	/* IDB +0xB9: Number of laser weapon slots iterated by recharge and reticle rendering; does not include
	 * the warhead launcher count. */
	uint8_t laserSlotCount;
	/* IDB +0xBA: Four-group laser control state at 0xBA; nextSlot/linkMode are consumed by firing and HUD
	 * routines, cooldowns are written on fire and decremented by weapon updates. */
	struct CraftLaserState laserState;
	/* IDB +0xD2: Number of nonzero warheadSlotTypeIds initialized from the two craft-type launcher
	 * definitions. */
	uint8_t warheadLauncherCount;
	/* IDB +0xD3: Projectile object-type IDs for the two launcher systems; 149/150 selected by rocketsonboard
	 * target genus. */
	uint8_t warheadSlotTypeIds[2];
	/* IDB +0xD5: Two banks. Low 7 bits: 1 single / 3 linked; bit 7 selects the next launcher and toggles
	 * after a successful shot. */
	uint8_t warheadLauncherFlags[2];
	/* IDB +0xD7: Per-bank weapon firing timers; successful fire dispatch resets a timer to 472 ticks. */
	int16_t warheadFireTimers[2];
	/* IDB +0xDB: Missile-lock accumulation in 236 Hz ticks. Cleared on target change; weapon update compares
	 * against 1180 and floors decay at zero. */
	int16_t warheadLockTicks;
	/* IDB +0xDD */
	struct XwWeaponHitStats weaponStats;
	/* IDB +0xEC: Per-mission kills. Layout verified by COLLIDE_updatekills, mission initialization, and
	 * FEDISKIO_updatepilotrecord scoring. */
	struct XwCraftKillStats killStats;
	/* IDB +0x108: 4-byte slot records at 0x108; next independently initialized component-state arrays start
	 * at 0x138. */
	struct CraftWeaponSlot weaponSlots[12];
	/* IDB +0x138: Per-mesh state: initialized to zero, selected mesh types start at 1; component destruction
	 * writes 2. Fifty entries. */
	uint8_t componentState[50];
	/* IDB +0x16A: Per-mesh rotation byte. Proving-ground component destruction multiplies it by 256 for
	 * fixed-angle sine/cosine. Fifty entries. */
	uint8_t meshRotation[50];
	/* IDB +0x19C: Per-mesh hit points in units of 16 hull-damage points: 0=destroyed, 0xFF=not damageable.
	 * Spawn initializes fifty entries to 0xFF and assigns special damageable meshes. */
	uint8_t componentHp[50];
};

/* Original IDB size: 211 bytes. */
struct XwCraftTypeDef {
	/* IDB +0x0 */
	char shortName[10];
	/* IDB +0xA */
	char name[24];
	/* IDB +0x22: Used when mission iffOverride is zero. */
	uint8_t defaultIff;
	/* IDB +0x23: Craft score weight applied to kill and capture counts. */
	uint8_t killValue;
	/* IDB +0x24: Number of per-engine throttle slots; read by paiman_setpower and craft dynamics. */
	uint8_t engineCount;
	/* IDB +0x25: Nonzero enables hyperspace departure. */
	uint8_t hasHyperdrive;
	/* IDB +0x26: Base speed value displayed by Hud_DrawPowerSettings2D, adjusted in eighths for power
	 * allocation. Separate from simulation maxSpeed at 0x32. */
	uint16_t displayMaxSpeed;
	/* IDB +0x28: Nominal front and rear shield strength; charging permits twice nominal. */
	int16_t nominalShieldEnergy[XW_SHIELD_BANK_COUNT];
	/* IDB +0x2C: AI breaks off an attack run when aiHitsThisManeuver reaches this byte. */
	uint8_t evadeHitThreshold;
	/* IDB +0x2D: Nonzero enables cargo text in the target computer. */
	uint8_t hasCargo;
	/* IDB +0x2E: Initial CraftData.hullMax. */
	uint16_t hullStrength;
	/* IDB +0x30: Initial CraftData.systemDamageHullThreshold. */
	uint16_t systemDamageHullThreshold;
	/* IDB +0x32: Simulation speed limit; initializes craft speed via the throttle fraction and is compared
	 * during AI evasion. */
	uint16_t maxSpeed;
	/* IDB +0x34: Acceleration scale used by DYNAMIX_adjustvelocity; quarter-base plus throttle-scaled
	 * remainder. */
	uint16_t maxAcceleration;
	/* IDB +0x36: Unsigned 16-bit fraction applied to speed overshoot when deceleration is enabled. */
	uint16_t decelerationGain;
	/* IDB +0x38: Copied to CraftData.yawRateLimit at spawn; base yaw angular rate used by
	 * DYNAMIX_planedynamics. */
	uint16_t yawRateLimit;
	/* IDB +0x3A: DYNAMIX_planedynamics multiplies a yaw step by this Q16 coefficient and rolls opposite the
	 * turn when the AI roll controller is idle/settled. */
	uint16_t rollPerYawQ16;
	/* IDB +0x3C: Copied to CraftData.rollRateLimit at spawn; base roll angular rate. */
	uint16_t rollRateLimit;
	/* IDB +0x3E: Copied to CraftData.pitchRateLimit at spawn; base pitch angular rate. */
	uint16_t pitchRateLimit;
	/* IDB +0x40 */
	uint16_t maxTumbleAngle;
	/* IDB +0x42: MOVE_moveobjects reads this as the per-axis maximum correction before division by
	 * g_simStepScale when consuming CraftData.aiDisplacementX/Y/Z. Maneuver 18 overrides it with 250; other
	 * writers/physical units not established. */
	uint16_t aiDisplacementRateLimit;
	/* IDB +0x44: NUL-terminated cockpit resource base name. */
	uint8_t cockpitBaseName[9];
	/* IDB +0x4D: Projectile object type for each cannon group; consumed by the fire routine. */
	uint8_t laserGroupWeaponType[4];
	/* IDB +0x51: First weapon slot in each cannon group, inclusive. */
	uint8_t laserGroupFirstSlot[4];
	/* IDB +0x55: Inclusive last laser slot of each of four laser groups. */
	uint8_t laserGroupLastSlot[4];
	/* IDB +0x59: Summed during craft initialization to produce laserSlotCount. */
	uint8_t laserGroupSlotCount[4];
	/* IDB +0x5D: Value 2 selects turret-style firing-gate initialization; other values count as forward
	 * cannon groups. */
	uint8_t laserGroupMountType[4];
	/* IDB +0x61: Projectile type ID for each of two warhead banks. */
	uint8_t warheadProjectileType[2];
	/* IDB +0x63: Inclusive first weapon slot for each of two launchers. */
	uint8_t warheadFirstSlot[2];
	/* IDB +0x65: Inclusive last weapon slot for each of two launchers. */
	uint8_t warheadLastSlot[2];
	/* IDB +0x67: Number of slots in each of two launcher banks. */
	uint8_t warheadLauncherSlotCount[2];
	/* IDB +0x69: Twelve packed 7-byte weapon hardpoints; shared with CraftData.weaponSlots. */
	struct XwWeaponHardpoint weaponHardpoints[12];
	/* IDB +0xBD: Five signed words at 0xBD..0xC5 used by boarding approach/alignment; small means genus 0/1,
	 * large means genus >=2. Roles established from both docking phases; geometry names remain neutral. */
	struct XwDockingOffsets docking;
	/* IDB +0xC7: Interior hangar point, rotated for launch position and entry aim point. */
	struct XwCraftLocalPoint hangarInside;
	/* IDB +0xCD: Exterior hangar point, rotated to aim a departing launch; fly-home approach scales this
	 * rotated vector by four. */
	struct XwCraftLocalPoint hangarOutside;
};

/* Declarations follow ascending original IDB address. */

extern XwCraftTypeDef g_craftTypeDefs[XW_CRAFT_TYPE_COUNT];
extern XwCraftModelBounds g_craftModelBounds[XW_CRAFT_TYPE_COUNT];

#ifdef __cplusplus
}
#endif

#endif
