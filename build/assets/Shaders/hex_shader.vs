#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColour;
layout (location = 3) in vec2 inTextureCoord;
layout (location = 4) in float inTextureID;

out vec3 Normal;
out vec3 Colour;
out vec2 TextureCoord;
out float TextureID;
out vec3 FragPos;

uniform mat4 Projection;
uniform mat4 Model;
uniform mat4 View;
uniform vec4 ClippingPlane;

void main() {
    //HARDCODE(Zen): Assumes water level will always be 2. Unsure if there
    //is a way to changed this for multiple different water heights
    //probably isn't too important anyway, can just keep them the same height
    gl_ClipDistance[0] = dot(vec4(inPosition, 1.0), ClippingPlane);

    gl_Position = Projection * View * Model * vec4(inPosition, 1.0);
    FragPos = vec3(Model * vec4(inPosition, 1.0));
    Colour = inColour;
    TextureCoord = inTextureCoord;
    TextureID = inTextureID;
    //Undo any non-uniform scaling
    Normal = mat3(transpose(inverse(Model))) * inNormal;
}
