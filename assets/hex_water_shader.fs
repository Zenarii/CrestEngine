#version 330 core

in vec3 Colour;
in vec2 TextureCoord;
in float TextureID;
in vec3 Normal;
in vec4 ClipSpace;
in vec3 FragPos;
in vec3 ToCameraVector;

uniform sampler2D ReflectionTexture;
uniform sampler2D RefractionTexture;
uniform sampler2D DistortionTexture;

uniform vec3 LightPosition;
uniform vec3 LightColour;
uniform vec3 ViewPosition;
uniform float Time;

out vec4 FragColour;

//TODO(Zen): Tweak these variables and the distortion sampling to improve
//    the look of the water
const float DistortionFactor = 0.005f;
const float WaveSpeed = 0.1f;

void main() {

    //Fresnel Effect
    vec3 ViewVec = normalize(ToCameraVector);
    float RefractiveFactor = dot(ViewVec, vec3(0.0, 1.0, 0.0));
    RefractiveFactor = pow(RefractiveFactor, 3);

    /*
        Apply FBOs
        Note(Zen): Reflection and Refraction are swapped here for some reason.
        Rather than messing up the effect I've just left this note here instead
    */
    vec2 ndc = (ClipSpace.xy / ClipSpace.w) * 0.5 + 0.5f;
    vec2 RefractionTexCoords = vec2(ndc.x, -ndc.y);
    vec2 ReflectionTexCoords = vec2(ndc.x, ndc.y);
    float MoveFactor = WaveSpeed * Time;
    vec2 Distortion1 = DistortionFactor * (texture(DistortionTexture, vec2(TextureCoord.x + MoveFactor, TextureCoord.y)).rg * 2.0 - 1.0);
    vec2 Distortion2 = DistortionFactor * (texture(DistortionTexture, vec2(-TextureCoord.x, TextureCoord.y + MoveFactor)).rg * 2.0 - 1.0);

    vec2 TotalDistortion = (Distortion2 + Distortion1) * ;

    vec4 ReflectionColour = texture(ReflectionTexture, ReflectionTexCoords + TotalDistortion);
    ReflectionColour = clamp(ReflectionColour, 0.0, 1.0);
    vec4 RefractionColour = texture(RefractionTexture, RefractionTexCoords + TotalDistortion);
    RefractionColour = clamp(RefractionColour, 0.0, 1.0);

    vec4 WaterColour = mix(RefractionColour, ReflectionColour, RefractiveFactor);

    // /*
    //     Lighting
    // */
    //
    //
    // //Ambient
    // float AmbientStrength = 0.3;
    // vec3 Ambient = LightColour * AmbientStrength;
    //
    // //Diffuse
    // vec3 Norm = normalize(Normal);
    // vec3 LightDirection = normalize(LightPosition - FragPos);
    //
    // float Diff = max(dot(Norm, LightDirection), 0.0);
    // vec3 Diffuse = Diff * LightColour;
    //
    // //Specular
    // float SpecularStrength = 0.3;
    // vec3 ViewDirection = normalize(ViewPosition - FragPos);
    // vec3 ReflectDirection = reflect(-LightDirection, Norm);
    // float Spec = pow(max(dot(ViewDirection, ReflectDirection), 0.0), 8);
    // vec3 Specular = SpecularStrength * Spec * LightColour;
    //
    // vec3 Result = (Ambient + Diffuse + Specular) * Colour;


    FragColour = mix(WaterColour, vec4(0.0, 0.3, 0.5, 1.0), 0.2);
    //FragColour = vec4(TextureCoord, 0.0, 1.0);
}
