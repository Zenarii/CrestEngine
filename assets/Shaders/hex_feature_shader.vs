#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in mat4 InstanceModel;

uniform mat4 Projection;
uniform mat4 View;


void main() {
    gl_Position = Projection * View * InstanceModel * vec4(inPosition, 1.0);
}
