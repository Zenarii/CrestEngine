#version 330 core

in vec3 Colour;
in vec2 TextureCoord;
in float TextureID;
in vec3 Normal;
in vec4 ClipSpace;
in vec3 FragPos;

uniform sampler2D ReflectionTexture;
uniform sampler2D RefractionTexture;

uniform vec3 LightPosition;
uniform vec3 LightColour;
uniform vec3 ViewPosition;
uniform float Time;

out vec4 FragColour;


void main() {
    /*
        Apply FBOs
        Note(Zen): Reflection and Refraction are swapped here for some reason.
        Rather than messing up the effect I've just left this note here instead
    */
    vec2 ndc = (ClipSpace.xy / ClipSpace.w) * 0.5 + 0.5f;
    vec2 RefractionTexCoords = vec2(ndc.x, -ndc.y);
    vec2 ReflectionTexCoords = vec2(ndc.x, ndc.y);

    vec4 ReflectionColour = texture(ReflectionTexture, ReflectionTexCoords);
    vec4 RefractionColour = texture(RefractionTexture, RefractionTexCoords);
    vec4 WaterColour = mix(RefractionColour, ReflectionColour, 0.5);

    /*
        Lighting
    */


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

    FragColour = WaterColour;// * vec4(Result, 1.0);
    //FragColour = vec4(TextureCoord, 0.0, 1.0);
}
