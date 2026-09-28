/* Analytic backgrounds for XWA's hyperspace entry, transfer, and exit phases. */

#include "hyperspace_tunnel_field.hlsli"

float4 main(float4 position : SV_Position) : SV_Target0 {
	float2 screen = position.xy / max(view.xy, float2(1.0f, 1.0f));
	if (appearance.w > 0.5f) {
		float2 offset = screen - view.zw;
		offset.x *= view.x / max(view.y, 1.0f);
		float radius_sq = dot(offset, offset);
		float halo = exp2(-3.0f * radius_sq);
		float core = exp2(-32.0f * radius_sq);
		float profile = saturate(0.2f + 0.55f * halo + 0.25f * core);
		float opacity = saturate(appearance.z) * profile;
		float3 color = cap_color.rgb * (appearance.x * appearance.y * opacity);
		return float4(color, opacity);
	}

	float2 ndc = float2(screen.x * 2.0f - 1.0f, 1.0f - screen.y * 2.0f);
	float3 ray = float3((ndc.x - projection.z) * projection.x,
					   -(ndc.y - projection.w) * projection.y, 1.0f);

	return float4(hyperspace_tunnel_radiance(ray), 1.0f);
}
