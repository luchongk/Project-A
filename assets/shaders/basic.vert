#version 460 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    FragPos = vec3(model * vec4(inPosition, 1.0));
    Normal = vec3(model * vec4(inNormal, 0.0));
    TexCoord = inTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}