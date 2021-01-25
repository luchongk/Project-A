#version 460 core

in vec3 Position;
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D s;

void main() {
    FragColor = texture(s, TexCoord) * vec4(1,1,1,1);
}