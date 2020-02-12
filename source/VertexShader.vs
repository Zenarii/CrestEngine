#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;


out vec2 TexCoord;
uniform vec3 Offset;


void main() {
    gl_Position = vec4(inPos.x + Offset.x, inPos.y + Offset.y, inPos.z, 1.0);
    TexCoord = inTexCoord;
};
