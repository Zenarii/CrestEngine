#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColour;

uniform vec3 LightPosition;
uniform vec3 LightColour;
uniform vec3 ViewPosition;

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

    vec3 Result = (Ambient + Diffuse + Specular) * vec3(0.1, 0.8, 0.3);

    FragColour = vec4(Result, 1.0);
}
