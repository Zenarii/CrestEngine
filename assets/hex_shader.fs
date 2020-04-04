#version 330 core

in vec3 Colour;
in vec2 TextureCoord;
in float TextureID;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D Images[16];
uniform vec3 LightPosition;
uniform vec3 LightColour;

out vec4 FragColour;

void main() {
    float AmbientStrength = 0.2f;
    vec3 Ambient = LightColour * AmbientStrength;


    vec3 Norm = normalize(Normal);
    vec3 LightDirection = normalize(LightPosition - FragPos);

    float Diff = max(dot(Norm, LightDirection), 0.f);
    vec3 Diffuse = Diff * LightColour;

    vec3 Result = (Ambient + Diffuse) * Colour;

    int index = int(TextureID);
    FragColour = texture(Images[index], TextureCoord) * vec4(Result, 1.0);
}
