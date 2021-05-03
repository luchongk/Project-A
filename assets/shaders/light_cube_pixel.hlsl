
cbuffer per_frame : register(b1) {
    float3 color : packoffset(c7);
}

float4 main() : SV_TARGET {
    //gamma correction
    float3 result = pow(float3(1, 0, 0), 1.0 / 2.2);
    
    return float4(result, 1);
}