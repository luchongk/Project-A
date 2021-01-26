#version 460 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Position;
out vec2 TexCoord;

void main() {
    //Position = inPosition;
    //TexCoord = inTexCoord;
    gl_Position = projection * view * model * vec4(inPosition, 1.0);
}