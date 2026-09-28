/* Low-resolution tunnel radiance cubemap used by scene mesh diffuse lighting. */

#define XWA_HYPERSPACE_UNIFORM_BINDING register(b0, space2)
#include "hyperspace_tunnel_field.hlsli"

cbuffer HyperspaceEnvironmentCS : register(b1, space2) {
	float environment_roughness;
	float3 _environment_pad;
};

RWTexture2D<float4> environment_output : register(u0, space1);

float3 environment_cube_direction(uint face, float2 uv) {
	if (face == 0u) return normalize(float3(1.0f, -uv.y, -uv.x));
	if (face == 1u) return normalize(float3(-1.0f, -uv.y, uv.x));
	if (face == 2u) return normalize(float3(uv.x, 1.0f, uv.y));
	if (face == 3u) return normalize(float3(uv.x, -1.0f, -uv.y));
	if (face == 4u) return normalize(float3(uv.x, -uv.y, 1.0f));
	return normalize(float3(-uv.x, -uv.y, -1.0f));
}

float3 environment_to_view(float3 local_direction) {
	return tunnel_right.xyz * local_direction.x +
		tunnel_up.xyz * local_direction.y +
		tunnel_forward.xyz * local_direction.z;
}

void environment_basis(float3 normal, out float3 tangent, out float3 bitangent) {
	float sign_z = normal.z >= 0.0f ? 1.0f : -1.0f;
	float a = -1.0f / (sign_z + normal.z);
	float b = normal.x * normal.y * a;
	tangent = float3(1.0f + sign_z * normal.x * normal.x * a, sign_z * b, -sign_z * normal.x);
	bitangent = float3(b, sign_z + normal.y * normal.y * a, -normal.y);
}

[numthreads(8, 8, 1)]
void main(uint3 thread_id : SV_DispatchThreadID) {
	uint width;
	uint height;
	environment_output.GetDimensions(width, height);
	if (thread_id.x >= width || thread_id.y >= height || height == 0u || width < height * 6u) return;

	uint environment_face = thread_id.x / height;
	uint2 face_texel = uint2(thread_id.x % height, thread_id.y);
	float2 uv = ((float2(face_texel) + 0.5f) / (float)height) * 2.0f - 1.0f;
	float3 direction = environment_cube_direction(environment_face, uv);
	float spread = saturate(environment_roughness);
	spread *= spread;

	float3 radiance = 0.0f;
	if (spread <= 1.0e-4f) {
		radiance = hyperspace_tunnel_radiance(environment_to_view(direction));
	} else {
		float3 tangent;
		float3 bitangent;
		environment_basis(direction, tangent, bitangent);
		[unroll]
		for (uint sample_index = 0u; sample_index < 16u; ++sample_index) {
			float radial = sqrt(((float)sample_index + 0.5f) / 16.0f);
			float azimuth = frac((float)sample_index * 0.61803398875f) * HYPER_TWO_PI;
			float sine;
			float cosine;
			sincos(azimuth, sine, cosine);
			float3 hemisphere = tangent * (radial * cosine) + bitangent * (radial * sine) +
				direction * sqrt(max(1.0f - radial * radial, 0.0f));
			float3 sample_direction = normalize(lerp(direction, hemisphere, spread));
			radiance += hyperspace_tunnel_radiance(environment_to_view(sample_direction));
		}
		radiance *= 1.0f / 16.0f;
	}
	environment_output[thread_id.xy] = float4(max(radiance, 0.0f), 1.0f);
}
