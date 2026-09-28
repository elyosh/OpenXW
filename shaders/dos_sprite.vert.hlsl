struct Vertex { float4 position : POSITION; float2 uv : TEXCOORD0; };
struct Output { float4 position : SV_Position; float2 uv : TEXCOORD0; };
Output main(Vertex v) {
    Output o; o.position = v.position; o.uv = v.uv; return o;
}
