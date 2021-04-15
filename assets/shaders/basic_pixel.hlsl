
cbuffer per_frame : register(b2) {
    float3 view_pos : packoffset(c4);
    struct {
        float3 position;
        float3 ambient;
        float3 diffuse;
        float3 specular;
    } light : packoffset(c5);
}

cbuffer per_object : register(b3) {
    struct {
        float3 ambient;
        float3 diffuse;
        float3 specular;
        float shininess;
    } material : packoffset(c4);
}

Texture2D tex : register(t0);
SamplerState smp;

float4 main(float3 world_position : POSITION, float3 normal : NORMAL, float2 uv : UV) : SV_TARGET
{
    //ambient
    float3 ambient = light.ambient * material.ambient;

    //diffuse
    float3 norm = normalize(normal);
    float3 light_dir = normalize(light.position - world_position);
    float diffuse_portion = max(dot(light_dir, norm), 0);
    float3 diffuse = diffuse_portion * light.diffuse * material.diffuse;

    //specular
    float3 view_dir = normalize(view_pos - world_position);
    float3 reflect_dir = reflect(-light_dir, norm);
    float specular_portion = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
    float3 specular = specular_portion * light.specular * material.specular;
    
    float3 result = ambient + diffuse + specular;
    return float4(result, 1.0) * tex.Sample(smp, uv);
}