Texture2D<float4> indices : register(t0, space2);
SamplerState index_sampler : register(s0, space2);
Texture2D<float4> palette : register(t1, space2);
SamplerState palette_sampler : register(s1, space2);
cbuffer SpriteParams : register(b0, space3) {
    float4 colors[4];
    float4 texel_bounds;
    float4 filter;
};

float4 Resolve(float4 texel) {
    uint index = (uint)round(texel.r * 255) & 15u;
    uint mapped = (uint)colors[index >> 2u][index & 3u];
    return float4(palette.Load(int3(mapped, 0, 0)).rgb * texel.a, texel.a);
}

float4 LoadColor(int2 pixel) {
    pixel = clamp(pixel, (int2)texel_bounds.xy, (int2)texel_bounds.zw);
    return Resolve(indices.Load(int3(pixel, 0)));
}

float4 main(float4 position : SV_Position, float2 uv : TEXCOORD0) : SV_Target0 {
    if (filter.x != 0) {
        uint width, height;
        indices.GetDimensions(width, height);
        float2 pixel = uv * float2(width, height) - 0.5f;
        int2 base = (int2)floor(pixel);
        float2 weight = frac(pixel);
        /* Interpolate linear, premultiplied colors, never palette indices. */
        float4 top = lerp(LoadColor(base), LoadColor(base + int2(1, 0)), weight.x);
        float4 bottom = lerp(LoadColor(base + int2(0, 1)), LoadColor(base + int2(1, 1)), weight.x);
        float4 color = lerp(top, bottom, weight.y);
        if (color.a < 0.01f) discard;
        return color;
    }
    float4 texel = indices.SampleLevel(index_sampler, uv, 0);
    if (texel.a < 0.5f) discard;
    return float4(Resolve(texel).rgb, 1);
}
