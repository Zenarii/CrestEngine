#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureSampler;

void main() {
    FragColor = texture(TextureSampler, TexCoord).rrrr;
}
