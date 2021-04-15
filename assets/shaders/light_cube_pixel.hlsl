
cbuffer per_frame : register(b2) {
    float3 color : packoffset(c7);
}

float4 main() : SV_TARGET {
    return float4(color, 1);
}