#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColour;


uniform vec3 ViewPosition;

struct material {
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Shininess;
};

struct light {
    vec3 Position;
    vec3 Colour;
};

uniform material Material;
uniform light Light;

void main() {
    //Ambient
    float AmbientStrength = 0.3;
    vec3 Ambient = AmbientStrength * Light.Colour * Material.Ambient;

    //Diffuse
    vec3 Norm = normalize(Normal);
    vec3 LightDirection = normalize(Light.Position - FragPos);

    float Diff = max(dot(Norm, LightDirection), 0.0);
    vec3 Diffuse = Diff * Light.Colour * Material.Diffuse;

    //Specular
    float SpecularStrength = 0.3;
    vec3 ViewDirection = normalize(ViewPosition - FragPos);
    vec3 ReflectDirection = reflect(-LightDirection, Norm);
    float Spec = pow(max(dot(ViewDirection, ReflectDirection), 0.0), Material.Shininess);
    vec3 Specular = SpecularStrength * Spec * vec3(1.0, 1.0, 1.0); //Note(Zen): Considered to be white.

    vec3 Result = (Ambient + Diffuse + Specular);

    FragColour = vec4(Result, 1.0);
}
