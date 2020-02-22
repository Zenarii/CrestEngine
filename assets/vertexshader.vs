#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inCol;

uniform float RendererWidth;
uniform float RendererHeight;
out vec4 Colour;

void main() {
    vec3 pos = vec3(inPos.x/RendererWidth, inPos.y/RendererHeight, inPos.z);
    pos = (pos * 2.0) - vec3(1.0, 1.0, 0.0);
    gl_Position = vec4(pos, 1.0);
    Colour = inCol;
}
