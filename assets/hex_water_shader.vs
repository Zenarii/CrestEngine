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
out vec4 ClipSpace;
out vec3 ToCameraVector;

uniform mat4 Projection;
uniform mat4 Model;
uniform mat4 View;
uniform float Time;
uniform vec3 ViewPosition;

//TODO(Zen): Remove unnecessary variables such as Texture coord and ID from this shader

void main() {
    vec3 Position = inPosition;
    ClipSpace = Projection * View * Model * vec4(Position, 1.0);
    gl_Position = ClipSpace;
    FragPos = vec3(Model * vec4(inPosition, 1.0));
    Colour = inColour;
    TextureCoord = vec2(Position.x/2.0 + 0.5, Position.y/2.0 + 0.5);

    TextureID = inTextureID;
    ToCameraVector = ViewPosition - Position;
    //Undo any non-uniform scaling
    Normal = mat3(transpose(inverse(Model))) * inNormal;
}
