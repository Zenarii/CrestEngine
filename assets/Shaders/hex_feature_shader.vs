#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in int inMaterialID;
layout (location = 3) in mat4 InstanceModel;

uniform mat4 Projection;
uniform mat4 View;
uniform vec4 ClippingPlane;

out vec3 FragPos;
out vec3 Normal;
flat out int MaterialID;

void main() {
    vec4 WorldPosition = InstanceModel * vec4(inPosition, 1.0);

    gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);

    gl_Position = Projection * View * InstanceModel * vec4(inPosition, 1.0);
    FragPos = vec3(InstanceModel * vec4(inPosition, 1.0));
    MaterialID = inMaterialID;
    //Undo any non-uniform scaling
    Normal = mat3(transpose(inverse(InstanceModel))) * inNormal;
}

//need an array of materials setup
//send material ID to frag shader to get lighting info
