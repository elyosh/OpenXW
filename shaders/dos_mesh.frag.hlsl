/* X-Wing DOS materials; Gouraud shading blends resolved ramp colours. */
Texture2D<float4> palette : register(t0, space2);
SamplerState palette_sampler : register(s0, space2);
Texture2D<float> materials : register(t1, space2);
SamplerState material_sampler : register(s1, space2);
struct Mark { uint first; uint count; uint color; uint width; float max_depth; float3 anchor; };
StructuredBuffer<Mark> marks : register(t2, space2);
StructuredBuffer<float4> mark_vertices : register(t3, space2);
cbuffer Draw : register(b0, space3) {
    float4 clip_x;
    float4 clip_y;
    float4 depth_row;
    float4 options; // markings enabled, marking mode, material count, DOS93
    float4 pixels; // physical pixels per classic pixel, render target extent
    float4 policy; // target match, gate offset, per-object material-14 offset
};
struct Input {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    nointerpolation float4 info : TEXCOORD1;
    float shade : TEXCOORD2;
    float facing : TEXCOORD3;
    noperspective float vertex_light : TEXCOORD4;
};
bool hit_polygon(float2 p, Mark m) {
    bool inside = false;
    float2 a = mark_vertices[m.first + m.count - 1].xy;
    [loop] for (uint i = 0; i < m.count; ++i) {
        float2 b = mark_vertices[m.first + i].xy;
        if ((a.y > p.y) != (b.y > p.y)) {
            float x = (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x;
            if (p.x < x) inside = !inside;
        }
        a = b;
    }
    return inside;
}
float3 project_mark(float4 p) {
    return float3(dot(clip_x, p), dot(clip_y, p), dot(depth_row, p));
}
float2 mark_screen(float3 p) {
    return (p.xy / p.z * float2(0.5f, -0.5f) + 0.5f) * pixels.zw / pixels.xy;
}
bool hit_line(Input v, Mark m) {
    float3 a = project_mark(mark_vertices[m.first]);
    float3 b = project_mark(mark_vertices[m.first + 1]);
    /* Match the geometry near plane before dividing by depth. */
    if (a.z < 1 && b.z < 1) return false;
    if (a.z < 1) a = lerp(a, b, (1 - a.z) / (b.z - a.z));
    if (b.z < 1) b = lerp(b, a, (1 - b.z) / (a.z - b.z));
    float2 start = mark_screen(a), end = mark_screen(b);
    float2 p = v.position.xy / pixels.xy;
    float2 ab = end - start;
    /* The original strip expands on the minor axis and ends at the major-axis endpoints. */
    if (abs(ab.y) > abs(ab.x)) {
        start = start.yx;
        end = end.yx;
        p = p.yx;
        ab = ab.yx;
    }
    if (ab.x == 0) return false;
    float t = (p.x - start.x) / ab.x;
    if (t < 0 || t > 1) return false;
    float distance = abs(p.y - lerp(start.y, end.y, t));
    float depth = 1.0f / max(v.position.w, 1e-10);
    float radius = 0.5f * ((float)m.width / max(depth / 256.0f, 1.0f) + 1.0f);
    return distance <= radius;
}
float3 ramp_color(uint row, int entry) {
    uint index = (uint)round(materials.Load(int3(clamp(entry, 0, 15), row, 0)) * 255);
    return palette.Load(int3(index, 0, 0)).rgb;
}
float3 smooth_ramp(uint row, float shade) {
    int entry = (int)floor(shade);
    float t = frac(shade), t2 = t * t, t3 = t2 * t, inverse = 1 - t;
    /* Cubic B-spline weights smooth repeated shades without colour overshoot.
     * Palette loads resolve to linear RGB; taps stay within this material row. */
    float4 weights = float4(inverse * inverse * inverse, 4 - 6 * t2 + 3 * t3,
                            1 + 3 * t + 3 * t2 - 3 * t3, t3) / 6.0f;
    return ramp_color(row, entry - 1) * weights.x + ramp_color(row, entry) * weights.y
         + ramp_color(row, entry + 1) * weights.z + ramp_color(row, entry + 2) * weights.w;
}
float4 main(Input v) : SV_Target0 {
    uint flags = (uint)v.info.y;
    if ((flags & 2u) == 0u && v.facing < 0) discard;
    uint color = (uint)v.info.x;
    bool marking = false;
    if (options.x > 0) {
        [loop] for (uint i = 0; i < (uint)v.info.w; ++i) {
            Mark m = marks[(uint)v.info.z + i];
            if (dot(depth_row, float4(m.anchor, 1)) > m.max_depth) break;
            bool hit = m.count == 2u ? hit_line(v, m) : hit_polygon(v.uv, m);
            if (hit) { color = m.color; marking = true; }
        }
    }
    bool dos93 = options.w > 0;
    bool gouraud = (flags & 4u) != 0u;
    uint index = color;
    bool raw = (flags & 1u) != 0u;
    if (!raw && !(dos93 && color == 255u && !marking)) {
        uint material;
        bool interpolated = false;
        uint brightness_offset = marking ? color >> (dos93 ? 5u : 6u) : 0u;
        if (marking) {
            material = color & (dos93 ? 31u : 63u);
            if (policy.x > 0 && material > 0u && material <= (uint)options.z) {
                uint address = material - 1u;
                material = (uint)round(materials.Load(int3(address & 15u, 43u + address / 16u, 0)) * 255);
            } else if (gouraud) {
                material = (color + (uint)(int)(material == 14u ? policy.z : 0)) & 255u;
                if (material >= 64u) return float4(palette.Load(int3(material, 0, 0)).rgb, 1);
                interpolated = true;
            }
        } else {
            if (!dos93 && (color & 63u) == 14u) color = (color + (uint)(int)policy.z) & 255u;
            color = (color + (uint)policy.y) & 255u;
            if (policy.x > 0) {
                if (color == 0u || color > (uint)options.z) discard;
                uint address = color - 1u;
                color = (uint)round(materials.Load(int3(address & 15u, 43u + address / 16u, 0)) * 255);
            }
            if (gouraud && (color & 128u) == 0u) {
                if (color >= 64u) return float4(palette.Load(int3(color, 0, 0)).rgb, 1);
                interpolated = true;
                material = color;
            } else material = dos93 ? color : color & 127u;
        }
        if (material == 0u || material > (marking ? 39u : (uint)options.z)) discard;
        uint row = material == 14u ? 39u + (uint)options.y : material - 1u;
        uint ramp_entry;
        if (interpolated) {
            /* Keep the original 63-light ramp coordinate fractional, including
             * signed C2 line wrapping. Palette indices themselves are discrete. */
            float light = ((flags & 10u) == 10u ? frac(v.vertex_light) : saturate(v.vertex_light)) * 64;
            float shade = clamp((63.0f - light) * 0.25f, 0.0f, 15.0f);
            return float4(smooth_ramp(row, shade), 1);
        } else {
            float light = max(0, (v.facing < 0 ? -1 : 1) * v.shade);
            ramp_entry = 15u - min(15u, (uint)floor(light * 16));
        }
        index = (uint)round(materials.Load(int3(ramp_entry, row, 0)) * 255);
        if (marking && !interpolated) index = (index - brightness_offset) & 255u;
    } else if (!dos93 && !marking && (color & 63u) == 14u) {
        index = (color + (uint)(int)policy.z) & 255u;
    }
    return float4(palette.Load(int3(index, 0, 0)).rgb, 1);
}
