#version 330 core

in vec3 Colour;
in vec2 TextureCoord;
in float TextureID;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D Images[16];
uniform vec3 LightPosition;
uniform vec3 LightColour;
uniform vec3 ViewPosition;

out vec4 FragColour;

void main() {
    //Ambient
    float AmbientStrength = 0.3;
    vec3 Ambient = LightColour * AmbientStrength;

    //Diffuse
    vec3 Norm = normalize(Normal);
    vec3 LightDirection = normalize(LightPosition - FragPos);

    float Diff = max(dot(Norm, LightDirection), 0.0);
    vec3 Diffuse = Diff * LightColour;

    //Specular
    float SpecularStrength = 0.3;
    vec3 ViewDirection = normalize(ViewPosition - FragPos);
    vec3 ReflectDirection = reflect(-LightDirection, Norm);
    float Spec = pow(max(dot(ViewDirection, ReflectDirection), 0.0), 8);
    vec3 Specular = SpecularStrength * Spec * LightColour;

    vec3 Result = (Ambient + Diffuse + Specular) * Colour;

    int index = int(TextureID);
    FragColour = texture(Images[index], TextureCoord) * vec4(Result, 1.0);
}
