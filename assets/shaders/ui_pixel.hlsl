struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
    float2 position_in_pixels : POSITION;
    float2 rect_center : CENTER;
    float2 rect_half_size : HALF_SIZE;
    float corner_radius : RADIUS;
};

Texture2D    _texture;
SamplerState _sampler;

float2 position_in_rect(float2 center, float2 half_size, float radius, float2 pos) {
    float2 result = abs(pos - center) - half_size + float2(radius, radius);
    return result;
}

float rounded_rect(float radius, float2 pos) {
    return length(max(pos,0)) - radius; //+ min(max(pos.x, pos.y), 0); This commented out part is for inner (negative) distance
}

float4 angle_dependent_test(PSInput input) {
    const float EDGE_SOFTNESS = 0.5;
    const float2 SIZE_COMPENSATION = (EDGE_SOFTNESS).xx;

    float2 pos = position_in_rect(input.rect_center, input.rect_half_size - SIZE_COMPENSATION, input.corner_radius, input.position_in_pixels);
    float d = rounded_rect(input.corner_radius, pos);
    float2 snapped = normalize(pos);
    float c = abs(dot(snapped, normalize(float2(1,0))));
    bool in_corner = pos.x > 0 && pos.y > 0;
    float4 color = input.color * _texture.Sample(_sampler, input.uv);
    if(input.corner_radius && in_corner) color.a *= 1 - d * pow(c, 0.25);

    return color;
}

float4 smooth(PSInput input) { 
    const float EDGE_SOFTNESS = 1;
    const float2 SIZE_COMPENSATION = (EDGE_SOFTNESS).xx;

    float2 pos = position_in_rect(input.rect_center, input.rect_half_size, input.corner_radius, input.position_in_pixels);
    float d = rounded_rect(input.corner_radius, pos);
    float4 color = input.color * _texture.Sample(_sampler, input.uv);
    if(input.corner_radius) color.a *= saturate(0.5 - d);

    return color;
}

float4 aliased(PSInput input) {
    float2 pos = position_in_rect(input.rect_center, input.rect_half_size, input.corner_radius, input.position_in_pixels);
    float d = rounded_rect(input.corner_radius, pos);
    float4 color = input.color * _texture.Sample(_sampler, input.uv);
    if(input.corner_radius) color.a *= step(d, 1);

    return color;
}

float4 main(PSInput input) : SV_TARGET {
    return smooth(input);
}