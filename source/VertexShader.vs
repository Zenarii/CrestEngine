#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;


out vec2 TexCoord;
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main() {
    gl_Position = Projection * View * Model * vec4(inPos, 1.0);
    TexCoord = inTexCoord;
}
