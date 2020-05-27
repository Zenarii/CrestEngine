#version 330 core

in vec3 Normal;
in vec3 FragPos;
flat in int MaterialID;

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
//HARDCODE(Zen): 4 * HEX_FEATURE_COUNT
uniform material Material[8];
uniform light Light;

void main() {
    //Ambient
    float AmbientStrength = 0.3;
    vec3 Ambient = AmbientStrength * Light.Colour * Material[MaterialID].Ambient;

    //Diffuse
    vec3 Norm = normalize(Normal);
    vec3 LightDirection = normalize(Light.Position - FragPos);

    float Diff = max(dot(Norm, LightDirection), 0.0);
    vec3 Diffuse = Diff * Light.Colour * Material[MaterialID].Diffuse;

    //Specular
    float SpecularStrength = 0.3;
    vec3 ViewDirection = normalize(ViewPosition - FragPos);
    vec3 ReflectDirection = reflect(-LightDirection, Norm);
    float Spec = pow(max(dot(ViewDirection, ReflectDirection), 0.0), Material[MaterialID].Shininess);
    vec3 Specular = SpecularStrength * Spec * Material[MaterialID].Specular;

    vec3 Result = (Ambient + Diffuse + Specular);

    FragColour = vec4(Result, 1.0);
}
