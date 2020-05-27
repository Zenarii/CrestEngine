#version 330 core

out vec4 FragColour;

in vec4 Colour;
in vec2 TexCoord;
in float TextureID;

uniform sampler2D Images[16];

void main() {
    int index = int(TextureID);
    FragColour = texture(Images[index], TexCoord) * Colour;
}
