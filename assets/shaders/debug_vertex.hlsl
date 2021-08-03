
struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR;
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

VOut main(float4 position : POSITION, float4 color : COLOR)
{
    VOut output;
    float4 world_position = mul(world, position);
    output.position = mul(projection, mul(view, world_position));

    output.color    = color;

    return output;
}