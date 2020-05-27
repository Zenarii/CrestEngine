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
uniform float Time;

out vec4 FragColour;
float near = 0.1;
float far  = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    FragColour = vec4(vec3(depth), 1.0);
}
