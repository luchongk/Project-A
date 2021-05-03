
struct VOut
{
    float4 world_position : POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
    float4 position : SV_POSITION;
};

cbuffer global : register(b0) {
    matrix projection;
};

cbuffer per_frame : register(b1) {
    matrix view;
};

cbuffer per_object : register(b3) {
    matrix world;
}

VOut main(float4 position : POSITION, float3 normal : NORMAL, float2 uv : UV)
{
    VOut output;
    output.world_position = mul(world, position);
    output.normal = mul(world, float4(normal, 0));
    output.uv = uv;

    output.position = mul(projection, mul(view, output.world_position));

    return output;
}