#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColour;
layout (location = 2) in vec2 inTextureCoord;
layout (location = 3) in float inTextureID;

out vec4 Colour;
out vec2 TextureCoord;
out float TextureID;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    Colour = vec4(inColour, 1.0);
    TextureCoord = inTextureCoord;
    TextureID = inTextureID;
}
