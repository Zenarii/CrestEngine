#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in mat4 InstanceModel;

uniform mat4 Projection;
uniform mat4 View;

out vec3 FragPos;
out vec3 Normal;

void main() {
    gl_Position = Projection * View * InstanceModel * vec4(inPosition, 1.0);
    FragPos = vec3(InstanceModel * vec4(inPosition, 1.0));

    //Undo any non-uniform scaling
    Normal = mat3(transpose(inverse(InstanceModel))) * inNormal;
}
