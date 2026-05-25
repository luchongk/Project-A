
#define PIXEL_SNAPPING 0

struct VSInput {
    float2 pos     : POSITION;
    float2 size    : SIZE;
    float2 uv_pos  : UV_POSITION;
    float2 uv_size : UV_SIZE;
    float4 color   : COLOR;
    uint vertex_id : SV_VERTEXID;
    float corner_radius : CORNER_RADIUS;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : UV;
    float2 position_in_pixels : POSITION;
    float2 rect_center : CENTER;
    float2 rect_half_size : HALF_SIZE;
    float corner_radius : RADIUS;
};

cbuffer global : register(b0) {
    int2 resolution;
};

PSInput main(VSInput input)
{
    // Triangle strip for a quad
    static float2 vertices[4] = {
        {-1, -1},
        {-1,  1},
        { 1, -1},
        { 1,  1}
    };

    PSInput output;

    // Passthroughs
    float2 half_size = input.size / 2;
    output.rect_half_size = half_size;
    output.corner_radius  = input.corner_radius;
    output.color          = input.color;
    
    // Positions
    {
        float2 center = input.pos + half_size;
        float2 pos = center + vertices[input.vertex_id] * half_size;

        // Switch positions to Y-up coordinates.
        center.y = resolution.y - center.y;
        pos.y    = resolution.y - pos.y;

#if PIXEL_SNAPPING
    pos.x = vertices[input.vertex_id].x > 0 ? ceil(pos.x) : floor(pos.x);
    pos.y = vertices[input.vertex_id].y > 0 ? ceil(pos.y) : floor(pos.y);
#endif

        output.position = float4(2 * pos / resolution - 1, 0, 1);
        output.position_in_pixels = pos;
        output.rect_center = center;
    }
    
    // UV
    {
        float2 uv_half_size = input.uv_size / 2;
        float2 uv_center    = input.uv_pos + uv_half_size;
        output.uv = uv_center + vertices[input.vertex_id] * uv_half_size;
    }

    return output;
}