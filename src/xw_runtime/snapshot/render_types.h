/* Owned host-thread records; see docs/modern-renderer-snapshot-contract.md.
 * No packed game layouts, borrowed game pointers or GPU handles cross this boundary. */
#ifndef XW_RENDER_TYPES_H
#define XW_RENDER_TYPES_H
#include <stdint.h>

enum { XW_SNAP_OBJECT_NONE, XW_SNAP_OBJECT_MOBILE, XW_SNAP_OBJECT_MISSION };

enum {
	XW_SNAP_OWNER_NONE,
	XW_SNAP_OWNER_FRONTEND,
	XW_SNAP_OWNER_FLIGHT,
	XW_SNAP_OWNER_FLIGHT_UI,
	XW_SNAP_OWNER_FLIGHT_TRANSITION
};

enum { XW_SNAP_CLASSIC_DOS, XW_SNAP_CLASSIC_WINDOWS_SOFTWARE, XW_SNAP_CLASSIC_WINDOWS_HARDWARE };

enum { XW_SNAP_SLOT_CRAFT, XW_SNAP_SLOT_MAIN_OTHER, XW_SNAP_SLOT_LOCAL_DEBRIS, XW_SNAP_SLOT_MISSION };

/* Original flight view selectors shared by the three source layouts. */
enum { XW_SNAP_VIEW_NO_COCKPIT = 18, XW_SNAP_VIEW_FULL_FORWARD = 19 };

enum {
	XW_SNAP_WIDGET_NONE,
	XW_SNAP_WIDGET_SPRITE,
	XW_SNAP_WIDGET_GAUGE,
	XW_SNAP_WIDGET_LASER,
	XW_SNAP_WIDGET_THROTTLE
};

enum { XW_SNAP_PAINT_FILL, XW_SNAP_PAINT_LINE, XW_SNAP_PAINT_FRAME };

enum { XW_SNAP_COLOR_INDEX8, XW_SNAP_COLOR_RGB555, XW_SNAP_COLOR_RGB565 };

enum {
	XW_SNAP_PANE_TARGET_HEADER,
	XW_SNAP_PANE_TARGET_BODY,
	XW_SNAP_PANE_TARGET_FOOTER,
	XW_SNAP_PANE_SPEED,
	XW_SNAP_PANE_MAX_SPEED,
	XW_SNAP_PANE_CLOCK,
	XW_SNAP_PANE_THROTTLE,
	XW_SNAP_PANE_WARHEADS,
	XW_SNAP_PANE_REPLAY_COUNTER,
	XW_SNAP_PANE_VIEW_LABEL,
	XW_SNAP_PANE_READY_MESSAGE,
	XW_SNAP_PANE_COURSE,
	XW_SNAP_PANE_REPLAY_TITLE,
	XW_SNAP_PANE_REPLAY_STATUS,
	XW_SNAP_PANE_REPLAY_BUTTONS,
	XW_SNAP_PANE_FRAME_RATE
};

enum {
	XW_SNAP_OBJECTS = 180,
	XW_SNAP_CRAFTS = 28,
	XW_SNAP_COMPONENTS = 50,
	XW_SNAP_TYPES = 211,
	XW_SNAP_BACKDROPS = 64,
	XW_SNAP_HYPERSTARS = 192,
	XW_SNAP_DAMAGE_CELLS = 64,
	XW_SNAP_DAMAGE_STATES = 14,
	XW_SNAP_WIDGETS = 75,
	XW_SNAP_PANEL_SPRITES = 174,
	XW_SNAP_COCKPIT_VIEWS = 20,
	XW_SNAP_RADAR_BLIPS = 48,
	XW_SNAP_HUD_PANES = 16,
	XW_SNAP_HUD_GLYPHS = 4096,
	XW_SNAP_HUD_PAINT = 256,
	XW_SNAP_HUD_SPRITES = 256
};

typedef uint64_t XwRenderAssetId; /* 0 means absent. */

typedef struct XwSnapRect {
	int32_t x, y, width, height;
} XwSnapRect;

typedef struct XwSnapRange {
	uint32_t first, count;
} XwSnapRange;

typedef struct XwSnapObjectId {
	uint32_t generation;
	uint16_t slot;
	uint8_t kind; /* NONE, MOBILE, MISSION. */
} XwSnapObjectId;

typedef struct XwSnapCamera {
	int32_t world_pos[3];
	float rows[9];       /* Precise camera rows R0, R1, R2, with Q15 fallback. */
	XwSnapRect viewport; /* Source-screen coordinates. */
	int32_t center_x, center_y, projection_offset_y;
	int32_t focal_x;
	uint16_t aspect_y_q16;
	uint16_t screen_width, screen_height;
	XwSnapObjectId player, focus;
	uint16_t hud_state, replay_mode;
	uint8_t external;
	uint8_t legacy_render_convention; /* XW_SNAP_CLASSIC_* at camera capture. */
} XwSnapCamera;

