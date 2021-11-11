
struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
};

VOut main(float3 position : POSITION, float4 color : COLOR, float2 uv : UV)
{
    VOut output;
    output.position = float4(position, 1);
    output.color    = color;
    output.uv       = uv;

    return output;
}