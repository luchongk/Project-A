
struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
};

cbuffer global : register(b0) {
    matrix projection;
    float2 resolution;
};

VOut main(float3 position : POSITION, float4 color : COLOR, float2 uv : UV)
{
    VOut output;
    output.position = float4(2 * position.xy / resolution - 1, 0, 1);
    output.color    = color;
    output.uv       = uv;
    return output;
}