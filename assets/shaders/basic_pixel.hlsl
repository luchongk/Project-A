
cbuffer per_frame : register(b1) {
    float3 view_pos : packoffset(c4);
    struct Light {
        float3 position;
        float3 ambient;
        float3 diffuse;
        float3 specular;
    } light : packoffset(c5);
    float time : packoffset(c8.w);
}

cbuffer per_object : register(b3) {
    matrix world : packoffset(c0);
    struct Material {
        float3 ambient;
        float3 diffuse;
        float3 specular;
        float shininess;
    } material : packoffset(c4);
    float3 position : packoffset(c7.y);
}

Texture2D _texture;
SamplerState _sampler;

float4 main(float3 world_position : POSITION, float3 normal : NORMAL, float2 uv : UV) : SV_TARGET
{
    float distance    = length(light.position - world_position);
    float attenuation = 1.0 / (1.0 + 0.01f * distance * distance);
    
    // Ambient
    float3 ambient = light.ambient * material.ambient;

    // Diffuse
    float3 norm            = normalize(normal);
    float3 light_dir       = normalize(light.position - world_position);
    float  diffuse_portion = max(dot(light_dir, norm), 0);
    float3 diffuse         = diffuse_portion * light.diffuse * material.diffuse;

    // Specular
    float3 view_dir    = normalize(view_pos - world_position);
    float3 halfway_dir = normalize(light_dir + view_dir);
    // This is Blinn-Phong, so we use the halfway vector. For plain old Phong use reflect_dir = reflect(-light_dir, norm) instead of halfway_dir.
    // Also, we modulate specular_portion by diffuse_portion to prevent it from having a value greater than 0 when the normal is facing away from the light direction.
    // We could also achieve that by wrapping the specular calculation in "if(diffuse_portion > 0) {...}", which would be correct, but doing so leads to specular highlights popping in and out
    // in some cases where the view direction is opposite to the light direction. Multiplying it makes the transition smoother, and also a bit less correct, but this isn't PBR anyways, so whatever.
    // See https://www.gamedev.net/forums/topic/445990-phong-reflections-for-normal-facing-away-from-light/?page=1 for a discussion.
    float  specular_portion = diffuse_portion  * pow(max(dot(norm, halfway_dir), 0), material.shininess);
    float3 specular         = specular_portion * light.specular * material.specular;
    float3 color            = ambient + (diffuse + specular) * attenuation;
    color *= _texture.Sample(_sampler, uv);

    // Gamma correction
    //color = pow(color, 1.0 / 2.2);
    color = max(1.055 * pow(color, 0.416666667) - 0.055, 0);

    // Fade in/out based on percieved brightness (non linear, thats why it goes after gamma correction).
    color = color * (cos(4 * time) * 0.5 + 0.5);

    return float4(color, 1);
}