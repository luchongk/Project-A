
cbuffer per_frame : register(b1) {
    float3 view_pos : packoffset(c4);
    struct {
        float3 position;
        float3 ambient;
        float3 diffuse;
        float3 specular;
    } light : packoffset(c5);
    float time : packoffset(c8.w);
}

cbuffer per_object : register(b3) {
    matrix world : packoffset(c0);
    struct {
        float3 ambient;
        float3 diffuse;
        float3 specular;
        float shininess;
    } material : packoffset(c4);
    float3 position : packoffset(c7.y);
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
    
    float3 color = ambient + diffuse + specular;
    //color = color * tex.Sample(smp, uv);

    //Gamma correction
    //color = pow(color, 1.0 / 2.2);
    color = max(1.055 * pow(color, 0.416666667) - 0.055, 0);

    //Fade in/out based on percieved brightness (non linear, thats why it goes after gamma correction)
    //color = color * (cos(2 * time) * 0.5 + 0.5);

    return float4(color, 1);
}