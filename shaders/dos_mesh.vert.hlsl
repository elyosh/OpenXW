/* OpenTIE's component-table and expanded-line design with X-Wing metadata. */
cbuffer View : register(b0, space1) {
    row_major float4x4 view_proj;
    float4 pixels; // 2 / target extent, physical pixels per original pixel
    float4 camera;
};
cbuffer Draw : register(b1, space1) { uint table_index; float2 endpoints; float hyper_color; };
StructuredBuffer<float4> tables : register(t0, space0);
struct Vertex {
    float3 position : POSITION;
    float3 other : POSITION1;
    float3 normal : NORMAL;
    float3 vertex_normal : NORMAL1;
    float3 other_normal : NORMAL2;
    float2 uv : TEXCOORD0;
    float4 info : TEXCOORD1;
    float2 line_data : TEXCOORD2;
};
struct Output {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    nointerpolation float4 info : TEXCOORD1;
    float shade : TEXCOORD2;
    float facing : TEXCOORD3;
    noperspective float vertex_light : TEXCOORD4;
};
float3 world(float3 p, float4 a, float4 b, float4 c) {
    return float3(dot(a, float4(p, 1)), dot(b, float4(p, 1)), dot(c, float4(p, 1)));
}
Output main(Vertex v) {
    if (hyper_color >= 0) { v.position.y = endpoints.x; v.other.y = endpoints.y; v.info.x = hyper_color; }
    float4 a = tables[table_index * 4], b = tables[table_index * 4 + 1];
    float4 c = tables[table_index * 4 + 2], light = tables[table_index * 4 + 3];
    float3 p = world(v.position, a, b, c);
    float3 normal = float3(dot(a.xyz, v.normal), dot(b.xyz, v.normal), dot(c.xyz, v.normal));
    Output o;
    o.position = mul(view_proj, float4(p, 1));
    o.uv = v.uv;
    o.info = v.info;
    o.shade = dot(v.normal, light.xyz);
    o.facing = dot(normal, p - camera.xyz);
    uint flags = (uint)v.info.y;
    if (light.w == 0) flags &= ~4u;
    o.info.y = flags;
    float vertex_light = dot(v.vertex_normal, light.xyz) * (o.facing < 0 ? -1 : 1);
    o.vertex_light = ((flags & 10u) == 10u) ? vertex_light : max(0, vertex_light);
    if (((uint)v.info.y & 8u) != 0u) {
        float4 ca = o.position;
        float4 cb = mul(view_proj, float4(world(v.other, a, b, c), 1));
        float depth = 0.5f * (ca.w + cb.w);
        float other_light = dot(v.other_normal, light.xyz) * (o.facing < 0 ? -1 : 1);
        if ((flags & 10u) != 10u) other_light = max(0, other_light);
        bool at_b = v.line_data.y >= 2;
        float own_depth = at_b ? cb.w : ca.w, other_depth = at_b ? ca.w : cb.w;
        if (own_depth < 1 && other_depth >= 1)
            o.vertex_light = lerp(o.vertex_light, other_light, (1 - own_depth) / (other_depth - own_depth));
        bool hidden = ca.w < 1 && cb.w < 1;
        if (!hidden) {
            if (ca.w < 1) ca = lerp(ca, cb, (1 - ca.w) / (cb.w - ca.w));
            if (cb.w < 1) cb = lerp(cb, ca, (1 - cb.w) / (ca.w - cb.w));
        }
        float2 pixel_step = pixels.xy * pixels.zw;
        float2 direction = (cb.xy / max(cb.w, 1) - ca.xy / max(ca.w, 1)) / pixel_step;
        float length_sq = dot(direction, direction);
        float2 perpendicular = length_sq > 1e-8 ? float2(-direction.y, direction.x) * rsqrt(length_sq) : float2(1, 0);
        uint divisor = depth < 0 ? 0u : ((uint)floor(depth / 256) & 65535u);
        float thickness = floor(divisor != 0u ? v.line_data.x / divisor : v.line_data.x) + 1;
        uint corner = (uint)v.line_data.y;
        o.position = corner >= 2 ? cb : ca;
        o.position.xy += perpendicular * pixel_step * thickness * 0.5f *
            ((corner & 1u) != 0u ? 1 : -1) * o.position.w;
        if (hidden) o.position = float4(0, 0, 0, -1);
    }
    return o;
}
