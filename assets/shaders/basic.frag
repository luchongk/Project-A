#version 460 core

in vec3 Position;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform sampler2D s;

out vec4 FragColor;

void main() {
    FragColor = vec4(lightColor * objectColor, 1);
}