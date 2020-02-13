#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColour;


out vec4 outColour;
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;


void main() {
    gl_Position = Projection * View * Model * vec4(inPos, 1.0);
    outColour = vec4(inColour, 1.0);
};
