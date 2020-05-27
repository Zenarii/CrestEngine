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
uniform sampler2D NormalMap;
uniform sampler2D DepthMap;

uniform vec3 LightPosition;
uniform vec3 LightColour;
uniform vec3 ViewPosition;
uniform float Time;

out vec4 FragColour;

//TODO(Zen): Tweak these variables and the distortion sampling to improve
//    the look of the water
const float DistortionFactor = 0.008;
const float WaveSpeed = 0.05;
const float SpecularExponent = 20.0;
const float Reflectivity = 0.6;
const vec3 FoamColour = vec3(0.1, 0.1, 0.1);

void main() {

    //Fresnel Effect
    vec3 ViewVec = normalize(ToCameraVector);
    float RefractiveFactor = dot(ViewVec, vec3(0.0, 1.0, 0.0));
    RefractiveFactor = pow(RefractiveFactor, 3);


    /*
        Note(Zen): Reflection and Refraction are swapped here for some reason.
        Rather than changing these and messing up the effect I've just left this note here instead
    */
    vec2 ndc = (ClipSpace.xy / ClipSpace.w) * 0.5 + 0.5f;
    vec2 RefractionTexCoords = vec2(ndc.x, -ndc.y);
    vec2 ReflectionTexCoords = vec2(ndc.x, ndc.y);
    float MoveFactor = WaveSpeed * Time;


    float Near = 0.1f;
    float Far = 100.f;
    float FloorDepth = texture(DepthMap, ndc).r;
    float FloorDistance = 2.0 * Near * Far / (Far + Near - (2.0 * FloorDepth - 1.0) * (Far - Near));
    float WaterDepth = gl_FragCoord.z;
    float WaterDistance = 2.0 * Near * Far / (Far + Near - (2.0 * WaterDepth - 1.0) * (Far - Near));
    float FinalDepth = FloorDistance - WaterDistance;

    vec2 DistortedUvs = texture(DistortionTexture, vec2(TextureCoord.x + MoveFactor, TextureCoord.y)).rg*0.1;
	DistortedUvs = TextureCoord + vec2(DistortedUvs.x, DistortedUvs.y + MoveFactor);
	vec2 TotalDistortion = (texture(DistortionTexture, DistortedUvs).rg * 2.0 - 1.0) * DistortionFactor * clamp(FinalDepth * 2.0, 0.0, 1.0);

    vec4 ReflectionColour = texture(ReflectionTexture, ReflectionTexCoords + TotalDistortion);
    ReflectionColour = clamp(ReflectionColour, 0.001, 0.999);
    vec4 RefractionColour = texture(RefractionTexture, RefractionTexCoords + TotalDistortion);
    RefractionColour = clamp(RefractionColour, 0.001, 0.999);

    vec4 WaterColour = mix(RefractionColour, ReflectionColour, RefractiveFactor);

    vec4 NormalMapColour = texture(NormalMap, DistortedUvs);
    vec3 NormalMapV = vec3(NormalMapColour.r * 2.0 - 1.0, NormalMapColour.b, NormalMapColour.g * 2.0 - 1.0);
    NormalMapV = normalize(NormalMapV);

    // vec3 LightDirection = normalize(LightPosition - FragPos);
    // vec3 ReflectedLight = reflect(-normalize(LightDirection), NormalMapV);
	// float Specular = max(dot(ReflectedLight, ViewVec), 0.0);
	// Specular = pow(Specular, SpecularExponent);
	// vec3 specularHighlights = LightColour * Specular * Reflectivity;

    FragColour = mix(WaterColour, vec4(0.0, 0.3, 0.5, 1.0), 0.2) ;//+ vec4(specularHighlights, 0.f);
    //FragColour = vec4(FloorDistance, FloorDistance, FloorDistance, 1.0);
    //FragColour = vec4(FloorDepth, FloorDepth, FloorDepth, 1.0);

}
