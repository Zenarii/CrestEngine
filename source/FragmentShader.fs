#version 330 core


out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TexSampler;

void main() {
   FragColor = texture(TexSampler, TexCoord);
};
