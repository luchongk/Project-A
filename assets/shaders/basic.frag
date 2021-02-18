#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform Material material;
uniform Light light;
uniform vec3 view_pos;
uniform sampler2D s;

out vec4 FragColor;

void main() {
    //ambient
    vec3 ambient = light.ambient * material.ambient;

    //diffuse
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(light.position - FragPos);
    float diffuse_portion = max(dot(light_dir, norm), 0);
    vec3 diffuse = diffuse_portion * light.diffuse * material.diffuse;

    //specular
    vec3 view_dir = normalize(view_pos - FragPos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float specular_portion = pow(max(dot(view_dir, reflect_dir), 0), material.shininess);
    vec3 specular = specular_portion * light.specular * material.specular;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0) * texture(s, TexCoord);
}