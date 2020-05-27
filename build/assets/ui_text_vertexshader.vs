#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;

uniform float RendererWidth;
uniform float RendererHeight;
out vec2 TexCoord;

void main() {
    vec3 pos = vec3(inPos.x/RendererWidth, 1.0 - (inPos.y/RendererHeight), inPos.z);
    pos = (pos * 2.0) - vec3(1.0, 1.0, 0.0);
    gl_Position = vec4(pos, 1.0);
    TexCoord = inTexCoord;
}
