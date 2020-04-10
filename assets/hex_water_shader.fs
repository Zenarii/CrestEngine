#version 330 core

in vec3 Colour;
in vec2 TextureCoord;
in float TextureID;
in vec3 Normal;
in vec3 FragPos;
in vec2 WorldPos;

uniform sampler2D Images[16];
uniform vec3 LightPosition;
uniform vec3 LightColour;
uniform vec3 ViewPosition;
uniform float Time;

out vec4 FragColour;


void main() {
    int index = int(TextureID);
    vec2 uv1 = WorldPos;
    uv1.y += Time * 0.2;
    vec4 Noise1 = texture(Images[index], uv1 * 0.25);

    vec2 uv2 = WorldPos;
    uv2.x += Time * 0.2;
    vec4 Noise2 = texture(Images[index], uv2 * 0.25);

    float BlendWave = sin((TextureCoord.x + TextureCoord.y) * 0.1 + Time + Noise1.y + Noise2.z);
    BlendWave *= BlendWave;

    float Waves = mix(Noise1.z, Noise1.y, BlendWave) + mix(Noise2.x, Noise2.y, BlendWave);
    Waves = smoothstep(0.1, 2.5, Waves);


    float Shore = TextureCoord.y;
    vec2 ShoreNoiseUV = TextureCoord + Time * 0.1;
    vec4 ShoreNoise = texture(Images[index], ShoreNoiseUV);
    float Distortion = ShoreNoise.x * (1.0-Shore);

    float Foam = sin((Distortion + Shore) * 10.0 - Time * 0.1);
	Foam *= Foam * Shore;


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

    FragColour = Foam + vec4(Waves * 0.2, Waves * 0.6, Waves, 1.0)  + vec4(Result, 1.0);
    FragColour.a = 0.3;
    //FragColour = vec4(TextureCoord, 0.0, 1.0);
}
