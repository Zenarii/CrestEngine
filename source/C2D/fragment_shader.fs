#version 330 core

out vec4 FragColour;

in vec4 Colour;
in vec2 TexCoord;

uniform sampler2D TextureSampler;

void main() {
    FragColour = texture(TextureSampler, TexCoord);// * Colour;
}
