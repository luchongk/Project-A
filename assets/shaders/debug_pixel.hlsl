
struct PSInput {
    float4 world_position : POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
    float4 screenPos : SV_POSITION;
};

float4 main(PSInput input) : SV_TARGET {
    return float4(0,1,0,1);
}