typedef struct XwSnapObject {
	XwSnapObjectId id;
	int32_t world_pos[3], previous_world_pos[3];
	uint16_t yaw, pitch, roll;
	int16_t cached_rows_q15[9]; /* Side, forward, up. */
	uint16_t craft_index;       /* Index in this frame's crafts[]. */
	uint16_t source_ref, speed;
	uint8_t type, genus, family, source_type;
	uint8_t iff, markings, billboard_scale;
	uint8_t animation_state, secondary_animation_state;
	uint8_t state, type_specific; /* Mission-object bytes; zero for mobile. */
	uint8_t checkpoint_lit;       /* Windows checkpoint 1..3 blink observations; DOS is steady. */
	uint8_t orientation_dirty;
	uint8_t slot_class; /* CRAFT, MAIN_OTHER, LOCAL_DEBRIS, MISSION. */
} XwSnapObject;

typedef struct XwSnapCraft {
	uint8_t component_count;
	uint8_t component_state[XW_SNAP_COMPONENTS];
	uint8_t component_hp[XW_SNAP_COMPONENTS];
	uint8_t mesh_rotation[XW_SNAP_COMPONENTS];
	uint16_t damage_frame; /* Resolved extra flame-frame selector. */
	uint8_t damage_frame_valid;
	uint8_t object_kind, sfoil_state, working_subsystems;
	uint8_t engine_count, laser_redirect, shield_redirect;
	uint16_t engine_output_q16[4];
} XwSnapCraft;

typedef struct XwSnapType {
	XwRenderAssetId geometry;
	XwRenderAssetId bitmaps, alternate_bitmaps, bitmap_remap;
	XwRenderAssetId animation;
	XwRenderAssetId dos_components;
	uint32_t animation_first, animation_count;
	uint16_t max_extent, half_extent;
	uint8_t flags, object_flags, family, genus;
} XwSnapType;

typedef struct XwRenderAssetSet {
	uint8_t flight_version;
	uint16_t type_count;
	XwSnapType types[XW_SNAP_TYPES];
	XwRenderAssetId component_damage_sequence, fragment_sequence;
	XwRenderAssetId dos_materials, special_layout;
	XwRenderAssetId surface_texture, trench_texture;
} XwRenderAssetSet;

typedef struct XwSnapAppearance {
	uint64_t palette_revision;
	uint32_t palette_argb[256]; /* Effective display colors, 0xAARRGGBB. */
	int32_t direction_q15[3];
	int32_t brightness_q8, local_lights_level;
	int16_t ship_detail_value;
	uint16_t ship_detail_polygons, starship_detail;
	uint16_t surface_detail_level, surface_object_limit, trench_object_limit;
	uint16_t target_highlight; /* Original packed parent/blink value. */
	uint8_t graphics_detail, directional_enabled;
	uint8_t gouraud_enabled, markings_enabled, engine_glow_enabled;
	uint8_t debris_enabled, backdrops_enabled;
} XwSnapAppearance;

typedef struct XwSnapSky {
	XwRenderAssetId stars;
	uint16_t density;
	int16_t oscillator;
	uint16_t face_counts[6]; /* +Y, -Y, +X, -X, +Z, -Z. */
	uint8_t backdrop_type[64], backdrop_direction[64];
} XwSnapSky;

typedef struct XwSnapHyperstar {
	int32_t world_pos[3];
	uint16_t roll;
	uint8_t source_slot, copy_index, color_index;
} XwSnapHyperstar;

typedef struct XwSnapHyperspace {
	uint8_t phase;
	uint16_t elapsed_ticks;
	uint32_t windows_streak_length;
	int16_t dos_endpoints[2];
	uint32_t count;
	XwSnapHyperstar stars[XW_SNAP_HYPERSTARS];
} XwSnapHyperspace;

typedef struct XwSnapDamageCell {
	uint16_t key;
	uint8_t state[XW_SNAP_DAMAGE_STATES];
} XwSnapDamageCell;

typedef struct XwSnapSpecialWorld {
	uint8_t surface_active, proving_grounds_active;
	XwSnapDamageCell damage[XW_SNAP_DAMAGE_CELLS];
} XwSnapSpecialWorld;

typedef struct XwSnapTargetBox {
	XwSnapObjectId object;
	int32_t world_pos[3], extent;
	uint16_t component;
	uint8_t color_index, visible, direct_overlay;
} XwSnapTargetBox;

