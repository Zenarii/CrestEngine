#version 330 core
/*
A 2D shader that works in pixel coordinates
 ^(0, 0)
 |
 |
 x---->(max_x, may_y)
*/

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColour;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in float inTextureID;

uniform float RendererWidth;
uniform float RendererHeight;
out vec4 Colour;
out vec2 TexCoord;
out float TextureID;

void main() {
    vec3 pos = vec3(inPos.x/RendererWidth, 1.0 - (inPos.y/RendererHeight), inPos.z);
    pos = (pos * 2.0) - vec3(1.0, 1.0, 0.0);
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
    Colour = vec4(inColour, 1.0);
    TexCoord = inTexCoord;
    TextureID = inTextureID;
}
