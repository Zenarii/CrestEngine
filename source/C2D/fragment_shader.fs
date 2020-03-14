#version 330 core

in vec4 Colour;
in vec2 TexCoord;
out vec4 FragColour;


void main() {
    FragColour = Colour;
}