typedef struct XwSnapWidget {
	uint16_t value;    /* Resolved sprite state or filled count. */
	uint16_t segments; /* For a repeated-segment gauge. */
	int16_t step_x, step_y;
	uint16_t empty_state, filled_state;
	uint8_t kind; /* NONE, SPRITE, GAUGE. */
	uint8_t visible;
} XwSnapWidget;

typedef struct XwSnapRadarBlip {
	int16_t x, y;
	uint8_t color_index, coverage;
} XwSnapRadarBlip;

typedef struct XwSnapRadar {
	uint16_t front_count, rear_count;
	XwSnapRadarBlip front[48], rear[48];
	int16_t target_x, target_y;
	uint8_t front_visible, rear_visible, target_visible;
	int16_t cross_x, cross_y;
	uint8_t cross_visible, cross_color, tall_bracket;
} XwSnapRadar;

typedef struct XwSnapHudPane {
	XwSnapRect clip;
	XwSnapRange glyphs, paint, sprites;
	uint8_t visible;
} XwSnapHudPane;

typedef struct XwSnapGlyph {
	XwRenderAssetId font;
	XwSnapRect clip;
	int16_t x, y;
	uint16_t character, advance, height;
	uint32_t order;
	uint16_t foreground, background, shadow;
	uint8_t background_enabled, shadow_enabled;
} XwSnapGlyph;

typedef struct XwSnapHudPaint {
	XwSnapRect clip;
	int16_t x0, y0, x1, y1;
	uint32_t order;
	uint16_t color;
	uint8_t kind; /* FILL, LINE, FRAME. */
} XwSnapHudPaint;

typedef struct XwSnapHudSprite {
	XwRenderAssetId image;
	uint32_t frame, order;
	XwSnapRect destination, clip;
	uint16_t transparent_color;
	uint8_t mirrored;
} XwSnapHudSprite;

typedef struct XwSnapCockpit {
	XwRenderAssetId definition;
	uint16_t view, hud_state;
	uint16_t screen_width, screen_height;
	XwSnapRect viewport;
	int32_t projection_offset_y;
	uint8_t mirrored, suppressed, color_mode;
	uint32_t palette_argb[256]; /* HUD palette at composition, including RGB text conversion. */
	XwSnapWidget widgets[XW_SNAP_WIDGETS];
	XwSnapRadar radar;
	XwSnapHudPane panes[XW_SNAP_HUD_PANES];
	uint32_t glyph_count, paint_count, sprite_count;
	XwSnapGlyph glyphs[XW_SNAP_HUD_GLYPHS];
	XwSnapHudPaint paint[XW_SNAP_HUD_PAINT];
	XwSnapHudSprite sprites[XW_SNAP_HUD_SPRITES];
} XwSnapCockpit;

typedef struct XwSnapViewKey {
	uint64_t mission_generation;
	uint64_t world_generation;
	uint64_t view_serial;
	uint64_t hud_revision;
} XwSnapViewKey;

/* Fresh Windows DX5 image identity. DOS pixels use their independent host publisher. */
typedef struct XwSnapClassicFrame {
	XwSnapViewKey key;
	uint64_t completion_serial;
	uint8_t valid;
	uint8_t source; /* DOS, WINDOWS_SOFTWARE, WINDOWS_HARDWARE. */
} XwSnapClassicFrame;

typedef struct XwRenderSnapshot {
	uint64_t host_serial;
	uint64_t capture_host_us;
	uint64_t presentation_generation; /* Retained-image continuity, independent of world history. */
	uint64_t simulation_ticks;
	/* Latched with component bytes; independent of presentation-reset simulation_ticks. */
	uint64_t component_view_time_ticks, component_event_serial, component_event_time_ticks;
	uint16_t component_event_interval_ticks;
	uint8_t flight_unlocked;
	XwSnapViewKey key;
	XwSnapClassicFrame classic;
	XwRenderAssetId flight_assets;
	XwRenderAssetId loaded_cockpit; /* Resident through loading and mission suspension. */
	uint8_t flight_version;         /* 93, 94, 98. */
	uint8_t mission_classic;        /* Pinned content choice, not renderer choice. */
	uint8_t owner;                  /* NONE, FRONTEND, FLIGHT, FLIGHT_UI, FLIGHT_TRANSITION. */
	uint8_t world_valid, hud_valid, focused, paused;
	XwSnapCamera camera;
	XwSnapAppearance appearance;
	XwSnapSky sky;
	XwSnapHyperspace hyperspace;
	XwSnapSpecialWorld special;
	XwSnapTargetBox target_box;
	uint32_t object_count, craft_count;
	XwSnapObject objects[XW_SNAP_OBJECTS];
	XwSnapCraft crafts[XW_SNAP_CRAFTS];
	XwSnapCockpit cockpit;
} XwRenderSnapshot;

#endif
