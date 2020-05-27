#version 330 core

in vec4 Colour;
in vec2 TextureCoord;
in float TextureID;

uniform sampler2D Images[16];

out vec4 FragColour;

void main() {
    int index = int(TextureID);
    FragColour = texture(Images[index], TextureCoord) * Colour;
}
